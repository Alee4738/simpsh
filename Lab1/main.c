// CS 111 simpsh (lab1) - Simpleton Shell
// Usage: TODO: 
// Return codes: TODO:
//      0 - Success

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>

// Initialization
#define SUCCESS 0
#define PARSE_ERROR 1

enum options {RDONLY, WRONLY, COMMAND, VERBOSE};

struct option long_options[] = 
{
	{"rdonly", required_argument, NULL, RDONLY},
	{"wronly", required_argument, NULL, WRONLY},
	{"command", required_argument, NULL, COMMAND},
	{"verbose", no_argument, NULL, VERBOSE},
	{0,0,0,0}
};

void print_usage()
{
	printf("Usage: ./simpsh [--verbose] [--rdonly INPUTFILE] [--wronly OUTPUTFILE] [--command # # # COMMAND] where # are file descriptors for stdin, stdout, and stderr (file descriptors assigned from left to right)\n");
}

// TODO: make function
void exec_cmd(const char* cmd, unsigned in, unsigned out, unsigned err, char** args)
{
	// TODO: fork
	

	// TODO: Change file descriptors
	

	// TODO: make string for exec function
	
	
	// TODO: execute command
}

int main(int argc, char** argv)
{
	// Parse options
	int ret, test_fd;
	bool verbose_on = false;
	int num_files = 0; // file num (--command # # #) == num_files+3, meaning when a command asks for file num 0 as stdin, you give them file descriptor 3
	do 
	{
		test_fd = 0;
		ret = getopt_long(argc, argv, "", long_options, NULL);
		switch (ret)
		{
			// TODO: fix case ./simpsh --rdonly --wronly out.out
			// which currently takes --wronly as the file to read in
			// It should print diagnostic to stderr, then skip
			// over rdonly, continue to parse options
			case RDONLY:
				printf("RDONLY option seen!\n");
				test_fd = open(optarg, O_RDONLY);
				break;
			case WRONLY:
				printf("WRONLY option seen!\n");
				test_fd = open(optarg, O_WRONLY);
				break;
			case COMMAND:
				printf("COMMAND option seen!\n");
				break;
			case VERBOSE:
				printf("VERBOSE option seen!\n");
				verbose_on = true;
				break;
			case -1: break;
			default:
				print_usage();
				exit(PARSE_ERROR);
		}

		if (test_fd == -1) // could not open file
		{
			fprintf(stderr, "Failed open file \"%s\". Skipping option...\n", optarg);
		}

		/* for_each command:
		 *		if (verbose) print command
		 *
		 *
		 */
		else { num_files++; }

	} while (ret != -1);

	
	
	exit(SUCCESS);
}
