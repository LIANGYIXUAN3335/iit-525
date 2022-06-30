#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
#include<string.h>
#include<math.h>
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
		fseek(pageFile, 0, SEEK_END);
    	int size = ftell(pageFile);
   		fseek(pageFile, 0, SEEK_SET);
    	fHandle->totalNumPages = size / PAGE_SIZE;
		fHandle->curPagePos = 0;	
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
    if(remove(fileName)){
        return RC_OK;
    }
    return RC_FILE_NOT_FOUND;
}
extern RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
	
	if (pageNum > fHandle->totalNumPages || pageNum < 0)
        	return RC_READ_NON_EXISTING_PAGE;
	pageFile = fopen(fHandle->fileName, "r");
	fseek(pageFile, (pageNum * PAGE_SIZE), SEEK_SET);
	fread(memPage, sizeof(char), PAGE_SIZE, pageFile);
	fHandle->curPagePos = ftell(pageFile); 
	fclose(pageFile);
    return RC_OK;
}
extern int getBlockPos (SM_FileHandle *fHandle) {
	return fHandle->curPagePos;
}
extern RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	pageFile = fopen(fHandle->fileName, "r");
	fread(memPage, sizeof(char), PAGE_SIZE, pageFile);
	fHandle->curPagePos = ftell(pageFile); 
	fclose(pageFile);
	return RC_OK;
}
extern RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
		int currentPageNumber = fHandle->curPagePos / PAGE_SIZE;
		int startPosition = (PAGE_SIZE * (currentPageNumber - 2));
		pageFile = fopen(fHandle->fileName, "r");
		fseek(pageFile, startPosition, SEEK_SET);
		fread(memPage, sizeof(char), PAGE_SIZE, pageFile);
		fHandle->curPagePos = ftell(pageFile); 
		fclose(pageFile);
		return RC_OK;
}
extern RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	
	int startPosition = (PAGE_SIZE * (fHandle->curPagePos / PAGE_SIZE - 2));
	pageFile = fopen(fHandle->fileName, "r");
	fseek(pageFile, startPosition, SEEK_SET);
	fread(memPage, sizeof(char), PAGE_SIZE, pageFile);
	fclose(pageFile);
	return RC_OK;		
}
extern RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	if(fHandle->curPagePos == PAGE_SIZE) {
		return RC_READ_NON_EXISTING_PAGE;	
	} 
	int startPosition = (PAGE_SIZE * (fHandle->curPagePos / PAGE_SIZE - 2));
	pageFile = fopen(fHandle->fileName, "r");
	if(pageFile == NULL)
		return RC_FILE_NOT_FOUND;
	fseek(pageFile, startPosition, SEEK_SET);
	fread(memPage, sizeof(char), PAGE_SIZE, pageFile);
	fHandle->curPagePos = ftell(pageFile); 
	fclose(pageFile);
	return RC_OK;
}
extern RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	pageFile = fopen(fHandle->fileName, "r");
	if(pageFile == NULL)
		return RC_FILE_NOT_FOUND;
	int startPosition = (fHandle->totalNumPages - 1) * PAGE_SIZE;
	fseek(pageFile, startPosition, SEEK_SET);
	fread(memPage, sizeof(char), PAGE_SIZE, pageFile);
	fHandle->curPagePos = ftell(pageFile); 
	fclose(pageFile);
	return RC_OK;	
}
extern RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
	pageFile = fopen(fHandle->fileName, "r+");

	int startPosition = pageNum * PAGE_SIZE;
	if(pageNum == 0) { 
		fseek(pageFile, startPosition, SEEK_SET);	

		fwrite(memPage, sizeof(char), PAGE_SIZE, pageFile);
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
	
	if( fseek(pageFile, 0, SEEK_END) != 0 ) {
		free(emptyBlock);
		return RC_WRITE_FAILED;
	}
	fwrite(emptyBlock, sizeof(char), PAGE_SIZE, pageFile);
	free(emptyBlock);
	fHandle->totalNumPages++;
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