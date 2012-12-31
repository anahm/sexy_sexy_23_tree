/***************************************************
 * Implementation of a red-black tree              *
 * Neil Sant Gat                                   *
 * Begun Sunday, December 30th, 2012               *
 * thanks to Wikipedia for a great                 *
 * description of RB tree semantics and operations *
 * http://en.wikipedia.org/wiki/Red_Black_tree     *
 ***************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define LEAF 0xDEADBEEF
#define BRANCH 0xFEEDBEEF
#define RED 0xDEAFBEEF
#define BLACK 0xACEDBEEF
#define LESS -1
#define EQUAL 0
#define GREATER 1

/****************
 * USER-DEFINED *
 ****************/

// struct to be put in the tree
typedef struct my_type {
  int x;
} my_type;

// comparison function over my_types
// returns LESS iff "a < b", 0 iff "a == b", 1 iff "a > b"
int int_compare(my_type *a, my_type *b) {
  if (a->x < b->x)
    return LESS;
  else if (a->x > b->x)
    return GREATER;
  else
    return EQUAL;
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

  // returns LESS iff "a < b", EQUAL iff "a == b", GREATER iff "a > b"
  int (*comp)(my_type *, my_type *);
} sexy_rb_tree;

// usual "create, insert, remove, search, and free" functions
// first 4 rely on the comparison function passed into create_rb()
sexy_rb_tree *create_rb(int (*)(my_type *, my_type *));
int insert_baby(my_type *, sexy_rb_tree *);
int remove_baby(my_type *, sexy_rb_tree *);
int search_baby(my_type *, sexy_rb_tree *);
void free_rb(sexy_rb_tree *);

// returns 1 if red, 0 if black
int is_red(rb_node *);

// helper functions: DO NOT EXPOSE
static rb_node *grand_parent(rb_node *);
static rb_node *uncle(rb_node *);
static void free_rb_nodes(rb_node *);

// returns 1 on success
static int insert_root(rb_node *, sexy_rb_tree *);

// insert node without fixing tree
// returns the node after insertion into the tree
static rb_node *node_insert_node(rb_node *, rb_node *, int (*)(my_type *, my_type *));

static rb_node *binary_insert_node(rb_node *, sexy_rb_tree *, int (*)(my_type *, my_type *));

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

static void free_rb_nodes(rb_node *n) {
  free(n->data);
  if (n->left != NULL)
    free_rb_nodes(n->left);
  if (n->right != NULL)
    free_rb_nodes(n->right);
  free(n);
}

void free_rb(sexy_rb_tree *t) {
  free_rb_nodes(t->root);
  free(t);
}

int is_red(rb_node *n) {
  if (n->node_color == RED)
    return 1;
  else if (n->node_color == BLACK)
    return 0;
  else
    assert(0);
}

static rb_node *grand_parent(rb_node *n) {
  if (n != NULL && n->parent != NULL)
    return n->parent->parent;
  else
    return NULL;
}

static rb_node *uncle(rb_node *n) {
  rb_node *g = grand_parent(n);
  if (g == NULL) {
    // no grand parent, so no uncle
    return NULL;
  } else {
    // return the child of g that isn't the parent of n
    if (g->left == n->parent)
      return g->right;
    else
      return g->left;
  }
}

// PREVIOUS CODE FOR node_insert_node()
  /*rb_node *cur = t->root;
  
  // tree is empty
  if (cur == NULL) {
    cur->node_type = BRANCH;
    cur->node_color = BLACK;
    cur->parent = NULL;
    cur->left = NULL;
    cur->right = NULL;
    t->root = cur;
    t->num_nodes++;
  }
  
  // traverse to proper location in tree using
  // passed in comparator function
  if (
  */

// assumes both inserting and cur are not NULL and that
// inserting has already been colored red
static rb_node *node_insert_node(rb_node *inserting, rb_node *cur, int (*comp)(my_type *, my_type *)) {
  rb_node *next = NULL;

  if (comp(inserting->data, cur->data) == LESS) {
    // go left
    next = cur->left;

    if (next == NULL) {
      // base case
      inserting->left = NULL;
      inserting->right = NULL;
      inserting->parent = cur;
      inserting->node_type = BRANCH;
      cur->left = inserting;
      return inserting;
    } else {
      // recursive case
      return node_insert_node(inserting, next, comp);
    }

  } else if (comp(inserting->data, cur->data) == EQUAL) {
    // exit because DON'T ALLOW DUPLICATES
    assert(0);
  } else if (comp(inserting->data, cur->data) == GREATER) {
    // go right
    next = cur->right;

    if (next == NULL) {
      // base case
      inserting->left = NULL;
      inserting->right = NULL;
      inserting->parent = cur;
      inserting->node_type = BRANCH;
      cur->right = inserting;
      return inserting;
    } else {
      // recursive case
      return node_insert_node(inserting, next, comp);
    }

  } else {
      // something wrong with constants
      assert(0);

  }

}

