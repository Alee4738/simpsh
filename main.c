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

void sig_handler(int signum)
{
	fprintf(stderr, "Signal %d caught! Exiting...\n", signum);
	exit(signum);
}

enum options { 
	PIPE = 3,
	COMMAND = 4, WAIT = 5, // assignments due to conflicts with other flags 
	CLOSE, VERBOSE, PROFILE, ABORT, CATCH, IGNORE, DEFAULT, PAUSE
};

struct option long_options[] = 
{
	// File Flags
	{"append", no_argument, NULL, O_APPEND},
	{"cloexec", no_argument, NULL, O_CLOEXEC},
	{"creat", no_argument, NULL, O_CREAT},
	{"dsync", no_argument, NULL, O_DSYNC},
	{"directory", no_argument, NULL, O_DIRECTORY},
	{"excl", no_argument, NULL, O_EXCL},
	{"nofollow", no_argument, NULL, O_NOFOLLOW},
	{"nonblock", no_argument, NULL, O_NONBLOCK},
	{"rsync", no_argument, NULL, O_RSYNC},
	{"sync", no_argument, NULL, O_SYNC},
	{"trunc", no_argument, NULL, O_TRUNC},

	// File opening options
	{"rdonly", required_argument, NULL, O_RDONLY},
	{"wronly", required_argument, NULL, O_WRONLY},
	{"rdwr", required_argument, NULL, O_RDWR},
	{"pipe", no_argument, NULL, PIPE},


	// Subcommand options
	{"command", required_argument, NULL, COMMAND},
	{"wait", no_argument, NULL, WAIT},

	// Misc. options
	{"close", required_argument, NULL, CLOSE},
	{"verbose", no_argument, NULL, VERBOSE},
	{"profile", no_argument, NULL, PROFILE},
	{"abort", no_argument, NULL, ABORT},
	{"catch", required_argument, NULL, CATCH},
	{"ignore", required_argument, NULL, IGNORE},
	{"default", required_argument, NULL, DEFAULT},
	{"pause", no_argument, NULL, PAUSE},

	{0,0,0,0}
};
int exit_status = 0;
bool has_command = false; // ran at least 1 subcommand


//
// Helper functions
//
void pusage(); // print usage
void popt_err(const char* option, const char* args, const char* desc); 
	// print option error - sets exit_status appropriately
	// params: option, argument (1 only), description

bool pno_operand(char** argv); 
	// prints missing operand - sets exit_status & optind appropriately
	// return true if no operand exists
	// 		(mistakenly got optarg starting with "--")


