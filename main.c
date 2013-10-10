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
#include "list.h"


int main(int argc, char **argv) {
	char** user_input = NULL;
	int mode = 0;
	while (mode != 2) {
		user_input = NULL;
		user_input = get_input();
		mode = respond(user_input, mode);		
	}
	free(user_input);
	return 0;
}
