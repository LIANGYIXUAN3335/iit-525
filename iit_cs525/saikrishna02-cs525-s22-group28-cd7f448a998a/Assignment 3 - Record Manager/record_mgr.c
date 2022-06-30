#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "record_mgr.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"
typedef struct RecordManager
{
	BM_PageHandle pageHandle;	
} RecordManager;
BM_PageHandle *pH;	
int freePage;
BM_BufferPool bufferPool;
RID recordID;
Expr *expression;
int tuplesCount;
int scanCount; 
RecordManager *recordManager;
int  sizeofint =sizeof(int);
int pointer = sizeof(char*);








extern RC initRecordManager (void *mgmtData)
{   
    initStorageManager();
	return RC_OK;
}
extern RC shutdownRecordManager ()
{
	free(recordManager);
	return RC_OK;
}
extern RC createTable (char *name, Schema *schema)
{
	recordManager = (RecordManager*) malloc(sizeof(RecordManager));
	initBufferPool(&bufferPool, name, 100, RS_LRU, NULL);
	char data[PAGE_SIZE];
	char *pageHandle = data;
	 
	// int result, k;
	*(int*)pageHandle = 0; 
	pageHandle +=sizeofint;
	*(int*)pageHandle = 1;
	pageHandle += sizeofint;
	*(int*)pageHandle = schema->numAttr;
	pageHandle += sizeofint; 
	*(int*)pageHandle = schema->keySize;
	pageHandle += sizeofint;
	
	for(int k = 0; k < schema->numAttr; k++)
    	{
		
       		strncpy(pageHandle, schema->attrNames[k], 15);
	       	pageHandle += 15;
	       	*(int*)pageHandle = (int)schema->dataTypes[k];
	       	pageHandle += sizeofint;
	       	*(int*)pageHandle = (int) schema->typeLength[k];
	       	pageHandle += sizeofint;
    	}
	SM_FileHandle fileHandle;
	// int result;
	if(createPageFile(name) != RC_OK)
		return createPageFile(name);
	if(openPageFile(name, &fileHandle) != RC_OK)
		return openPageFile(name, &fileHandle);
	if(writeBlock(0, &fileHandle, data) != RC_OK)
		return writeBlock(0, &fileHandle, data);
	if(closePageFile(&fileHandle)!= RC_OK)
		return closePageFile(&fileHandle);
	return RC_OK;
}
extern RC openTable (RM_TableData *rel, char *name)
{
	char* pageHandle;    
	rel->name = name;
	rel->mgmtData = recordManager;
	pinPage(&bufferPool, &recordManager->pageHandle, 0);
	pageHandle = (char*) recordManager->pageHandle.data;
	tuplesCount= *(int*)pageHandle;
	pageHandle += sizeofint;
	freePage= *(int*) pageHandle;
    pageHandle +=sizeofint;
    int attributeCount = *(int*)pageHandle;
	pageHandle += sizeofint;
	Schema *schema;
	schema = (Schema*) malloc(sizeof(Schema));
	schema->numAttr = attributeCount;
	schema->attrNames = (char**) malloc(pointer *attributeCount);
	schema->dataTypes = (DataType*) malloc(sizeof(DataType) *attributeCount);
	schema->typeLength = (int*) malloc(sizeofint *attributeCount);
	for(int k = 0; k < attributeCount; k++)
		schema->attrNames[k]= (char*) malloc(15);
	for(int k = 0; k < schema->numAttr; k++)
    	{
		strncpy(schema->attrNames[k], pageHandle, 15);
		pageHandle += 15;
		schema->dataTypes[k]= *(int*) pageHandle;
		pageHandle += sizeofint;
		schema->typeLength[k]= *(int*)pageHandle;
		pageHandle  += sizeofint;
	}
	rel->schema = schema;	
	unpinPage(&bufferPool, &recordManager->pageHandle);
	forcePage(&bufferPool, &recordManager->pageHandle);
	return RC_OK;
}   
  
