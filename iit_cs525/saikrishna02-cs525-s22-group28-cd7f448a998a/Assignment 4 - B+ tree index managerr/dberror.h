#ifndef DBERROR_H
#define DBERROR_H

#include <stdlib.h>
#include "stdio.h"
#include <errno.h>
#define PAGE_SIZE 4096
/* return code definitions */
typedef int RC;

#define RC_OK 0

#define RC_FILE_NOT_FOUND 1
#define RC_FILE_HANDLE_NOT_INIT 2
#define RC_WRITE_FAILED 3
#define RC_READ_NON_EXISTING_PAGE 4

#define RC_FS_ERROR 6

#define RC_RM_COMPARE_VALUE_OF_DIFFERENT_DATATYPE 200
#define RC_RM_EXPR_RESULT_IS_NOT_BOOLEAN 201
#define RC_RM_BOOLEAN_EXPR_ARG_IS_NOT_BOOLEAN 202
#define RC_RM_NO_MORE_TUPLES 203
#define RC_RM_NO_PRINT_FOR_DATATYPE 204
#define RC_RM_UNKOWN_DATATYPE 205
#define RC_ERROR_PAGE 206
#define RC_ERROR_OTHER 207
#define RC_ERROR_Frame_Number 208
#define RC_ERROR_No_BF 209
#define RC_ERROR_No_PG 210
#define RC_ERROR_No_FIND_NODE 211
#define RC_ERROR_ALL_NODE_IN_PIN 212
#define RC_ERROR_ALL_INVALID_BIN 213
#define RC_NOT_OPEN 214
#define RC_NOT_Datatype 215
#define RC_ERROR_No_TUP 216
#define RC_Expression_NOT_FOUND 217
#define RC_other_error 218
#define RC_IM_KEY_NOT_FOUND 300
#define RC_IM_KEY_ALREADY_EXISTS 301
#define RC_IM_N_TO_LAGE 302
#define RC_IM_NO_MORE_ENTRIES 303
#define RC_ERROR 400 // Added a new definiton for ERROR
#define RC_PINNED_PAGES_IN_BUFFER 401 // Added a new definition for Buffer Manager

#define RC_GENERAL_ERROR 500
#define RC_SCAN_CONDITION_NOT_FOUND 601

/* holder for error messages */
extern char *RC_message;

/* print a message to standard out describing the error */
extern void printError (RC error);
extern char *errorMessage (RC error);


#define newArray(type, size) (type *) malloc(sizeof(type) * size)
#define newCleanArray(type, size) (type *) calloc(size, sizeof(type))
#define new(type) newArray(type, 1)
#define newStr(size) newCleanArray(char, size + 1) // +1 for \0 terminator
#define newIntArr(size) newArray(int, size)
#define newFloatArr(size) newArray(float, size)
#define newCharArr(size) newArray(char, size)


#define THROW(rc,message) \
  do {			  \
    RC_message=message;	  \
    return rc;		  \
  } while (0)		  \

// check the return code and exit if it is an error
#define CHECK(code)							\
  do {									\
    int rc_internal = (code);						\
    if (rc_internal != RC_OK)						\
      {									\
	char *message = errorMessage(rc_internal);			\
	printf("[%s-L%i-%s] ERROR: Operation returned error: %s\n",__FILE__, __LINE__, __TIME__, message); \
	free(message);							\
	exit(1);							\
      }									\
  } while(0);

void throwError();

#endif
