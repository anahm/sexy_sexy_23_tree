#include <stdio.h>
#include <stdlib.h>

typedef struct my_type {
  int x;
} my_type;

typedef struct vectore {
  my_type *storage;
  int capacity;
  int last_used_index;
} vectore;

// instantiate and initialize a new vectore
// returns pointer to new vectore on success, NULL on failure
vectore *new_vectore(void);

// add a my_type to a vectore at a specific location
// returns 1 on success, 0 on failure
int add_to_vectore(my_type, vectore, int);

// resizes the vectore (increases capacity by factor of 2)
// returns 1 on success, 0 on failure
int resize_vectore(vectore);

// returns a pointer to the my_type stored at a certain index
// does not remove the my_type
my_type *get_from_vectore(vectore, int);

// overwrites a location in the vectore with NULL
// returns 1 on success
int clean_index(vectore, int);

int main(void) {
  my_type x;
  vectore y;
  (void) x;
  (void) y;
}