static rb_node *binary_insert_node(rb_node *n, sexy_rb_tree *t, int (*comp)(my_type *, my_type *)) {

  if (t->root == NULL) {
    n->node_type = BRANCH;
    n->node_color = BLACK;
    n->parent = NULL;
    n->left = NULL;
    n->right = NULL;
    t->root = n;
  } else {
    n->node_color = RED;
    node_insert_node(n, t->root, comp);
  }

}

// returns 1 on success
int insert_root(rb_node *n, sexy_rb_tree *t) {
  
}

/***************
 * TEST SCRIPT *
 ***************/

// BREAKS ABSTRACTION BARRIER
// primarily tests binary_insert_node
void test_1(void) {
  // a, b, c ordering
  sexy_rb_tree *t = create_rb(&int_compare);
  
  my_type *x = (my_type *) malloc(sizeof(my_type));
  my_type *y = (my_type *) malloc(sizeof(my_type));
  my_type *z = (my_type *) malloc(sizeof(my_type));

  x->x = 1;
  y->x = 2;
  z->x = 3;

  rb_node *a = (rb_node *) malloc(sizeof(rb_node));
  rb_node *b = (rb_node *) malloc(sizeof(rb_node));
  rb_node *c = (rb_node *) malloc(sizeof(rb_node));

  a->data = x;
  b->data = y;
  c->data = z;

  // assert basic inserts don't fail
  assert(binary_insert_node(a, t, int_compare) != NULL);
  assert(binary_insert_node(b, t, int_compare) != NULL);
  assert(binary_insert_node(c, t, int_compare) != NULL);

  // assert structure is as it should be
  assert(t->root == a);
  assert(t->root->left == NULL);
  assert(t->root->right == b);
  assert(t->root->right->right == c);
  assert(t->root->right->left == NULL);
  assert(t->root->right->right->left == NULL);
  assert(t->root->right->right->right == NULL);

  free_rb(t);

}

void test_2(void) {
  // a, c, b ordering
  sexy_rb_tree *t = create_rb(&int_compare);
  
  my_type *x = (my_type *) malloc(sizeof(my_type));
  my_type *y = (my_type *) malloc(sizeof(my_type));
  my_type *z = (my_type *) malloc(sizeof(my_type));

  x->x = 1;
  y->x = 2;
  z->x = 3;

  rb_node *a = (rb_node *) malloc(sizeof(rb_node));
  rb_node *b = (rb_node *) malloc(sizeof(rb_node));
  rb_node *c = (rb_node *) malloc(sizeof(rb_node));

  a->data = x;
  b->data = y;
  c->data = z;

  // assert basic inserts don't fail
  assert(binary_insert_node(a, t, int_compare) != NULL);
  assert(binary_insert_node(c, t, int_compare) != NULL);
  assert(binary_insert_node(b, t, int_compare) != NULL);

  // assert structure is as it should be
  assert(t->root == a);
  assert(t->root->left == NULL);
  assert(t->root->right == c);
  assert(t->root->right->right == NULL);
  assert(t->root->right->left == b);
  assert(t->root->right->left->left == NULL);
  assert(t->root->right->left->right == NULL);

  free_rb(t);
}

void test_3(void) {
  // b, a, c ordering
  sexy_rb_tree *t = create_rb(&int_compare);
  
  my_type *x = (my_type *) malloc(sizeof(my_type));
  my_type *y = (my_type *) malloc(sizeof(my_type));
  my_type *z = (my_type *) malloc(sizeof(my_type));

  x->x = 1;
  y->x = 2;
  z->x = 3;

  rb_node *a = (rb_node *) malloc(sizeof(rb_node));
  rb_node *b = (rb_node *) malloc(sizeof(rb_node));
  rb_node *c = (rb_node *) malloc(sizeof(rb_node));

  a->data = x;
  b->data = y;
  c->data = z;

  assert(binary_insert_node(b, t, int_compare) != NULL);
  assert(binary_insert_node(a, t, int_compare) != NULL);
  assert(binary_insert_node(c, t, int_compare) != NULL);

  assert(t->root == b);
  assert(t->root->left == a);
  assert(t->root->right == c);
  assert(t->root->left->left == NULL);
  assert(t->root->left->right == NULL);
  assert(t->root->right->left == NULL);
  assert(t->root->right->right == NULL);

  free_rb(t);

}

