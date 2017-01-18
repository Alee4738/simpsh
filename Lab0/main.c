// CS 111 lab0 - print standard input to standard output
// Usage: -./lab0 [-input <inputfile>] [--output <outputfile>] [--segfault] [--catch]
//		segfault results in following a null ptr
// 		catch initializes a segfault handler
// Return codes:
// 		0 - Success
// 		1 - error opening input file
// 		2 - error creating output file
// 		3 - caught a segfault
// 		4 - setting input/output file to stdin/stdout (dup2) failed
// 		5 - read error
// 		6 - write error
// 		7 - memory allocation error
// 		8 - option parsing error

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
#define BUF_SIZE 8

struct option long_options[] =
{
	{"input", required_argument, NULL, INPUT},
	{"output", required_argument, NULL, OUTPUT},
	{"segfault", no_argument, NULL, SEGFAULT},
	{"catch", no_argument, NULL, CATCH},
	{0,0,0,0}
};

void sig_handler(int signum)
{
	perror("Caught segfault (SIGSEGV)");
	exit(3);
}

void print_usage()
{
	perror("Usage: ./lab0 [--input <filename>] [--output <filename>] [--segfault] [--catch]");
}

int main(int argc, char* const argv[])
{
	// Parse arguments
	char* input = NULL; // if stays null, use regular stdin
	char* output = NULL; // if stays null, use regular stdout
	int hasSegfault = 0;
	int hasCatch = 0;
	int ret = getopt_long(argc, argv, "", long_options, NULL);
	while (ret != -1)
	{
		switch(ret)
		{
			case INPUT:
				if (input != NULL) { print_usage(); exit(8); } // Note(2)
				input = optarg; // Note(1) 
				break;
			case OUTPUT:
				if (output != NULL) { print_usage(); exit(8); } // Note(2)
				output = optarg; // Note(1)
				break;
			case SEGFAULT:
				hasSegfault = 1;
				break;
			case CATCH:
				hasCatch = 1;
				break;
			default:
				print_usage();
				exit(8);
		}
		// Notes
		// Note(1): optarg is an external variable that is set to point to the data in argv, which is char const; thereore, we don't need to malloc or copy the string to input or output, we just need to save the addresses
		// Note(2): make sure input/output option is not specified mlultiple times

		// Get next argument
		ret = getopt_long(argc, argv, "", long_options, NULL);
	}

	// Catch and segfault if flags were given
	if (hasCatch == 1)
	{
		signal(SIGSEGV, sig_handler);
	}
	if (hasSegfault == 1)
	{
		char* ptr = NULL;
		*ptr = SEGFAULT; 
	}
	
	// Change standard input and output if needed
	int fd_keyboard = dup(0); // save default stdin
	int fd_screen = dup(1); // save default stdout
	if (input != NULL)
	{
		int fd0 = open(input, O_RDONLY);
		// check open for errors
		if (fd0 == -1) // open failed
		{ 
			perror(NULL);
			fprintf(stderr, "Opening file %s failed", input);
			exit(1);
		}

		if (dup2(fd0, 0) == -1) // stdin is now the input file
		{
			perror(NULL);
			fprintf(stderr, "Setting file %s to stdin failed", input);
			exit(4); // Self-added
		}
		
		close(fd0); // close extraneous fd
	}
	if (output != NULL)
	{
		int fd1 = creat(output, 0644);
		if (fd1 == -1) // if open failed
		{
			perror(NULL);
			fprintf(stderr, "Creating file %s failed\n", output);
			exit(2);
		}

		if (dup2(fd1, 1) == -1) // stdout is now the output file
		{
			perror(NULL);
			fprintf(stderr, "Setting file %s to stdout failed\n", output);
			exit(4); // Self-added
		}
		close(fd1); // close extraneous fd
	}


	// Read from 0 (stdin), write to 1 (stdout)
	int ret_code = 0;
	char* buf = malloc(BUF_SIZE);
	if (buf == NULL) // malloc failed
	{
		perror("Memory allocation failed");
		exit(7);
	}
	int num_read, num_wrote;
	do {
		num_read = read(0, buf, BUF_SIZE);
		if (num_read < 1) // Didn't read anything
		{
			if (num_read < 0)
			{
				perror("Read error");
				ret_code = 5;
				break;
			}
			else { break; }	// End of file reached
		}
		
		num_wrote = write(1, buf, num_read);
		if (num_wrote < 0)
		{
			perror("Write error");
			ret_code = 6;
			break;
		}
	} while (num_read != 0);
	
	free(buf);

	// Reassign stdin and stdout to keyboard and screen
	dup2(fd_keyboard, 0);
	close(fd_keyboard);
	dup2(fd_screen, 1);
	close(fd_screen);
	
	exit(ret_code);
}
