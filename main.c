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
const int STD_BUFF_START = 10; // standard start size of generic dyn. array

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

struct cmd
{
	pid_t pid;
	int e_status;
	char* name;
	char** argv;
	int num_args; // current num of args in argv
	int argv_size; // limit on # of args to hold, argv is actually argv_size+1 because it must have null at end
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
	struct cmd* cmds;
	int num_cmds = 0;
	cmds = malloc((STD_BUFF_START+1)*sizeof(struct cmd)); // TODO: free later

	// Parse the options
	do 
	{
		// fds dynamically resizable
		if (num_files >= fds_limit-1) { // -1 bc pipe adds 2 fds at once
			fds = realloc(fds, 2*num_files*sizeof(int));
			// TODO: realloc failed
			fds_limit = 2*num_files;
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

			case PIPE:
				if (verbose_on) {
					printf("%s\n", argv[optind-1]); 
				}
				
				int pipefd[2];
				if (pipe(pipefd) == -1) { // open 
					popt_err(argv[optind-1], NULL, "pipe could not be opened");
					break;
				}
				else { 
					fds[num_files] = pipefd[0];
					fds[num_files+1] = pipefd[1];
					num_files += 2; 
				}
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

				// See how many arguments there are
				// while putting them into the command struct
				arg_ind = optind + 2; // account for file numbers
				num_args = 0; // num of args we'll put into argv for execvp
				while (argv[arg_ind] != NULL 
					&& strstr(argv[arg_ind], "--") != argv[arg_ind]) 
				{

					arg_ind++; num_args++;
				}

				// errchk: doesn't have even a command 
				if (num_args == 0) {
					// Lots of init; should prob skip reading this
					int len = strlen(optarg) + strlen(argv[optind]) + strlen(argv[optind+1]);
					char* tmp = malloc((len+2+1)*sizeof(char)); // 2 spaces, 1 null
					tmp = strcat(tmp, optarg);
					tmp = strcat(tmp, argv[optind]);
					tmp = strcat(tmp, argv[optind + 1]);
					
					// main part of the errchk
					popt_err(argv[optind-2], tmp, "no command specified");

					free(tmp);
				}

				// errchk: invalid file descriptors, fds[ind] < 0 == closed fd
				if (in >= num_files || in < 0 || fds[in] < 0
				 || out >= num_files || out < 0 || fds[out] < 0
				 || err >= num_files || err < 0 || fds[err] < 0
				 ) {
					fprintf(stderr, "Invalid file descriptor in \"--command %d %d %d %s...\"\n", in, out, err, argv[optind+2]);
					break;
				}


				// Now that we know it has valid command structure,
				// put command into cmds array
				cmds[num_cmds].name = strdup(argv[optind+2]); // TODO: free later
				cmds[num_cmds].num_args = num_args;
				cmds[num_cmds].argv = malloc((num_args+1)*sizeof(char*)); // TODO: free later; +1 means need NULL at end
				// TODO: malloc failed
				cmds[num_cmds].argv_size = num_args;

				// Capture all arguments
				// copy cmd's args into argv of cmds (including cmd)
				for (int j = 0; j < num_args; j++) {
					cmds[num_cmds].argv[j] = strdup(argv[j+optind+2]); // TODO: free later
				}
				optind = arg_ind; // set optind to next command

				// Verbose
				if (verbose_on) {
					printf("--command ");
					printf("%d %d %d ", in, out, err);
					for (int i = 0; i < num_args-1; i++) // -1 bc no space at end
					{
						printf("%s ", cmds[num_cmds].argv[i]);
					}
					printf("%s\n", cmds[num_cmds].argv[num_args-1]);
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
					execvp(cmds[num_cmds].argv[0], cmds[num_cmds].argv);
					// TODO: execvp failed
					_exit(1);
				}
				else { // command was properly executed
					cmds[num_cmds].pid = forker;
					has_command = true;
					num_cmds++;
				}
				break;
			
			case WAIT:
				;
				int i = 0;
				int num_passed = 0;
				while (num_passed < num_cmds) {
					if (waitpid(cmds[i].pid, &cmds[i].e_status, WNOHANG) == 0) {
						// TODO: wait failed
					}
					else {
						num_passed++;
					}
					
				}
				
				int max_exit;
				// print exit statuses - We can assume wait is last option specified
				for (int i = 0; i < num_cmds; i++) {
					if (WIFEXITED(cmds[i].e_status)) {
						int e_status = WEXITSTATUS(cmds[i].e_status);
						if (i == 0) {
							max_exit = e_status;
						}

						// Actually print
						printf("%d", WEXITSTATUS(cmds[i].e_status), cmds[i].name);
						for (int j = 0; j < cmds[i].num_args; j++) {
							printf(" %s", cmds[i].argv[j]);
						}
						printf("\n");

						// Also record into exit_status
						if (e_status > max_exit) {
							max_exit = WEXITSTATUS(cmds[i].e_status);
						}
					}
					else {
						popt_err(cmds[i].name, NULL, "did not exit normally");
					}
				}
				exit_status = max_exit;
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


	// FREEDOM
	for (int i = 0; i < num_cmds; i++)
	{
		for (int j = 0; j < cmds[i].num_args; j++)
		{
			free(cmds[i].argv[j]);
		}
		free(cmds[i].argv);
		free(cmds[i].name);
	}
	free(cmds);

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
