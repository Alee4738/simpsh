#!/bin/sh

# Initialization: make some files, 
files="in1 in2 in3 out1 out2 out3 err1 err2 err3"
echo "blah bloo bloo blee\n" > in1
echo "Words and more\nwords balhkfdsf\n" > in2
echo "fsldjf\0 ls kdfj2\2 lsdkjf3\n \tsdlkf" > in3
touch out1
touch out2
touch out3
touch err1
touch err2
touch err3

if [ -e simpsh ]; then
       continue;
else
       make
fi

# Test Case 1: standard rdonly, wronly, command
echo "---Test case 1:"
./simpsh --rdonly in1 --wronly out1 --wronly err1 --command 0 1 2 cat
diff in1 out1
if [ $? -ne 0 ]; then \
       echo "---Test case 1 failed"
else
       echo "---Test case 1 passed"
fi

# Test Case 2: Unreadable file
echo ""
echo "---Test case 2:"
chmod ugo-r in2
./simpsh --rdonly in2 2> err2
ret=$?
grep Skip err2
if [ $? -ne 0 ] || [ $ret -ne 1 ]; then \
       echo "---Test case 2 failed"
else
       echo "---Test case 2 passed"
fi

# Test Case 3: print verbose options only AFTER it is rea
echo ""
echo "---Test case 3:"
./simpsh --rdonly in1 --wronly out1 --verbose --rdonly in3 --wronly out3 --wronly err3 --command 2 3 4 cat > out2
diff in3 out3; same=$?
grep 'rdonly in1' out2; noprint=$?
grep 'rdonly in3' out2; print=$?

if [ $same -ne 0 ] \
|| [ $noprint -eq 0 ] \
|| [ $print != 0 ]; then \
       echo "--Test case 3 failed"
else
       echo "---Test case 3 passed"
fi


rm -f $files

