/*
* disklist.c
* File System Implementation
* Part2 file list in root directory
*
* @Chenkai Hao V00819367
* July 20th, 2015
*/


#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<sys/mman.h>
#include<fcntl.h>  
#include<string.h>
#include<stdint.h>


#define FILESIZE 4
//-----------------------------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------------------------
void getFileInfo(char * mmap, int offset, int length, int numRootblock)  //getFileInfo(p,53*512,64,8)
{
	int i,j,k = 0;
	// 64B: size of each directory entry, length=64
	
	char *root_entry = (char *)malloc(sizeof(char) * length);     
	char *file_name_bytes = (char *)malloc(sizeof(char) * 31);
	unsigned char *file_size_bytes = (unsigned char *)malloc(sizeof(unsigned char) * FILESIZE);  
	
	//char *mod_time_year = (char *)malloc(sizeof(char) * 2);
	//char *mod_time_month = (char *)malloc(sizeof(char) * 1);
	
	int year, month, day;
	unsigned int hour, min, sec;

	// FILESIZE in each directory entry is 4 Bytes.
	int file_size = 0;
	for (i=0; i < numRootblock; i ++)
	{	
		for (j = 0; j < 8; j ++){    // 8 directory entries per 512B blcok 
			root_entry = memcpy(root_entry, mmap+offset+512*i+length*j, length);
			// check the status of the entry (is it s file?)  2: File / 4: Directory
			if ((root_entry[0] & 0x02) == 0x02){
				printf("F ");
				
				// FILE SIZE is placed 9B after the beginning of each entry (4B)
				file_size_bytes = memcpy(file_size_bytes, root_entry + 9, 4);  
				file_size = 0;
				for(k=0; k < FILESIZE; k++){
					file_size += ((int)file_size_bytes[k]<<(8*(-k + FILESIZE - 1)));
				}
				printf("%d ", file_size);

				// FILE NAME is placed 27B after the beginning of each entry (31B)
				file_name_bytes = memcpy(file_name_bytes, root_entry + 27, 31);
				printf("%s\t", file_name_bytes);

				//mod_time_year = memcpy(mod_time_year, root_entry + 58, 2);
				//printf("%d \n", mod_time_year);
				year = (root_entry[20]<<8)+(root_entry[21])+256;
				printf("%d/", year);
				month = root_entry[22];
				printf("%d/", month);
				day = root_entry[23];
				printf("%d/ ", day);
				hour = root_entry[24];
				printf("%d:", hour);
				min = root_entry[25];
				printf("%2u:", min);
				sec = root_entry[26];
				printf("%2u\n", sec);

			}
		}
	}
	free(root_entry);
	free(file_name_bytes);
	free(file_size_bytes);
}
//-----------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	int fd;
	struct stat sf;
	char *p;

	if(argc!=2){
		printf("Usg: ./disklist <*.img>\n");
		return 0;
	}

	if ((fd=open(argv[1], O_RDONLY)))
	{
		fstat(fd, &sf);
		p = mmap(NULL,sf.st_size, PROT_READ, MAP_SHARED, fd, 0);

		int where_root_starts = getSuperBlockinfo(p,22,4);
		int num_root_blocks   = getSuperBlockinfo(p,26,4);
                //printf("\nBlock where root directory starts: %d\n", where_root_starts);
                //printf("Number of Blocks in root directory: %d\n", num_root_blocks);
		printf("\n=================== List of files in root dir ====================\n");
		getFileInfo(p,where_root_starts*512,64,8);
	}
	else
		printf("Fail to open the image file.\n");
	close(fd);
	return 0;
}
