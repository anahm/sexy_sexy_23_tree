rm a.out
./compile.sh
valgrind --leak-check=full ./a.out
