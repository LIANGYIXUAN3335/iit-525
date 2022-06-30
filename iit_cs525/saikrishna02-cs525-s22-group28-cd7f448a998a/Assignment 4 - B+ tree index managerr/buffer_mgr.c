#include<stdio.h>
#include<stdlib.h>
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include <math.h>
typedef struct Page
{
	SM_PageHandle data; 
	PageNumber pageNum; 
	int dirtyBit; 
	int fixCount; 
	int hitNum;   
	int refNum;   
} PageFrame;
SM_FileHandle fh;
PageFrame *pageFrame;
int bufferSize = 0;
int rearIndex = 0;
int writeCount = 0;
int hit = 0;
extern void Writepage(PageFrame *pageFrame, int i ,PageFrame *page){
			pageFrame[i].dirtyBit = page->dirtyBit;
			pageFrame[i].fixCount = page->fixCount;
			pageFrame[i].data = page->data;
			pageFrame[i].pageNum = page->pageNum;
}
extern void Writeblock(PageFrame *pageFrame,int i){
		writeBlock(pageFrame[i].pageNum, &fh, pageFrame[i].data);
		pageFrame[i].dirtyBit = 0;
		writeCount++;
}
extern void FIFO(BM_BufferPool *const bm, PageFrame *page)
{
	pageFrame = (PageFrame *) bm->mgmtData;
	int frontIndex = rearIndex % bufferSize;
	int i =0;
	openPageFile(bm->pageFile, &fh);
	while(i<bufferSize){
		
		if(pageFrame[frontIndex].fixCount != 0)
		{	
			frontIndex++;
			if(frontIndex % bufferSize == 0){
			frontIndex =  0; }
			else{ frontIndex =frontIndex;}
			break;
		}
		else
		{
			if(pageFrame[frontIndex].dirtyBit == 1)
			{
				writeBlock(pageFrame[frontIndex].pageNum, &fh, pageFrame[frontIndex].data);
				writeCount++;
			}
			Writepage(pageFrame,frontIndex,page);
		}i++;
	}
}	
extern void LRU(BM_BufferPool *const bm, PageFrame *page)
{
	pageFrame = (PageFrame *) bm->mgmtData;
	int i, leastHitIndex, leastHitNum;
	while(i < bufferSize)
	{
		if(pageFrame[i].fixCount == 0)
		{
			leastHitIndex = i;
			leastHitNum = pageFrame[i].hitNum;
			break;
		}i++;
	}	
	i = leastHitIndex + 1;
	
	openPageFile(bm->pageFile, &fh);
	while( i < bufferSize)
	{
		if(pageFrame[i].hitNum < leastHitNum)
		{
			leastHitIndex = i;
			leastHitNum = pageFrame[i].hitNum;
		}i++;
	}
	if(pageFrame[leastHitIndex].dirtyBit == 1)
	{
		writeBlock(pageFrame[leastHitIndex].pageNum, &fh, pageFrame[leastHitIndex].data);
		writeCount++;
	}
	// 可以
	Writepage(pageFrame, leastHitIndex ,page);
	pageFrame[leastHitIndex].hitNum = page->hitNum;	
}
extern RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, 
		  const int numPages, ReplacementStrategy strategy, 
		  void *stratData)
{
	bm->pageFile = (char *)pageFileName;
	bm->numPages = bufferSize  =numPages;
	bm->strategy = strategy;
	PageFrame *page = malloc(sizeof(PageFrame) * numPages);
	
	for(int i = 0; i < bufferSize; i++)
	{
		page[i].data = NULL;
		page[i].pageNum = -1;
		page[i].dirtyBit = 0;
		page[i].fixCount = 0;
		page[i].hitNum = 0;	
	}
	bm->mgmtData = page;
	writeCount  = 0;
	return RC_OK;	
}
extern RC shutdownBufferPool(BM_BufferPool *const bm)
{
	    if (bm == NULL){
        return RC_ERROR_No_BF;
    }
    if (bm->numPages < 0){
        return RC_ERROR_No_PG;
    }
    if (bm->pageFile == "fail"){
        return RC_NOT_OPEN;
    }
	PageFrame *pageFrame = (PageFrame *)bm->mgmtData;
	forceFlushPool(bm);
	int i;	
	for(i = 0; i < bufferSize; i++)
	{
		if(pageFrame[i].fixCount != 0)
		{
			return RC_PINNED_PAGES_IN_BUFFER;
		}
	}
	free(pageFrame);
	bm->mgmtData = NULL;
	return RC_OK;
}
extern RC forceFlushPool(BM_BufferPool *const bm)
{
	    if (bm == NULL){
        return RC_ERROR_No_BF;
    }
    if (bm->numPages < 0){
        return RC_ERROR_No_PG;
    }
    if (bm->pageFile == "fail"){
        return RC_NOT_OPEN;
    }
	pageFrame = (PageFrame *)bm->mgmtData;
	int i =0;
	openPageFile(bm->pageFile, &fh);
	while(i < bufferSize)
	{
		if(pageFrame[i].fixCount == 0 && pageFrame[i].dirtyBit == 1)
		{
			Writeblock(pageFrame,i);
		}i++;
	}	
	 closePageFile(&fh);
	return RC_OK;
}
extern RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page)
{
	pageFrame = (PageFrame *)bm->mgmtData;
	int i =0;
	while(i < bufferSize)
	{
		if(pageFrame[i].pageNum == page->pageNum)
		{
			pageFrame[i].dirtyBit = 1;
			return RC_OK;		
		}	i++;		
	}	
    
    return RC_OK;	
}
extern RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page)
{	
	pageFrame = (PageFrame *)bm->mgmtData;
	int i = 0 ;
	while(i < bufferSize)
	{
		if(pageFrame[i].pageNum == page->pageNum)
		{
			pageFrame[i].fixCount--;
			break;		
		}	i++;	
	}
	return RC_OK;
}
extern RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page)
{
	pageFrame = (PageFrame *)bm->mgmtData;

	int i =0;
	openPageFile(bm->pageFile, &fh);
	while(i < bufferSize)
	{
		if(pageFrame[i].pageNum == page->pageNum)
		{		
			Writeblock(pageFrame,i);
		}i++;
	}	
	return RC_OK;
}
extern RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, 
	    const PageNumber pageNum)
{
	    if (bm == NULL){
        return RC_ERROR_No_BF;
    }
    if (bm->numPages < 0){
        return RC_ERROR_No_PG;
    }
    if (page < 0){
        return RC_ERROR_Frame_Number;
    }
   if (pageNum < 0){
        return RC_ERROR_Frame_Number;
    }
    if (bm->pageFile == "fail"){
        return RC_NOT_OPEN;
    }
	PageFrame *pageFrame = (PageFrame *)bm->mgmtData;
	if(pageFrame[0].pageNum == -1)
	{
		openPageFile(bm->pageFile, &fh);
		pageFrame[0].data = (SM_PageHandle) malloc(PAGE_SIZE);
		ensureCapacity(pageNum,&fh);
		readBlock(pageNum, &fh, pageFrame[0].data);
		pageFrame[0].pageNum = pageNum;
		pageFrame[0].fixCount++;
		rearIndex = hit = 0;
		pageFrame[0].hitNum = hit;	
		page->pageNum = pageNum;
		page->data = pageFrame[0].data;
		return RC_OK;		
	}
	else
	{	
		int i;
		bool isBufferFull = true;
		for(i = 0; i < bufferSize; i++)
		{
			if(pageFrame[i].pageNum != -1)
			{
				if(pageFrame[i].pageNum == pageNum)
				{
					pageFrame[i].fixCount++;
					isBufferFull = false;
					hit++; 
					if(bm->strategy == RS_LRU)
						pageFrame[i].hitNum = hit;
					page->pageNum = pageNum;
					page->data = pageFrame[i].data;
					break;
				}				
			} else {
				
				openPageFile(bm->pageFile, &fh);
				pageFrame[i].data = (SM_PageHandle) malloc(PAGE_SIZE);
				readBlock(pageNum, &fh, pageFrame[i].data);
				pageFrame[i].pageNum = pageNum;
				pageFrame[i].fixCount = 1;
				rearIndex++;	
				hit++; 
				if(bm->strategy == RS_LRU)
					pageFrame[i].hitNum = hit;					
				page->pageNum = pageNum;
				page->data = pageFrame[i].data;
				isBufferFull = false;
				break;
			}
		}
		if(isBufferFull == true)
		{
			PageFrame *newPage = (PageFrame *) malloc(sizeof(PageFrame));		
			
			openPageFile(bm->pageFile, &fh);
			newPage->data = (SM_PageHandle) malloc(PAGE_SIZE);
			readBlock(pageNum, &fh, newPage->data);
			newPage->pageNum = pageNum;
			newPage->dirtyBit = 0;		
			newPage->fixCount = 1;
			rearIndex++;
			hit++;
			if(bm->strategy == RS_LRU)
				
				newPage->hitNum = hit;				
			page->pageNum = pageNum;
			page->data = newPage->data;			
			if((bm->strategy)==RS_FIFO) 
				FIFO(bm, newPage);
			else if (bm->strategy==RS_LRU) 
				LRU(bm, newPage);		
		return RC_OK;
		}
	}	
}
extern PageNumber *getFrameContents (BM_BufferPool *const bm)
{
	int *frameContents = malloc(sizeof(int) * bufferSize);
	pageFrame = (PageFrame *) bm->mgmtData;
	int i = 0;
	while(i < bufferSize) {
		if(pageFrame[i].pageNum != -1)
		{frameContents[i] = pageFrame[i].pageNum; }
		else{ frameContents[i] = NO_PAGE;}
		i++;
	}
	return frameContents;
}
extern bool *getDirtyFlags (BM_BufferPool *const bm)
{
	bool *dirtyFlags = malloc(sizeof(bool) * bufferSize);
	pageFrame = (PageFrame *)bm->mgmtData;
	for(int i = 0; i < bufferSize; i++)
	{	dirtyFlags[i] =false;
		if(pageFrame[i].dirtyBit == 1)
		{dirtyFlags[i] =  true;} 
	}	
	return dirtyFlags;
}
extern int *getFixCounts (BM_BufferPool *const bm)
{
	int *fixCounts = malloc(sizeof(int) * bufferSize);
	pageFrame= (PageFrame *)bm->mgmtData;
	int i = 0;
	while(i < bufferSize)
	{	if(pageFrame[i].fixCount != -1)
		{fixCounts[i] =  pageFrame[i].fixCount;}
		else{ fixCounts[i] =0;}
		i++;
	}	
	return fixCounts;
}
extern int getNumReadIO (BM_BufferPool *const bm)
{
	
	return (rearIndex + 1);
}
extern int getNumWriteIO (BM_BufferPool *const bm)
{
	return writeCount;
}