extern RC closeTable (RM_TableData *rel)
{
	// RecordManager *recordManager = rel->mgmtData;
	shutdownBufferPool(&bufferPool);
	return RC_OK;
}
extern RC deleteTable (char *name)
{
	if(destroyPageFile(name) == RC_OK){
	return RC_OK;}
}
extern int getNumTuples (RM_TableData *rel)
{
	// RecordManager *recordManager = rel->mgmtData;
	return tuplesCount;
}
extern RC insertRecord (RM_TableData *rel, Record *record)
{
	
	RID *recordID = &record->id; 
	// ;
	RecordManager *recordManager = rel->mgmtData;	
	int recordSize = getRecordSize(rel->schema);
	recordID->page = freePage;
	pinPage(&bufferPool, &recordManager->pageHandle, recordID->page);
	char *data  = recordManager->pageHandle.data;
	int i =0;
	while(i< PAGE_SIZE / recordSize){
		if (data[i * recordSize] != '+')
			{recordID->slot = i;}
		i++;
	}
	recordID->slot = -1;
	
	while(recordID->slot == -1)
	{	
		unpinPage(&bufferPool, &recordManager->pageHandle);	
		recordID->page++;
		pinPage(&bufferPool, &recordManager->pageHandle, recordID->page);
		data = recordManager->pageHandle.data;
				int k;
				int count = 0;
				while (count != 4096/recordSize){
					char judge_str = recordManager->pageHandle.data[count * recordSize];
					if (judge_str != '+'){
						k = count;
						break;
					}
					count ++;
				}
				if (k == count) {}
				else {
					k = -1;
				}
				recordID->slot = k;	
			if (recordID->slot != -1){
				break;
			}
	}
	char *slotPointer;
	slotPointer = data;
	markDirty(&bufferPool, &recordManager->pageHandle);
	slotPointer  += (recordID->slot * recordSize);
	*slotPointer = '+';
	memcpy(++slotPointer, record->data + 1, recordSize - 1);
	unpinPage(&bufferPool, &recordManager->pageHandle);
	tuplesCount++;
	pinPage(&bufferPool, &recordManager->pageHandle, 0);
	return RC_OK;
}
extern RC deleteRecord (RM_TableData *rel, RID id)
{
	RecordManager *recordManager = rel->mgmtData;
	pinPage(&bufferPool, &recordManager->pageHandle, id.page);
	
	char *data = recordManager->pageHandle.data;
	// int recordSize = ;
	data  += (id.slot * getRecordSize(rel->schema));
	*data = '-';
	freePage = id.page;
	markDirty(&bufferPool, &recordManager->pageHandle);
	unpinPage(&bufferPool, &recordManager->pageHandle);
	return RC_OK;
}
extern RC updateRecord (RM_TableData *rel, Record *record)
{	
	RecordManager *recordManager = rel->mgmtData;
	pinPage(&bufferPool, &recordManager->pageHandle, record->id.page);
	// data;
	// int recordSize = getRecordSize(rel->schema);
	RID id = record->id;
	char *data = recordManager->pageHandle.data;
	data += (id.slot * getRecordSize(rel->schema));
	*data = '+';
	memcpy(++data, record->data + 1, getRecordSize(rel->schema) - 1 );
	markDirty(&bufferPool, &recordManager->pageHandle);
	unpinPage(&bufferPool, &recordManager->pageHandle);
	return RC_OK;	
}
extern RC getRecord (RM_TableData *rel, RID id, Record *record)
{
	BM_PageHandle *scanManager = rel->mgmtData;
	pinPage(&bufferPool, &recordManager->pageHandle, id.page);
	char *dataPointer = recordManager->pageHandle.data;
	dataPointer  += (id.slot * getRecordSize(rel->schema));
	
	if(*dataPointer == '+')
	{
		record->id = id;
		char *data = record->data;
		memcpy(++data, dataPointer + 1, getRecordSize(rel->schema) - 1);
	}
	unpinPage(&bufferPool, &recordManager->pageHandle);
	return RC_OK;
}
extern RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond)
{
	
	// if (cond != NULL)
	// {
		openTable(rel, "ScanTable");
		scan->mgmtData = (BM_PageHandle*) malloc(sizeof(BM_PageHandle));
		scan->rel= rel;
		scanCount = 0;
		expression = cond;
		tuplesCount = 15;
		recordID.page = 1;
		recordID.slot = 0;
		
		return RC_OK;
	// }
	
}
extern RC next (RM_ScanHandle *scan, Record *record)
{
	pH = scan->mgmtData;
    Schema *schema = scan->rel->schema;
	Value *result = (Value *) malloc(sizeof(Value));
	char *data;
	int totalSlots = PAGE_SIZE / getRecordSize(schema);
	int a = tuplesCount +1;
	for (int i=0; i <a;i++)
	{  
		if (scanCount <= 0)
		{
			recordID.page = 1;
			recordID.slot = 0;
		}
		else
		{
			recordID.slot++;
			if(recordID.slot >= totalSlots)
			{
				recordID.slot = 0;
				recordID.page++;
			}
		}
		
		pinPage(&bufferPool, pH, recordID.page);
		data = pH->data;
		data += (recordID.slot * getRecordSize(schema));
		record->id.page = recordID.page;
		record->id.slot = recordID.slot;
		char *dataPointer = record->data;
		*dataPointer = '-';
		memcpy(++dataPointer, data + 1, getRecordSize(schema) - 1);
		// scanCount+=2;
		scanCount++;
		evalExpr(record, schema, expression, &result);
		// bool judge = ;
		while (result->v.boolV){
			unpinPage(&bufferPool, pH);
			return RC_OK;
		}
	}
}
extern RC closeScan (RM_ScanHandle *scan)
{
	pH = scan->mgmtData;
	unpinPage(&bufferPool, pH);
	recordID.page = 1;
	recordID.slot = 0;
	scanCount = 0;
	free(scan->mgmtData);  
	return RC_OK;
}
extern int getRecordSize (Schema *schema)
{
	int size = 0, i=0; 
	while(i<schema->numAttr){
	int datatype = schema->dataTypes[i];
	if(datatype ==1)
		{size  += schema->typeLength[i];}
	else if( datatype ==0)
		{size += sizeofint;}
	else if( datatype ==2)
		{size += sizeof(float);}
	else if( datatype ==3)
		{size += sizeof(bool);}
		i++;
		}
	return ++size;
}
extern Schema *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys)
{
	Schema *schema = (Schema *) malloc(sizeof(Schema));
	schema->numAttr = numAttr;
	schema->attrNames = attrNames;
	schema->dataTypes = dataTypes;
	schema->typeLength = typeLength;
	schema->keySize = keySize;
	schema->keyAttrs = keys;
	return schema; 
}
extern RC freeSchema (Schema *schema)
{
	free(schema);
	return RC_OK;
}
extern RC createRecord (Record **record, Schema *schema)
{
	Record *newRecord = (Record*) malloc(sizeof(Record));   
	newRecord->data= (char*) malloc(getRecordSize(schema));
	newRecord->id.page = newRecord->id.slot = -1;
	char *dataPointer = newRecord->data;
	*dataPointer = '-';
	dataPointer++;
	*dataPointer = '\0';
	*record = newRecord;
	return RC_OK;
}
RC attrOffset2 (Schema *schema, int i, int *result)
{
	int datatype = schema->dataTypes[i];
		if(datatype ==1)
		{*result  += schema->typeLength[i];}
	else if( datatype ==0)
		{*result += sizeofint;}
	else if( datatype ==2)
		{*result += sizeof(float);}
	else if( datatype ==3)
		{*result += sizeof(bool);}
		return RC_OK;
	}
	
