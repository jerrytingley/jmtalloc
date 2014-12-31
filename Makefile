all:
	clear
	gcc-4.9 -Wall -Wextra -fdiagnostics-color=auto -g -ggdb -c jmtalloc.c
	gcc-4.9 -Wall -Wextra -fdiagnostics-color=auto -g -ggdb -o jmtalloc_tests jmtalloc.o jmtalloc_tests.c
	./jmtalloc_tests
