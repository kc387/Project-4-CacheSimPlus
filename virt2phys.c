// No starter code here
// At this point in ECE/CS 250, you should be able to write a basic C I/O program from scratch.

// TODO: Everything
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

int main(int argc, char* argv[]){
    if (argc != 3)
    {	
    	return 0;
    }
    
    // load in information from execute file
    char v_address[] = argv[2];
    FILE* pg_tbl_form  = fopen(argv[1], "r"); // page table format

    // convert v_address from hex to binary
    unsigned v_add_bin = hex_to_bin(v_address);
    // scan first row for address bit and pg size
    int addbit, pgsize;
    fscanf(pg_tbl_form, "%d %d", &addbit, &pgsize); 
    
    // calculate pg offset
    int pgoffset = log2(pgsize);

    // split v_address into "index" bit and offset bits
    

    // define variables needed to save ppn of virtual page
    int xppn;
    int N = 2^addbit/pgsize;
    int ppn[N];
    int count = 0;
    
    // save ppn of virtual page of N-1
    while(count < N) {
        fscanf(pg_tbl_form, "%d", &xppn);
        ppn[count] = xppn;
        count ++;
    }

    // take index of v_address and find corresponding ppn & convert ppn into binary


    // add ppn and offset together for binary value and then convert to hex


    // print hex value or PAGEFAULT + \n


    return 0;
}

int log2(int n) {
    int r=0;
    while (n>>=1) r++;
    return r; 
}

unsigned hex_to_bin(char hex) {
    unsigned bin = 0;

    return bin;
}