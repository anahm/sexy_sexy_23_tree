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
#define RED 0xDEAFBEEF
#define BLACK 0xACEDBEEF

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
  int node_color;
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

// returns 1 if red, 0 if black
int is_red(rb_node *);

// helper functions: DO NOT EXPOSE
static int grand_parent(rb_node *);
static int uncle(rb_node *);
static int free_rb_nodes(rb_node *n);

/******************
 * IMPLEMENTATION *
 ******************/

sexy_rb_tree *create_rb(int (*comp)(my_type *, my_type *)) {
  sexy_rb_tree *ret = (sexy_rb_tree *) malloc(sizeof(sexy_rb_tree));
  if (ret == NULL)
    return NULL;
  
  ret->root = NULL;
  ret->num_nodes = 0;
  ret->comp = comp;
}

static int free_rb_nodes(rb_node *n) {
  free(n->data);
  if (n->left != NULL)
    free_rb_nodes(n->left);
  if (n->right != NULL)
    free_rb_nodes(n->right);
  free(n);
}

int free_rb(sexy_rb_tree *t) {
  return free_rb_nodes(t->root);
}

int is_red(rb_node *n) {
  if (n->node_color == RED)
    return 1;
  else if (n->node_color == BLACK)
    return 0;
  else
    assert(0);
}


/***************
 * TEST SCRIPT *
 ***************/

// currently does nothing
int main(void) {
  return 1;
}
