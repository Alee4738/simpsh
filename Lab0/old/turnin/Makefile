# Makefile for CS 111 Project 0: Warm-up
# Ensure student is prepared for CS 111
# Also introduces basic POSIX file operations, processing command line arguments, catching and handling run-time exceptions, and returning informative exit status.

CC=gcc
OUTPUT=lab0
CFLAGS=-g
SHELL := /bin/bash

default: main.c
	@$(CC) $(CFLAGS) $< -o $(OUTPUT) 

clean:
	@rm -f ./lab0 ./lab0-804621520.tar.gz exist1 exist2

dist:
	tar -cvzf lab0-804621520.tar.gz main.c Makefile README seg.JPG break.JPG 

check: 
	@echo I existed first > exist1; echo I existed second > exist2
	@-./$(OUTPUT) --input=noexist1 --output=noexist2 > /dev/null 2> /dev/null; if [ $? -eq 1 ] echo "Test 1 passed" fi 
	@echo "make check passed"

