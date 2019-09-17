# External sorting
Projet universitaire de système d'exploitation : trions ensembles

## Build the documentation

In the root directory of the project, run the following command:

```$ make doc```

Then open the file doc.html.

## Run specific version of Project

> generate file to sort

```
$ bin/project -m generation -o /tmp/test.txt -n 50000000
```

```
$ make
$ bin/project -m projectV1 -i /tmp/test.txt -o /tmp/test.sort.txt -k 5
```

> to get summarize of system resource used

```
$ /usr/bin/time -v ./bin/project -m projectV2 -i /tmp/test.txt -o /tmp/test.sort.txt -k 5
```