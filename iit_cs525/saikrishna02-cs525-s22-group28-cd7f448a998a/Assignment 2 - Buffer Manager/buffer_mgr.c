#include<stdio.h>
#include<stdlib.h>
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include <math.h>
#define zero 0
#define one 1
#define minusone -1
typedef struct Page
{
	SM_PageHandle data; 
	PageNumber pageNum; 
	int dirtyBit; 
	int fixCount; 
	int hitNum;   
	int refNum;   
} PageFrame;
int bufferSize = zero ,rearIndex = zero,writeCount = zero,hit = zero;
SM_FileHandle fh;
PageFrame *pageFrame;
extern void Writepage(PageFrame *pageFrame, int i ,PageFrame *page){
			pageFrame[i].dirtyBit = page->dirtyBit;
			pageFrame[i].fixCount = page->fixCount;
			pageFrame[i].data = page->data;
			pageFrame[i].pageNum = page->pageNum;
}
extern void Writeblock(PageFrame *pageFrame,int i){
		writeBlock(pageFrame[i].pageNum, &fh, pageFrame[i].data);
		writeCount+= one;
}
extern void LRU(BM_BufferPool *const bm, PageFrame *page)
{	
	PageFrame *pageFrame = (PageFrame *) bm->mgmtData;
	int i=zero, leastHitIndex, leastHitNum;	
	openPageFile(bm->pageFile, &fh);
	while(i < bufferSize)
	{		
		if(pageFrame[i].fixCount == zero)
		{
			leastHitIndex = i;
			leastHitNum = pageFrame[i].hitNum;
			break;
		}i+= one;
	}		
	while( i < bufferSize)
	{
		if(pageFrame[i].hitNum < leastHitNum)
		{
			leastHitIndex = i;
			leastHitNum = pageFrame[i].hitNum;
		}i+= one;
	}
	if(pageFrame[leastHitIndex].dirtyBit == one)
	{
		Writeblock(pageFrame,leastHitIndex);				
		writeCount+= one;
	}	
	Writepage(pageFrame, leastHitIndex ,page);
	pageFrame[leastHitIndex].hitNum = page->hitNum;
}
extern void FIFO(BM_BufferPool *const bm, PageFrame *page)
{	
	pageFrame = (PageFrame *) bm->mgmtData;	
	int frontIndex = rearIndex % bufferSize;
	int i =zero;
	openPageFile(bm->pageFile, &fh);
	while(i<bufferSize)
	{
		if(pageFrame[frontIndex].fixCount != zero)
		{			
			frontIndex+= one;
			if(frontIndex % bufferSize == zero){
			frontIndex =  zero; }
			else{ frontIndex =frontIndex;}
		}
		else
		{	
			if(pageFrame[frontIndex].dirtyBit == one)
			{
				Writeblock(pageFrame,frontIndex);				
			}						
			Writepage(pageFrame,frontIndex,page);
			break;		
		}i+= one;
	}
}
extern RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, 
		  const int numPages, ReplacementStrategy strategy, 
		  void *stratData)
{
	bm->pageFile = (char *)pageFileName;
	bm->numPages = numPages;
	bm->strategy = strategy;	
	PageFrame *page = malloc(sizeof(PageFrame) * numPages);	
	bufferSize = numPages;	
	for(int i = zero; i < bufferSize; i+= one)
	{
		page[i].data = NULL;
		page[i].pageNum = minusone;
		page[i].dirtyBit = page[i].fixCount = page[i].hitNum = page[i].refNum = zero;
	}
	bm->mgmtData = page;
	writeCount = zero;
	return RC_OK;		
}
extern RC shutdownBufferPool(BM_BufferPool *const bm)
{
	pageFrame = (PageFrame *)bm->mgmtData;	
	forceFlushPool(bm);
	for(int i = zero; i < bufferSize; i+= one)
	{		
		if(pageFrame[i].fixCount != zero)
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
	PageFrame *pageFrame = (PageFrame *)bm->mgmtData;
	openPageFile(bm->pageFile, &fh);
	for(int i = zero; i < bufferSize; i+= one)
	{
		if(pageFrame[i].fixCount == zero && pageFrame[i].dirtyBit == one)
		{
			writeBlock(pageFrame[i].pageNum, &fh, pageFrame[i].data);
			pageFrame[i].dirtyBit = zero;
			
			writeCount+= one;
		}
	}	
	return RC_OK;
}
extern RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page)
{
	pageFrame = (PageFrame *)bm->mgmtData;		
	for(int i = zero; i < bufferSize; i+= one)
	{		
		if(pageFrame[i].pageNum == page->pageNum)
		{
			pageFrame[i].dirtyBit = one;
			return RC_OK;		
		}			
	}		
	return RC_ERROR;
}
extern RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page)
{	
	pageFrame = (PageFrame *)bm->mgmtData;	
	for(int i = zero; i < bufferSize; i+= one)
	{		
		if(pageFrame[i].pageNum == page->pageNum)
		{
			pageFrame[i].fixCount--;
			break;		
		}		
	}
	return RC_OK;
}
extern RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page)
{
	pageFrame = (PageFrame *)bm->mgmtData;
	openPageFile(bm->pageFile, &fh);
	for(int i=zero;i < bufferSize;i+= one)
	{
		if(pageFrame[i].pageNum == page->pageNum)
		{		
			Writeblock(pageFrame,i);
		}
	}	
	return RC_OK;
}
extern RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, 
	    const PageNumber pageNum)
{
	openPageFile(bm->pageFile, &fh);
	pageFrame = (PageFrame *)bm->mgmtData;	
	if(pageFrame[zero].pageNum == minusone)
	{		
		pageFrame[zero].data = (SM_PageHandle) malloc(PAGE_SIZE);
		ensureCapacity(pageNum,&fh);
		readBlock(pageNum, &fh, pageFrame[zero].data);
		pageFrame[zero].pageNum = pageNum;
		pageFrame[zero].fixCount+= one;
		rearIndex = hit = pageFrame[zero].refNum = zero;
		pageFrame[zero].hitNum = hit;	
		page->pageNum = pageNum;
		page->data = pageFrame[zero].data;		
		return RC_OK;		
	}
	else
	{	
		bool isBufferFull = true;
		for(int i = zero; i < bufferSize; i+= one)
		{
			if(pageFrame[i].pageNum != minusone)
			{					
				if(pageFrame[i].pageNum == pageNum)
				{					
					pageFrame[i].fixCount+= one;
					isBufferFull = false;
					hit+= one; 
					if(bm->strategy == RS_LRU)						
						pageFrame[i].hitNum = hit;					
					page->pageNum = pageNum;
					page->data = pageFrame[i].data;
					break;
				}				
			} else {
				pageFrame[i].data = (SM_PageHandle) malloc(PAGE_SIZE);
				readBlock(pageNum, &fh, pageFrame[i].data);
				pageFrame[i].pageNum = pageNum;
				pageFrame[i].fixCount = one;
				pageFrame[i].refNum = zero;
				rearIndex+= one;	
				hit+= one; 
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
			newPage->data = (SM_PageHandle) malloc(PAGE_SIZE);
			readBlock(pageNum, &fh, newPage->data);
			newPage->pageNum = pageNum;
			newPage->dirtyBit = newPage->refNum = zero;		
			newPage->fixCount = one;
			rearIndex+= one;
			hit+= one;				
			page->pageNum = pageNum;
			page->data = newPage->data;						
			if(bm->strategy==RS_FIFO) 
				FIFO(bm, newPage);
			else if (bm->strategy==RS_LRU) {
				newPage->hitNum = hit;	
				LRU(bm, newPage);			}		
		}		
		return RC_OK;
	}	
}
extern PageNumber *getFrameContents (BM_BufferPool *const bm)
{
	PageNumber *frameContents = malloc(sizeof(PageNumber) * bufferSize);
	pageFrame = (PageFrame *) bm->mgmtData;	
	for(int i =zero;i < bufferSize;i+= one) {
		if(pageFrame[i].pageNum != minusone)
		{frameContents[i] = pageFrame[i].pageNum; }
		else{ frameContents[i] = NO_PAGE;}
	}
	return frameContents;
}
extern bool *getDirtyFlags (BM_BufferPool *const bm)
{
	bool *dirtyFlags = malloc(sizeof(bool) * bufferSize);
	pageFrame = (PageFrame *)bm->mgmtData;	
	for(int i = zero; i < bufferSize; i+= one)
	{	dirtyFlags[i] =false;
		if(pageFrame[i].dirtyBit == one)
		{dirtyFlags[i] =  true;} 
	}	
	return dirtyFlags;
}
extern int *getFixCounts (BM_BufferPool *const bm)
{
	int *fixCounts = malloc(sizeof(int) * bufferSize);
	pageFrame= (PageFrame *)bm->mgmtData;	
	int i = zero;	
	while(i < bufferSize)
	{
		if(pageFrame[i].fixCount != minusone)
		{fixCounts[i] =  pageFrame[i].fixCount;}
		else{ fixCounts[i] =zero;}
		i+= one;
	}	
	return fixCounts;
}
extern int getNumReadIO (BM_BufferPool *const bm)
{	
	return (rearIndex + one);
}
extern int getNumWriteIO (BM_BufferPool *const bm)
{
	return writeCount;
}