all:
	gcc -Wall -Wextra -Wformat -g -ggdb -c jmtalloc.c
	gcc -Wall -Wextra -Wformat -g -ggdb -o jmtalloc_tests jmtalloc.o jmtalloc_tests.c

	./jmtalloc_tests
