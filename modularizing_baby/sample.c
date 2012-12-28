#include <stdio.h>
#include <stdlib.h>

// struct representing data structure for ints
typedef struct sample_int {
  int (*compare)(int, int);
} sample_int;

// struct representing data structure for floats
typedef struct sample_float {
  int (*compare)(float, float);
} sample_float;

// comparison function for ints
// passed into struct representing data structure
int compare_int(int x, int y) {
  if (x < y)
    return 1;
  else
    return 0;
}

// comparison function for flaots
int compare_float(float x, float y) {
  if (x < y)
    return 1;
  else
    return 0;
}


int main(void) {
  // set integer example
  sample_int *eg_int = malloc(sizeof(sample_int));
  eg_int->compare = &compare_int;

  // set float example
  sample_float *eg_float = malloc(sizeof(sample_float));
  eg_float->compare = &compare_float;

  // WHY THIS IS COOL: THE FUNCTION IS STORED IN THE DATA STRUCTURE
  // SO WHEN BUILDING THE DATA STRUCTURE FOR A NEW TYPE
  // WE ONLY NEED TO UPDATE CERTAIN FUNCTIONS INSTEAD OF THE ENTIRE LOGIC
  // also, encapsulation of functions but whatev
  printf("%d %d\n", eg_int->compare(1, 2), eg_float->compare(4., 3.));

  // clean up
  free(eg_int);
  free(eg_float);
}
