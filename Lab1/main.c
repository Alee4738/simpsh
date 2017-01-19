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
#include <string.h>

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
	char test_buf[100];
	int tb_size = 0;
	int i;
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
				if (verbose_on) {
					printf("--rdonly %s\n", optarg);
				}
				test_fd = open(optarg, O_RDONLY);
				break;

			case WRONLY:
				if (verbose_on) {
					printf("--wronly %s\n", optarg);
				}
				test_fd = open(optarg, O_WRONLY);
				break;

			case COMMAND:
				// Put relevent args for command in single string
				i = optind - 1;
				while (argv[i] != NULL && strstr(argv[i], "--") != argv[i] )
				{
					// copy argv[i] into test_buf
					strcpy((char*)test_buf+tb_size, argv[i]);

					// replace '\0' with ' '
					while (test_buf[tb_size] != '\0')
					{
						tb_size++;
					}
					test_buf[tb_size] = ' ';
					tb_size++; // we want to keep the space

					// go to next argument
					i++;
				}
				tb_size--; // we do not want to keep the last space
				test_buf[tb_size] = '\0';
				
				if (verbose_on) {
					printf("--command %s\n", test_buf);
				}

				// Execute command

				// Attempt to read in everything until you see "--"
				/*
				char lastChar = 0;
				char currChar = 0;
				while (lastChar != '-' && currChar != '-')
				{
					strncpy
					tb_size++;
				}
				*/
				break;

			case VERBOSE:
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
