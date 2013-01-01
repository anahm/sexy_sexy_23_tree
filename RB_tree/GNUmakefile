all:
	rm -f a.out
	rm -f *~
	gcc -std=c99 -ggdb3 -Werror RBT_implementation.c

clean:
	rm -f a.out
	rm -f *~

test:
	rm -f a.out
	rm -f *~
	gcc -std=c99 -ggdb3 -Werror RBT_implementation.c
	./a.out

check:

	rm -f a.out
	rm -f *~
	gcc -std=c99 -ggdb3 -Werror RBT_implementation.c
	./a.out

valgrind:

	rm -f a.out
	rm -f *~
	gcc -std=c99 -ggdb3 -Werror RBT_implementation.c
	valgrind --leak-check=full ./a.out
