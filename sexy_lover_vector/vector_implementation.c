#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

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
vectore *new_vectore(void); // implemented

// add a my_type *to a vectore at a specific location
// returns 1 on success, 0 on failure
// automatically handles resizing vectore if needed
int add_to_vectore(my_type *, vectore *, int);

// resizes the vectore (increases capacity by factor of 2)
// returns pointer to new vectore on success, NULL on failure
vectore *resize_vectore(vectore *); // implemented

// returns the my_type *stored at a certain index
// does not remove the my_type
my_type *get_from_vectore(vectore *, int);

// overwrites a location in the vectore with NULL
// returns 1 on success, 0 if out of bounds (still usable)
int clean_index(vectore *, int); // implemented

// clean up and free vector
// returns 1 on success, 0 on failure
int free_vectore(vectore *); // implemented

int get_capacity(vectore *); // implemented
int get_last_used_index(vectore *); // implemented

/******************
 * IMPLEMENTATION *
 ******************/
vectore *new_vectore(void) {
  vectore *ret = malloc(sizeof(vectore));
  my_type **storage = malloc(sizeof(my_type *) * CAP_INIT);
  ret->storage = storage;
  ret->capacity = CAP_INIT;
  ret->last_used_index = NO_INDEX_USED;

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

  vectore *ret = (vectore *) 
    realloc(v, new_cap * sizeof(my_type *));
  
  for (int i = old_cap; i < new_cap; i++) {
    storage[i] = NULL;
  }

  return ret;
}

int free_vectore(vectore *v) {
  int cap = v->capacity;
  my_type **storage = v->storage;
  for (int i = 0; i < cap; i++) {
    free(storage[i]);
  }

  return 1;
}

int clean_index(vectore *v, int index) {
  if (index >= v->capacity) {
    return 0;
  } else {
    v->storage[index] = NULL;
  }
}

/***************
 * TEST SCRIPT *
 ***************/
// currently does nothing
int main(void) {
  my_type x;
  vectore y;
  (void) x;
  (void) y;
}