//
// The meaty part
//
int main(int argc, char** argv)
{
	// For getopt_long
	int ret;
	opterr = 0; // do not print default error msg

	// For --verbose
	bool verbose_on = false;

	// For creating files (rdonly, wronly, rdwr, pipe)
	int* fds = malloc(FD_TABLE_START*sizeof(int));
	int fds_limit = FD_TABLE_START;
	int num_files = 0; // === fds's actual size 
	int flags = 0;
	int ret_flag;

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
			// 
			// 1. FILE FLAGS
			//
			case O_APPEND:
			case O_CLOEXEC:
			case O_CREAT:
			case O_DIRECTORY:
			case O_DSYNC:
			case O_EXCL:
			case O_NOFOLLOW:
			case O_NONBLOCK:
			// case O_RSYNC: (glibc says O_RSYNC == O_SYNC)
			case O_SYNC:
			case O_TRUNC:		
				if (verbose_on) {
					printf("%s\n", argv[optind-1]); 
				}
				flags = flags|ret;
				break;

			// 
			// 2. FILE-OPENING OPTIONS
			//
			case O_RDONLY:
			case O_WRONLY: 
			case O_RDWR: // Note: the difference is captured in ret
				// errchk: missing operand
				if (pno_operand(argv)) { break; }

				if (verbose_on) {
					printf("%s %s\n", argv[optind-2], optarg);
				}
				
				// open the file
				fds[num_files] = open(optarg, ret|flags);
				flags = 0; // reset
				// errck: open failed and returned -1
				if (fds[num_files] < 0) { 
					popt_err(argv[optind-2], optarg, "file could not be opened");
				}
				else { num_files++;	}
				break;

			case PIPE: // TODO
				break;


			// 
			// 3. Subcommand options
			//
			case COMMAND:
				// Scan in file numbers, TODO: check not null, they're numbers 
				// Process all strings related to cmd (i o e cmd args)
				sscanf(optarg, "%d", &in);
				sscanf(argv[optind], "%d", &out);
				sscanf(argv[optind + 1], "%d", &err);
				// TODO: sscanf failed x3

				// errchk: valid file descriptors, fds[ind] < 0 == closed fd
				if (in >= num_files || in < 0 || fds[in] < 0
				 || out >= num_files || out < 0 || fds[out] < 0
				 || err >= num_files || err < 0 || fds[err] < 0
				 ) {
					fprintf(stderr, "Invalid file descriptor in \"--command %d %d %d %s...\"\n", in, out, err, my_argv[0]);
					break;
				}

				// Capture all arguments
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
			
			case WAIT: // TODO:
				break;
			

			// 
			// 4. Misc. Options
			//
			case CLOSE:
				// errchk: missing operand
				if (pno_operand(argv)) { break; }

				if (verbose_on) {
					printf("%s %s\n", argv[optind-2], optarg); 
				}

				// More error checking, closing is easy
				ret = atoi(optarg); // Note: atoi returns 0 on fail to convert
				if (ret >= num_files || ret < 0) { // Invalid file num
					fprintf(stderr, "--close: Invalid file number %d\n", ret);
					if (ret == -1) { // messes up while loop
						ret = -2;
					}
				}
				else if (optarg[0] < '0' || optarg[0] > '9') { // arg not a num
					popt_err(argv[optind-2], argv[optind-1], "must be int");
				}
				else if (fds[ret] == -1) { // Already closed fd
					fprintf(stderr, "--close: Already closed file number %d\n", ret);
				}
				else { // valid - close it
					close(fds[ret]);
					fds[ret] = -1;
					ret = -2; // hack for next step
				}
				if (ret != -2) {
					if (!has_command && exit_status == 0) {
						exit_status = 1;
					}
				}
				break;

			case VERBOSE: verbose_on = true; break;

			case PROFILE: // TODO:
				break;

			case ABORT:
				if (verbose_on) {
					printf("%s\n", argv[optind-1]); 
				}
				; char* die = 0; // get ready
				*die = ABORT; // DIE
				break; // and gracefully exit if you don't

			case CATCH: 
			case IGNORE:
			case DEFAULT:
				// errchk: missing operand
				if (pno_operand(argv)) { break; }

				if (verbose_on) {
					printf("%s %s\n", argv[optind-2], optarg); 
				}

				// More error checking, closing is easy
				int sig = atoi(optarg); // Note: atoi returns 0 on fail to convert
				if (ret == CATCH) {
					signal(sig, &sig_handler);
				}
				else if (ret == IGNORE) {
					signal(sig, SIG_IGN);
				}
				else { // == DEFAULT
					signal(sig, SIG_DFL);
				}
				// Clean up in signal is weird
				if (ret == -1) {
					ret = -2; // because of the while (ret != -1)
				}
				break;
			
			case PAUSE: pause(); break;


			//
			// 5. Built-in errors
			//
			case -1: break; // getopt done processing args
			case 63: // arg not provided, required_argument enforced
				pno_operand(argv);
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

void popt_err(const char* option, const char* arg, const char* desc)
{
	char* empty = "";
	fprintf(stderr, "%s %s: %s\n", 
		(option != NULL) ? option : empty, 
		(arg != NULL) ? arg : empty, 
		(desc != NULL) ? desc : empty );
	if (!has_command && exit_status == 0) {
		exit_status = 1;		
	}
}

bool pno_operand(char** argv)
{
	// not at end, took next option as argument
	if (optarg != NULL && strstr(optarg, "--") == optarg) { 
		optind--; // fix indexing
		popt_err(argv[optind-1], NULL, "missing operand");
		return true;
	}
	// at the end, no arg provided
	else if (argv[optind] == NULL) {
		popt_err(argv[optind-1], NULL, "missing operand");
		return true;
	}
	return false;
}
