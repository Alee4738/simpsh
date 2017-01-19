# variables
DELIVERABLES=main.c Makefile
CC=gcc
FLAGS=-g

# build the simpsh program
default: simpsh

simpsh: main.c
	$(CC) $(FLAGS) $? -o $@

# removes the program and all other temporary files and object files that can be regenerated with 'make'
clean: 
	rm simpsh lab1-AndrewLee.*
# tests the simpsh program on test cases that you design. You should have at least three test cases
check: 

# makes a software distribution compressed tarball lab1-yourname.tar.gz and does some simple testing on it
dist: 
	tar --transform 's,^,lab1-AndrewLee/,g' -cf lab1-AndrewLee.tar $(DELIVERABLES) ; \
	gzip lab1-AndrewLee.tar
