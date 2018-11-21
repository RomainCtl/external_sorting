/**
 * @file
 * @brief Implementation by students of usefull function for the system project.
 */

#include "system_utils.h"

/**
 * @brief Maximum length (in character) for a command line.
 **/
#define SU_MAXLINESIZE (1024*8)

/********************** File managment ************/

void SU_removeFile(const char * file){
	char buffer[SU_MAXLINESIZE];
	snprintf(buffer, SU_MAXLINESIZE, "%s",file);
	//fprintf(stderr, "%s\n", buffer);

	//Define command for deleting file
	pid_t pid = fork();
	
	if( pid == -1 )
		perror("fork");
	else if( pid == 0)
	{
		char *command = "/bin/rm";
		execl(command, "rm", buffer, NULL );
	} else
		wait(NULL);
}
