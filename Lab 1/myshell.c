// ACADEMIC INTEGRITY PLEDGE
//
// - I have not used source code obtained from another student nor
//   any other unauthorized source, either modified or unmodified.
//
// - All source code and documentation used in my program is either
//   my original work or was derived by me from the source code
//   published in the textbook for this course or presented in
//   class.
//
// - I have not discussed coding details about this project with
//   anyone other than my instructor. I understand that I may discuss
//   the concepts of this program with other students and that another
//   student may help me debug my program so long as neither of us
//   writes anything during the discussion or modifies any computer
//   file during the discussion.
//
// - I have violated neither the spirit nor letter of these restrictions.
//
//
//
// Signed: Connor Taylor 	Date: 3/1/2021

// 3460:426 Lab 1 - Basic C shell rev. 9/10/2020

/* Basic shell */

/*
 * This is a very minimal shell. It finds an executable in the
 * PATH, then loads it and executes it (using execv). Since
 * it uses "." (dot) as a separator, it cannot handle file
 * names like "minishell.h"
 *
 * The focus on this exercise is to use fork, PATH variables,
 * and execv. 
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#define MAX_ARGS		64
#define MAX_ARG_LEN		16
#define MAX_LINE_LEN	80
#define WHITESPACE		" ,\t\n"

struct command_t {
   char *name;
   int argc;
   char *argv[MAX_ARGS];
};

/* Function prototypes */
int parseCommand(char *, struct command_t *);
void printPrompt();
void readCommand(char *);

int main(int argc, char *argv[]) {
	int pid;
	int status;
	int backgroundProc = 0;
	char cmdLine[MAX_LINE_LEN];
	struct command_t command;
	char args[1][1];
	args[0][0] = '\0';
	
	do {
		printPrompt();
		/* Read the command line and parse it */
		readCommand(cmdLine);
		parseCommand(cmdLine, &command);
		command.argv[command.argc] = NULL;
		
		// sets equivalent command from command line
		if(strcmp(command.name, "C") == 0) {
			command.name = "cp";
		}
		else if(strcmp(command.name, "D") == 0) {
			command.name = "rm";
		}
		else if(strcmp(command.name, "E") == 0) {
			command.name = "echo";
		}
		else if(strcmp(command.name, "H") == 0) {
			printf("%s\n", "C\tfile1 file2 Copy; create file2, copy all bytes of file1 to file2 without deleting file1.");
			printf("%s\n", "D\tfile Delete the named file.");
			printf("%s\n", "E\tcomment Echo; display comment on screen followed by a new line.");
			printf("%s\n", "H\tHelp; display the user manual.");
			printf("%s\n", "L\tList the contents of the current directory.");
			printf("%s\n", "M\tfile Make; create the named text file by launching a text editor.");
			printf("%s\n", "P\tfile Print; display the contents of the named file on screen.");
			printf("%s\n", "Q\tQuit the shell.");
			printf("%s\n", "S\tSurf the web by launching a browser as a background process.");
			printf("%s\n", "W\tWipe; clear the screen.");
		}
		else if(strcmp(command.name, "L") == 0) {
			command.name = "ls";
			command.argv[0] = "ls";
			command.argv[1] = "-l";
			char temp[1024];
			char* curDir = getcwd(temp, 1024);
			printf("%s\n", curDir);
			
		}
		else if(strcmp(command.name, "M") == 0) {
			command.name = "nano";
		}
		else if(strcmp(command.name, "P") == 0) {
			command.name = "more";
		}
		else if(strcmp(command.name, "Q") == 0) {
			signal(SIGQUIT, SIG_IGN);
			kill(0, SIGQUIT);
		}
		else if(strcmp(command.name, "S") == 0) {
			argv[0][0] = argc;
			backgroundProc = 1;
			command.name = "firefox";
		}
		else if(strcmp(command.name, "W") == 0) {
			argv[0][0] = argc;
			command.name = "clear";
		}
		/* Create a child process to execute the command */
		if ((pid = fork()) == 0 && strcmp(command.name, "Q") != 0 && strcmp(command.name, "H") != 0) {
			/* Child executing command */
			if(execvp(command.name, command.argv) != 0) {
				printf("Command failed to execute.\n");
				exit(1);
			}
		}
		else {
			// determines if process is in background
			if(backgroundProc == 0)
				waitpid(pid, &status, 0);
			else 
				backgroundProc = 0;
		}
	}while (strcmp(command.name, "Q") != 0);
	/* Shell termination */
	if(pid != 0) // ensures only parent prints message on exit
		printf("\n\n shell: Terminating successfully\n");
	return 0;
}

/* End basic shell */

/* Parse Command function */

/* Determine command name and construct the parameter list.
 * This function will build argv[] and set the argc value.
 * argc is the number of "tokens" or words on the command line
 * argv[] is an array of strings (pointers to char *). The last
 * element in argv[] must be NULL. As we scan the command line
 * from the left, the first token goes in argv[0], the second in
 * argv[1], and so on. Each time we add a token to argv[],
 * we increment argc.
 */
int parseCommand(char *cLine, struct command_t *cmd) {
   int argc;
   char **clPtr;
   /* Initialization */
   clPtr = &cLine;	/* cLine is the command line */
   argc = 0;
   cmd->argv[argc] = (char *) malloc(MAX_ARG_LEN);
   /* Fill argv[] */
   while ((cmd->argv[argc] = strsep(clPtr, WHITESPACE)) != NULL) {
      cmd->argv[++argc] = (char *) malloc(MAX_ARG_LEN);
   }

   /* Set the command name and argc */
   cmd->argc = argc-1;
   cmd->name = (char *) malloc(sizeof(cmd->argv[0]));
   strcpy(cmd->name, cmd->argv[0]);
   return 1;
}

/* End parseCommand function */

/* Print prompt and read command functions - Nutt pp. 79-80 */

void printPrompt() {
	/* Build the prompt string to have the machine name,
     * current directory, or other desired information
     */
	 
	// gets the current directory
	char temp[1024];
	char* curDir = getcwd(temp, 1024);
	
	// gets the machine (host) name
	char machName[1024];
	machName[1023] = '\0';
	gethostname(machName, 1023);
	
	char* promptString = "|linux clt94|>";
	printf("%s%s%s%s ", machName, ":", curDir, promptString);
}

void readCommand(char *buffer) {
   /* This code uses any set of I/O functions, such as those in
    * the stdio library to read the entire command line into
    * the buffer. This implementation is greatly simplified,
    * but it does the job.
    */
   fgets(buffer, 80, stdin);
}

/* End printPrompt and readCommand */
