#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "memory.h"

// Note: You won't find <YOUR CODE HERE> comments in this file.
// This is to get you to read and understand everything here.
// Good luck!

// - Anshu

typedef struct cache {
	int valid;
	int tag;
	unsigned char* data;
	int dirty;
	int time;
} cache;

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

int main (int argc, char* argv[]) {
	init_memory();
	int cacheSize, associativity, blockSize;

    // Buffer to store instruction (i.e. "load" or "store")
	char instruction_buffer[6];

    // Open the trace file in read modea
	FILE* myFile = fopen(argv[1], "r");

    // Read in the command line arguments
	sscanf(argv[2], "%d", &cacheSize);
	sscanf(argv[3], "%d", &associativity);
	sscanf(argv[4], "%d", &blockSize);

	int sets = (cacheSize * x_to_the_n(2, 10))/blockSize/associativity;
	int num_bit_offset = l2(blockSize);
	int num_bit_index = l2(sets);
	int num_bit_tag = 24 - num_bit_offset - num_bit_index;
	
	// make cache
	struct cache ***mycache = (cache***) malloc (sizeof(cache**) * sets);
	for(int i = 0; i < sets; i ++){
		mycache[i] = (cache**) malloc (sizeof(cache*) * associativity);
	}
	for(int i = 0; i < sets; i ++){
		for(int j = 0; j < associativity; j ++) {
			mycache[i][j] = (cache*) malloc (sizeof(cache));
			mycache[i][j] -> dirty = 0;
			mycache[i][j] -> tag = -1;
			mycache[i][j] -> time = 0;
			mycache[i][j] -> valid = 0;
			mycache[i][j] -> data = (unsigned char*) malloc(sizeof(unsigned char) * blockSize);
		}
	}

    // Keep reading the instruction until end of file
	while(fscanf(myFile,"%s", &instruction_buffer)!=EOF) {
		int lru = 0;
		int currAddress, accessSize;

        // Read the address and access size info
		fscanf(myFile, "%x", &currAddress);
		fscanf(myFile, "%d", &accessSize);

		// split current address into tag, index, and offset
		int offset = (currAddress << (num_bit_tag + num_bit_index)) >> (num_bit_tag + num_bit_index);
		int index = (currAddress << num_bit_tag) >> (num_bit_tag + num_bit_offset);
		int tag = currAddress >> (num_bit_index + num_bit_offset);
		
		if (instruction_buffer[0]=='l'){    // If load
			printf("load 0x%x", currAddress);
			int hit = 0;
            // check index & tag
			for(int i = 0; i < associativity; i ++){
				if(mycache[index][i] -> tag == tag){
					printf("hit");
					for(int j = offset; j < (offset + accessSize); j ++){
						printf("%02hhx", mycache[index][i] -> data[j]);
					}
					lru ++;
					mycache[index][i] -> time = lru;
					hit = 1;
					break;
				}
			}
			if (hit != 1) {
				printf("miss");
				for(int i = 0; i < associativity; i ++){
					if(mycache[index][i] -> valid == 0){
						mycache[index][i] -> valid = 1;
						unsigned char* buff = (unsigned char*) malloc(sizeof(unsigned char) * blockSize);
						read_from_memory(buff, (currAddress-offset), blockSize);
						for(int j = offset; j < (offset + accessSize); j ++){
							printf("%02hhx", mycache[index][i] -> data[j]);
						}
						lru ++;
						mycache[index][i] -> time = lru;
						break;
					}
				}
				int lowest;
				int ilowest = 0;
				for(int i = 0; i < associativity; i ++){
					if(i == 0){
						lowest = mycache[index][i] -> time;
					}
					else{
						if(lowest > mycache[index][i] -> time){
							lowest = mycache[index][i] -> time;
							ilowest = i;
						}
					}	
				}
				
				if(mycache[index][ilowest] -> valid == 1){
					unsigned char* buff = (unsigned char*) malloc(sizeof(unsigned char) * blockSize);
					write_to_memory(buff, (currAddress-offset), blockSize);
				}

				unsigned char* buff = (unsigned char*) malloc(sizeof(unsigned char) * blockSize);
				read_from_memory(buff, (currAddress-offset), blockSize);

				for(int j = offset; j < (offset + accessSize); j ++){
					printf("%02hhx", mycache[index][ilowest] -> data[j]);
				}

				lru ++;
				mycache[index][ilowest] -> time = lru;
			}
		}
        else {                              // Else store
            // Buffer to store data to be stored
            //char data_buffer[16];
			char data_buffer[16];

            // Read the data
            fscanf(myFile, "%s", &data_buffer);

            // Print the store line in the same format as trace file
            printf("store 0x%x %d %s\n", currAddress, accessSize, data_buffer);
		}
	}

	destroy_memory();
	return EXIT_SUCCESS;
}