void test_4(void) {
  // b, c, a ordering
  sexy_rb_tree *t = create_rb(&int_compare);
  
  my_type *x = (my_type *) malloc(sizeof(my_type));
  my_type *y = (my_type *) malloc(sizeof(my_type));
  my_type *z = (my_type *) malloc(sizeof(my_type));

  x->x = 1;
  y->x = 2;
  z->x = 3;

  rb_node *a = (rb_node *) malloc(sizeof(rb_node));
  rb_node *b = (rb_node *) malloc(sizeof(rb_node));
  rb_node *c = (rb_node *) malloc(sizeof(rb_node));

  a->data = x;
  b->data = y;
  c->data = z;

  assert(binary_insert_node(b, t, int_compare) != NULL);
  assert(binary_insert_node(c, t, int_compare) != NULL);
  assert(binary_insert_node(a, t, int_compare) != NULL);

  assert(t->root == b);
  assert(t->root->left == a);
  assert(t->root->right == c);
  assert(t->root->left->left == NULL);
  assert(t->root->left->right == NULL);
  assert(t->root->right->left == NULL);
  assert(t->root->right->right == NULL);

  free_rb(t);

}


void test_5(void) {
  // c, a, b ordering
  sexy_rb_tree *t = create_rb(&int_compare);
  
  my_type *x = (my_type *) malloc(sizeof(my_type));
  my_type *y = (my_type *) malloc(sizeof(my_type));
  my_type *z = (my_type *) malloc(sizeof(my_type));

  x->x = 1;
  y->x = 2;
  z->x = 3;

  rb_node *a = (rb_node *) malloc(sizeof(rb_node));
  rb_node *b = (rb_node *) malloc(sizeof(rb_node));
  rb_node *c = (rb_node *) malloc(sizeof(rb_node));

  a->data = x;
  b->data = y;
  c->data = z;

  assert(binary_insert_node(c, t, int_compare) != NULL);
  assert(binary_insert_node(a, t, int_compare) != NULL);
  assert(binary_insert_node(b, t, int_compare) != NULL);

  assert(t->root == c);
  assert(t->root->left == a);
  assert(t->root->right == NULL);
  assert(t->root->left->left == NULL);
  assert(t->root->left->right == b);
  assert(t->root->left->right->left == NULL);
  assert(t->root->left->right->right == NULL);

  free_rb(t);

}

void test_6(void) {
  // c, b, a ordering
  sexy_rb_tree *t = create_rb(&int_compare);
  
  my_type *x = (my_type *) malloc(sizeof(my_type));
  my_type *y = (my_type *) malloc(sizeof(my_type));
  my_type *z = (my_type *) malloc(sizeof(my_type));

  x->x = 1;
  y->x = 2;
  z->x = 3;

  rb_node *a = (rb_node *) malloc(sizeof(rb_node));
  rb_node *b = (rb_node *) malloc(sizeof(rb_node));
  rb_node *c = (rb_node *) malloc(sizeof(rb_node));

  a->data = x;
  b->data = y;
  c->data = z;

  assert(binary_insert_node(c, t, int_compare) != NULL);
  assert(binary_insert_node(b, t, int_compare) != NULL);
  assert(binary_insert_node(a, t, int_compare) != NULL);

  assert(t->root == c);
  assert(t->root->left == b);
  assert(t->root->right == NULL);
  assert(t->root->left->left == a);
  assert(t->root->left->right == NULL);
  assert(t->root->left->left->left == NULL);
  assert(t->root->left->left->right == NULL);

  free_rb(t);

}


void test_binary_insert(void) {
  printf("beginning test of binary_insert_node()\n");
  test_1();
  test_2();
  test_3();
  test_4();
  test_5();
  test_6();
  printf("binary_insert_node() passed!\n");
}

int main(void) {
  test_binary_insert();
}
