#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>

#define CAP_INIT 1
#define NO_INDEX_USED -1

/******************************************************
 * STRUCT DEFINITION OF ELEMENTS THE VECTOR WILL HOLD *
 ******************************************************/

typedef struct my_type {
  int x;
} my_type;



/*******************************
 * CONTAINER STRUCT FOR VECTOR *
 *******************************/

typedef struct vectore {
  my_type **storage;
  int capacity;
  int last_used_index;
} vectore;



/*************
 * INTERFACE *
 *************/

// instantiate and initialize a new vectore
// returns pointer to new vectore on success, NULL on failure
vectore *new_vectore(void);

// add a my_type *to a vectore at a specific location
// returns the new vectore on success, NULL on failure
// automatically handles resizing vectore if needed
vectore *add_to_vectore(my_type *, vectore *, int);

// resizes the vectore (increases capacity by factor of 2)
// returns pointer to new vectore on success, NULL on failure
vectore *resize_vectore(vectore *);

// returns the my_type *stored at a certain index
// does not remove the my_type
// returns NULL if out of bounds; doesn't error out
my_type *get_from_vectore(vectore *, int);

// overwrites a location in the vectore with NULL
// returns 1 on success, 0 if out of bounds (still usable)
// DOESN'T FREE WHAT WAS THERE!
int clean_index(vectore *, int);

// clean up and free vector
// returns 1 on success, 0 on failure
int free_vectore(vectore *);

// get capacity of vector
// probably not necessary but good OO design
int get_capacity(vectore *);

// get last used index in vectore
int get_last_used_index(vectore *);

// test the vectore code
int test_vectore(void);



/******************
 * IMPLEMENTATION *
 ******************/
vectore *new_vectore(void) {
  vectore *ret = malloc(sizeof(vectore));
  my_type **storage = malloc(sizeof(my_type *) * CAP_INIT);
  assert(ret != NULL);
  assert(storage != NULL);

  ret->storage = storage;
  ret->capacity = CAP_INIT;
  ret->last_used_index = NO_INDEX_USED;

  for (int i = 0; i < CAP_INIT; i++) {
    ret->storage[i] = NULL;
  }

  return ret;
}

int get_capacity(vectore *v) {
  return v->capacity;
}

int get_last_used_index (vectore *v) {
  return v->last_used_index;
}

vectore *resize_vectore(vectore *v) {
  if (get_capacity(v) > INT_MAX/2) {
    // vector too large
    return NULL;
  }

  // get local copies of values in v to avoid
  // multiple dereferences of same data
  int old_cap = v->capacity;
  int new_cap = old_cap * 2;
  my_type **storage = v->storage;

  // update capacity
  v->capacity = new_cap;

  my_type **new_storage = (my_type **) realloc(v, new_cap * sizeof(my_type *));
  v->storage = new_storage;

  for (int i = old_cap; i < new_cap; i++) {
    v->storage[i] = NULL;
  }

  return v;
}

int free_vectore(vectore *v) {
  int cap = v->capacity;
  my_type **storage = v->storage;
  for (int i = 0; i < cap; i++) {
    free(storage[i]);
  }

  free(v);

  return 1;
}

int clean_index(vectore *v, int index) {
  if (index >= v->capacity || index < 0) {
    return 0;
  } else {
    v->storage[index] = NULL;
    
    // make faster by having local copies of storage and last_used_index
    // so that not dereferencing many times
    if (index == v->last_used_index) {
      while (v->storage[v->last_used_index] ==  NULL && v->last_used_index >= 0) {
	v->last_used_index--;
      }
    }
  }
}

my_type *get_from_vectore(vectore *v, int index) {
  if (index >= v->capacity || index < 0) {
    // out of bounds
    return NULL;
  } else {
    return v->storage[index];
  }
}

vectore *add_to_vectore(my_type *elem, vectore *v, int index) {
  if (index < 0)
    return NULL;
  
  vectore *res = v;
  while (index >= res->capacity) {
    res = resize_vectore(v);
    if (res == NULL)
      return NULL;
  }

  res->storage[index] = elem;
  if (index > res->last_used_index)
    res->last_used_index = index;

  return res;
}

/***************
 * TEST SCRIPT *
 ***************/

int test_vectore(void) {
  vectore *v = new_vectore();
  assert(get_capacity(v) == 1);
  assert(get_last_used_index(v) == NO_INDEX_USED);
  
  assert(get_from_vectore(v, 0) == NULL);
  assert(get_from_vectore(v, 1) == NULL);

  my_type *x = malloc(sizeof(my_type));
  my_type *y = malloc(sizeof(my_type));
  my_type *z = malloc(sizeof(my_type));

  x->x = 1;
  y->x = 2;
  z->x = 3;

  assert(add_to_vectore(x, v, 0) != NULL);
  assert(get_capacity(v) == 1);
  assert(get_last_used_index(v) == 0);

  assert(add_to_vectore(z, v, 2) != NULL);
  assert(get_capacity(v) == 4);
  assert(get_last_used_index(v) == 2);
  
  assert(add_to_vectore(y, v, 1) != NULL);
  assert(get_capacity(v) == 4);
  assert(get_last_used_index(v) == 2);

  free_vectore(v);

  return 1;
}

int main(void) {
  test_vectore();
}

