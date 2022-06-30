# Assignment 4: Index Manager

The objective is to implement a disk-based B+-tree index structure

### Group member
Yixuan Liang
Rohith
Chaochao Zhan

### To make/compile files...
```
make
```

### To run the file "test_expr"...
```
./test_expr
```

### To run the file "test_assign4"...
```
./test_assign4
```

### To remove old compiled .o files...
```
make clean
``` 



### init and shutdown index manager
These functions are used to initialize the index manager and shut it down, freeing up all acquired resources.

1. initIndexManager()

2. shutdownIndexManager()


### B+-tree Functions
These functions are used to create or delete a b-tree index

1. createBtree()
	

2. openBtree()
	
3. closeBtree()
	

4. deleteBtree()
	

### Functions to Access Information about B+- tree

1. getNumNodes()
	
2. getNumEntries()
	
3. getKeyType()
	
###Index Access Functions

1. findKey()
	
2. insertKey()


3. deleteKey()
	
	
4. openTreeScan()


5. nextEntry()
	

6. closeTreeScan()
	



