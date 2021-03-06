#!/bin/sh
# test.sh - tests Simpleton Shell (simpsh)

# Initialization: make some files
files="in1 in2 in3 out1 out2 out3 err1 err2 err3"
rm -f $files; touch $files
for i in $files
do
	chmod ugo+rw $i
done
echo "blah bloo bloo blee\n" > in1
echo "Words and more\nwords balhkfdsf\n" > in2
echo "fsldjf\0 ls kdfj2\2 lsdkjf3\n \tsdlkf" > in3
testsPassed=0
testNum=0

# create simpsh if not already
if [ ! -e simpsh ]; then make; fi

# Lab 1A Test Cases
echo "LAB 1A TEST CASES"
echo "-------------------------------" 
# Test Case: standard rdonly, wronly, command
(( testnum++ ))
echo "---Test case $testNum: rdonly, wronly, command (cat -> diff), should return 0"
./simpsh --rdonly in1 --wronly out1 --wronly err1 --command 0 1 2 cat
sleep 0.25
diff in1 out1
if [ $? -ne 0 ]; then \
	echo "---Test case $testNum failed"
else
    echo "---Test case $testNum passed"
	(( passed++ ))
fi

# Test Case: Unreadable file, should say something like "Skipping"
(( testnum++ ))
echo ""
echo "---Test case $testnum: Unreadable file, should say \"Skipping\""
touch in2; chmod ugo-r in2
./simpsh --rdonly in2 
ret=$?
tmp=$(mktemp)
echo -e $(./simpsh --rdonly in2) > $tmp; grep Skipping $tmp
if [ $? -ne 1 ] || [ $ret -ne 1 ]; then \
    echo "---Test case $testnum failed"
else
    echo "---Test case $testnum passed"
	(( passed++ ))
fi

# Test Case: print verbose options only AFTER it is read
(( testnum++ ))
echo ""
echo "---Test case $testnum: print verbose options only after verbose is read"
./simpsh --rdonly in1 --wronly out1 --verbose --rdonly in3 --wronly out3 --wronly err3 --command 2 3 4 cat > out2
sleep 0.25
diff in3 out3; same=$?
grep 'rdonly in1' out2; noprint=$?
grep 'rdonly in3' out2; print=$?

if [ $same -ne 0 ] \
|| [ $noprint -eq 0 ] \
|| [ $print != 0 ]; then \
    echo "---Test case $testnum failed"
else
    echo "---Test case $testnum passed"
	(( passed++ ))
fi

echo -e "-------------------------------\n\n\n" 


echo "LAB 1B TEST CASES"
echo "-------------------------------" 
# Test Case: O_* file flags work, test creat and append
(( testnum++ ))
echo "---Test case $testnum: O_* file flags work, test creat and append"
rm out1; 
./simpsh --rdonly in1 --creat --wronly out1 --wronly err1 --command 0 1 2 cat
chmod ugo+rw out1
sleep 0.25
diff in1 out1 > /dev/null; ret=$?
./simpsh --rdonly in1 --append --wronly out1 --wronly err1 --command 0 1 2 cat
sleep 0.25
tmp=$(mktemp)
cat in1 in1 > $tmp; diff $tmp out1
if [ $? -ne 0 ] \
|| [ $ret -ne 0 ]; then \
	echo "---Test case $testnum failed"
else
	echo "---Test case $testnum passed"
	(( passed++ ))
fi 

# Test Case: --rdwr works
(( testnum++ ))
echo ""
echo "---Test case $testnum: --rdwr works"
chmod ugo+rw in2;
./simpsh --rdwr in2 --trunc --rdwr out2 --rdwr err2 --command 0 1 2 cat
sleep 0.25
diff in2 out2
if [ $? -ne 0 ]; then \
	echo "---Test case $testnum failed"
else
	echo "---Test case $testnum passed"
	(( passed++ ))
fi

# Test Case: --close N works
(( testnum++ ))
echo ""
echo "---Test case $testnum: --close N works"
rm out1 err1 out2 err2; touch out1 err1 out2 err2
./simpsh --rdonly in1 --wronly out1 --wronly err1 --rdwr in2 --rdwr out2 --close 5 --rdwr err2 --close 5 --close 5 --close 10 --close -1 --command 0 1 2 cat --command 3 4 5 cat
sleep 0.25
diff in1 out1; ret=$?
diff in2 out2 > /dev/null
if [ $? -ne 1 ] \
|| [ $ret -ne 0 ]; then \
	echo "---Test case $testnum failed"
else
	echo "---Test case $testnum passed"
	(( passed++ ))
fi

# Test Case: --abort works
(( testnum++ ))
echo ""
echo "---Test case $testnum: --abort works"
ret=$(mktemp)
echo -e "$(./simpsh --rdonly in1 --abort --wronly out1)" > $ret
grep egmentation $ret
if [ $? -ne 1 ]; then \
	echo "---Test case $testnum failed"
