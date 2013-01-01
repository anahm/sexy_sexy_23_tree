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

#define RED 13
#define BLACK 14
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
my_type *remove_baby(my_type *, sexy_rb_tree *);
my_type *search_baby(my_type *, sexy_rb_tree *);
void free_rb(sexy_rb_tree *);
rb_node *get_root(sexy_rb_tree *);

// returns 1 if red, 0 if black
int is_red(rb_node *);

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

static rb_node *binary_insert_node(rb_node *, sexy_rb_tree *, int (*)(my_type *, my_type *));

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

static rb_node *binary_insert_node(rb_node *n, sexy_rb_tree *t, int (*comp)(my_type *, my_type *)) {

  if (t->root == NULL) {
    n->node_color = BLACK;
    n->parent = NULL;
    n->left = NULL;
    n->right = NULL;
    t->root = n;

    return n;
  } else {
    n->node_color = RED;
    node_insert_node(n, t->root, comp);
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
  rb_node *inserting = binary_insert_node(n, t, t->comp);
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
  printf("beginning rrot() test\n");
  test_rrot();
  printf("rrot passed!\n");
  printf("beginning lrot() test\n");
  test_lrot();
  printf("lrot passed!\n");
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
  int n = 100000;

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
  
  free_rb(t);

  printf("test_search passed!\n");
}



int main(void) {
  test_binary_insert();
  test_rots();
  test_insert();
  test_search();
}
