// No starter code here
// At this point in ECE/CS 250, you should be able to write a basic C I/O program from scratch.

// TODO: Everything
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

int l2(int n) {
    int r=0;
    while (n>>=1) r++;
    return r; 
}

int x_to_the_n (int x,int n)
{
    int i; /* Variable used in loop counter */
    int number = 1;

    for (i = 0; i < n; ++i)
        number *= x;

    return(number);
}

int main(int argc, char* argv[]){
    if (argc != 3)
    {	
    	return 0;
    }
    
    // load in information from execute file
    char * stopstring;
    char * strvadd = argv[2];
    unsigned int v_address = strtoul(strvadd, &stopstring, 16);
    FILE* pg_tbl_form  = fopen(argv[1], "r"); // page table format


    // scan first row for address bit and pg size
    int addbit, pgsize;
    fscanf(pg_tbl_form, "%d %d", &addbit, &pgsize); 
    
    // calculate pg offset
    int pgoffset = l2(pgsize);
    // split v_address into "index" bit and offset bits
    int index = v_address >> pgoffset;
    int ifull = index << pgoffset;
    int offset = v_address - ifull;

    // define variables needed to save ppn of virtual page
    int xvpn;
    int N = x_to_the_n(2, addbit)/pgsize;
    int vpn[N];
    int count = 0;
    
    // save ppn of virtual page of N-1
    while(count < N) {
        fscanf(pg_tbl_form, "%d", &xvpn);
        vpn[count] = xvpn;
        count ++;
    }

    // take index of v_address and find corresponding ppn & convert ppn into binary
    int ppn = vpn[index];

    // add ppn and offset together for binary value and then convert to hex & print
    if(ppn == -1){
        char z[100] = "PAGEFAULT";
        printf("%s\n", z);
    }
    else{
        int ppnfull = ppn << pgoffset;
        int padd = ppnfull + offset;
        printf("%x\n", padd);
    }

    return 0;
}