else
	echo "---Test case $testnum passed"
	(( passed++ ))
fi

# Test Case: --catch N works
(( testnum++ ))
echo ""
echo "---Test case $testnum: --catch N works"
tmp=$(mktemp)
./simpsh --rdonly in2 --catch 11 --abort
ret=$?
if [ $ret -ne 11 ]; then
	echo "---Test case $testnum failed"
else
	echo "---Test case $testnum passed"
	(( passed++ ))
fi

# Test Case: --ignore N works
(( testnum++ ))
echo ""
echo "---Test case $testnum: --ignore N works"
tmp=$(mktemp)
echo -e "$(./simpsh --rdonly in1 --catch 11 --ignore 11 --abort --wronly out1)" > $tmp
grep egmentation $tmp
if [ $? -ne 1 ]; then \
	echo "---Test case $testnum failed"
else
	echo "---Test case $testnum passed"
	(( passed++ ))
fi

# Test Case: --default N works
(( testnum++ ))
echo ""
echo "---Test case $testnum: --default N works"
tmp=$(mktemp)
echo -e "$(./simpsh --rdonly in1 --catch 11 --default 11 --abort --wronly out1)" > $tmp
grep egmentation $tmp
if [ $? -ne 1 ]; then \
	echo "---Test case $testnum failed"
else
	echo "---Test case $testnum passed"
	(( passed++ ))
fi

# Test Case: Spec's example shell script works, outputs exit statuses
(( testnum++ ))
echo ""
echo "---Test case $testnum: Spec's example script works, outputs exit statuses"
tmp=tmp; t1=t1; t2=t2; t3=t3
echo "0 sort" > $t1; echo "0 cat err2 - " > $t2; echo "0 tr A-Z a-z" > $t3
echo -e "$(./simpsh --rdonly in1 --pipe --pipe --creat --trunc --wronly out1 --creat --append --wronly err1 --command 3 5 6 tr A-Z a-z --command 0 2 6 sort --command 1 4 6 cat err2 - --wait)" > $tmp
grep $t1 $tmp; r1=$?
grep $t2 $tmp; r2=$?
grep $t3 $tmp; r3=$?
if [ $r1 -ne 1 ] \
|| [ $r2 -ne 1 ] \
|| [ $r3 -ne 1 ]; then \
	echo "---Test case $testnum failed"
else
	echo "---Test case $testnum passed"
	(( passed++ ))
fi
rm $tmp $t1 $t2 $t3

echo -e "-------------------------------\n\n\n" 


echo "Passed $passed tests out of $testnum"


echo "-------------------------------" 
echo "LAB 1C TEST CASES (--profile) - Tested against bash and dash"
echo "-------------------------------" 
# Reminder: needs to be nontrivial
# Initialization: make sure files are accessible
files="$files err4 err5 err6 err7 err8 out4 out5 out6"
for i in $files
do
	touch $i; chmod ugo+rw $i
done

# this uses a0.txt. If you don't have it, get it
if [ ! -e a0.txt ]; then \
	wget http://web.cs.ucla.edu/~zbu/a0.txt
fi
chmod ugo+r a0.txt

# Test Case 1: ( cat < a0.txt | sort | grep French > out4) 2>> err4
echo "---Test case $testnum: non-trivial test 1"
./simpsh --profile --rdonly a0.txt --pipe --pipe --wronly out4 --wronly err4 --command 0 2 6 cat --command 1 4 6 sort --command 3 5 6 grep French --wait
rm out4; touch out4; chmod ugo+rw out4

# Test Case 2: (echo -e "Capitalizing everything...\n\n" < in2 | cat - a0.txt | tr a-z A-Z > out5) 2>> err5
echo ""
echo "---Test case $testnum: non-trivial test 2"
./simpsh --profile --rdonly a0.txt --pipe --pipe --creat --trunc --wronly out5 --creat --append --wronly err5 --command 0 2 6 echo -e "Capitalizing everything...\n\n" --command 1 4 6 cat - a0.txt --command 3 5 6 tr a-z A-Z --wait
rm out5; touch out5; chmod ugo+rw out5

# Test Case 3: grep business < a0.txt 2> err6 | sort 2> err7 | wc > out6 2>> err8
echo ""
echo "---Test case $testnum: non-trivial test 3"
./simpsh --profile --rdonly a0.txt --creat --trunc --wronly err6 --pipe --creat --trunc --wronly err7 --pipe --creat --trunc --wronly out6 --creat --append --wronly err8 --command 0 3 1 grep business --command 2 6 4 sort --command 5 7 8 wc --wait

rm -f $files


