# Operating Systems lab1 - simpsh
Andrew Lee

# Introduction
simpsh is a simple shell, inspired to work like bash and dash. It supports commands, redirection, pipes, and file flags.
Basic Usage: Call simpsh, then specify the file flags, then the file opening options, then commands, then misc. options.

Example:
```
./simpsh \
  --rdonly a \
  --pipe \
  --pipe \
  --creat --trunc --wronly c \
  --creat --append --wronly d \
  --command 3 5 6 tr A-Z a-z \
  --command 0 2 6 sort \
  --command 1 4 6 cat b - \
  --wait
```
This corresponds to the following command in bash:
```
(sort < a | cat b - | tr A-Z a-z > c) 2>> d

```

# Description
## File Flags, correspond to oflag value for [open](http://pubs.opengroup.org/onlinepubs/9699919799/functions/open.html)  
* --append 
  - O_APPEND  
* --cloexec
  - O_CLOEXEC  
* --creat
  - O_CREAT  
* --directory
  - O_DIRECTORY  
* --dsync
  - O_DSYNC  
* --excl
  - O_EXCL  
* --nofollow
  - O_NOFOLLOW  
* --nonblock
  - O_NONBLOCK  
* --rsync
  - O_RSYNC  
* --sync
  - O_SYNC  
* --trunc
  - O_TRUNC  

## Opening Options
* --rdonly f  
  - O_RDONLY. Open the file f for reading only.
* --rdwr f  
  - O_RDWR. Open the file f for reading and writing.
* --wronly f  
  - O_WRONLY. Open the file f for writing only.
* --pipe  
  - Open a pipe. Unlike the other file options, this option does not take an argument. Also, it consumes two file numbers, not just one.

## Commands
* --command i o e cmd args
  - Execute a command with standard input i, standard output o and standard error e; these values should correspond to earlier file or pipe options. The executable for the command is cmd and it has zero or more arguments args. None of the cmd and args operands begin with the two characters "--".
* --wait
  - Wait for all commands to finish. As each finishes, output its exit status, and a copy of the command (with spaces separating arguments) to standard output.

## Misc. Options
* --close N
  - Close the Nth file that was opened by a file-opening option. For a pipe, this closes just one end of the pipe. Once file N is closed, it is an error to access it, just as it is an error to access any file number that has never been opened. File numbers are not reused by later file-opening options.
* --verbose
  - Just before executing an option, output a line to standard output containing the option. If the option has operands, list them separated by spaces. Ensure that the line is actually output, and is not merely sitting in a buffer somewhere.
* --profile
  - Just after executing an option, output a line to standard output containing the resources used. Use getrusage and output a line containing as much useful information as you can glean from it.
* --abort
  - Crash the shell. The shell itself should immediately dump core, via a segmentation violation.
* --catch N
  - Catch signal N, where N is a decimal integer, with a handler that outputs the diagnostic N caught to stderr, and exits with status N. This exits the entire shell. N uses the same numbering as your system; for example, on GNU/Linux, a segmentation violation is signal 11.
* --ignore N
  - Ignore signal N.
* --default N
  - Use the default behavior for signal N.
* --pause
  - Pause, waiting for a signal to arrive.



# Report - Testing against existing shells

Three test cases were run on 3 different shells (9 runs total).  
Here are the test cases:  
1. ( cat < a0.txt | sort | grep French > out4) 2>> err4
2. (echo -e "Capitalizing everything...\n\n" < in2 | cat - a0.txt | tr a-z A-Z > out5) 2>> err5
3. grep business < a0.txt 2> err6 | sort 2> err7 | wc > out6 2>> err8 

## Raw Data:
The raw data is located in the file shell_times.pdf

## Analysis of Data
* Bash and dash seemed to be about the same speed.  
* According to my data (shell_times.pdf), simpsh did a lot worse than bash and dash. Looking at the details section, the work that the children processes do for simpsh is comparable to the work done by those from bash and dash, but the work done by the parent process is significantly different. I think it is because once bash and dash spawn their child processes, they go to sleep and wait for a signal, therefore saving resources and system/user time. simpsh, however, does not do this; instead, it simply waits for the child processes to finish, taking up user and system time.

## Conclusions
* Existing shells are quite fast. If I am able to make simpsh sleep and wait for a signal for when the child processes are done, I will be able to reduce the time significantly.

# Limitations
* Does not check --command input arguments
* Does not check rare cases (malloc failing, dup2 failing, etc.)
* Without --wait, there is a race condition between the child and the parent

# Features
* Skips options that have missing operands or whose files cannot be opened
