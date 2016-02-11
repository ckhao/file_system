/*
* diskget.c
* File System Implementation
* Part3 move a file from test.img to current directory in Linux
*
* @Chenkai Hao V00819367
* July 21th, 2015
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



int next_fat_entry(char *ptr, int lowerBound, int upperBound,int current){
		int next_entry;
		int i;
		int count = 0;
		for(i=lowerBound; i<=upperBound-3; i+=4){
			if(count == current){
				next_entry = (ptr[i]<<24) + (ptr[i+1]<<16) + (ptr[i+2]<<8) + (ptr[i+3]);
			}
			count++;
		
		}

		return next_entry;
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

void write_file(char *p, char *file_name, int entry, FILE* fp){

		
		int read_byte;
		int i=0;

		//printf("%d ", entry);//print out the all entries of a file
		
		for(i = entry*512; i<(entry+1)*512; i+=4){
			read_byte = (p[i]<<24) + (p[i+1]<<16) +(p[i+2]<<8) + (p[i+3]);
			fwrite(&read_byte, 1, sizeof(char), fp);
		}
		
		
}

void find_file(char *argv[], char * mmap, int offset, int length, int numRootblock, char *file_name)  //getFileInfo(p,53*512,64,8)
{
	int i,j,k = 0;
	
	int current_entry;
	int next_entry;
	
	char *root_entry = (char *)malloc(sizeof(char) * length);     
	char *file_name_bytes = (char *)malloc(sizeof(char) * 31);

	int blockSize = getSuperBlockinfo(mmap,8,2);
	int fatStart = getSuperBlockinfo(mmap,14,4);
	int fatBlock = getSuperBlockinfo(mmap,18,4);

	int lowerBound = fatStart * blockSize;//1024
	int upperBound = lowerBound + fatBlock * blockSize;//26624
	
	int block_array[999];
	for(k=0; k<999; k++){
		block_array[k] = 0;
	}
	int count = 0;
	
	int file_start_block;

	for (i=0; i < numRootblock; i ++)
	{	
		for (j = 0; j < 8; j ++){    // 8 directory entries per 512B blcok 
			root_entry = memcpy(root_entry, mmap+offset+512*i+length*j, length);
			// check the status of the entry (is it s file?)  2: File / 4: Directory
			if ((root_entry[0] & 0x02) == 0x02){//file and also available
				
				file_name_bytes = memcpy(file_name_bytes, root_entry + 27, 31);
				//printf("%s\n", file_name_bytes);
				//file_start_block = memcpy(file_start_block, root_entry + 1, 4);
				file_start_block = (root_entry[1]<<24)+(root_entry[2]<<16)+(root_entry[3]<<8)+(root_entry[4]);
				if(strncmp(file_name_bytes, file_name, 13) == 0){
					//printf("Hey we got it ! %s %s\n", file_name, file_name_bytes);
					//printf("Go to block: %d\n", file_start_block);
					//FILE *new_file = fopen(argv[3], "w");
					next_entry = file_start_block;
					while(next_entry >= 0){
						//printf("%d ",next_entry);
						current_entry = next_entry;
						block_array[count] = current_entry;
						//write_file(mmap, file_name, current_entry);
						next_entry = next_fat_entry(mmap, lowerBound, upperBound,next_entry);
						count++;
					}
					//printf("%d ", count);
					
					//for(k=0; k<count; k++){
						//write_file(mmap, file_name, block_array[k]);
					//}
					
				}

			}
		}
	}

	FILE *fp;
	fp = fopen(argv[3], "w");

	for(k=0; k<count; k++){
			write_file(mmap, file_name, block_array[k], fp);
	}

	//write_file(mmap, file_name, 61);
	fclose(fp);


	free(root_entry);
	free(file_name_bytes);
	
}





int main(int argc, char *argv[]){
	//[0]./disklist [1]test.img [2]/sub_dir/foo2.txt [3]foo.txt

	int fd;
	struct stat sf;
	char *p;
	
	
	if(argc!=4){
		printf("Usg: ./diskget *.img /sub_dir/foo.txt foo.txt\n");
		return 0;
	}
	

	char* file_name;
	file_name = argv[2];
	
	//printf("%s\n", file_name);
	if ((fd=open(argv[1], O_RDONLY)))
	{
		
		fstat(fd, &sf);
		p = mmap(NULL,sf.st_size, PROT_READ, MAP_SHARED, fd, 0);

		int where_root_starts = getSuperBlockinfo(p,22,4);
		int num_root_blocks   = getSuperBlockinfo(p,26,4);
		

		//printf("%d %d\n", lowerBound, upperBound);

		
		find_file(argv, p,where_root_starts*512,64,8, file_name);

		printf("\nSuccessfully copy the file from disk image.\n");
	}
	else
		printf("Fail to open the image file.\n");
	close(fd);
	return 0;
}
