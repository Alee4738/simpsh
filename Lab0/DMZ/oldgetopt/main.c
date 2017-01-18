#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define INPUT 'i'
#define OUTPUT 'o'
#define SEGFAULT 's'
#define CATCH 'c'

struct option long_options[] =
{
	{"input", required_argument, NULL, INPUT},
	{"output", required_argument, NULL, OUTPUT},
	{"segfault", no_argument, NULL, SEGFAULT},
	{"catch", no_argument, NULL, CATCH},
	{0,0,0,0}
};

/*
void sig_handler (int signo) {
	printf("Got it\n");
	fprintf(stdout, "Signal number %d passed. Quitting...\n", signo);
	_exit(1);
}
*/

int main(int argc, char* argv[]) {

	if (argc < 2) {
		fprintf(stderr, "Usage: ./opts <options>\n");
		_exit(2);
	}	

	// Test understanding on getopt_long
	char ret;
	ret = getopt_long(argc, argv, "", long_options, NULL);

	switch (ret) 
	{
		case INPUT:
			printf("%s option was seen first!\n", "input");
			break;
		case OUTPUT:
			printf("%s option was seen first!\n", "output");
			break;
		case SEGFAULT:
			printf("%s option was seen first!\n", "segfault");
			break;
		case CATCH:
			printf("%s option was seen first!\n", "catch");
			break;
		default:
			break;
	}

	return 0;



	/* 
	if (argc != 2) {
		printf("Usage: ./signal <signal number>\n");
		_exit(2);
	}

	if (signal(SIGINT, sig_handler) == SIG_ERR) {
		perror("EINVAL error. signal didn't work\n");
		_exit(3);
	}

	while(1) 
    		sleep(1);
	return 0;
	*/
}
