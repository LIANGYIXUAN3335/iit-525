#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<sys/types.h>

#include<string.h>
#include<math.h>
#define zero 0
#define one 1
#include "storage_mgr.h"

FILE *pageFile;
extern void initStorageManager (void) {
	pageFile = NULL;
}
extern RC createPageFile (char *fileName) {
	pageFile = fopen(fileName, "w+");
	if(!pageFile) {
		return RC_FILE_NOT_FOUND;
	} else {
		char zeros[PAGE_SIZE] ;
		if(fwrite(zeros, sizeof(char), PAGE_SIZE,pageFile) < PAGE_SIZE)
			return RC_WRITE_FAILED;
		fclose(pageFile);
		return RC_OK;
	}
}

extern RC openPageFile (char *fileName, SM_FileHandle *fHandle) {
	pageFile = fopen(fileName, "r");
	if(!pageFile) {
		return RC_FILE_NOT_FOUND;
	} else { 
		fHandle->fileName = fileName;
		fseek(pageFile, zero, SEEK_END);
    	int size = ftell(pageFile);
   		fseek(pageFile, zero, SEEK_SET);
    	fHandle->totalNumPages = size / PAGE_SIZE;
		fHandle->curPagePos = zero;	
		fclose(pageFile);
		return RC_OK;
	}
}
extern RC closePageFile (SM_FileHandle *fHandle) {
	if(!pageFile)
		pageFile = NULL;	
	return RC_OK; 
}
extern RC destroyPageFile (char *fileName) {
	
	pageFile = fopen(fileName, "r");
	
	if(pageFile == NULL)
		return RC_FILE_NOT_FOUND; 
	
	
	remove(fileName);
	return RC_OK;
}

extern RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
	
	if (pageNum > fHandle->totalNumPages || pageNum < zero)
        	return RC_READ_NON_EXISTING_PAGE;

	
	pageFile = fopen(fHandle->fileName, "r");

	
	if(pageFile == NULL)
		return RC_FILE_NOT_FOUND;
	
	
	
	int isSeekSuccess = fseek(pageFile, (pageNum * PAGE_SIZE), SEEK_SET);
	if(isSeekSuccess == zero) {
		
		if(fread(memPage, sizeof(char), PAGE_SIZE, pageFile) < PAGE_SIZE)
			return RC_ERROR;
	} else {
		return RC_READ_NON_EXISTING_PAGE; 
	}
    	
	
	fHandle->curPagePos = ftell(pageFile); 
	
	
	fclose(pageFile);
	
    	return RC_OK;
}

extern int getBlockPos (SM_FileHandle *fHandle) {
	
	return fHandle->curPagePos;
}

extern RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	
	pageFile = fopen(fHandle->fileName, "r");
	
	
	if(pageFile == NULL)
		return RC_FILE_NOT_FOUND;

	int i;
	for(i = zero; i < PAGE_SIZE; i+= one) {
		
		char c = fgetc(pageFile);
	
		
		if(feof(pageFile))
			break;
		else
			memPage[i] = c;
	}

	
	fHandle->curPagePos = ftell(pageFile); 

	
	fclose(pageFile);
	return RC_OK;
}

extern RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	
	

	
	if(fHandle->curPagePos <= PAGE_SIZE) {
		printf("\n First block: Previous block not present.");
		return RC_READ_NON_EXISTING_PAGE;	
	} else {
		
		int currentPageNumber = fHandle->curPagePos / PAGE_SIZE;
		int startPosition = (PAGE_SIZE * (currentPageNumber - 2));

		
		pageFile = fopen(fHandle->fileName, "r");
		
		
		if(pageFile == NULL)
			return RC_FILE_NOT_FOUND;

		
		fseek(pageFile, startPosition, SEEK_SET);
		
		int i;
		
		for(i = zero; i < PAGE_SIZE; i+= one) {
			memPage[i] = fgetc(pageFile);
		}

		
		fHandle->curPagePos = ftell(pageFile); 

		
		fclose(pageFile);
		return RC_OK;
	}
}

