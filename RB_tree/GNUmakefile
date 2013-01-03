all:
	@rm -f a.out
	@rm -f *~
	@rm -f *perf*
	@gcc -std=c99 -ggdb3 -Werror RBT_implementation.c

clean:
	@rm -f a.out
	@rm -f *~
	@rm -f *perf*

test:
	@rm -f a.out
	@rm -f *~
	@rm -f *perf*
	@gcc -std=c99 -ggdb3 -Werror RBT_implementation.c
	@./a.out

check:

	@rm -f a.out
	@rm -f *~
	@rm -f *perf*
	@gcc -std=c99 -ggdb3 -Werror RBT_implementation.c
	@./a.out

valgrind:

	@rm -f a.out
	@rm -f *~
	@rm -f *perf*
	@gcc -std=c99 -ggdb3 -Werror RBT_implementation.c
	@valgrind --leak-check=full --show-reachable=yes ./a.out
