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

	int lru = 0;

    // Keep reading the instruction until end of file
	while(fscanf(myFile,"%s", &instruction_buffer)!=EOF) {
		int currAddress, accessSize;

        // Read the address and access size info
		fscanf(myFile, "%x", &currAddress);
		fscanf(myFile, "%d", &accessSize);

		// split current address into tag, index, and offset
		int cA = currAddress;
		int imask = x_to_the_n(2, 24) - 1; 
		int omask = imask >> (num_bit_tag + num_bit_index);
		int offset = cA & omask;

		cA = currAddress;
		int cAind = cA >> num_bit_offset;
		int inmask = imask >> (num_bit_tag + num_bit_offset);
		int index = cAind & inmask;
		
		cA = currAddress;
		int tag = cA >> (num_bit_index + num_bit_offset);

		//printf("TI: %d %d\n", tag, index);

		if (instruction_buffer[0]=='l'){    // If load
			printf("load 0x%x ", currAddress);
			
			int hit = 0;
			int cmiss = 0;
            // check index & tag
			for(int i = 0; i < associativity; i ++){
				if(mycache[index][i] -> tag == tag){
					printf("hit ");
					unsigned char* d = mycache[index][i] -> data;
					for(int j = offset; j < (offset + accessSize); j ++){
						printf("%02hhx", d[j]);
					}
					lru ++;
					mycache[index][i] -> time = lru;
					hit = 1;
					mycache[index][i] -> dirty = 0;
					printf("\n");
					break;
				}
			}
			if (hit != 1) {
				printf("miss ");
				for(int i = 0; i < associativity; i ++){
					if(mycache[index][i] -> valid == 0){
						mycache[index][i] -> valid = 1;
						unsigned char* d = mycache[index][i] -> data;
						unsigned char* buff = (unsigned char*) malloc(sizeof(unsigned char) * blockSize);
						read_from_memory(buff, (currAddress-offset), blockSize);
						mycache[index][i] -> data = buff;
						for(int j = offset; j < (offset + accessSize); j ++){
							printf("%02hhx", d[j]);
						}
						mycache[index][i] -> tag = tag;
						lru ++;
						mycache[index][i] -> time = lru;
						printf("\n");
						cmiss = 1;
						mycache[index][i] -> dirty = 0;
						break;
					}
				}
				if (cmiss != 1){
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
					
					if(mycache[index][ilowest] -> dirty == 1){
						int tagind = (currAddress >> offset) << offset;
						write_to_memory(mycache[index][ilowest] -> data, tagind, blockSize);
					}

					unsigned char* buff = (unsigned char*) malloc(sizeof(unsigned char) * blockSize);
					read_from_memory(buff, (currAddress-offset), blockSize);
					mycache[index][ilowest] -> data = buff;
					for(int j = offset; j < (offset + accessSize); j ++){
						printf("%02hhx", mycache[index][ilowest] -> data[j]);
					}
					printf("\n");
					mycache[index][ilowest] -> tag = tag;
					lru ++;
					mycache[index][ilowest] -> time = lru;
					mycache[index][ilowest] -> dirty = 0;
				}
			}
			hit = 0;
			cmiss = 0;
		}
        else {                              // Else store
            // Buffer to store data to be stored
            //char data_buffer[16];
			unsigned char* data_buffer = (unsigned char*) malloc(sizeof(unsigned char) * blockSize);

            // Read the data
			for(int i = 0; i < accessSize; i ++){
            	fscanf(myFile, "%02hhx", data_buffer + i);
			}

            printf("store 0x%x ", currAddress);
			
			int hit = 0;
			int cmiss = 0;
            // check index & tag
			for(int i = 0; i < associativity; i ++){
				// hit
				if(mycache[index][i] -> tag == tag){
					printf("hit");
					for(int j = offset; j < (offset + accessSize); j ++){
						mycache[index][i] -> data[j] = data_buffer[j-offset];
					}
					lru ++;
					mycache[index][i] -> time = lru;
					hit = 1;
					mycache[index][i] -> dirty = 1;
					printf("\n");
					break;
				}
			}
			if (hit != 1) {
				printf("miss ");
				for(int i = 0; i < associativity; i ++){
					if(mycache[index][i] -> valid == 0){
						mycache[index][i] -> valid = 1;
						unsigned char* buff = (unsigned char*) malloc(sizeof(unsigned char) * blockSize);
						read_from_memory(buff, (currAddress-offset), blockSize);
						for(int j = offset; j < (offset + accessSize); j ++){
							mycache[index][i] -> data[j] = data_buffer[j-offset];
						}
						mycache[index][i] -> tag = tag;
						lru ++;
						mycache[index][i] -> time = lru;
						printf("\n");
						cmiss = 1;
						mycache[index][i] -> dirty = 1;
						break;
					}
				}
				if (cmiss != 1){
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
					
					if(mycache[index][ilowest] -> dirty == 1){
						int tagind = (currAddress >> offset) << offset;
						write_to_memory(mycache[index][ilowest] -> data, tagind, blockSize);
					}
					unsigned char* buff = (unsigned char*) malloc(sizeof(unsigned char) * blockSize);
					read_from_memory(buff, (currAddress-offset), blockSize);
					mycache[index][ilowest] -> tag = tag;
					for(int j = offset; j < (offset + accessSize); j ++){
						mycache[index][ilowest] -> data[j] = data_buffer[j-offset];
					}
					printf("\n");
					lru ++;
					mycache[index][ilowest] -> time = lru;
					mycache[index][ilowest] -> dirty = 1;
				}
			}
			hit = 0;
			cmiss = 0;
		}
	}

	destroy_memory();
	return EXIT_SUCCESS;
}
