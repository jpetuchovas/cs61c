beargit: main.c beargit.c beargit.h util.h
	gcc -ggdb -std=c99 main.c beargit.c -o beargit

clean:
	rm -rf beargit autotest test

check::
	python2.7 tester.pyc beargit.c
