#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>

#define INPUT 'i'
#define OUTPUT 'o'
#define SEGFAULT 's'
#define CATCH 'c'
#define BUFSIZE 256

struct option long_options[] =
{
	{"input", required_argument, NULL, INPUT},
	{"output", required_argument, NULL, OUTPUT},
	{"segfault", no_argument, NULL, SEGFAULT},
	{"catch", no_argument, NULL, CATCH},
	{0,0,0,0}
};


void seg_handler (int signo) {
	fprintf(stderr, "Caught segmentation fault. Exiting...\n");
	_exit(3);
}


int main(int argc, char* argv[]) {

	// Init: save stdin, stdout becasue file desc's 0 and 1 may change
	int fd_stdin = dup(0);
	int fd_stdout = dup(1);


	///////////////////////
	// OPTION PARSING
	///////////////////////
	
	// in_fd and out_fd could change what files they point to
	const int fd0 = 0;
	const int fd1 = 1;
	bool seg = false;
	bool catch_seg = false;

	// Read in options
	int ret;
	do {
		ret = getopt_long(argc, argv, "", long_options, NULL);
		if (ret == INPUT) {
			printf("%s option with arg: %s\n", "input", optarg); // Remove after
			
			// Open input and change fd0 to point to that
			int fd_start = open(optarg, O_RDONLY);
			if (fd_start < 0) {
				perror("Error opening input file");
				_exit(1);
			}
			printf("Opened %s\n", optarg); // Remove after

			// TODO: Extra
			// Set fd0 to the input file, check it's ok
			if (dup2(fd_start, fd0) != 0) {
				perror("Error input file descriptor");
				_exit(5);	// TODO: what code
			}
			
		}
		else if (ret == OUTPUT) {
			printf("%s option with arg: %s\n", "output", optarg); // Remove after
			
			// Open input and change fd0 to point to that
			int fd_start = open(optarg, O_CREAT | O_WRONLY | O_TRUNC, 0666);
			if (fd_start < 0) {
				perror("Error opening output file");
				_exit(1);
			}
			printf("Opened %s\n", optarg); // Remove after

			// TODO: Extra
			// Set fd1 to the output file, check it's ok
			if (dup2(fd_start, fd1) != 1) {
				perror("Error input file descriptor");
				_exit(5);	// TODO: what code
			}
		}
		else if (ret == SEGFAULT) {
			printf("%s option enabled\n", "segfault"); // Remove after
			seg = true;
		}
		else if (ret == CATCH) {
			printf("%s option enabled\n", "catch"); // Remove after
			// Signal Handler
			if (signal(SIGSEGV, seg_handler) == SIG_ERR) {
				perror("EINVAL error. signal didn't work\n");
				_exit(5);	// TODO: what code
			}
			catch_seg = true;
		}
	} while (ret != -1);

	printf("seg: %s, catch_seg: %s\n", (seg ? "true" : "false"), (catch_seg ? "true" : "false")); // Remove after

	// Do not read/write if segfault wants to run
	if (!seg) {


	////////////////////////	
	// READING FROM STDIN
	////////////////////////

	void *buf = malloc(BUFSIZE+1);

	int bytes_read;
	do {
		bytes_read = read(fd0, buf, BUFSIZE);
		if (bytes_read < 0) {
			perror("Error reading file");
			_exit(5);	// TODO: what code?
		}

		////////////////////////
		// WRITING TO STDOUT
		////////////////////////
		int bytes_wrote = write(fd1, buf, bytes_read);
		if (bytes_wrote < 0) {
			perror("Error writing to file");
			_exit(5);	// TODO: what code?
		}
		

	} while (bytes_read > 0);
	free(buf);

	}

	////////////////////////
	// CLOSING UP
	////////////////////////
	
	// Close the files and Set fd0 and fd1 back to stdin and stdout
	int chk1 = dup2(fd_stdin, fd0);		// closes input file and reassigns
	int chk2 = dup2(fd_stdout, fd1);	// closes output file and reassigns
	if (chk1 != 0 || chk2 != 1) {
		fprintf(stderr, "Reassigment to stdin and stdout failed. That's bad.\n");
		_exit(5);	// TODO: what code
	}
	// At this point, no use for the temp ints fd_stdin and fd_stdout
	// which always pointed to stdin and stdout because now fds 0 and 1 do again
	close(fd_stdin);
	close(fd_stdout);

	
	////////////////////////
	// Segfault Processor
	////////////////////////
	if (seg) {
		char *ptr = NULL;
		*ptr = 'c';
	}

	_exit(0); 


}
