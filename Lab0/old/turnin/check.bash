#!/bin/bash

# no input, no output, return 1
./lab0 --input=noexist1 --output=noexist2
if [ $? -eq 1 ] then 
	echo "Test 1 passed" 
fi

# yes input, no output, return 0
./lab0 --input=exist1 --output=noexist1

# yes input, yes output, return 0
./lab0 --input=exist1 --output=exist2

# unable to create output file, return 2
./lab0 --input=exist1 --output

# switch options, return 0
./lab0 --output=exist1 --input=exist2

# segfault, no catch, goes to segfault
./lab0 --segfault

# segfault, yes catch, segfault is caught, return 3
./lab0 --segfault --catch
