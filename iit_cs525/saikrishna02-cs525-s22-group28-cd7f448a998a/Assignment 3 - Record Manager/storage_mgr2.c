//Importing all the required header files.
#include "storage_mgr.h"
#include "dberror.h"
#include "test_helper.h"
//A pointer to point to an opened file.
FILE *fp;
/*
Function to print the storage manager
*/
void initStorageManager (void){
    printf("Storage manager\n");
}
/*
This function creates a page file. The input of the function is File_name.
The initial size of the file is 1 page, which is 4096 bytes. The page is filled with '\0'.
If file creation fails, it returns RC_FILE NOT FOUND. If file writing fails, it returns RC_WRITE_FAILED.
*/
RC createPageFile (char *fileName){
    fp = fopen(fileName, "w+");
    if(!fp){
        return RC_FILE_NOT_FOUND;
    }
    char zeros[PAGE_SIZE] = {};
    if(PAGE_SIZE != fwrite(zeros, 1, PAGE_SIZE, fp)){
        fclose(fp);
        return RC_WRITE_FAILED;
    }
    fclose(fp);
    return RC_OK;
}
/* 
This function opens a existing file for writing.
It requires a filehandle and a file name.
If the file is not found, it will return RC_FILE_NOT_FOUND. Otherwise, RC_OK.
*/
RC openPageFile (char *fileName, SM_FileHandle *fHandle){
    fp = fopen(fileName, "r+");
    if(!fp){
        return RC_FILE_NOT_FOUND;
    }
    fHandle->fileName = fileName;
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    fHandle->totalNumPages = size / PAGE_SIZE;
    fHandle->curPagePos = 0;
    return RC_OK;
}
/*
This function closes the opened file.
It will return RC_OK if successful, otherwisise RC_FILE_NOT_FOUND.
*/
RC closePageFile (SM_FileHandle *fHandle){
    if(!fclose(fp)){
        return RC_OK;
    }
    return RC_FILE_NOT_FOUND;
}
/*
This function will delete the file. 
It will return RC_OK if successful, otherwisise RC_FILE_NOT_FOUND.
*/
RC destroyPageFile (char *fileName){
    if(remove(fileName)){
        return RC_OK;
    }
    return RC_FILE_NOT_FOUND;
}
/*
This function reads a block at position pageNum, and stores the value in SM_pageHandle memPage.
The function returns RC_OK when successful, otherwise RC_READ_NON_EXISTING_PAGE.
*/
RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
    if(pageNum >= fHandle->totalNumPages || pageNum < 0){
        return RC_READ_NON_EXISTING_PAGE;
    }

    fseek(fp, pageNum * PAGE_SIZE, SEEK_SET);
    fread(memPage, PAGE_SIZE, 1, fp);

    return RC_OK;
}
/*
This function returns the page position of current block.
*/
int getBlockPos (SM_FileHandle *fHandle){
    return fHandle->curPagePos;
}
/*
This function reads the first block of the opened file.
The content of read block is put into SM_PageHandle memPage.
*/
RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    fseek(fp, 0, SEEK_SET);
    fread(memPage, PAGE_SIZE, 1, fp);
    return RC_OK;
}
/*
This function reads previous block of the opened file.
The content of read block is put into SM_PageHandle memPage.
*/
RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    if(fHandle->curPagePos == 0){
        return RC_READ_NON_EXISTING_PAGE;
    }
    fseek(fp, (--fHandle->curPagePos) * PAGE_SIZE, SEEK_SET);
    fread(memPage, PAGE_SIZE, 1, fp);
    return RC_OK;
}
/*
This function reads current block of the opened file.
The content of read block is put into SM_PageHandle memPage.
*/
RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    fseek(fp, fHandle->curPagePos * PAGE_SIZE, SEEK_SET);
    fread(memPage, PAGE_SIZE, 1, fp);
    return RC_OK;
}
/*
This function reads next block of the opened file.
The content of read block is put into SM_PageHandle memPage.
*/
RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    if(fHandle->curPagePos == fHandle->totalNumPages - 1){
        return RC_READ_NON_EXISTING_PAGE;
    }
    fseek(fp, (++fHandle->curPagePos) * PAGE_SIZE, SEEK_SET);
    fread(memPage, PAGE_SIZE, 1, fp);
    return RC_OK;
}
/*
This function reads the last block of the opened file.
The content of read block is put into SM_PageHandle memPage.
*/
RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    fseek(fp, -PAGE_SIZE, SEEK_END);
    fread(memPage, PAGE_SIZE, 1, fp);
    return RC_OK;
}
/*
This function crite the content of page memPage into a block whose position is determined pageNum.
*/
RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
    if(pageNum >= fHandle->totalNumPages || pageNum < 0){
        return RC_READ_NON_EXISTING_PAGE;
    }
    fseek(fp, PAGE_SIZE * pageNum, SEEK_SET);
    fwrite(memPage, PAGE_SIZE, 1, fp);
    return RC_OK;
}
/*
This function writes the content of page memPage into the current block of opened file.
*/
RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    fseek(fp, PAGE_SIZE * fHandle->curPagePos, SEEK_SET);
    fwrite(memPage, PAGE_SIZE, 1, fp);
    return RC_OK;
}
/*
This function appends an empty block to the opened file.
*/
RC appendEmptyBlock (SM_FileHandle *fHandle){
    char zeros[PAGE_SIZE] = {};
    fseek(fp, 0, SEEK_END);
    if(PAGE_SIZE != fwrite(zeros, 1, PAGE_SIZE, fp)){
        return RC_WRITE_FAILED;
    }
    fHandle->totalNumPages++;
    return RC_OK;
}
/*
This function increases the size of opened file until the file can contains numberOfPages number of pages.
*/
RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){
    while(numberOfPages > fHandle->totalNumPages){
        if(appendEmptyBlock(fHandle) != RC_OK){
            return RC_WRITE_FAILED;
        }
    }
    return RC_OK;
}