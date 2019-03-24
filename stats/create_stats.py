#!/usr/bin/env python3
from enum import Enum
from multiprocessing import Process, Manager
from logging.handlers import TimedRotatingFileHandler, RotatingFileHandler
import logging
import subprocess
import select
import os, sys
import time
import re
import signal

TIMEOUT = 80 # 1min 20sec
CMD = "/usr/bin/time -v %sbin/project -m %s -i %s -o %s -k %d"
MAKE = "make"
HOME = "../"

def kill_project(logger):
    cmd = "ps -a -o pid,cmd | grep -P '([0-9]+) ./bin/project -m projectV.*' | awk '{print  $1}' | xargs -I{} kill -9 {}"
    logger.info(cmd)
    logger.info(subprocess.call(cmd, shell=True))

def call(popenargs, logger, is_stats=False, timeout=TIMEOUT, stdout_log_level=logging.DEBUG, stderr_log_level=logging.INFO, **kwargs):
    """
    Variant of subprocess.call that accepts a logger instead of stdout/stderr,
    and logs stdout messages via logger.debug and stderr messages via
    logger.error.

    author = bgreenlee
    src = https://gist.github.com/bgreenlee/1402841

    re-edit by Romain Chantrel
    """
    child = subprocess.Popen(popenargs, stdout=subprocess.PIPE, stderr=subprocess.PIPE, **kwargs)

    log_level = {child.stdout: stdout_log_level, child.stderr: stderr_log_level}

    def check_io():
        ready_to_read = select.select([child.stdout, child.stderr], [], [], 1000)[0]
        for io in ready_to_read:
            line = io.readline()
            if line[:-1] != "" and line[:-1] != b'':
                logger.log(log_level[io], line[:-1].decode())

    if not is_stats:
        # keep checking stdout/stderr until the child exits
        while child.poll() is None:
            check_io()
        check_io()  # check again to catch anything after the process exits

        return child.wait()
    else:
        try:
            return child.communicate(timeout=timeout)
        except subprocess.TimeoutExpired as e:
            logger.error(str(e))
            kill_project(logger)
            return child.communicate()

def create_logger():
    formatter = logging.Formatter('%(asctime)s :: %(name)s :: %(levelname)s :: %(message)s')
    logger = logging.getLogger()
    logger.setLevel(0)

    # create console handler and set level to debug
    ch = logging.StreamHandler()
    ch.setLevel(logging.INFO)
    ch.setFormatter(formatter)
    logger.addHandler(ch)

    # Create logger file
    file_handler = TimedRotatingFileHandler(HOME+'stats/log/'+time.strftime("%Y-%m-%d")+'.log', interval=1, backupCount=100)
    file_handler.setLevel(logging.DEBUG)
    file_handler.setFormatter(formatter)
    logger.addHandler(file_handler)

    return logger

class SortAlgo(Enum):
    __order__ = "Quick Heap Insert"
    Quick  = "#define SORTALGO(nb_elem, values) SU_QSort(nb_elem, values)"
    Heap   = "#define SORTALGO(nb_elem, values) SU_HSort(nb_elem, values)"
    Insert = "#define SORTALGO(nb_elem, values) SU_ISort(nb_elem, values)"

    @staticmethod
    def set(alg, file):
        assert alg in SortAlgo
        with open(file, 'r') as f :
            filedata = f.read()

        in_lines = filedata.split("\n")
        for i in range(len(in_lines)):
            for a in SortAlgo:
                tmp = a.value.replace("(", "\(").replace(")", "\)")
                if a == alg:
                    in_lines[i] = re.sub(r"^//[/ ]*"+tmp, a.value, in_lines[i])
                elif re.match(r"^//[/ ]*"+tmp, in_lines[i]) is None:
                    in_lines[i] = re.sub(tmp, "//"+a.value, in_lines[i])

        with open(file, 'w') as f:
            f.write("\n".join(in_lines))

