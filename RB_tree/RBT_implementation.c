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
#include <limits.h>

#define RED 50
#define BLACK 51
#define LESS 61
#define EQUAL 121
#define GREATER 124
#define PRED 153
#define SUCC 161

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

  // either RED or BLACK
  int node_color;

  struct rb_node *parent;
  struct rb_node *left;
  struct rb_node *right;
} rb_node;

typedef struct sexy_rb_tree {
  rb_node *root;
  int num_nodes;

  // keeps track of whether replacement
  // (subroutine of remove_baby) should try to use
  // successor or predecessor; can be either PRED or SUPP
  int sorp;

  // returns LESS iff "a < b", EQUAL iff "a == b", GREATER iff "a > b"
  int (*comp)(my_type *, my_type *);
} sexy_rb_tree;

// usual "create, insert, remove, search, and free" functions
// first 4 rely on the comparison function passed into create_rb()
sexy_rb_tree *create_rb(int (*)(my_type *, my_type *));
int insert_baby(my_type *, sexy_rb_tree *);
my_type *remove_baby(my_type *, sexy_rb_tree *);
my_type *search_baby(my_type *, sexy_rb_tree *);
void free_rb(sexy_rb_tree *);
rb_node *get_root(sexy_rb_tree *);

// primarily for testing purposes
// DOES NOT TEST BST INVARIANTS!
// returns 1 for "yes," 0 for "no"
int is_valid_rb_tree(sexy_rb_tree *);

static int red_parent_black_children(rb_node *);
static int path_black_nodes(rb_node *);

// returns 1 if red, 0 if black
static int is_red(rb_node *);

// helper functions: DO NOT EXPOSE
static void free_rb_nodes(rb_node *);
static int insert_rb_node(rb_node *, sexy_rb_tree *);


// traversing functions
static rb_node *grand_parent(rb_node *);
static rb_node *uncle(rb_node *);
static rb_node *parent(rb_node *);
static rb_node *get_left(rb_node *);
static rb_node *get_right(rb_node *);

// left and right rotate for trees
static int lrot(rb_node *, sexy_rb_tree *);
static int rrot(rb_node *, sexy_rb_tree *);

static int set_color (rb_node *, int);


// returns 1 on success
static int insert_base(rb_node *, sexy_rb_tree *);
static int insert_black_parent(rb_node *, sexy_rb_tree *);

// both parent and uncle are red
static int insert_both_red(rb_node *, sexy_rb_tree *);

// parent red, uncle black
static int insert_pred_ublack_opp(rb_node *, sexy_rb_tree *);

static int insert_pred_ublack_same(rb_node *, sexy_rb_tree *);

// insert node without fixing tree
// returns the node after insertion into the tree
static rb_node *node_insert_node(rb_node *, rb_node *, int (*)(my_type *, my_type *));

static rb_node *binary_insert_node(rb_node *, sexy_rb_tree *);

// update data with predecessor
// returns predecessor on success, NULL on failure
static rb_node *replace_with_pred(rb_node *);

// updates data with successor
// returns successor on success, NULL on failure
static rb_node *replace_with_succ(rb_node *);

// updates data with either predecessor or successor;
// tries to keep tree relatively balanced by taking sorp into
// account; sorp MUST be the sorp of the tree from which
// n comes; otherwise, NASAL DAEMONS MAN, NASAL DAEMONS!
// returns predecessor or successor (whichever was used)
// on success, NULL on failure; should only fail when 
// both n->left and n->right are NULL
static rb_node *simple_replace(rb_node *n, int sorp);

/******************
 * IMPLEMENTATION *
 ******************/

rb_node *get_root(sexy_rb_tree *t) {
  return t->root;
}

static rb_node *parent(rb_node *n) {
  return n->parent;
} 