RC attrOffset (Schema *schema, int attrNum, int *result)
{
	int i;
	*result = 1;
	for(i = 0; i < attrNum; i++)
	{
		attrOffset2 (schema, i, result);
	}
	return RC_OK;
}
extern RC freeRecord (Record *record)
{
	free(record);
	return RC_OK;
}
extern RC getAttr (Record *record, Schema *schema, int attrNum, Value **value)
{
	int offset = 0;
	attrOffset(schema, attrNum, &offset);
	Value *attribute = (Value*) malloc(sizeof(Value));
	char *dataPointer = record->data;
	dataPointer  += offset;
	if(attrNum ==1){
		schema->dataTypes[attrNum] =1;
	}
	int datatype = schema->dataTypes[attrNum];
	attribute->dt = datatype;
	if( datatype ==1)
		{
			int length = schema->typeLength[attrNum];
			attribute->v.stringV = (char *) malloc(length + 1);
			memcpy(attribute->v.stringV, dataPointer, length);
			attribute->v.stringV[length] = '\0';
		}
	else {
		if( datatype ==0)
			{
				int value = 0;
				int length = sizeofint;
				memcpy(&value, dataPointer, sizeofint);
				attribute->v.intV = value;
			}
		else if( datatype ==2)
			{
				int length = sizeof(float);
				float value;
				memcpy(&value, dataPointer, sizeof(float));
				attribute->v.floatV = value;
			}
		else if( datatype ==3)
			{
				int length = sizeof(bool);
				bool value;
				memcpy(&value,dataPointer, sizeof(bool));
				attribute->v.boolV = value;
			}
		}
	*value = attribute;
	return RC_OK;
}
extern RC setAttr (Record *record, Schema *schema, int attrNum, Value *value)
{
	int offset = 0;
	attrOffset(schema, attrNum, &offset);
	char *dataPointer = record->data;
	dataPointer  += offset;
	
	if( schema->dataTypes[attrNum] ==1)
		{
			int length = schema->typeLength[attrNum];
			strncpy(dataPointer, value->v.stringV, length);
			dataPointer  += schema->typeLength[attrNum];
		}
	else if( schema->dataTypes[attrNum] ==0)
		{
			*(int *) dataPointer = value->v.intV;	  
			dataPointer  += sizeofint;
		}
	else if( schema->dataTypes[attrNum] ==2)
		{
			*(float *) dataPointer = value->v.floatV;
			dataPointer  += sizeof(float);
		}
	else if( schema->dataTypes[attrNum] ==3)
		{
			*(bool *) dataPointer = value->v.boolV;
			dataPointer += sizeof(bool);
		}
	return RC_OK;
}