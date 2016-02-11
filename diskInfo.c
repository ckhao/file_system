/*
* diskInfo.c
* File System Implementation
* Part1 super block + FAT information
*
* @Chenkai Hao V00819367
* July 16th, 2015
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>


int blockSize;
int blockCount;
int fatStart;
int fatBlock;
int rootStart;
int rootBlock;

void retriveBlockInfo(char* ptr){
	blockSize = (ptr[8]<<8) + (ptr[9]);
	blockCount = (ptr[10]<<24) + (ptr[11]<<16) + (ptr[12]<<8) + ptr[13];
	fatStart = (ptr[14]<<24) + (ptr[15]<<16) + (ptr[16]<<8) + (ptr[17]);
	fatBlock = (ptr[18]<<24) + (ptr[19]<<16) + (ptr[20]<<8) + (ptr[21]);
	rootStart = (ptr[22]<<24) + (ptr[23]<<16) + (ptr[24]<<8) + (ptr[25]);
	rootBlock = (ptr[26]<<24) + (ptr[27]<<16) + (ptr[28]<<8) + (ptr[29]);
}

int main(int argc, char *argv[]){

	int fd; 
    struct stat sf;
    //Open file
    fd = open(argv[1], O_RDONLY, 0);
	
	if(-1 == fd) 
    { 
        printf("\n NULL File descriptor\n"); 
        return -1; 
    } 
	
    fstat(fd, &sf);
	int errno = 0; 
	if(fstat(fd, &sf)) 
    { 
        printf("\nfstat error: [%s]\n",strerror(errno)); 
        close(fd); 
        return -1; 
    } 
	
    //Execute mmap
    void* mmappedData = mmap(NULL, sf.st_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);

	char *ptr = mmappedData;
	//int i;
	//for(i=8; i<9; i++){
	//printf("%c ",ptr[8]<<8+ptr[9]);
	//}
	
	retriveBlockInfo(ptr);
	
	
	printf("\nSuper block information:\n");
	printf("Block size: %d\n", blockSize);
	printf("Block count: %d\n", blockCount);
	printf("FAT starts: %d\n", fatStart);
	printf("FAT blocks: %d\n", fatBlock);
	printf("Root directory start: %d\n", rootStart);
	printf("Root directory blocks: %d\n", rootBlock);
	
	
	
	int lowerBound = fatStart * blockSize;//1024
	int upperBound = lowerBound + fatBlock * blockSize;//26624
	//printf("%d %d", lowerBound, upperBound);
	
	int free = 0, reserved = 0, allocated = 0;
	int i;
	for(i=lowerBound; i<=upperBound-3; i+=4){
		int entry = (ptr[i]<<24) + (ptr[i+1]<<16) + (ptr[i+2]<<8) + (ptr[i+3]);
		if(entry == 0){
			free++;
		}else if(entry == 1){
			reserved++;
		}else{
			allocated++;
		}
		
	}
	
	printf("\nFAT information:\n");
	//printf("FAT first entry is %d\n", (ptr[1024]<<24) + (ptr[1025]<<16) + (ptr[1026]<<8) + (ptr[1027]));
	printf("Free Blocks: %d\n", free);
	printf("Reserved Blocks: %d\n", reserved);
	printf("Allocated Blocks: %d\n", allocated);



	
}