class Files(Enum):
    __cmd__ = ""
    __order__ = "M50 M40 M30 M20 M10 M1 K500 K100 K10 K1"
    M50 = ("/tmp/test.50m.txt", 50000000)
    M40 = ("/tmp/test.40m.txt", 40000000)
    M30 = ("/tmp/test.30m.txt", 30000000)
    M20 = ("/tmp/test.20m.txt", 20000000)
    M10 = ("/tmp/test.10m.txt", 10000000)
    M1 = ("/tmp/test.1m.txt", 1000000)
    K500 = ("/tmp/test.500k.txt", 500000)
    K100 = ("/tmp/test.100k.txt", 100000)
    K10 = ("/tmp/test.10k.txt", 10000)
    K1 = ("/tmp/test.1k.txt", 1000)

    @staticmethod
    def generate(file, logger):
        assert file in Files
        command = "%sbin/project -m generation -o %s -n %d"%(HOME, *file.value)
        logger.info(command)
        logger.info("return : "+str(call(command, logger=logger, shell=True)))

    @staticmethod
    def generate_all(logger):
        Files.rm(logger)
        for k in Files:
            Files.generate(k, logger)

    @staticmethod
    def rm(logger):
        command = "ls /tmp | grep '^\(test\|tmp\).*\.txt$' | xargs -I{} rm /tmp/{}"
        logger.info(command)
        status = call(command, logger=logger, shell=True)

class Project(Enum):
    __file__ = "src/project_v%d.c"
    __m_option__ = "projectV%d"
    __order__ = "v0 v1 v2 v3 v4"# v5 v6"
    v0 = 0
    v1 = 1
    v2 = 2
    v3 = 3
    v4 = 4
    # v5 = 5
    # v6 = 6

    @staticmethod
    def create_stats():
        logger = create_logger()
        Files.generate_all(logger)

        # "sort_algo","entry_lines","split","time(sec)","cpu(%)"
        line = '"%s",%d,%d,%s,%s\r\n'
        # Launch stats
        for p in Project: # each project version
            stats=open(HOME+"stats/Project_"+p.name+".csv", "a+")
            stats.write('\r\n"sort_algo","entry_lines","split","time(sec)","cpu(%)"\r\n')
            stats.close()
            for alg in SortAlgo: # each sort algo
                # set algo active
                SortAlgo.set(alg, Project.__file__%(p.value))
                logger.info("SortAlog set %s in %s"%( alg.name, Project.__file__%(p.value) ))
                # compile
                call(MAKE, logger=logger, shell=True)
                for f in Files: # for each file size
                    for i in range(1,11): # 1 to 10 split
                        tmp = f.value[0].split(".")
                        sorted_file = ".".join(tmp[:-1] + ["sort"] + tmp[-1:])

                        command = CMD%(HOME, Project.__m_option__%(p.value), f.value[0], sorted_file, i)
                        logger.info(command)
                        outs, errs = call(command, logger=logger, is_stats=True, shell=True)

                        # write results of this test
                        res = (outs + errs).decode()

                        cpu = re.match(r"[^']*Percent of CPU this job got: (?P<cpu>[0-9]{1,4})%[^']*", res)
                        time = re.match(r"[^']*Elapsed \(wall clock\) time \(h:mm:ss or m:ss\): (?P<time>[0-9]?:[0-9]{2}\.[0-9]{2})[^']*", res)

                        if time is not None:
                            time = time.group(1)
                            tmp_time = int(time.split(":")[0]) *60 # minutes
                            time = tmp_time + float(time.split(":")[1]) # seconds

                        cpu = int(cpu.group(1)) if cpu is not None else "err"
                        if time is None:
                            time = "err"
                        elif time is not None and time >= TIMEOUT:
                            time = "inf"

                        stats=open(HOME+"stats/log/Project_"+p.name+".csv", "a+")
                        stats.write(line % (alg.name, f.value[1], i, time, cpu))
                        stats.close()

if __name__ == "__main__":
    Project.create_stats()
