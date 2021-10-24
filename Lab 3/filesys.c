// ACADEMIC INTEGRITY PLEDGE
//
// - I have not used source code obtained from another student nor
//   any other unauthorized source, either modified or unmodified.
//
// - All source code and documentation used in my program is either
//   my original work or was derived by me from the source code
//   published in the textbook for this course or presented in
//   class.
//
// - I have not discussed coding details about this project with
//   anyone other than my instructor. I understand that I may discuss
//   the concepts of this program with other students and that another
//   student may help me debug my program so long as neither of us
//   writes anything during the discussion or modifies any computer
//   file during the discussion.
//
// - I have violated neither the spirit nor letter of these restrictions.
//
//
//
// Signed: Connor Taylor Date: 4/30/21

//filesys.c
//Based on a program by Michael Black, 2007
//Revised 11.3.2020 O'Neil

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void listFiles(char[]);

void printFile(char[], char[], FILE*);

void createFile(char[], char[], char[], FILE*);

void deleteFile(char[], char[], char[], FILE*);

void writeToFloppy(char[], char[], FILE*);

int main(int argc, char* argv[])
{
	int i, j, size, noSecs, startPos;

	//open the floppy image
	FILE* floppy;
	floppy = fopen("floppya.img", "r+");
	if(floppy == 0)
	{
		printf("floppya.img not found\n");
		return 0;
	}

	//load the disk map from sector 256
	char map[512];
	fseek(floppy, 512 * 256, SEEK_SET);
	for(i = 0; i < 512; ++i)
		map[i] = fgetc(floppy);

	//load the directory from sector 257
	char dir[512];
	fseek(floppy, 512 * 257, SEEK_SET);
	for(i = 0; i < 512; ++i)
		dir[i] = fgetc(floppy);
	
	// parses arguments from user
	if(strcmp(argv[1], "L") == 0 || strcmp(argv[1], "l") == 0 && argc == 2)
	{
		listFiles(dir);
	}
	else if(strcmp(argv[1], "P") == 0 || strcmp(argv[1], "p") == 0 && argc == 3)
	{
		printFile(dir, argv[2], floppy);
	}
	else if(strcmp(argv[1], "M") == 0 || strcmp(argv[1], "m") == 0 && argc == 3)
	{
		createFile(dir, map, argv[2], floppy);
	}
	else if(strcmp(argv[1], "D") == 0 || strcmp(argv[1], "d") == 0 && argc == 3)
	{
		deleteFile(dir, map, argv[2], floppy);
	}
	else
	{
		printf("Invalid command.\n");
	}

	fclose(floppy);
}

void listFiles(char dir[])
{
	int i, j;
	
	// print directory
	printf("Disk directory:\n");
	int bytesTaken = 0;
	for(i = 0; i < 512; i += 16)
	{
		// verifies there is a file in the directory
		if(dir[i] != 0) 
		{
			// outputs file name with file type
			for(j = 0; j < 9; ++j) 
			{
				if(j == 8)
					printf("%c", '.');
				if(dir[i + j] != 0) 
					printf("%c", dir[i + j]); 
			}
			
			// outputs the number of bytes used by the file
			printf("\t%d bytes\n", 512 * dir[i + 10]);
			bytesTaken += 512 * dir[i + 10];
		}
	}
	// outputs the taken and remaining bytes
	printf("Bytes taken: %d   bytes\n", bytesTaken);
	printf("Bytes free:  %d bytes\n", 511 * 512 - bytesTaken);
}

void printFile(char dir[], char file[], FILE* floppy)
{
	int i, j;
	size_t size = strlen(file);
	char curFile[size];
	
	// iterates through the floppy to get data from files
	for(i = 0; i < 512; i += 16)
	{
		if(dir[i] != 0) 
		{
			for(j = 0; j < 8; ++j) 
			{
				if(dir[i + j] != 0) 
				{
					if(strcmp(file, &dir[i + j]) == 0)
					{
						// verfifies file is a text file
						if((dir[i + 8] == 't') || (dir[i + 8] == 'T')) 
						{
							int startsector = dir[i + 9];
							int data = 1;
							fseek(floppy, 512 * startsector, SEEK_SET);
							// outputs data from file
							while (data != 0) 
							{
								data = fgetc(floppy);
								printf("%c", data);
							}
							return;
						}
						else 
						{
							printf("This file is not printable.\n");
							return;
						}
					}
				} 
			}
		}
	}
	
	printf("File was not found.\n");
}

