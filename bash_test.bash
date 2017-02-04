#!/bin/bash
if [ $# -ne 2 ]; then \
	echo "Usage: ./bash_test.bash <test_num> <num_runs>"
	exit
fi

testnum=$1
numruns=$2

if [ $testnum -eq 1 ]; then \
	cmd="( cat < a0.txt | sort | grep French > out4) 2>> err4"
elif [ $testnum -eq 2 ]; then \
	cmd="( echo -e \"Capitalizing everything...\n\n\" < in2 | cat - a0.txt | tr a-z A-Z > out5) 2>> err5"
elif [ $testnum -eq 3 ]; then \
	cmd="grep business < a0.txt 2> err6 | sort 2> err7 | wc > out6 2>> err8"
else
	echo "Invalid test number"; exit
fi

echo "--Test case $testnum: $cmd"
echo "---------------"
for (( i=0; i<$numruns; i++ ))
do
	(eval $cmd; times)
	echo "---------------" 
done
