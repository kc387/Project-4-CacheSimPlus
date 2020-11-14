// No starter code here
// You've already written virt2phys.c and cachesim.c
// You have all the skills required to put them together.

// TODO: Everything
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "memory.h"

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
	FILE* myFile = fopen(argv[2], "r");

    // Read in the command line arguments
	sscanf(argv[3], "%d", &cacheSize);
	sscanf(argv[4], "%d", &associativity);
	sscanf(argv[5], "%d", &blockSize);

	int sets = (cacheSize * x_to_the_n(2, 10))/blockSize/associativity;
	int num_bit_offset = l2(blockSize);
	int num_bit_index = l2(sets);
	int num_bit_tag = 24 - num_bit_offset - num_bit_index;
	//printf("TIO bits: %d %d %d\n", num_bit_tag, num_bit_index, num_bit_offset);

	

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
		int v_address, accessSize;

        // Read the address and access size info
		fscanf(myFile, "%x", &v_address);
		fscanf(myFile, "%d", &accessSize);

        // load in information from execute file
        FILE* pg_tbl_form  = fopen(argv[1], "r"); // page table format

        // scan first row for address bit and pg size
        int addbit, pgsize;
        fscanf(pg_tbl_form, "%d %d", &addbit, &pgsize); 
        
        // calculate pg offset
        int pgoffset = l2(pgsize);
        // split v_address into "index" bit and offset bits
        int vindex = v_address >> pgoffset;
        int ifull = vindex << pgoffset;
        int voffset = v_address - ifull;

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
        int ppn = vpn[vindex];
        fclose(pg_tbl_form);

        // add ppn and offset together for binary value and then convert to hex & print
        if(ppn == -1){
            printf("PAGEFAULT\n");
            if(instruction_buffer[0] != 'l'){
                int da;
                fscanf(myFile, "%x", &da);
            }
        }
        else{
            int ppnfull = ppn << pgoffset;
            int currAddress = ppnfull + voffset;
            //printf("phys: %x\n", currAddress);
        

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
    
            //printf("I: %d", index);

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
                            mycache[index][i] -> data = buff;
                            unsigned char* d = mycache[index][i] -> data;
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
                        int lowest = mycache[index][0] -> time;
                        int ilowest = 0;
                        for(int i = 0; i < associativity; i ++){
                            if(mycache[index][i] -> time < lowest){
                                lowest = mycache[index][i] -> time;
                                ilowest = i;
                            }	
                        }
                        //printf("ilowest: %d", ilowest);
                        
                        //("d: %d", mycache[index][ilowest] -> dirty);
                        if(mycache[index][ilowest] -> dirty == 1){
                            //printf("dirtyisone");
                            int tg = mycache[index][ilowest] -> tag << (24 - num_bit_tag);
                            int in = index << num_bit_offset;
                            int tagind = tg + in;
                            write_to_memory(mycache[index][ilowest] -> data, tagind, blockSize);
                            //printf("writemem");
                        }
                        unsigned char* buff = (unsigned char*) malloc(sizeof(unsigned char) * blockSize);
                        read_from_memory(buff, (currAddress-offset), blockSize);
                        // for(int j = offset; j < (offset + accessSize); j ++){
                        // 	printf("buff %02hhx", buff[j]);
                        // }
                        mycache[index][ilowest] -> data = buff;
                        unsigned char* d = mycache[index][ilowest] -> data;
                        for(int j = offset; j < (offset + accessSize); j ++){
                            printf("%02hhx", d[j]);
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
                unsigned char* data_buffer = (unsigned char*) malloc(sizeof(unsigned char) * accessSize);

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
                        int k = 0;
                        for(int j = offset; j < (offset + accessSize); j ++){
                            mycache[index][i] -> data[j] = data_buffer[k];
                            k ++;
                        }
                        lru ++;
                        mycache[index][i] -> time = lru;
                        hit = 1;
                        mycache[index][i] -> dirty = 1;
                        //printf("dirty %d", mycache[index][i] -> dirty);
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
                            // for(int j = offset; j < (offset + accessSize); j ++){
                            // 	printf("buff %02hhx", buff[j]);
                            // }
                            mycache[index][i] -> data = buff;

                            mycache[index][i] -> tag = tag;
                            lru ++;
                            mycache[index][i] -> time = lru;
                            printf("\n");
                            cmiss = 1;
                            mycache[index][i] -> dirty = 1;
                            //printf("dirty %d", mycache[index][i] -> dirty);
                            int k = 0;
                            for(int j = offset; j < (offset + accessSize); j ++){
                                mycache[index][i] -> data[j] = data_buffer[k];
                                k ++;
                            }
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
                        
                        //printf("d: %d", mycache[index][ilowest] -> dirty);
                        if(mycache[index][ilowest] -> dirty == 1){
                            //printf("dirtyisone");
                            int tg = mycache[index][ilowest] -> tag << (24 - num_bit_tag);
                            int in = index << num_bit_offset;
                            int tagind = tg + in;
                            write_to_memory(mycache[index][ilowest] -> data, tagind, blockSize);
                            unsigned char* d = mycache[index][ilowest] -> data;
                            //printf("writemem");
                        }
                        unsigned char* buff = (unsigned char*) malloc(sizeof(unsigned char) * blockSize);
                        read_from_memory(buff, (currAddress-offset), blockSize);
                        // for(int j = offset; j < (offset + accessSize); j ++){
                        // 	printf("buff %02hhx", buff[j]);
                        // }
                        mycache[index][ilowest] -> data = buff;
                        mycache[index][ilowest] -> tag = tag;
                        int k = 0;
                        for(int j = offset; j < (offset + accessSize); j ++){
                            mycache[index][ilowest] -> data[j] = data_buffer[k];
                            //printf("data: %02hhx", mycache[index][ilowest] -> data[j]);
                            k ++;
                        }
                        printf("\n");
                        lru ++;
                        mycache[index][ilowest] -> time = lru;
                        mycache[index][ilowest] -> dirty = 1;
                        //printf("dirty %d", mycache[index][ilowest] -> dirty);
                        /*unsigned char* d = mycache[index][ilowest] -> data;
                        for(int j = offset; j < (offset + accessSize); j ++){
                            printf("data: %02hhx\n", d[j]);
                        }*/
                    }
                }
                hit = 0;
                cmiss = 0;
            }
        }
	}

	destroy_memory();
	return EXIT_SUCCESS;
}