void createFile(char dir[], char map[], char fileName[], FILE* floppy)
{
	int i, j, freeDir, freeSect;
	freeDir = freeSect = -1;
	
	// gets the free first free sector and sets it as occupied
	for(i = 0; i < 16 && freeSect == -1; ++i) 
	{
		for(j = 0; j < 16 && freeSect == -1; ++j) 
		{
			if(map[16 * i + j] != -1) 
			{
				freeSect = 16 * i + j;
				map[16 * i + j] = 255;
				break;
			}
		}
	}
	// exits function if there are no more available sectors
	if(freeSect == -1)
	{
		printf("Insufficient disk space.\n");
		return;
	}
	
	// iterates through the directory sector
	for(i = 0; i < 512; i += 16)
	{
		if(dir[i] != 0) 
		{
			for(j = 0; j < 8; ++j) 
			{
				if(dir[i + j] != 0) 
				{
					// exits function if file is a duplicate or nonexistent
					if(strcmp(fileName, &dir[i + j]) == 0)
					{
						printf("Duplicate or invalid file name.\n");
						return;
					}
				} 
			}
		}
		// marks sector of the free directory
		else if(freeDir == -1)
		{
			freeDir = i;
			dir[i + 9] = freeSect;
		}
	}
	
	// appends null character to the last character of the file name
	// NOTE:: I chose to use \0 instead of 0 to preser the correct Hex in the directory
	for(i = strlen(fileName); i < 8; ++i)
	{
		fileName[i] = '\0';
	}
	// adds the file type, sector, and length information to the file
	fileName[8] = 't';
	fileName[9] = freeSect;
	fileName[10] = 1;
	// sets the file info in the directory
	for(i = 0; i < 11; ++i)
	{
		dir[freeDir + i] = fileName[i];
	}
	
	// gets data for the file
	char data[512];
	printf("Enter file data: ");
	fgets(data, 512, stdin);
	
	// writes the data to the floppy 
	fseek(floppy, 512 * freeSect, SEEK_SET);
	for(i = 0; i < 512; ++i)
	{
		fputc(data[i], floppy);
	}
	
	// writes the updated directory and map to the floppy
	writeToFloppy(dir, map, floppy);
}

void deleteFile(char dir[], char map[], char fileName[], FILE* floppy)
{
	int i, j, k, firstSect, numSects;
	firstSect = -1;
	
	// iterates through directory sector
	for(i = 0; i < 512; i += 16)
	{
		if(dir[i] != 0) 
		{
			for(j = 0; j < 8; ++j) 
			{
				if(dir[i + j] != 0) 
				{
					// if the file exists, gets the first sector and number of sectors on the map
					if(strcmp(fileName, &dir[i + j]) == 0)
					{
						firstSect = dir[i + 9];
						numSects = dir[i + 10];
						// sets the file as empty on the directory sector, preserves first sector and length
						for (k = 0; k < 9; ++k) 
						{
							dir[i + j + k] = 0;
						}
						break;
					}
				} 
			}
		}
	}
	// if the file was not found, exit the function
	if(firstSect == -1)
	{
		printf("File not found.\n");
		return;
	}
	
	// does not eliminate the file from map so it can be recovered
	
	writeToFloppy(dir, map, floppy);
}

void writeToFloppy(char dir[], char map[], FILE* floppy)
{ 
	int i;
	
	//write the map and directory back to the floppy image
    fseek(floppy, 512 * 256, SEEK_SET);
    for(i = 0; i < 512; ++i) 
		fputc(map[i], floppy);

    fseek(floppy,512 * 257,SEEK_SET);
    for(i = 0; i < 512; ++i) 
		fputc(dir[i], floppy);
}
