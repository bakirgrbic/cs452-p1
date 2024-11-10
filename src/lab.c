#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h> /* for gettimeofday system call */
#include "lab.h"

/**
 * @brief Standard insertion sort that is faster than merge sort for small array's
 *
 * @param A The array to sort
 * @param p The starting index
 * @param r The ending index
 */
static void insertion_sort(int A[], int p, int r)
{
  int j;

  for (j = p + 1; j <= r; j++)
    {
      int key = A[j];
      int i = j - 1;
      while ((i > p - 1) && (A[i] > key))
        {
	  A[i + 1] = A[i];
	  i--;
        }
      A[i + 1] = key;
    }
}


void mergesort_s(int A[], int p, int r)
{
  if (r - p + 1 <=  INSERTION_SORT_THRESHOLD)
    {
      insertion_sort(A, p, r);
    }
  else
    {
      int q = (p + r) / 2;
      mergesort_s(A, p, q);
      mergesort_s(A, q + 1, r);
      merge_s(A, p, q, r);
    }

}

void merge_s(int A[], int p, int q, int r)
{
  int *B = (int *)malloc(sizeof(int) * (r - p + 1));

  int i = p;
  int j = q + 1;
  int k = 0;
  int l;

  /* as long as both lists have unexamined elements */
  /*  this loop keeps executing. */
  while ((i <= q) && (j <= r))
    {
      if (A[i] < A[j])
        {
	  B[k] = A[i];
	  i++;
        }
      else
        {
	  B[k] = A[j];
	  j++;
        }
      k++;
    }

  /* now only at most one list has unprocessed elements. */
  if (i <= q)
    {
      /* copy remaining elements from the first list */
      for (l = i; l <= q; l++)
        {
	  B[k] = A[l];
	  k++;
        }
    }
  else
    {
      /* copy remaining elements from the second list */
      for (l = j; l <= r; l++)
        {
	  B[k] = A[l];
	  k++;
        }
    }

  /* copy merged output from array B back to array A */
  k = 0;
  for (l = p; l <= r; l++)
    {
      A[l] = B[k];
      k++;
    }

  free(B);
}

double getMilliSeconds()
{
  struct timeval now;
  gettimeofday(&now, (struct timezone *)0);
  return (double)now.tv_sec * 1000.0 + now.tv_usec / 1000.0;
}

void mergesort_mt(int *A, int n, int num_thread) {
    int threads_used = num_thread;
    int chunk_size = 0;
    int extra_pieces = 0;
    int start = 0;
    int end = 0;
    int middle = 0;
    struct parallel_args** thread_args;

    if (threads_used > MAX_THREADS) {
        threads_used = MAX_THREADS;
    }

    if (threads_used > n) {
        fprintf(stderr, "Number of threads used exceeded size of array\n");
        exit(2);
    }

    chunk_size = n / threads_used;
    extra_pieces = n % threads_used;

    // Split A into chunks for all threads
    thread_args = (struct parallel_args**) malloc(threads_used * sizeof(struct parallel_args*));
    for (int i = 0; i < threads_used; i++) {
        thread_args[i] = (struct parallel_args*) malloc(sizeof(struct parallel_args));
        thread_args[i]->A = A;
        thread_args[i]->start = start;

        end += chunk_size - 1;
        if (extra_pieces > 0) {
            end++;
            extra_pieces--;
        }

        thread_args[i]->end = end;

        start = end + 1;
        end = start;
    }

    // Create threads and have them call parallel_mergesort 
    for (int i = 0; i < threads_used; i++) {
        pthread_create(&(thread_args[i]->tid), NULL, parallel_mergesort, thread_args[i]);
    }

    // Wait on all threads to finish before master merge
    for (int i = 0; i < threads_used; i++) {
        pthread_join(thread_args[i]->tid, NULL);
    }

    // Merge all results
    for (int i = 1; i < threads_used; i++) {
        start = thread_args[i-1]->start;
        middle = thread_args[i-1]->end;
        end = thread_args[i]->end;
        merge_s(A, start, middle, end);

        thread_args[i]->start = start;
        thread_args[i]->end = end;
    }

    for (int i = 0; i < threads_used; i++) {
        free(thread_args[i]);
    }
    free(thread_args);
}

void *parallel_mergesort(void *args) {
    struct parallel_args* my_args = (struct parallel_args*) args;
    int *A = my_args->A;
    int start = my_args->start;
    int end = my_args->end;

    mergesort_s(A, start, end);
 
    return NULL;
}
