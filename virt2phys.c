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

// int* hex_to_bin(char* hexdec, int addressbit, int pgoffset) {
//     int in = 0;
//     int offset = 0;
//     int binary = 0;
//     int index = addressbit - pgoffset;
    
//     for(int i = 0; i < strlen(hexdec); i ++) {
//         int x = pow(10, (4*i));
//         if(hexdec[i] == '0'){
//             binary = binary + 0000 * x;
//         }
//         if(hexdec[i] == '1'){
//             binary = binary + 0001 * x;
//         }
//         if(hexdec[i] == '2'){
//             binary = binary + 0010 * x;
//         }
//         if(hexdec[i] == '3'){
//             binary = binary + 0011 * x;
//         }
//         if(hexdec[i] == '4'){
//             binary = binary + 0100 * x;
//         }
//         if(hexdec[i] == '5'){
//             binary = binary + 0101 * x;
//         }
//         if(hexdec[i] == '6'){
//             binary = binary + 0110 * x;
//         }
//         if(hexdec[i] == '7'){
//             binary = binary + 0111 * x;
//         }
//         if(hexdec[i] == '8'){
//             binary = binary + 1000 * x;
//         }
//         if(hexdec[i] == '9'){
//             binary = binary + 1001 * x;
//         }
//         if(hexdec[i] == 'A' || hexdec[i] == 'a'){
//             binary = binary + 1010 * x;
//         }
//         if(hexdec[i] == 'B' || hexdec[i] == 'b'){
//             binary = binary + 1011 * x;
//         }
//         if(hexdec[i] == 'C' || hexdec[i] == 'c'){
//             binary = binary + 1100 * x;
//         }
//         if(hexdec[i] == 'D' || hexdec[i] == 'd'){
//             binary = binary + 1101 * x;
//         }
//         if(hexdec[i] == 'E' || hexdec[i] == 'e'){
//             binary = binary + 1110 * x;
//         }
//         if(hexdec[i] == 'F' || hexdec[i] == 'f'){
//             binary = binary + 1111 * x;
//         }
//     }
//     int len = floor(log10(abs(binary))) + 1;
//     if(len < addressbit){
//         for(int i = 0; i < (addressbit-len); i++){
//             int x = pow(10, len + i);
//             binary = binary + 0 * x;
//             printf("%d\n", binary);
//         }
//     }
//     printf("%d\n", len);
//     int* bin = (int*) malloc (3 * sizeof(int));
//     *(bin) = (int)binary;
//     *(bin + 1) = (int)in;
//     *(bin + 2) = (int)offset;
//     printf("%d\n", bin[0]);
//     return bin;
// }

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
    int N = pow(2, addbit)/pgsize;
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

