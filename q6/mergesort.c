#include <pthread.h>
#include <unistd.h>
#include <errno.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// STRUCTS ========================== //

/* Argumentos passados para cada Thread */


// DECLARAÇÃO DAS FUNÇÕES =========== //

void merge(int *arr, int l, int m, int r);
void merge_sort(int *arr, int l, int r);

// ================================== //

// VVVVVVVVVVVVV MAIN VVVVVVVVVVVVVVV //

int main (int argc, char *argv[]) {

    return 0;
}

// VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV //


void merge(int *arr, int l, int m, int r) {
    
    int i, j, k;
    
    int n1 = m - l + 1;
    int n2 = r - m;

    int leftArr[n1], rightArr[n2];
    
    for (i = 0; i < n1; i++) leftArr[i] = arr[l + i];
    for (j = 0; j < n2; j++) rightArr[j]= arr[m + 1 + j];
    
    i = 0;
    j = 0;
    k = l;
    
    while (i < n1 && j < n2) {
        if (leftArr[i] <= rightArr[j]) {
            arr[k] = leftArr[i];
            i++;
        } else {
            arr[k] = rightArr[j];
            j++;
        }
        k++;
    }
    
    while (i < n1) {
        arr[k] = leftArr[i];
        i++;
        k++;
    }
    
    while (j < n2) {
        arr[k] = rightArr[j];
        j++;
        k++;
    }
    
};

void merge_sort(int *arr, int l, int r) {
    if (l < r) {
        int mid = l + (r - l) / 2;

        merge_sort(arr, l, mid);
        merge_sort(arr, mid + 1, r);

        merge(arr, l, mid, r);
    }
};
    