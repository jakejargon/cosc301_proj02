#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <signal.h>


char* insertspace(char* str) {
	int i = 0;
	char newstr[strlen(str)+2];
	newstr[0] = ' ';
	while (str[i] != '\0') {
		newstr[i+1] = str[i];
		i++;
	}
	newstr[i+1] = '\0';
	char* s = strdup(newstr);
	return s;
}

char* trimwhitespace(char* str) {
	
	int i = 0;
	
	char* returnthis;
	while (isspace(str[i])) {
		i++;
	}
	returnthis = &str[i];
	int k = strlen(returnthis)-1;
	while (isspace(returnthis[k])) {
		k--;
	}
	returnthis[k+1] ='\0';
	char* s = strdup(returnthis);
	return s;
}


char** get_input() {

	//find out user input
	char* str = (char*)malloc(1024);
	printf("shell_input: ");
	fgets(str, 1024, stdin);	
		
	//get rid of all comments in the string
	int buff_len = strlen(str);
	int i = 0;
	for (; i < buff_len; i++) {
		if (str[i] == '#') {
			str[i] = '\0';
			break;
		}
	}
	
	//establish parameters for strtok_r
	int iterator = 0;
	const char * sep = ";";
	char * tmp, * word;
	char * s = strdup(str);

	//find out upper bound on the array
	for (word = strtok_r (s, sep, &tmp); word != NULL; word = strtok_r(NULL, sep, &tmp)) {
		iterator++; 
	}
	
	free(s);
	s = strdup(str);
	
	//malloc the pointer array
	char* norms_array[iterator+1]; char * modes_array[iterator+1]; char* exits_array[iterator+1];
	norms_array[0] = NULL;
	exits_array[0] = NULL;
	modes_array[0] = NULL;
	
	iterator = 0; int modei = 0; int exiti = 0;
	
	//add all items to their respective arrays
	for (word = strtok_r (s, sep, &tmp); word != NULL; word = strtok_r(NULL, sep, &tmp)) {
		word = trimwhitespace(word);
		//this is weird. Check if either the command "mode" or "mode 'something'"
		if (strncmp(word, "mode", 4) == 0) {
			//insert only the command following mode into the list
			if (strncmp(word, "mode ", 5) == 0) {
				word = trimwhitespace(&word[4]);
				//adding a space at the end prevents the user from typing the command "sequential" without the mode and having the state change
				word = insertspace(word);
				modes_array[modei] = word;		
				modei++;
				modes_array[modei] = NULL;
			}
			//insert the command mode into the list
			else if (word[4] == '\0') {
				modes_array[modei] = word;
				modei++;
				modes_array[modei] = NULL;
			}
			//turns out the word was a weird command like moderate...
			else {
				norms_array[iterator] = word;
				iterator++;
				norms_array[iterator] = NULL;
			}
		}
		else if (strncmp(word, "exit", 4) == 0) {
			if (word[4] =='\0') {
				exits_array[exiti] = word;
				exiti++;
				exits_array[exiti] = NULL;
			}
			//turns out the word was a weird command like exiter...
			else {
				norms_array[iterator] = word;
				iterator++;
				norms_array[iterator] = NULL;
			}
		}
		//hooray it's just a normal command!
		else {
			norms_array[iterator] = word;
			iterator++;
			norms_array[iterator] = NULL;
		}
	}

	free (s);
	//we now have three arrays of various commands, time to combine and sort.
	char ** return_array = (char**) malloc((iterator+exiti+modei+1)*sizeof(char*));
	exits_array[exiti] = NULL; modes_array[modei] = NULL; norms_array[iterator] = NULL;
	iterator = 0;
	int finali = 0;
	
	//I use three while loops to assemble the final list
	while (norms_array[iterator] != NULL) {
		return_array[finali] = strdup(norms_array[iterator]);
		iterator++;
		finali++;
	}
	iterator = 0;
	while (exits_array[iterator] != NULL) {
		return_array[finali] = strdup(exits_array[iterator]);
		iterator++;
		finali++;
	}
	iterator = 0;
	while (modes_array[iterator] != NULL) {
		return_array[finali] = strdup(modes_array[iterator]);
		iterator++;
		finali++;
	}
	return_array[finali] = NULL;
	

	return return_array;
}


int set_mode (char* mode, int mode_data) {
	/*mode code
	0 - sequential
	1 - parallel
	2 - printdemode
	3 - invalid comand
	*/
	
		
	if ((strcmp(mode, " sequential") == 0) || (strcmp(mode, " s") == 0)) {
		printf("mode status changed --> sequential\n");
		mode_data = 0;
	}
	else if ((strcmp(mode, " parallel") == 0) || (strcmp(mode, " p") == 0)) {
		printf("mode status changed --> parallel\n");
		mode_data = 1;
	}
		
	else if (strcmp(mode, "mode") == 0) {
		
		if (mode_data == 0) {
			printf("current mode status --> sequential\n");
		}
		else {
			printf("current mode status --> parallel\n");
		}
	}
	else {
		printf("command: mode %s not recognized\n", mode);
	}
	return mode_data;
}


	

int respond(char** user_input, int mode) {
	/*
	respond codes
	0 - sequential
	1 - parallel
	2 - exit
	3 - nothing
	*/
	
	//user entered nothing
	if (user_input[0] == NULL) {
		return 3;
	}
	int i = 0;
	int pid;
	
	while (user_input[i] != NULL) {

		//user wants out, lets leave!
		if (strcmp(user_input[i], "exit") == 0) {
			return 2;
		}
		//user entered a command to change the mode
		else if (((user_input[i])[0] == ' ') || strcmp(user_input[i], "mode") == 0) {
			mode = set_mode(user_input[i], mode);
		}
			//either a 0 or 1 will be returned
		else {
		
			int iterator = 0;
			const char * sep = " \n\t";
			char * tmp, * word;	
			char * s = strdup(user_input[i]);
	
			for (word = strtok_r (s, sep, &tmp); word != NULL; word = strtok_r(NULL, sep, &tmp)) {
				iterator++;
			}
			free (s);
			s = strdup(user_input[i]);
	
			char* return_array[iterator+1];
			return_array[iterator] = NULL;
			iterator = 0;
	
			for (word = strtok_r (s, sep, &tmp); word != NULL; word = strtok_r(NULL, sep, &tmp)) {
				return_array[iterator] = word;
				iterator++;
			}
			
			return_array[iterator] = NULL;
			free (s);
				
			if (mode == 0) {
				
				//sequential code
			
				pid = fork();
				if (pid == 0) {
					if (execv(return_array[0], return_array) < 0) {
						printf("error: execsv failed\n");
						//kill(pid, SIGTERM);
					}
				}
				else {
				 	wait(&pid);
				 	exit(0);
				 }
			}	
			
			else {
				//parallel code
			}
		}
		i++;
	}
	return mode;
}