extern RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	
	int currentPageNumber = fHandle->curPagePos / PAGE_SIZE;
	int startPosition = (PAGE_SIZE * (currentPageNumber - 2));
	
	
	pageFile = fopen(fHandle->fileName, "r");

	
	if(pageFile == NULL)
		return RC_FILE_NOT_FOUND;

	
	fseek(pageFile, startPosition, SEEK_SET);
	
	int i;
	
	
	for(i = zero; i < PAGE_SIZE; i+= one) {
		char c = fgetc(pageFile);		
		if(feof(pageFile))
			break;
		memPage[i] = c;
	}
	
	
	fHandle->curPagePos = ftell(pageFile); 

	
	fclose(pageFile);
	return RC_OK;		
}

extern RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	
	if(fHandle->curPagePos == PAGE_SIZE) {
		printf("\n Last block: Next block not present.");
		return RC_READ_NON_EXISTING_PAGE;	
	} else {
		
		int currentPageNumber = fHandle->curPagePos / PAGE_SIZE;
		int startPosition = (PAGE_SIZE * (currentPageNumber - 2));

		
		pageFile = fopen(fHandle->fileName, "r");
		
		
		if(pageFile == NULL)
			return RC_FILE_NOT_FOUND;
		
		
		fseek(pageFile, startPosition, SEEK_SET);
		
		int i;
		
		
		for(i = zero; i < PAGE_SIZE; i+= one) {
			char c = fgetc(pageFile);		
			if(feof(pageFile))
				break;
			memPage[i] = c;
		}

		
		fHandle->curPagePos = ftell(pageFile); 

		
		fclose(pageFile);
		return RC_OK;
	}
}

extern RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	
	pageFile = fopen(fHandle->fileName, "r");

	
	if(pageFile == NULL)
		return RC_FILE_NOT_FOUND;
	
	int startPosition = (fHandle->totalNumPages - 1) * PAGE_SIZE;

	
	fseek(pageFile, startPosition, SEEK_SET);
	
	int i;
	
	
	for(i = zero; i < PAGE_SIZE; i+= one) {
		char c = fgetc(pageFile);		
		if(feof(pageFile))
			break;
		memPage[i] = c;
	}
	
	
	fHandle->curPagePos = ftell(pageFile); 

	
	fclose(pageFile);
	return RC_OK;	
}

extern RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
	
	if (pageNum > fHandle->totalNumPages || pageNum < zero)
        	return RC_WRITE_FAILED;
	
	
	pageFile = fopen(fHandle->fileName, "r+");
	
	
	if(pageFile == NULL)
		return RC_FILE_NOT_FOUND;

	int startPosition = pageNum * PAGE_SIZE;

	if(pageNum == zero) { 
		
		fseek(pageFile, startPosition, SEEK_SET);	
		int i;
		for(i = zero; i < PAGE_SIZE; i+= one) 
		{
			
			if(feof(pageFile)) 
				 appendEmptyBlock(fHandle);
			
			fputc(memPage[i], pageFile);
		}

		
		fHandle->curPagePos = ftell(pageFile); 

		
		fclose(pageFile);	
	} else {	
		
		fHandle->curPagePos = startPosition;
		fclose(pageFile);
		writeCurrentBlock(fHandle, memPage);
	}
	return RC_OK;
}

extern RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	
	pageFile = fopen(fHandle->fileName, "r+");

	
	if(pageFile == NULL)
		return RC_FILE_NOT_FOUND;
	
	
	appendEmptyBlock(fHandle);

	
	fseek(pageFile, fHandle->curPagePos, SEEK_SET);
	
	
	fwrite(memPage, sizeof(char), strlen(memPage), pageFile);
	
	
	fHandle->curPagePos = ftell(pageFile);

	
	fclose(pageFile);
	return RC_OK;
}


extern RC appendEmptyBlock (SM_FileHandle *fHandle) {
	
	SM_PageHandle emptyBlock = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));
	
	
	
	int isSeekSuccess = fseek(pageFile, zero, SEEK_END);
	
	if( isSeekSuccess == zero ) {
		
		fwrite(emptyBlock, sizeof(char), PAGE_SIZE, pageFile);
	} else {
		free(emptyBlock);
		return RC_WRITE_FAILED;
	}
	
	
	
	free(emptyBlock);
	
	
	fHandle->totalNumPages+= one;
	return RC_OK;
}

extern RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle) {
	
	pageFile = fopen(fHandle->fileName, "a");
	
	if(pageFile == NULL)
		return RC_FILE_NOT_FOUND;
	
	
	
	while(numberOfPages > fHandle->totalNumPages)
		appendEmptyBlock(fHandle);
	
	
	fclose(pageFile);
	return RC_OK;
}