sexy_rb_tree *create_rb(int (*comp)(my_type *, my_type *)) {
  sexy_rb_tree *ret = (sexy_rb_tree *) malloc(sizeof(sexy_rb_tree));
  if (ret == NULL)
    return NULL;
  
  ret->root = NULL;
  ret->num_nodes = 0;
  ret->comp = comp;
  ret->sorp = SUCC;
  
  return ret;
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

static int set_color (rb_node *n, int color) {
  assert(color == RED || color == BLACK);
  n->node_color = color;
}

static int is_red(rb_node *n) {
  if ((n == NULL) || (n->node_color == BLACK))
    return 0;
  else if (n->node_color == RED)
    return 1;
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

static rb_node *get_right(rb_node *n) {
  return n->right;
}

static rb_node *get_left(rb_node *n) {
  return n->left;
}

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

static rb_node *binary_insert_node(rb_node *n, sexy_rb_tree *t) {
  if (t->root == NULL) {
    n->node_color = BLACK;
    n->parent = NULL;
    n->left = NULL;
    n->right = NULL;
    t->root = n;

    return n;
  } else {
    n->node_color = RED;
    node_insert_node(n, t->root, t->comp);
  }

}

// returns 1 on success
static int insert_base(rb_node *n, sexy_rb_tree *t) {
  if (n == get_root(t)) {
    // inserted root; don't need to fix
    set_color(n, BLACK);
    assert(n->parent == NULL);
    return 1;
  } else {
    // check next case
    return insert_black_parent(n, t);
  }
}

static int insert_black_parent(rb_node *n, sexy_rb_tree *t) {
  rb_node *p = parent(n);
  assert(p != NULL);

  if (!is_red(p))
    // black parent; don't need to fix
    return 1;
  else
    return insert_both_red(n, t);
}

static int insert_both_red(rb_node *n, sexy_rb_tree *t) {
  rb_node *u = uncle(n);
  if (u != NULL && is_red(u)) {
    set_color(parent(n), BLACK);
    set_color(u, BLACK);
    // grand parent should exist because parent
    // was originally red and root can't be red
    rb_node *g = grand_parent(n);
    set_color(g, RED);

    // recursively call on grandparent
    // in case we just made the root red
    return insert_base(g, t);
  } else {
    return insert_pred_ublack_opp(n, t);
  }
}

// parent red, uncle black, parent and child "opposites"
// i.e. if parent is left of grandparent, child is
// right of parent, etc.
static int insert_pred_ublack_opp(rb_node *n, sexy_rb_tree *t) {
  rb_node *p = parent(n);
  assert(p != NULL);
  rb_node *g = grand_parent(n);
  assert(g != NULL);
  

  int res = 0;
  if (get_right(p) == n && p == get_left(g)) {
    res = lrot(p, t);
    if (res == 0)
      return 0;
    n = n->left;
  } else if (get_left(p) == n && p == get_right(g)) {
    res = rrot(p, t);
    n = n->right;
  }

  return insert_pred_ublack_same(n, t);
}

static int insert_pred_ublack_same(rb_node *n, sexy_rb_tree *t) {
  rb_node *p = parent(n);
  assert(p != NULL);
  rb_node *g = grand_parent(n);
  assert(g != NULL);

  // p and g about to switch, so need to fix colors
  set_color(p, BLACK);
  set_color(g, RED);

  int res = 0;
  // do the actual switch
  if (get_left(p) == n)
    res = rrot(g, t);
  else
    res = lrot(g, t);

  return res;
}


static int rrot(rb_node *n, sexy_rb_tree *t) {
  int update_root = 0;
  if (get_root(t) == n)
    update_root = 1;
  rb_node *top = n; // c
  rb_node *top_parent = parent(n); // a
  rb_node *l = get_left(n); // d
  rb_node *r = get_right(n); // e

  // these values cannot be NULL
  assert(top != NULL);
  //assert(top_parent != NULL);
  assert(l != NULL);
  //assert(r != NULL);

  rb_node *ll = get_left(l); // f
  rb_node *lr = get_right(l); // g

  // do actual shifting
  n->left = n->left->right;
  l->right = n;
  n->parent = l;
  l->parent = top_parent;
  
  if (lr != NULL)
    lr->parent = n;
  
  if (top_parent != NULL) {
    if (top_parent->left == top)
      top_parent->left = l;
    else
      top_parent->right = l;
  }

  if (update_root)
    t->root = l;

  return 1;
}

static int lrot(rb_node *n, sexy_rb_tree *t) {
  int update_root = 0;
  if (get_root(t) == n)
    update_root = 1;
  rb_node *top = n;
  rb_node *top_parent = parent(n);
  rb_node *l = get_left(n);
  rb_node *r = get_right(n);

  // these values cannot be NULL
  assert(top != NULL);
  //assert(top_parent != NULL);
  //assert(l != NULL);
  assert(r != NULL);

  rb_node *rl = get_left(r);
  rb_node *rr = get_right(r);

  // do actual shifting
  n->right = rl;
  r->left = n;
  n->parent = r;
  r->parent = top_parent;

  if (rl != NULL)
    rl->parent = n;
  
  if (top_parent != NULL) {
    if (top_parent->left == top)
      top_parent->left = r;
    else
      top_parent->right = r;
  }

  if (update_root)
    t->root = r;

  return 1;
}

static int insert_rb_node(rb_node *n, sexy_rb_tree *t) {
  rb_node *inserting = binary_insert_node(n, t);
  return insert_base(inserting, t);
}

int insert_baby(my_type *data, sexy_rb_tree *t) {
  rb_node *n = (rb_node *) malloc(sizeof(rb_node));
  n->data = data;
  n->node_color = RED;
  n->parent = NULL;
  n->left = NULL;
  n->right = NULL;

  if (insert_rb_node(n, t))
    t->num_nodes++;
}

my_type *search_baby(my_type *elem, sexy_rb_tree *t) {
  int (*comp)(my_type *, my_type *) = t->comp;

  assert(elem != NULL);

  rb_node *cur = t->root;
  
  while (cur != NULL) {
    if (comp(elem, cur->data) == LESS)
      cur = cur->left;
    else if (comp(elem, cur->data) == GREATER)
      cur = cur->right;
    else
      return cur->data;
  }
  
  return NULL;
  
}

static rb_node *replace_with_pred(rb_node *n) {
  rb_node *l = get_left(n);

  if (l == NULL) {
    // no left child, so can't replace with predecessor
    return NULL;
  } else {
    // iterate down the tree to the predecessor
    while (get_right(l) != NULL)
      l = get_right(l);

    assert(l->data != NULL);
    n->data = l->data;

    return l;
  }
  
}


static rb_node *replace_with_succ(rb_node *n) {
  rb_node *r = get_right(n);

  if (r == NULL) {
    return NULL;
  } else {

    while (get_left(r) != NULL)
      r = get_left(r);

    assert(r->data != NULL);
    n->data = r->data;

    return r;
  }

}

static rb_node *simple_replace(rb_node *n, int sorp) {
  rb_node *res = NULL;

  if (sorp == SUCC) {
    res = replace_with_succ(n);

    if (res == NULL)
      res = replace_with_pred(n);
    
  } else if (sorp == PRED) {
    
    res = replace_with_pred(n);
    
    if (res == NULL)
      res = replace_with_succ(n);

  } else {
    // something wrong with constants
    assert(0);
  }

  return res;

}

int is_valid_rb_tree(sexy_rb_tree *t) {
  if (t == NULL)
    return 0;

  rb_node *r = get_root(t);

  if (r == NULL)
    return 0;

  // root should be black
  if (is_red(r))
    return 0;

  // both children of red parent must be black (or NULL)
  if (!red_parent_black_children(r))
    return 0;

  // every simple path from node to descendant
  // has same number of black nodes
  if (!path_black_nodes(r))
    return 0;

  return 1;
}

static int one_red_parent_black_children(rb_node *n) {
  rb_node *l = get_left(n);
  rb_node *r = get_right(n);
  
  if (is_red(n)) {
    if (is_red(l) || is_red(r))
      return 0;
  }

  //return (red_parent_black_children(l) && red_parent_black_children(r));

  return 1;

}

static int red_parent_black_children(rb_node *n) {
  if (!one_red_parent_black_children(n))
    return 0;
  
  if (n->left != NULL) {
    if (!red_parent_black_children(n->left))
      return 0;
  }

  if (n->right != NULL) {
    if (!red_parent_black_children(n->right))
      return 0;
  }
  
  return 1;

}

static int path_max = INT_MIN;
static int path_min = INT_MAX;

static void path_black_nodes_helper(rb_node *n, int count) {
  if (n == NULL) {
    count++;
    
    if (count > path_max)
      path_max = count;

    if (count < path_min)
      path_min = count;

    return;

  } else {

  if (!is_red(n))
    count++;

  path_black_nodes_helper(n->left, count);
  path_black_nodes_helper(n->right, count);

  }

}

static int path_black_nodes(rb_node *n) {
  path_max = INT_MIN;
  path_min = INT_MAX;

  path_black_nodes_helper(n, 0);
  
  if (path_max != path_min)
    return 0;

  if (n->left != NULL) {
    path_max = INT_MIN;
    path_min = INT_MAX;
    
    path_black_nodes(n->left);

    path_max = INT_MIN;
    path_min = INT_MAX;
  }
  
  if (n->right != NULL) {
    path_max = INT_MIN;
    path_min = INT_MAX;
    
    path_black_nodes(n->right);

    path_max = INT_MIN;
    path_min = INT_MAX;
  }

  path_max = INT_MIN;
  path_min = INT_MAX;
  
  return 1;
}

/***************
 * TEST SCRIPT *
 ***************/

// BREAKS ABSTRACTION BARRIER
// primarily tests binary_insert_node
static void test_1(void) {
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
  assert(binary_insert_node(a, t) != NULL);
  assert(binary_insert_node(b, t) != NULL);
  assert(binary_insert_node(c, t) != NULL);

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

static void test_2(void) {
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
  assert(binary_insert_node(a, t) != NULL);
  assert(binary_insert_node(c, t) != NULL);
  assert(binary_insert_node(b, t) != NULL);

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

static void test_3(void) {
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

  assert(binary_insert_node(b, t) != NULL);
  assert(binary_insert_node(a, t) != NULL);
  assert(binary_insert_node(c, t) != NULL);

  assert(t->root == b);
  assert(t->root->left == a);
  assert(t->root->right == c);
  assert(t->root->left->left == NULL);
  assert(t->root->left->right == NULL);
  assert(t->root->right->left == NULL);
  assert(t->root->right->right == NULL);

  free_rb(t);

}

static void test_4(void) {
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

  assert(binary_insert_node(b, t) != NULL);
  assert(binary_insert_node(c, t) != NULL);
  assert(binary_insert_node(a, t) != NULL);

  assert(t->root == b);
  assert(t->root->left == a);
  assert(t->root->right == c);
  assert(t->root->left->left == NULL);
  assert(t->root->left->right == NULL);
  assert(t->root->right->left == NULL);
  assert(t->root->right->right == NULL);

  free_rb(t);

}


static void test_5(void) {
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

  assert(binary_insert_node(c, t) != NULL);
  assert(binary_insert_node(a, t) != NULL);
  assert(binary_insert_node(b, t) != NULL);

  assert(t->root == c);
  assert(t->root->left == a);
  assert(t->root->right == NULL);
  assert(t->root->left->left == NULL);
  assert(t->root->left->right == b);
  assert(t->root->left->right->left == NULL);
  assert(t->root->left->right->right == NULL);

  free_rb(t);

}

static void test_6(void) {
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

  assert(binary_insert_node(c, t) != NULL);
  assert(binary_insert_node(b, t) != NULL);
  assert(binary_insert_node(a, t) != NULL);

  assert(t->root == c);
  assert(t->root->left == b);
  assert(t->root->right == NULL);
  assert(t->root->left->left == a);
  assert(t->root->left->right == NULL);
  assert(t->root->left->left->left == NULL);
  assert(t->root->left->left->right == NULL);

  free_rb(t);

}


static void test_binary_insert(void) {
  printf("beginning test of binary_insert_node()\n");
  test_1();
  test_2();
  test_3();
  test_4();
  test_5();
  test_6();
  printf("binary_insert_node() passed!\n");
}

static void test_rrot1(void) {
  // instantiate and initialize data
  my_type *a = malloc(sizeof(my_type));
  my_type *b = malloc(sizeof(my_type));
  my_type *c = malloc(sizeof(my_type));
  my_type *d = malloc(sizeof(my_type));
  my_type *e = malloc(sizeof(my_type));
  my_type *f = malloc(sizeof(my_type));
  my_type *g = malloc(sizeof(my_type));
  my_type *h = malloc(sizeof(my_type));
  my_type *i = malloc(sizeof(my_type));
  my_type *j = malloc(sizeof(my_type));
  my_type *k = malloc(sizeof(my_type));
  my_type *l = malloc(sizeof(my_type));
  my_type *m = malloc(sizeof(my_type));

  a->x = 1;
  b->x = 2;
  c->x = 3;
  d->x = 4;
  e->x = 5;
  f->x = 6;
  g->x = 7;
  h->x = 8;
  i->x = 9;
  j->x = 10;
  k->x = 11;
  l->x = 12;
  m->x = 13;

  rb_node *na = malloc(sizeof(rb_node));
  rb_node *nb = malloc(sizeof(rb_node));
  rb_node *nc = malloc(sizeof(rb_node));
  rb_node *nd = malloc(sizeof(rb_node));
  rb_node *ne = malloc(sizeof(rb_node));
  rb_node *nf = malloc(sizeof(rb_node));
  rb_node *ng = malloc(sizeof(rb_node));
  rb_node *nh = malloc(sizeof(rb_node));
  rb_node *ni = malloc(sizeof(rb_node));
  rb_node *nj = malloc(sizeof(rb_node));
  rb_node *nk = malloc(sizeof(rb_node));
  rb_node *nl = malloc(sizeof(rb_node));
  rb_node *nm = malloc(sizeof(rb_node));

  na->data = a;
  nb->data = b;
  nc->data = c;
  nd->data = d;
  ne->data = e;
  nf->data = f;
  ng->data = g;
  nh->data = h;
  ni->data = i;
  nj->data = j;
  nk->data = k;
  nl->data = l;
  nm->data = m;


  // set up initial tree
  na->parent = NULL;
  na->left = nb;
  na->right = nc;

  nb->parent = na;
  nb->left = NULL;
  nb->right = NULL;

  nc->parent = na;
  nc->left = nd;
  nc->right = ne;

  nd->parent = nc;
  nd->left = nf;
  nd->right = ng;

  ne->parent = nc;
  ne->left = nh;
  ne->right = ni;

  nf->parent = nd;
  nf->left = nj;
  nf->right = nk;

  ng->parent = nd;
  ng->left = nl;
  ng->right = nm;

  nh->parent = ne;
  nh->left = NULL;
  nh->right = NULL;

  ni->parent = ne;
  ni->left = NULL;
  ni->right = NULL;

  nj->parent = nf;
  nj->left = NULL;
  nj->right = NULL;

  nk->parent = nf;
  nk->left = NULL;
  nk->right = NULL;

  nl->parent = ng;
  nl->left = NULL;
  nl->right = NULL;

  nm->parent = ng;
  nm->left = NULL;
  nm->right = NULL;

  // rotate nc, which is right of root
  sexy_rb_tree *t = (sexy_rb_tree *) malloc(sizeof(sexy_rb_tree));
  t->root = NULL;

  rrot(nc, t);

  free(t);

  // test structure
  assert(na->parent == NULL);
  assert(na->left == nb);
  assert(na->right == nd);

  assert(nb->parent == na);
  assert(nb->left == NULL);
  assert(nb->right == NULL);

  assert(nc->parent == nd);
  assert(nc->left == ng);
  assert(nc->right == ne);

  assert(nd->parent == na);
  assert(nd->left == nf);
  assert(nd->right == nc);

  assert(ne->parent == nc);
  assert(ne->left == nh);
  assert(ne->right == ni);

  assert(nf->parent == nd);
  assert(nf->left == nj);
  assert(nf->right == nk);

  assert(ng->parent == nc);
  assert(ng->left == nl);
  assert(ng->right == nm);

  assert(nh->parent == ne);
  assert(nh->left == NULL);
  assert(nh->right == NULL);

  assert(ni->parent == ne);
  assert(ni->left == NULL);
  assert(ni->right == NULL);

  assert(nj->parent == nf);
  assert(nj->left == NULL);
  assert(nj->right == NULL);

  assert(nk->parent == nf);
  assert(nk->left == NULL);
  assert(nk->right == NULL);

  assert(nl->parent == ng);
  assert(nl->left == NULL);
  assert(nl->right == NULL);

  assert(nm->parent == ng);
  assert(nm->left == NULL);
  assert(nm->right == NULL);

  // clean up
  free(a);
  free(b);
  free(c);
  free(d);
  free(e);
  free(f);
  free(g);
  free(h);
  free(i);
  free(j);
  free(k);
  free(l);
  free(m);

  free(na);
  free(nb);
  free(nc);
  free(nd);
  free(ne);
  free(nf);
  free(ng);
  free(nh);
  free(ni);
  free(nj);
  free(nk);
  free(nl);
  free(nm);

  return;
}

// same as test_rrot1 but tree is on other side of parent
static void test_rrot2(void) {
  return;
}

static void test_rrot(void) {
  test_rrot1();
  test_rrot2();
}

// reverse of rrot1
static void test_lrot1(void) {
  // instantiate and initialize data
  my_type *a = malloc(sizeof(my_type));
  my_type *b = malloc(sizeof(my_type));
  my_type *c = malloc(sizeof(my_type));
  my_type *d = malloc(sizeof(my_type));
  my_type *e = malloc(sizeof(my_type));
  my_type *f = malloc(sizeof(my_type));
  my_type *g = malloc(sizeof(my_type));
  my_type *h = malloc(sizeof(my_type));
  my_type *i = malloc(sizeof(my_type));
  my_type *j = malloc(sizeof(my_type));
  my_type *k = malloc(sizeof(my_type));
  my_type *l = malloc(sizeof(my_type));
  my_type *m = malloc(sizeof(my_type));

  a->x = 1;
  b->x = 2;
  c->x = 3;
  d->x = 4;
  e->x = 5;
  f->x = 6;
  g->x = 7;
  h->x = 8;
  i->x = 9;
  j->x = 10;
  k->x = 11;
  l->x = 12;
  m->x = 13;

  rb_node *na = malloc(sizeof(rb_node));
  rb_node *nb = malloc(sizeof(rb_node));
  rb_node *nc = malloc(sizeof(rb_node));
  rb_node *nd = malloc(sizeof(rb_node));
  rb_node *ne = malloc(sizeof(rb_node));
  rb_node *nf = malloc(sizeof(rb_node));
  rb_node *ng = malloc(sizeof(rb_node));
  rb_node *nh = malloc(sizeof(rb_node));
  rb_node *ni = malloc(sizeof(rb_node));
  rb_node *nj = malloc(sizeof(rb_node));
  rb_node *nk = malloc(sizeof(rb_node));
  rb_node *nl = malloc(sizeof(rb_node));
  rb_node *nm = malloc(sizeof(rb_node));

  na->data = a;
  nb->data = b;
  nc->data = c;
  nd->data = d;
  ne->data = e;
  nf->data = f;
  ng->data = g;
  nh->data = h;
  ni->data = i;
  nj->data = j;
  nk->data = k;
  nl->data = l;
  nm->data = m;


  // set up initial tree
  na->parent = NULL;
  na->left = nb;
  na->right = nd;
  
  nb->parent = na;
  nb->left = NULL;
  nb->right = NULL;
  
  nc->parent = nd;
  nc->left = ng;
  nc->right = ne;
  
  nd->parent = na;
  nd->left = nf;
  nd->right = nc;
  
  ne->parent = nc;
  ne->left = nh;
  ne->right = ni;
  
  nf->parent = nd;
  nf->left = nj;
  nf->right = nk;
  
  ng->parent = nc;
  ng->left = nl;
  ng->right = nm;
  
  nh->parent = ne;
  nh->left = NULL;
  nh->right = NULL;
  
  ni->parent = ne;
  ni->left = NULL;
  ni->right = NULL;
  
  nj->parent = nf;
  nj->left = NULL;
  nj->right = NULL;
  
  nk->parent = nf;
  nk->left = NULL;
  nk->right = NULL;
  
  nl->parent = ng;
  nl->left = NULL;
  nl->right = NULL;
  
  nm->parent = ng;
  nm->left = NULL;
  nm->right = NULL;
  
  // rotate nd, which is right of root
  sexy_rb_tree *t = (sexy_rb_tree *) malloc(sizeof(sexy_rb_tree));
  t->root = NULL;

  lrot(nd, t);
  
  free(t);

  // test structure
  assert(na->parent == NULL);
  assert(na->left == nb);
  assert(na->right == nc);
  
  assert(nb->parent == na);
  assert(nb->left == NULL);
  assert(nb->right == NULL);
  
  assert(nc->parent == na);
  assert(nc->left == nd);
  assert(nc->right == ne);
  
  assert(nd->parent == nc);
  assert(nd->left == nf);
  assert(nd->right == ng);
  
  assert(ne->parent == nc);
  assert(ne->left == nh);
  assert(ne->right == ni);
  
  assert(nf->parent == nd);
  assert(nf->left == nj);
  assert(nf->right == nk);
  
  assert(ng->parent == nd);
  assert(ng->left == nl);
  assert(ng->right == nm);
  
  assert(nh->parent == ne);
  assert(nh->left == NULL);
  assert(nh->right == NULL);
  
  assert(ni->parent == ne);
  assert(ni->left == NULL);
  assert(ni->right == NULL);
  
  assert(nj->parent == nf);
  assert(nj->left == NULL);
  assert(nj->right == NULL);
  
  assert(nk->parent == nf);
  assert(nk->left == NULL);
  assert(nk->right == NULL);
  
  assert(nl->parent == ng);
  assert(nl->left == NULL);
  assert(nl->right == NULL);
  
  assert(nm->parent == ng);
  assert(nm->left == NULL);
  assert(nm->right == NULL);
  
  // clean up
  free(a);
  free(b);
  free(c);
  free(d);
  free(e);
  free(f);
  free(g);
  free(h);
  free(i);
  free(j);
  free(k);
  free(l);
  free(m);

  free(na);
  free(nb);
  free(nc);
  free(nd);
  free(ne);
  free(nf);
  free(ng);
  free(nh);
  free(ni);
  free(nj);
  free(nk);
  free(nl);
  free(nm);

  return;
}

static void test_lrot2(void) {
  return;
}

static void test_lrot(void) {
  test_lrot1();
  test_lrot2();
}

static void test_rots(void) {
  printf("beginning rot() test\n");
  test_rrot();
  test_lrot();
  printf("rot() passed!\n");
}

static void test_insert_1(void) {
  
  my_type *a = (my_type *) malloc(sizeof(my_type));
  my_type *b = (my_type *) malloc(sizeof(my_type));
  my_type *c = (my_type *) malloc(sizeof(my_type));

  a->x = 1;
  b->x = 2;
  c->x = 3;

  sexy_rb_tree *t = create_rb(&int_compare);

  insert_baby(a, t);
  insert_baby(b, t);
  insert_baby(c, t);

  free_rb(t);


}

static void test_insert_2(void) {
  my_type *d9 = (my_type *) malloc(sizeof(my_type));
  my_type *d8 = (my_type *) malloc(sizeof(my_type));
  my_type *d7 = (my_type *) malloc(sizeof(my_type));
  my_type *d3 = (my_type *) malloc(sizeof(my_type));
  my_type *d5 = (my_type *) malloc(sizeof(my_type));
  my_type *d2 = (my_type *) malloc(sizeof(my_type));
  

  d9->x = 9;
  d8->x = 8;
  d7->x = 7;
  d3->x = 3;
  d5->x = 5;
  d2->x = 2;
  
  sexy_rb_tree *t = create_rb(&int_compare);

  insert_baby(d9, t);
  insert_baby(d8, t);
  insert_baby(d7, t);
  insert_baby(d3, t);
  insert_baby(d5, t);
  insert_baby(d2, t);

  free_rb(t);
  
}

static void test_insert(void) {
  printf("beginning test_insert()\n");

  test_insert_1();
  test_insert_2();

  printf("test_insert() passed!\n");
}

static void test_search(void) {
  // number of nodes going into tree for testing
  int n = 100;

  printf("beginning test_search with %d elements\n", n);

  sexy_rb_tree *t = create_rb(&int_compare);

  my_type *dat[2*n];

  for(int i = 0; i < 2*n; i++) {
    dat[i] = (my_type *) malloc(sizeof(my_type));
  }

  for(int i = 0; i < n; i++) {
    dat[i]->x = i;
  }

  for(int i = n; i < 2*n; i++) {
    dat[i]->x = i;
  }

  for(int i = 0; i < n; i++) {
    insert_baby(dat[i], t);
  }

  for(int i = 0; i < n; i++) {
    assert(search_baby(dat[i], t) != NULL);
  }
  
  for(int i = n; i < 2*n; i++) {
    assert(search_baby(dat[i], t) == NULL);
  }

  for(int i = n; i < 2*n; i++) {
    free(dat[i]);
  }

  assert(is_valid_rb_tree(t));
  
  free_rb(t);

  printf("test_search passed!\n");
}

static void replace_w_pred_test(void) {
  // initialize data for nodes
  my_type *a = (my_type *) malloc(sizeof(my_type));
  my_type *b = (my_type *) malloc(sizeof(my_type));
  my_type *c = (my_type *) malloc(sizeof(my_type));
  my_type *d = (my_type *) malloc(sizeof(my_type));
  my_type *e = (my_type *) malloc(sizeof(my_type));

  a->x = 1;
  b->x = 2;
  c->x = 3;
  d->x = 4;
  e->x = 5;

  // initialize nodes
  rb_node *na = (rb_node *) malloc(sizeof(rb_node));
  rb_node *nb = (rb_node *) malloc(sizeof(rb_node));
  rb_node *nc = (rb_node *) malloc(sizeof(rb_node));
  rb_node *nd = (rb_node *) malloc(sizeof(rb_node));
  rb_node *ne = (rb_node *) malloc(sizeof(rb_node));

  na->data = a;
  nb->data = b;
  nc->data = c;
  nd->data = d;
  ne->data = e;
  
  // set up structure
  na->left = nb;
  na->right = nc;
  na->parent = NULL;
  
  nb->left = nd;
  nb->right = ne;
  nb->parent = na;

  nc->left = NULL;
  nc->right = NULL;
  nc->parent = na;

  nd->left = NULL;
  nd->right = NULL;
  nd->parent = nb;

  ne->left = NULL;
  ne->right = NULL;
  ne->parent = nb;
  
  // test that "fails" in desired circumstances
  assert(replace_with_pred(nc) == NULL);
  assert(replace_with_pred(nd) == NULL);
  assert(replace_with_pred(ne) == NULL);

  // test general case
  assert(replace_with_pred(na) != NULL);
  assert(na->data == ne->data);

  // test "minimum working case"
  assert(replace_with_pred(nb) != NULL);
  assert(nb->data == nd->data);

  // clean up
  free(a);
  free(b);
  free(c);
  free(d);
  free(e);

  free(na);
  free(nb);
  free(nc);
  free(nd);
  free(ne);

}


// very similar to replace_w_pred_test()
static void replace_w_succ_test(void) {
  // initialize data for nodes
  my_type *a = (my_type *) malloc(sizeof(my_type));
  my_type *b = (my_type *) malloc(sizeof(my_type));
  my_type *c = (my_type *) malloc(sizeof(my_type));
  my_type *d = (my_type *) malloc(sizeof(my_type));
  my_type *e = (my_type *) malloc(sizeof(my_type));

  a->x = 1;
  b->x = 2;
  c->x = 3;
  d->x = 4;
  e->x = 5;

  // initialize nodes
  rb_node *na = (rb_node *) malloc(sizeof(rb_node));
  rb_node *nb = (rb_node *) malloc(sizeof(rb_node));
  rb_node *nc = (rb_node *) malloc(sizeof(rb_node));
  rb_node *nd = (rb_node *) malloc(sizeof(rb_node));
  rb_node *ne = (rb_node *) malloc(sizeof(rb_node));

  na->data = a;
  nb->data = b;
  nc->data = c;
  nd->data = d;
  ne->data = e;
  
  // set up structure
  na->left = nb;
  na->right = nc;
  na->parent = NULL;
  
  nb->left = NULL;
  nb->right = NULL;
  nb->parent = na;

  nc->left = nd;
  nc->right = ne;
  nc->parent = na;

  nd->left = NULL;
  nd->right = NULL;
  nd->parent = nc;

  ne->left = NULL;
  ne->right = NULL;
  ne->parent = nc;
  
  // test that "fails" in desired circumstances
  assert(replace_with_succ(nb) == NULL);
  assert(replace_with_succ(nd) == NULL);
  assert(replace_with_succ(ne) == NULL);

  // test general case
  assert(replace_with_succ(na) != NULL);
  assert(na->data == nd->data);

  // test "minimum working case"
  assert(replace_with_succ(nc) != NULL);
  assert(nc->data == ne->data);

  // clean up
  free(a);
  free(b);
  free(c);
  free(d);
  free(e);

  free(na);
  free(nb);
  free(nc);
  free(nd);
  free(ne);

}

static void simple_replace_test(void) {
  // initialize data for nodes
  my_type *a = (my_type *) malloc(sizeof(my_type));
  my_type *b = (my_type *) malloc(sizeof(my_type));
  my_type *c = (my_type *) malloc(sizeof(my_type));
  my_type *d = (my_type *) malloc(sizeof(my_type));
  my_type *e = (my_type *) malloc(sizeof(my_type));

  a->x = 1;
  b->x = 2;
  c->x = 3;
  d->x = 4;
  e->x = 5;

  // initialize nodes
  rb_node *na = (rb_node *) malloc(sizeof(rb_node));
  rb_node *nb = (rb_node *) malloc(sizeof(rb_node));
  rb_node *nc = (rb_node *) malloc(sizeof(rb_node));
  rb_node *nd = (rb_node *) malloc(sizeof(rb_node));
  rb_node *ne = (rb_node *) malloc(sizeof(rb_node));

  na->data = a;
  nb->data = b;
  nc->data = c;
  nd->data = d;
  ne->data = e;
  
  // set up structure
  na->left = nb;
  na->right = nc;
  na->parent = NULL;
  
  nb->left = nd;
  nb->right = ne;
  nb->parent = na;

  nc->left = NULL;
  nc->right = NULL;
  nc->parent = na;

  nd->left = NULL;
  nd->right = NULL;
  nd->parent = nb;

  ne->left = NULL;
  ne->right = NULL;
  ne->parent = nb;
  
  // test "fails" when should
  assert(simple_replace(nc, SUCC) == NULL);
  assert(simple_replace(nc, PRED) == NULL);

  assert(simple_replace(nd, SUCC) == NULL);
  assert(simple_replace(nd, PRED) == NULL);

  assert(simple_replace(ne, SUCC) == NULL);
  assert(simple_replace(ne, PRED) == NULL);

  assert(nc->data == c);
  assert(nd->data == d);
  assert(ne->data == e);

  // test "minimal working case"
  assert(simple_replace(nb, SUCC) == ne);
  assert(nb->data == e);

  assert(simple_replace(nb, PRED) == nd);
  assert(nb->data == d);

  // test general case
  assert(simple_replace(na, PRED) == ne);
  assert(na->data == e);
  
  assert(simple_replace(na, SUCC) == nc);
  assert(na->data == c);

  // clean up
  free(a);
  free(b);
  free(c);
  free(d);
  free(e);

  free(na);
  free(nb);
  free(nc);
  free(nd);
  free(ne);

}

static void replace_test(void) {
  printf("testing basic data replacement\n");
  replace_w_pred_test();
  replace_w_succ_test();
  simple_replace_test();
  printf("replacement passed!\n");
}

static void test_all(void) {
  printf("\n");
  test_binary_insert();
  printf("\n");
  test_rots();
  printf("\n");
  test_insert();
  printf("\n");
  test_search();
  printf("\n");
  replace_test();
  printf("\n");
}

int main(void) {
  test_all();
}
