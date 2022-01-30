#include "psort.h"
#include <omp.h>
#include <stdio.h>
#include <iostream>

struct node {
    uint32_t val;
    struct node* next;
};

class ll{
public:
    node * head;
    node * tail;
    int size =0 ;
    void insert(uint32_t d){
        node * temp = new node();
        temp->val = d;
        size++;
        if(!head){
            head = temp;
            tail = temp;
        }else{
            tail->next = temp;
            tail = temp;
        }
    }
};


uint32_t partition(uint32_t * arr, uint32_t low, uint32_t high){
    uint32_t piv = arr[low];
    uint32_t l = low,h = high, temp;
    
    while (l<h){
        while(l<h && arr[l] <= piv){
            l++;
        }
        while(l<h && arr[h] > piv){
            h--;
        }
        if(l<h && arr[l] > arr[h]){
            temp = arr[l];
            arr[l] = arr[h];
            arr[h] = temp;
        }
    }
    if(piv>=arr[h]){
        arr[low] = arr[h];
        arr[h] = piv;
        return h;
    }
    if(h>low and piv>=arr[h-1]){
        arr[low] = arr[h-1];
        arr[h-1] = piv;
        return h-1;
    }
    return low;

}

void quickSort(uint32_t * arr, uint32_t low, uint32_t high){
    if(high - low < 1){
        return;
    }
    uint32_t mid = partition(arr, low, high);
    if(mid>low){
        quickSort(arr, low,mid-1);
    }
    if(mid<high){
        quickSort(arr, mid+1,high);
    }
}

void ParallelSort(uint32_t *data, uint32_t n, int p)
{
    // Entry point to your sorting implementation.
    // Sorted array should be present at location pointed to by data.
    if(n<p*p){
        quickSort(data, 0, n-1);
        return;
    }

    uint32_t step = n/p;
    uint32_t ones = n%p;
    uint32_t starts[p];
    uint32_t i=0, j=0;
    do{
        starts[j] = i;
        j++;
        if(ones>0){
            ones--;
            i++;
        }
        i+=step;
    }
    while(i<n);

    uint32_t R[p*p]; //pseudo splitters
    uint32_t start, end;
    i=0;
    for (uint32_t k=0; k<p; k++){
        start = starts[k];
        end = starts[k] +p;
        for (uint32_t j=start; j<end; j++){
            R[i] = data[j];
            i++;
        }
    }
    
    quickSort(R, 0, p*p -1);
    uint32_t S[p-1]; // splitters

    for (uint32_t j=0; j<p-1; j++){
        S[j] = R[ (j+1)*p ];
    }

    ll * split[p];//to store the splitted data
    for (i=0; i<p;i++){
        split[i] = new ll();
    }

    for (uint32_t j=0;j<p;j++){
        #pragma omp task
        {
            if(j == 0){
                for (uint32_t k=0; k<n; k++){
                    if(data[k] <= S[0]){
                        split[0]->insert(data[k]);
                    }
                }
            }else if(0<j && j<p-1){
                for (uint32_t k=0; k<n; k++){
                    if( (S[j-1] < data[k]) && (data[k] <= S[j]) ){
                        split[j]->insert(data[k]);
                    }
                }
            }
            else if(j == (p-1)){
                for (uint32_t k=0; k<n; k++){
                    if( data[k] > S[p-2]){
                        split[p-1]->insert(data[k]);
                    }
                }
            }
        }
    }
    /*bool put = false;
    for (uint32_t k=0; k<n; k++){//splitting without tasks
        put = false;
        
        for (int j=0;j<p-1;j++){//can insert by binary search
            if(data[k]<= S[j]){
                split[j]->insert(data[k]);
                put = true;
                break;
            }
        }

        if(!put){
            split[p-1]->insert(data[k]);
        }
    }*/
    #pragma omp taskwait
    //copy to the data
    uint32_t splitIndex[p+1];
    splitIndex[0] = 0;
    for (uint32_t i=0; i<p; i++){
        splitIndex[i+1] = splitIndex[i] + split[i]->size;
    }


    
    for(uint32_t k=0; k<p; k++){
        #pragma omp task
        {
            uint32_t loc = splitIndex[k];
            node * temp = split[k]->head;
            while(temp){
                data[loc] = temp->val;
                loc++;
                temp = temp->next;
            }
        }
    }

    #pragma omp taskwait

    for(uint32_t k=0; k<p; k++){
        if(split[k]->size < 2*n/p){
            #pragma omp task
            quickSort(data, splitIndex[k], splitIndex[k+1]-1);
        }
        else{
            uint32_t N = split[k]->size;
            
            uint32_t * childData = &data[ splitIndex[k] ];
            #pragma omp task
            ParallelSort(childData, N, p);
        }
    }

    //split A into P partitons using S, tasks
    //the function is being called inside the parallel, single
    /*#pragma omp task
    {

    }*/
    
    
}