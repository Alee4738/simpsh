default: lab0

cc=gcc
flags=-g
files=lab0 lab0-804621520.tar.gz hastext.in notext.in nowrite.in t.out

lab0: main.c
	@$(cc) $(flags) $? -o $@

testinit:
	@head main.c > hastext.in
	@touch notext.in
	@touch nowrite.in; chmod ugo-w nowrite.in
	
check: lab0 testinit
	@# 1. check stdin -> stdout
	@if [ $(shell ./lab0 < hastext.in > t.out; echo $$?) -ne 0 ] ; then echo "error stdin -> stdout" ; fi
	@# 2. check input and output flags work
	@if [ $(shell ./lab0 --input hastext.in --output t.out; echo $$?) -ne 0 ] \
	|| [ $(shell ./lab0 --input=hastext.in > t.out; echo $$?) -ne 0 ] \
    || [ $(shell ./lab0 --output=t.out < hastext.in; echo $$?) -ne 0 ] \
    || [ $(shell ./lab0 --input=hastext.in --output=t.out; echo $$?) -ne 0 ] \
    || [ $(shell ./lab0 --input noexist.txt --output t.out 2> /dev/null; echo $$?) -ne 1 ] ; \
	then echo "error with input/output flags" ; fi
	@# 3. check segfault
	@if [ $(shell { ./lab0 --segfault; } >& t.out; grep --silent Segme t.out; echo $$?) -ne 0 ]; \
	 then echo "segmentation fault doesn't work"; fi
	@# 4. check catch
	@if [ $(shell ./lab0 --input hastext.in --output t.out --segfault --catch 2>/dev/null; echo $$?) -ne 3 ] \
	|| [ $(shell ./lab0 --catch < hastext.in > t.out; echo $$?) -ne 0 ]; then echo "error with catch" ; fi
	@# remove test files
	@rm -f hastext.in notext.in t.out nowrite.in 2>/dev/null

clean: 
	@rm -f $(files) 2>/dev/null || true

dist: main.c Makefile seg.jpg break.jpg README
	@tar -cf lab0-804621520.tar $?
	@gzip lab0-804621520.tar
