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

int main(int argc, char** argv)
{
	int ret, test_fd; // for parsing
	bool verbose_on = false;
	int num_files = 0; // file num (--command # # #) == num_files+3, meaning when a command asks for file num 0 as stdin, you give them file descriptor 3

	// For command processing
	int size_count, in, out, err, arg_ind, num_args;
	char** my_argv;
	
	// Parse the options
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
				// Scan in file numbers, TODO: check not null, they're numbers, finally, that they're less than num_files
				sscanf(optarg, "%d", &in);
				sscanf(argv[optind], "%d", &out);
				sscanf(argv[optind + 1], "%d", &err);
				// TODO: sscanf failedx3

				arg_ind = optind + 2; // account for file numbers
				num_args = 0; // num of args we'll put into argv for execvp
				while (argv[arg_ind] != NULL 
					&& strstr(argv[arg_ind], "--") != argv[arg_ind])
				{
					arg_ind++;
					num_args++;
				}
				// TODO: if (num_args == 0) missing command!
				my_argv = malloc((num_args+1)*sizeof(char*));
				// TODO: malloc failed
				for (int j = 0; j < num_args; j++)
				{	
					my_argv[j] = argv[j+optind+2];
				}	

				// Verbose
				if (verbose_on) {
					printf("--command ");
					printf("%d %d %d ", in, out, err);
					int i;
					for (i = 0; i < num_args-1; i++)
					{
						printf("%s ", my_argv[i]);
					}
					printf("%s\n", my_argv[i]);
				}

				// TODO: Fork
				
				// TODO: file descriptor editing

				// TODO: Execute command
				// execvp(argv[0], argv);
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
			fprintf(stderr, 
			"Failed open file \"%s\". Skipping option...\n", optarg);
		}
		else { num_files++; }

	} while (ret != -1);

	
	// TODO: free my_argv
	
	exit(SUCCESS);
}
