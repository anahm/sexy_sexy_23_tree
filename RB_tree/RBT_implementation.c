/***********************************************
 * Implementation of a red-black tree          *
 * Neil Sant Gat                               *
 * Begun Sunday, December 30th, 2012           *
 * thanks to Wikipedia for a great             *
 * description of RB tree semantics            *
 * http://en.wikipedia.org/wiki/Red_Black_tree *
 ***********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define LEAF 0xDEADBEEF
#define BRANCH 0xFEEDBEEF

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
typedef struct rb_node {
  my_type *data;
  int node_type;
  struct rb_node *parent;
  struct rb_node *left;
  struct rb_node *right;
} rb_node;

typedef struct sexy_rb_tree {
  rb_node *root;
  int num_nodes;

  // returns -1 iff "a < b", 0 iff "a == b", 1 iff "a > b"
  int (*comp)(my_type *, my_type *);
} sexy_rb_tree;

// usual "create, insert, remove, search, and free" functions
// first 4 rely on the comparison function passed into create_rb()
sexy_rb_tree *create_rb(int (*)(my_type *, my_type *));
int insert_baby(my_type *, sexy_rb_tree *);
int remove_baby(my_type *, sexy_rb_tree *);
int search_baby(my_type *, sexy_rb_tree *);
int free_rb(sexy_rb_tree *);

// helper functions: DO NOT EXPOSE

/***************
 * TEST SCRIPT *
 ***************/

// currently does nothing
int main(void) {
  return 1;
}
