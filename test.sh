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

# create simpsh if not already
if [ ! -e simpsh ]; then make; fi

# Lab 1A Test Cases
echo "LAB 1A TEST CASES"
echo "-------------------------------" 
# Test Case: standard rdonly, wronly, command
echo "---Test case 1: rdonly, wronly, command (cat -> diff), should return 0"
./simpsh --rdonly in1 --wronly out1 --wronly err1 --command 0 1 2 cat
sleep 0.25
diff in1 out1
if [ $? -ne 0 ]; then \
	echo "---Test case 1 failed"
else
    echo "---Test case 1 passed"
fi

# Test Case: Unreadable file, should say something like "Skipping"
echo ""
echo "---Test case 2: Unreadable file, should say \"Skipping\""
chmod ugo-r in2
./simpsh --rdonly in2 
ret=$?
sleep 0.25
grep Skip err2
if [ $? -ne 0 ] || [ $ret -ne 1 ]; then \
    echo "---Test case 2 failed"
else
    echo "---Test case 2 passed"
fi

# Test Case: print verbose options only AFTER it is read
echo ""
echo "---Test case 3: print verbose options only after verbose is read"
./simpsh --rdonly in1 --wronly out1 --verbose --rdonly in3 --wronly out3 --wronly err3 --command 2 3 4 cat > out2
sleep 0.25
diff in3 out3; same=$?
grep 'rdonly in1' out2; noprint=$?
grep 'rdonly in3' out2; print=$?

if [ $same -ne 0 ] \
|| [ $noprint -eq 0 ] \
|| [ $print != 0 ]; then \
    echo "---Test case 3 failed"
else
    echo "---Test case 3 passed"
fi

echo -e "-------------------------------\n\n\n" 


echo "LAB 1B TEST CASES"
echo "-------------------------------" 
# Test Case: O_* file flags work, test creat and append
echo "---Test case 1: O_* file flags work, test creat and append"
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
	echo "---Test 1 failed"
else
	echo "---Test 1 passed"
fi 

# Test Case: --rdwr works
echo ""
echo "---Test case 2: --rdwr works"
chmod ugo+rw in2;
# rm in2; touch in2; chmod ugo+rw in2;
./simpsh --rdwr in2 --trunc --rdwr out2 --rdwr err2 --command 0 1 2 cat
sleep 0.25
diff in2 out2
if [ $? -ne 0 ]; then \
	echo "---Test 2 failed"
else
	echo "---Test 2 passed"
fi

# TODO Test Case: --pipe works


# TODO Test Case: --close N works
echo ""
echo "---Test case 3: --close N works"
rm out1 err1 out2 err2; touch out1 err1 out2 err2
./simpsh --rdonly in1 --wronly out1 --wronly err1 --rdwr in2 --rdwr out2 --close 5 --rdwr err2 --close 5 --close 5 --close 10 --close -1 --command 0 1 2 cat --command 3 4 5 cat
diff in1 out1; ret=$?
diff in2 out2 > /dev/null
if [ $? -ne 1 ] \
|| [ $ret -ne 0 ]; then \
	echo "---Test case 3 failed"
else
	echo "---Test case 3 passed"
fi

# TODO Test Case: --wait works


# TODO Test Case: --abort works


# TODO Test Case: --catch N works


# TODO Test Case: --ignore N works


# TODO Test Case: --default N works


# TODO Test Case: --pause works


# TODO Test Case: Spec's example shell script works, outputs exit statuses


echo -e "-------------------------------\n\n\n" 

# Test Case:

# Test Case:

# Test Case:

# Test Case:

# Test Case:

# Test Case:




# rm -f $files

