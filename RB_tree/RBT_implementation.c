/**************************************
 * Implementation of a red-black tree *
 * Neil Sant Gat                      *
 * Begun Sunday, December 30th, 2012  *
 **************************************/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/****************
 * USER-DEFINED *
 ****************/

// struct to be put in the tree
typedef struct my_type {
  int x;
} my_type;

// comparison function over my_types
// returns -1 iff "a < b", 0 iff "a == b", 1 iff "a > b"
int int_compare(my_type *a, my_type *b) {
  if (a->x < b->x)
    return -1;
  else if (a->x > b->x)
    return 1;
  else
    return 0;
}

/*************
 * INTERFACE *
 *************/

typedef struct sexy_rb_tree {
  my_type *root;
  int num_nodes;

  // returns -1 iff "a < b", 0 iff "a == b", 1 iff "a > b"
  int (*comp)(my_type *, my_type *);
} sexy_rb_tree;

// usual "insert, remove, and search" functions
// all 3 rely on the passed in comparison function
int insert_baby(my_type *, sexy_rb_tree *);
int remove_baby(my_type *, sexy_rb_tree *);
int search_baby(my_type *, sexy_rb_tree *);

/***************
 * TEST SCRIPT *
 ***************/

// currently does nothing
int main(void) {
  return 1;
}
