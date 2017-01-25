# variables
DELIVERABLES=main.c Makefile README test.sh
CC=gcc
FLAGS=-g

# build the simpsh program
default: simpsh

simpsh: main.c
	@$(CC) $(FLAGS) $? -o $@

# removes the program and all other temporary files and object files that can be regenerated with 'make'
clean: 
	@rm -f simpsh lab1-AndrewLee.* 2>/dev/null || true

# tests the simpsh program on test cases that you design. You should have at least three test cases
check: 
	@chmod ugo+x ./test.sh; ./test.sh

# makes a software distribution compressed tarball lab1-yourname.tar.gz and does some simple testing on it
dist: 
	@tar --transform 's,^,lab1-AndrewLee/,g' -cf lab1-AndrewLee.tar $(DELIVERABLES) ; \
	gzip lab1-AndrewLee.tar
