# file_system


/*
 * This project implementes a simple File System
 * Four C files: diskinfo.c disklist.c diskget.c diskput.c
 * One Makefile to compile 4 .c files to .o file
 * 
*/

/*
 * diskinfo.c
 * show super block information and FAT information
 * retriveBlockInfo() function assigns value for variables that are 
 * used to show information of super block and FAT (File allocation table) 
 * later improved by getSuperBlockinfo()
 * 
 * disklist.c
 * show all files in the disk image (test.img has been used when implementing)
 * getFileInfo() prints out file information such as file status, file name, etc.
 * 
 * diskget.c
 * copy a file from disk image to current directory in linux
 * find_file() allocates all file entries by calling next_fat_entry
 * every time the program finds an entry !0xFFFFFFFF go to the respect block
 * call write_file() to fwrite the file content into a file in current linux directory
 * 
 * diskput.c
 * copy a file from current linux directory to out disk image
 * find_directory() is called if we are not transfering the file to root
 * directory. if the directory exits, write to that directory and update
 * the superblock info and FAT FDT info. if the directory does not exit,
 * create a new directory and do the same thing.
 * disk_put() and info_update function are called when we copy the file to
 * disk image and update all informations.
 * 
*/
