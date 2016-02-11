/*
* diskput.c
* File System Implementation
* Part4 move a file from current directory in Linux to test.img
*
* @Chenkai Hao V00819367
* July 22th, 2015
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>  
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <time.h>

char *pointer;

int size;
int start;
int block;
long byte;

int FDT_start;
int FDT_block;
int FAT_start;

void info_update(FILE *disk, char *new_file, int status, int flag) {
    
    	
    	int startBlock = htonl(start);
    	int blockNum = htonl(block);
    	int fileSize = htonl(byte);
    	int dirSize = htonl(byte+64);
    	unsigned long unused = 0xFFFFFFFFFFFF;
    	unsigned int file_len = strlen(new_file);
    	unsigned int remain = 31 - file_len;
    	int i;
    	int record_block;
    	int record_block_num;
    	unsigned long clear = 0x000000000000;
	unsigned long clear2 = 0x00;
    	
    	if (flag == 0) {
    	    	record_block = FDT_start;
    	    	record_block_num = FDT_block;
    	} else {
    	    	record_block = flag;
    	    	record_block_num = 1;
    	}
    	
    	i = size*record_block;
    	
    	while (i<size*(record_block+record_block_num)) {
    	    	if(pointer[i] == 0) {
    	    	    	break;
    	    	} else {
    	    	    	i += 64;
    	    	}
    	}
    	
    	if(i>=size*(record_block+record_block_num)) {
    	    	printf("No free directory entry\n");
    	    	return;
    	}
    	
    	//update file info
    	fseek(disk, i, SEEK_SET);
    	fwrite(&status, 1, 1, disk);
    	fwrite(&startBlock, 1, 4, disk);
    	fwrite(&blockNum, 1, 4, disk);
    	if(status == 0x03) {
    	    	fwrite(&fileSize, 1, 4, disk);
    	} else {
    	    	fwrite(&dirSize, 1, 4, disk);
    	}
    	
    	// Create time (now)
	time_t ct;
	time(&ct);
	struct tm *ctime;

	// Get create time in local time
	ctime = localtime(&ct);

	unsigned int cyear = (ctime->tm_year)+1900;
	unsigned int cmonth = (ctime->tm_mon) + 1;
	unsigned int cday = (ctime->tm_mday);
	unsigned int chour = (ctime->tm_hour);
	unsigned int cmin = (ctime->tm_min);
	unsigned int csec = (ctime->tm_sec);

	unsigned int ccyear = (cyear>>8) | (cyear<<8);

	// fwrite modify time here
	fwrite(&ccyear, 1, 2, disk);
	fwrite(&cmonth, 1, 1, disk);
	fwrite(&cday, 1, 1, disk);
	fwrite(&chour, 1, 1, disk);
	fwrite(&cmin, 1, 1, disk);
	fwrite(&csec, 1, 1, disk);
	
	time_t mt;
	time(&mt);
	struct tm *mtime;
	
	mtime = localtime(&mt);

	unsigned int myear = (mtime->tm_year)+1900;
	unsigned int mmonth = (mtime->tm_mon) + 1;
	unsigned int mday = (mtime->tm_mday);
	unsigned int mhour = (mtime->tm_hour);
	unsigned int mmin = (mtime->tm_min);
	unsigned int msec = (mtime->tm_sec);

	unsigned int mmyear = (myear>>8) | (myear<<8);

	// update the time information
	fwrite(&mmyear, 1, 2, disk);
	fwrite(&mmonth, 1, 1, disk);
	fwrite(&mday, 1, 1, disk);
	fwrite(&mhour, 1, 1, disk);
	fwrite(&mmin, 1, 1, disk);
	fwrite(&msec, 1, 1, disk);
    	
	fwrite(&clear, 1, 6, disk);
	fwrite(&clear, 1, 6, disk);
	fwrite(&clear, 1, 6, disk);
	fwrite(&clear, 1, 6, disk);
	fwrite(&clear, 1, 6, disk);
	fwrite(&clear2, 1, 1, disk);
	fseek(disk, -31, SEEK_CUR);
	
	//update the file name
	fwrite(new_file, 1, file_len, disk);
        fseek(disk, remain, SEEK_CUR);
        
        //update unused sectors
        fwrite(&unused, 1, 6, disk);
}

void disk_put(FILE *disk, char *filename) {
    
    	FILE *fp = fopen(filename, "rb");
    	int count = 0;
    	size = (pointer[8]<<8) + pointer[9];
    	
    	unsigned int prev;
    	unsigned int i;
    	int j;
    	char temp;
    	unsigned int index;
    	unsigned long last = 0xFFFFFFFF;
    	
    	if (fp == NULL) {
    	    	printf("File not found\n");
    	    	return;
    	}
    	
    	fseek(fp, 0, SEEK_END);
    	byte = ftell(fp);
	if(byte%size == 0) {
		block = byte/size;
	} else {
		block = (byte/size)+1;
	}
    	i = FAT_start*size;
    	
    	fseek(fp, 0, SEEK_SET);
    	fseek(disk, 0, SEEK_SET);
    	
    	while(count<block) {
    	    	if(pointer[i]+pointer[i+1]+pointer[i+2]+pointer[i+3] == 0) {
    	    	    	if(count == 0) {
    	    	    	    	start = (i-FAT_start*size)/4;
    	    	    	} else {
    	    	    	    	fseek(disk, prev, SEEK_SET);
    	    	    	    	index = htonl((i-FAT_start*size)/4);
    	    	    	    	fwrite(&index, 1, 4, disk);
    	    	    	}
    	    	    	fseek(disk, size*((i-FAT_start*size)/4), SEEK_SET);
    	    	    	for(j=0; j<size; j++) {
    	    	    	    	temp = getc(fp);
    	    	    	    	fwrite(&temp, 1, 1, disk);
    	    	    	}
    	    	    	prev = i;
    	    	    	count ++;
    	    	}
    	    	i += 4;
    	}

    	fseek(disk, i-4, SEEK_SET);
    	fwrite(&last, 1, 4, disk);
    	fseek(disk, i-4, SEEK_SET);
    	fwrite(&last, 1, 4, disk);
    	fclose(fp);
}

int getSuperBlockinfo(char * mmap, int offset, int length)
{
        char *tmp = (char *)malloc(sizeof(char) * length);
        tmp = memcpy(tmp, mmap+offset, length);

        int i = 0;
        int retVal = 0;

        for(i=0; i < length; i++){
                retVal += ((int)tmp[i]<<(8*(length - i - 1)));
        }

        free(tmp);
        return retVal;
}

void find_directory(char *p, FILE *disk, char *filename, char *file_name) {
    
    	char* dir = 0;
    	char* file = 0;
    	char* pch = strtok (filename, "/");
    	
    	
    	int i = size*FDT_start;
    	int j = size*FAT_start;
    	unsigned int dir_block = 0;
    	unsigned long end = 0xFFFFFFFF;
    	int dirSize;
    	char *fileName;
    	
    	dir = pch;
    	if (pch != NULL) {
    	    	pch = strtok (NULL, "/");
     	 	file = pch;
     	}
     	
     	while (i<size*(FDT_start+FDT_block)) {
	    	fileName = p+i+27;
	    	if(strncmp(fileName, dir, 31) == 0) {
	    	    	dir_block = getSuperBlockinfo(p,i+1,4);
	    	    	break;
	    	} else {
	    	    	i += 64;
	    	}
	}
	
     	if(dir_block == 0) {
     	    	disk_put(disk, file_name);
     	    	
     	    	while (getSuperBlockinfo(p, j, 4) != 0) {
     	    	    	j += 4;
     	    	}
     	    	dir_block = (j-size*FAT_start)/4;
     	    	fseek(disk, j, SEEK_SET);
     	    	fwrite(&end, 1, 4, disk);
     	    	info_update(disk, file, 0x03, dir_block);
     	    	start = dir_block;
     	    	block = 1;
     	    	info_update(disk, dir, 0x05, 0);
     	} else {
     	    	disk_put(disk, file_name);
     	    	
     	    	info_update(disk, file, 0x03, dir_block);
     	    	
     	    	fseek(disk, i+9, SEEK_SET);
     	    	dirSize = htonl(byte+64+getSuperBlockinfo(p, i+9, 4));
     	    	fwrite(&dirSize, 1, 4, disk);
     	    	
     	    	fseek(disk, i+20, SEEK_SET);
     	    	time_t mt;
     	    	time(&mt);
     	    	struct tm *mtime;
	
     	    	// Get modify time in local time
     	    	mtime = localtime(&mt);

     	    	unsigned int myear = (mtime->tm_year)+1900;
     	    	unsigned int mmonth = (mtime->tm_mon) + 1;
     	    	unsigned int mday = (mtime->tm_mday);
     	    	unsigned int mhour = (mtime->tm_hour);
     	    	unsigned int mmin = (mtime->tm_min);
     	    	unsigned int msec = (mtime->tm_sec);

     	    	unsigned int mmyear = (myear>>8) | (myear<<8);

     	    	// fwrite modify time here
     	    	fwrite(&mmyear, 1, 2, disk);
     	    	fwrite(&mmonth, 1, 1, disk);
     	    	fwrite(&mday, 1, 1, disk);
     	    	fwrite(&mhour, 1, 1, disk);
     	    	fwrite(&mmin, 1, 1, disk);
     	    	fwrite(&msec, 1, 1, disk);
     	}
}

int main(int argc, char *argv[]){
	//[0]./disklist [1]test.img [2]foo.txt [3]/sub_dir/foo2.txt

	int fd;
	char *p;
	FILE *disk;
	struct stat sf;
	char c;
	
	
	if(argc!=4){
		printf("Usg: ./diskget test.img source.* /sub_dir/destination.*\n");
		return 0;
	}
	
	
	
	//printf("%s\n", file_name);
	if ((fd=open(argv[1], O_RDONLY)))
	{
		
		fstat(fd, &sf);
		p = mmap(NULL,sf.st_size, PROT_READ, MAP_SHARED, fd, 0);

		disk = fopen(argv[1],"r+");
		pointer = p;

		FDT_start = getSuperBlockinfo(p,22, 4);
    	FDT_block =getSuperBlockinfo(p,26, 4);
     	FAT_start = getSuperBlockinfo(p,14, 4);
    	size = getSuperBlockinfo(p,8, 2);

		c = *argv[3];
		if(c == '/'){
			find_directory(p, disk, argv[3], argv[2]);
		}else{
			disk_put(disk, argv[2]);
			info_update(disk, argv[3], 0x03, 0);
		}
		

		

		printf("\nSuccessfully copy the file to disk image.\n");
	}
	else
		printf("Fail to open the image file.\n");
	close(fd);
	return 0;
}
