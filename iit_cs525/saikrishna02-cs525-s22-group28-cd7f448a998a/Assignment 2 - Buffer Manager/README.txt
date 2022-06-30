RUNNING THE SCRIPT
=========================

1) Go to Project root (assign2) using Terminal.

2) Type ls to list the files and check that we are in the correct directory.

3) Type "make clean" to delete old compiled .o files.

4) Type "make" to compile all project files including "test_assign2_1.c" file 

5) Type "./test1" to run "test_assign2_1.c" file.


TEAM MEMBERS
=========================
Yixuan Liang
Ruhith
Chaochao Zhan

SOLUTION DESCRIPTION
===========================

MakeFile was made using following tutorial -
http://mrbook.org/blog/tutorials/make/

We have ensured proper memory management while making this storage manager by freeing any reserved space wherever possible. We have implemented FIFO (First In First Out), LRU (Least Recently Used), LFU (Least Frequently Used) and CLOCK page replacement algorithms.

1. BUFFER POOL FUNCTIONS
===========================

The buffer pool related functions are used to create a buffer pool for an existing page file on disk. The buffer pool is created in memory while the page file is present on disk. We make the use of Storage Manager (Assignment 1) to perform operations on page file on disk.

initBufferPool(...) 

shutdownBufferPool(...)


forceFlushPool(...)


2. PAGE MANAGEMENT FUNCTIONS
==========================


pinPage(...)

unpinPage(...)


makeDirty(...)


forcePage(....)



3. STATISTICS FUNCTIONS
===========================

The statistics related functions are used to gather some information about the buffer pool. So it provides various statistical information about the buffer pool.

getFrameContents(...)


getDirtyFlags(...)


getFixCounts(...) 


getNumReadIO(...)


getNumWriteIO(...)



4. PAGE REPLACEMENT ALGORITHM FUNCTIONS
=========================================

The page replacement strategy functions implement FIFO, LRU, LFU, CLOCK algorithms which are used while pinning a page. If the buffer pool is full and a new page has to be pinned, then a page should be replaced from the buffer pool. These page replacement strategies determine which page has to be replaced from the buffer pool.

FIFO(...)
--> First In First Out (FIFO) is the most basic page replacement strategy used.
--> FIFO is generally like a queue where the page which comes first in the buffer pool is in front and that page will be replaced first if the buffer pool is full.
--> Once the page is located, we write the content of the page frame to the page file on disk and then add the new page at that location.



LRU(...)
--> Least Recently Used (LRU) removes the page frame which hasn't been used for a long time (least recent) amongst the other page frames in the buffer pool.
--> The variable (field) hitNum in each page frame serves this purpose. hitNum keeps a count of of the page frames being accessed and pinned by the client. Also a global variable "hit" is used for this purpose.
--> So when we are using LRU, we just need to find the position of the page frame having the lowest value of hitNum.
--> We then write the content of the page frame to the page file on disk and then add the new page at that location.


 
TEST CASES 2
===============
We have added additional test cases in source file test_assign2_2.c. The instructions to run these test cases is mentioned in this README file. These test cases test LFU and CLOCK page replacement strategies.
