#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "memory.h"

// Note: You won't find <YOUR CODE HERE> comments in this file.
// This is to get you to read and understand everything here.
// Good luck!

// - Anshu

int main (int argc, char* argv[]) {
	init_memory();
	int cacheSize, associativity, blockSize;

    // Buffer to store instruction (i.e. "load" or "store")
	char instruction_buffer[6];

    // Open the trace file in read mode
	FILE* myFile = fopen(argv[1], "r");

    // Read in the command line arguments
	sscanf(argv[2], "%d", &cacheSize);
	sscanf(argv[3], "%d", &associativity);
	sscanf(argv[4], "%d", &blockSize);

    // Keep reading the instruction until end of file
	while(fscanf(myFile,"%s", &instruction_buffer)!=EOF) {
		int currAddress, accessSize;

        // Read the address and access size info
		fscanf(myFile, "%x", &currAddress);
		fscanf(myFile, "%d", &accessSize);
		
		if (instruction_buffer[0]=='l'){    // If load
            // Print the load line in the same format as trace file
			printf("load 0x%x %d\n", currAddress, accessSize);
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
