// CS 111 simpsh (lab1) - Simpleton Shell
// Usage: TODO: 
// Return codes: TODO:
//      0 - Success

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <getopt.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>

const int FD_TABLE_START = 10; // start fd_table size
const int ARGV_START = 10; // start my_argv size

enum options {RDONLY, WRONLY, COMMAND, VERBOSE};

struct option long_options[] = 
{
	{"rdonly", required_argument, NULL, RDONLY},
	{"wronly", required_argument, NULL, WRONLY},
	{"command", required_argument, NULL, COMMAND},
	{"verbose", no_argument, NULL, VERBOSE},
	{0,0,0,0}
};
int exit_status = 0;
bool has_command = false; // ran at least 1 subcommand

// Helper functions
void pusage();
void psyntax_err(char* option); // print syntax error for given option, sets exit_status appropriately
void popen_err(const char* option); // print open error for given option, sets exit_status appropriately

int main(int argc, char** argv)
{
	// For getopt_long
	int ret;
	opterr = 0; // do not print default error msg

	// For --verbose
	bool verbose_on = false;

	// For creating files (rdonly, wronly, pipe)
	int* fds = malloc(FD_TABLE_START*sizeof(int));
	int fds_limit = FD_TABLE_START;
	int num_files = 0; // === fds's actual size 

	// For --command processing
	int size_count, in, out, err, arg_ind, num_args;
	char** my_argv = malloc(ARGV_START*sizeof(char*));
	int my_argv_size = ARGV_START;

	// Parse the options
	do 
	{
		// fds dynamically resizable
		if (num_files >= fds_limit) {
			fds = realloc(fds, 2*fds_limit*sizeof(int));
			// TODO: realloc failed
			fds_limit *= 2;
		}

		
		ret = getopt_long(argc, argv, "", long_options, NULL);
		switch (ret)
		{
			case RDONLY:
				// errchk: missing operand and mistook next option as its arg
				if (strstr(optarg, "--") == optarg) {
					optind--; // fix indexing
					psyntax_err("--rdonly");
					break;
				}

				if (verbose_on) {
					printf("--rdonly %s\n", optarg);
				}
				
				// open the file
				fds[num_files] = open(optarg, O_RDONLY);
				// errck: open failed
				if (fds[num_files] < 0) { 
					popen_err("--rdonly");
				}
				else { num_files++;	}
				break;

			case WRONLY:
				// errchk: missing operand and mistook next option as its arg
				if (strstr(optarg, "--") == optarg) {
					optind--; // fix indexing
					psyntax_err("--wronly");
					break;
				}

				if (verbose_on) {
					printf("--wronly %s\n", optarg);
				}
				
				// open the file
				fds[num_files] = open(optarg, O_WRONLY);
				// errck: open failed
				if (fds[num_files] < 0) { 
					popen_err("--wronly");
				}
				else { num_files++;	}
				break;

			case COMMAND:
				// Scan in file numbers, TODO: check not null, they're numbers, finally, that they're less than num_files
				// Process all strings related to cmd (i o e cmd args)
				sscanf(optarg, "%d", &in);
				sscanf(argv[optind], "%d", &out);
				sscanf(argv[optind + 1], "%d", &err);
				// TODO: sscanf failed x3
				arg_ind = optind + 2; // account for file numbers
				num_args = 0; // num of args we'll put into argv for execvp
				while (argv[arg_ind] != NULL 
					&& strstr(argv[arg_ind], "--") != argv[arg_ind]) 
				{
					arg_ind++; num_args++;
				}
				// TODO: if (num_args == 0) missing command!
				if (my_argv_size <= num_args) {
					my_argv = realloc(my_argv, (num_args+1)*sizeof(char*)); // need NULL at end
					// TODO: realloc failed
					my_argv_size = num_args + 1;
				}

				// copy cmd's args (including cmd)
				for (int j = 0; j < num_args; j++) {
					my_argv[j] = argv[j+optind+2];
				}
				optind = arg_ind; // set optind to next command
				

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

				// Fork
				pid_t forker = fork();
				if (forker < 0) {
					perror(NULL);
					fprintf(stderr, 
						"Could not execute command. Skipping...\n");
				}
				else if (forker == 0) {
					// File descriptor editing
					// TODO: once we implement closing fd's, need to check more (not using a closed fd)
					if (in >= num_files || out >= num_files || err >= num_files) {
						fprintf(stderr, "Invalid file descriptor in \"--command %d %d %d %s...\"", in, out, err, my_argv[0]);
						break;
					}

					dup2(fds[in], 0);
					dup2(fds[out], 1);
					dup2(fds[err], 2);
					// TODO: dup2 failed x 3
					
					// Close all other fds created by simpsh
					for (int i = 0; i < num_files; i++) {
						close(fds[i]);
					}

					// Execute command
					execvp(my_argv[0], my_argv);
					// TODO: execvp failed
				}
				else { // command was properly executed
					has_command = true;
				}
				break;

			case VERBOSE:
				verbose_on = true;
				break;

			case -1: break;
			case 63: // no required_argument for option
				psyntax_err(argv[optind-1]);
				break;
			default:
				pusage();
				break;
		}

	} while (ret != -1);

	
	free(my_argv);
	
	exit(exit_status);
}

void pusage()
{
	printf("Usage: ./simpsh [--verbose] [--rdonly INPUTFILE] [--wronly OUTPUTFILE] [--command # # # COMMAND] where # are file descriptors for stdin, stdout, and stderr (file descriptors assigned from left to right)\n");
}

void psyntax_err(char* option)
{
	fprintf(stderr, "%s: syntax error. Skipping option...\n", option);
	if (!has_command && exit_status == 0) {
		exit_status = 1;
	}
}

void popen_err(const char* option) 
{
	fprintf(stderr, 
	"Failed to open file \"%s\". Skipping option...\n", option);
	if (!has_command && exit_status == 0) {
		exit_status = 1;		
	}
}

