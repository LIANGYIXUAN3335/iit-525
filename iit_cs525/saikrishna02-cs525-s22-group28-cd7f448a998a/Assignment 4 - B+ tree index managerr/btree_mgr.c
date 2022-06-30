#include "btree_mgr.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include "string.h"
#include <stdarg.h>
#include <unistd.h>
#include <math.h>
btree* copy_leaf_node(btree *node1,btree *node2,int silbling){
  memcpy(node1->values->Item, node2->values->Item + silbling, sizeof(int)* node2->values->store);
  memcpy(node1->Leaf_Page->Item, node2->Leaf_Page->Item + silbling, sizeof(int)* node2->Leaf_Page->store);  
  memcpy(node1->Leaf_Spot->Item, node2->Leaf_Spot->Item + silbling, sizeof(int)* node2->Leaf_Spot->store);
  return node1;
}
btree* copy_int(btree *node1,btree *node2,int sibling1,int sibling2){
  memcpy(node1->values->Item, node2->values->Item, sizeof(int)* sibling1);
  memcpy(node1->Child_Page->Item, node2->Child_Page->Item, sizeof(int)* sibling2);
  memcpy(node1->Child_Node, node2->Child_Node, sizeof(btree *) * sibling2);
  return node1;
}
int minus_1 = -1;
int len =1;
int zero = 0;
int Sort_Find(Sort *list, int element, int *suit) {
  int initial = zero;
  int end= list->store - len;
  if (end < zero) {                              
    (*suit) = initial;
    return minus_1;
  }
  int pos;
  while(true) {
    pos = (initial + end) / 2;              
    if(element == list->Item[pos]) {
      (*suit) = pos;
      return pos;
    }
    if(initial >= end) {
      if (element > list->Item[initial]) {
        initial +=  len;
      }
      (*suit) = initial;
      return minus_1;
    }
    if(element > list->Item[pos]) {
      initial = pos + len;  
    }
    else {
      end = pos -   len;       
    }
  }
}
Sort *Sort_Initialize(int len) {
  Sort *list =new(Sort);
  list->Item = newIntArr(len);
  list->store = zero;
  list->len = len;
  return list;
}

int Sort_Place_Pos(Sort *list, int element, int index) {
  if (list->len <= list->store || index > list->store) {
    return minus_1;
  }
  if (index != list->store) {
    for(int i= list->store;i > index;i--) {
      list->Item[i] = list->Item[i -   len];
    }
  }
  list->Item[index] = element;
  list->store +=  len;
  return index;
}

int Sort_Place(Sort *list, int element) {
  int suit = minus_1;                                  
  if (list->len < list->store) {
    return suit;
  }
  Sort_Find(list, element, &suit);
  suit = Sort_Place_Pos(list, element, suit);
  return suit;
}

void Sort_Remove_Pos(Sort *list, int index, int count) {
  list->store = list->store - count;
  for(int i = index;i < list->store;i +=  len) {
    list->Item[i] = list->Item[i + count];
  }
}

void MakeEmpty(int number, ...) {
  va_list var;
  va_start(var, number);
  for(int i =zero;i < number;i +=  len) {
    free(va_arg(var, void *));
  }
  va_end(var);
}

btree *Build_Btree_Node(int capacity, int IsLeaf, int PageNumber) {
  btree *node = new(btree);
  node->PageNumber = PageNumber;
  node->capacity = capacity;
  node->IsLeaf = IsLeaf;

  Sort *list =new(Sort);
  list->Item = newIntArr(capacity);
  list->store = zero;
  list->len = capacity;
  node->values = list;

  node->Parent_Node = NULL;
  node->Right_Node = NULL;
  node->Left_Node = NULL;
  if(!IsLeaf) {
    node->Child_Page = Sort_Initialize(capacity +   len);
    node->Child_Node = newArray(btree *, capacity +   len);
  }else{
    node->Leaf_Page = Sort_Initialize(capacity);
    node->Leaf_Spot = Sort_Initialize(capacity);
  }
  return node;
}

void Remove_Btree_Node(btree *node) {
  free(node->values);
  if (node->IsLeaf){
    free(node->Leaf_Page);
    free(node->Leaf_Spot);
  }else{
    free(node->Child_Page);
    free(node->Child_Node);
  }
  free(node);
}

RC Scan_Btree_Node(btree **node, BTreeHandle *binary_tree, int PageNumber) {
  BM_PageHandle *pg_handle;
  pg_handle = new(BM_PageHandle);
  pinPage(binary_tree->mgmtData, pg_handle, PageNumber);
  int IsLeaf ;
  char *pointer;
  pointer = pg_handle->data;
  memcpy(&IsLeaf, pointer, sizeof(int));
  pointer += sizeof(int);
  int store;
  memcpy(&store, pointer, sizeof(int));
  pointer = pg_handle->data + HEADER_LENGTH;
  btree *Node;
  Node = Build_Btree_Node(binary_tree->capacity, IsLeaf, PageNumber);
  int value;
  int Child_Node_Page;
  int RID_Page, RID_Spot;
  if (!IsLeaf) {
    for(int i =zero;i < store;i +=  len) {
      memcpy(&Child_Node_Page, pointer, sizeof(int));
      pointer += sizeof(int);
      memcpy(&value, pointer, sizeof(int));
      pointer += sizeof(int);
      Sort_Place_Pos(Node->Child_Page, Child_Node_Page, i);
      Sort_Place_Pos(Node->values, value, i);
  }
  memcpy(&Child_Node_Page, pointer, sizeof(int));
  Sort_Place_Pos(Node->Child_Page, Child_Node_Page, store);
  }else{
    for(int i = zero;i < store;i +=  len) {
      memcpy(&RID_Page, pointer, sizeof(int));
      pointer +=  sizeof(int);
      memcpy(&RID_Spot, pointer, sizeof(int));
      pointer += sizeof(int);
      memcpy(&value, pointer, sizeof(int));
      pointer +=  sizeof(int);
      Sort_Place_Pos(Node->values, value, i);
      Sort_Place_Pos(Node->Leaf_Page, RID_Page, i);
      Sort_Place_Pos(Node->Leaf_Spot, RID_Spot, i);
    }
  }
  unpinPage(binary_tree->mgmtData, pg_handle);
  free(pg_handle);
  *node = Node;
  return RC_OK;
}

RC Write_Btree_Node(btree *node, BTreeHandle *binary_tree) {
  BM_PageHandle *pg_handle;
  pg_handle = new(BM_PageHandle);
  pinPage(binary_tree->mgmtData, pg_handle, node->PageNumber);
  char *pointer;
  pointer = pg_handle->data;
  memcpy(pointer, &node->IsLeaf, sizeof(int));
  pointer +=  sizeof(int);
  memcpy(pointer, &node->values->store, sizeof(int));
  pointer = pg_handle->data + HEADER_LENGTH;
  if (!node->IsLeaf){
    for(int i =zero;i < node->values->store;i +=  len) {
      memcpy(pointer, &node->Child_Page->Item[i], sizeof(int));
      pointer += sizeof(int);
      memcpy(pointer, &node->values->Item[i], sizeof(int));
      pointer += sizeof(int);
  }
  memcpy(pointer, &node->Child_Page->Item[node->values->store], sizeof(int));
  }else{
    for(int i =zero;i < node->values->store;i +=  len) {
      memcpy(pointer, &node->Leaf_Page->Item[i], sizeof(int));
      pointer +=  sizeof(int);
      memcpy(pointer, &node->Leaf_Spot->Item[i], sizeof(int));
      pointer += sizeof(int);
      memcpy(pointer, &node->values->Item[i], sizeof(int));
      pointer += sizeof(int);
    }
  }
  markDirty(binary_tree->mgmtData, pg_handle);
  unpinPage(binary_tree->mgmtData, pg_handle);
  forceFlushPool(binary_tree->mgmtData);
  free(pg_handle);
  return RC_OK;
}

RC Load_Btree_Node(BTreeHandle *binary_tree, btree *root, btree **left_node, int level) {
  btree *Left_Node= left_node[level];
  if(!root->IsLeaf) {
    for(int i =zero;i < root->Child_Page->store;i +=  len) {
      Scan_Btree_Node(&root->Child_Node[i], binary_tree, root->Child_Page->Item[i]);
      root->Child_Node[i]->Left_Node = Left_Node;
      if (Left_Node) {
        Left_Node->Right_Node = root->Child_Node[i];
      }
      Left_Node = root->Child_Node[i];
      root->Child_Node[i]->Parent_Node = root;
      left_node[level] = Left_Node;
    }
  }
  return RC_OK;
}

btree *Locate_Node(BTreeHandle *binary_tree, int key) {
  btree *current= binary_tree->root;
  int place;
  while(current && !current->IsLeaf) {
    if (Sort_Find(current->values, key, &place) >= zero) {
      place +=   len;
    }
    current = current->Child_Node[place];
  }
  return current;
}

RC Load_Btree(BTreeHandle *binary_tree) {
  binary_tree->root = NULL;
  if (binary_tree->level) {
    Scan_Btree_Node(&binary_tree->root, binary_tree, binary_tree->Root_Location);
    btree **left_node;
    left_node = newArray(btree *, binary_tree->level);
    for(int i =zero;i < binary_tree->level;i +=  len) {
      left_node[i] = NULL;
    }
    Load_Btree_Node(binary_tree, binary_tree->root, left_node, zero);
    free(left_node);
  }
  return RC_OK;
}

RC Output_Btree(BTreeHandle *binary_tree) {
  BM_BufferPool *bm = binary_tree->mgmtData;
  BM_PageHandle *pg_handle; 
  pg_handle = new(BM_PageHandle);
  pinPage(bm, pg_handle, zero);
  markDirty(bm, pg_handle);
  unpinPage(bm, pg_handle);
  forceFlushPool(bm);
  MakeEmpty(len, pg_handle);
  return RC_OK;
}

RC Move_Btree_Parent(BTreeHandle *binary_tree, btree *Left_Node, btree *Right_Node, int key) {
  btree *Parent_Node= Left_Node->Parent_Node;
  int index, i;
  if(Parent_Node) {
  }
  else{
    Parent_Node = Build_Btree_Node(binary_tree->capacity, zero, binary_tree->Next);
    Sort_Place_Pos(Parent_Node->Child_Page, Left_Node->PageNumber, zero);
    Parent_Node->Child_Node[zero] = Left_Node;
    binary_tree->Next +=  len;
    binary_tree->level +=  len;
    binary_tree->Root_Location = Parent_Node->PageNumber;
    binary_tree->TotalNodes +=  len;
    binary_tree->root = Parent_Node;
    Output_Btree(binary_tree);
  }
  Right_Node->Parent_Node = Parent_Node;
  Left_Node->Parent_Node = Parent_Node;
  index = Sort_Place(Parent_Node->values, key);
  btree * overflowed = NULL;
  if (index >= zero) {
    index  +=  len;
    Sort_Place_Pos(Parent_Node->Child_Page, Right_Node->PageNumber, index);
    
    for(int i = Parent_Node->values->store;i > index;i--) {
      Parent_Node->Child_Node[i] = Parent_Node->Child_Node[i -   len];
    }
    Parent_Node->Child_Node[index] = Right_Node;
    return Write_Btree_Node(Parent_Node, binary_tree);
  }else{
    overflowed = Build_Btree_Node(binary_tree->capacity +   len, zero, minus_1);
    overflowed->values->store = Parent_Node->values->store;
    overflowed->Child_Page->store = Parent_Node->Child_Page->store;
    memcpy(overflowed->values->Item, Parent_Node->values->Item, sizeof(int)* Parent_Node->values->store);
    memcpy(overflowed->Child_Page->Item, Parent_Node->Child_Page->Item, sizeof(int)* Parent_Node->Child_Page->store);
    memcpy(overflowed->Child_Node, Parent_Node->Child_Node, sizeof(btree *) * Parent_Node->Child_Page->store);
    index = Sort_Place(overflowed->values, key);
    Sort_Place_Pos(overflowed->Child_Page, Right_Node->PageNumber, index + len);
    
    for(int i = Parent_Node->Child_Page->store;i > index +   len;i--) {
      overflowed->Child_Node[i] = overflowed->Child_Node[i -   len];
      
    }
    overflowed->Child_Node[index +   len] = Right_Node;
    int sibling = overflowed->values->store;
    int left_sibling = sibling / 2;
    int right_sibling = sibling - left_sibling;
    btree *rp = Build_Btree_Node(binary_tree->capacity, zero, binary_tree->Next);
    binary_tree->Next +=  len;
    binary_tree->TotalNodes +=  len;
    Parent_Node->values->store = left_sibling;
    Parent_Node->Child_Page->store = left_sibling +   len;
    int size_left = Parent_Node->Child_Page->store;
    memcpy(Parent_Node->values->Item, overflowed->values->Item, sizeof(int)* left_sibling);
    memcpy(Parent_Node->Child_Page->Item, overflowed->Child_Page->Item, sizeof(int)* size_left);
    memcpy(Parent_Node->Child_Node, overflowed->Child_Node, sizeof(btree *) * size_left);
    
    rp->values->store = right_sibling;
    rp->Child_Page->store = overflowed->Child_Page->store - size_left;
    int size_right = rp->Child_Page->store;
    memcpy(rp->values->Item, overflowed->values->Item + left_sibling, sizeof(int)* right_sibling);
    memcpy(rp->Child_Page->Item, overflowed->Child_Page->Item + size_left, sizeof(int)* size_right);
    memcpy(rp->Child_Node, overflowed->Child_Node + size_left, sizeof(btree *) * size_right);
    
    Remove_Btree_Node(overflowed);
    key = rp->values->Item[zero];
    rp->values->store = rp->values->store -   len;
    for(int i = zero;i < rp->values->store;i +=  len) {
      rp->values->Item[i] = rp->values->Item[i +   len];
    }
    rp->Right_Node = Parent_Node->Right_Node;
    if (rp->Right_Node ) {
      rp->Right_Node->Left_Node = rp;
    }
    Parent_Node->Right_Node = rp;
    rp->Left_Node = Parent_Node;
    Write_Btree_Node(Parent_Node, binary_tree);
    Write_Btree_Node(rp, binary_tree);
    Output_Btree(binary_tree);
    return Move_Btree_Parent(binary_tree, Parent_Node, rp, key);
  }
}
void Remove_Btree_Nodes(btree *root) {
  if (!root) {
    return;
  }
  btree *leaf_node= root;
  while(leaf_node->IsLeaf){
    leaf_node = leaf_node->Child_Node[zero];
  }
  btree *Parent_Node= leaf_node->Parent_Node;
  while (true) {
    while(leaf_node){
      Remove_Btree_Node(leaf_node);
      leaf_node = leaf_node->Right_Node;
    }
    if (!Parent_Node) {
      break;
    }
    leaf_node = Parent_Node;
    Parent_Node = leaf_node->Parent_Node;
  }
}
RC initIndexManager (void *mgmtData) {
  return RC_OK;
}
RC shutdownIndexManager () {
  return RC_OK;
}
RC createBtree(char *idxId, DataType keyType, int n)
{
  if (n > (PAGE_SIZE - HEADER_LENGTH) / (3 * sizeof(int))) {       
    return RC_IM_N_TO_LAGE;
  }
  createPageFile (idxId);
  SM_FileHandle *fh_handle = new(SM_FileHandle);                
  openPageFile(idxId, fh_handle);
  char *start = newCleanArray(char, PAGE_SIZE);
  char *pointer = start;
  memcpy(pointer, &n, sizeof(int));
  pointer += sizeof(int);
  memcpy(pointer, &keyType, sizeof(int));
  pointer += sizeof(int);
  int Root_Location = zero;
  memcpy(pointer, &Root_Location, sizeof(int));
  pointer += sizeof(int);
  int TotalNodes = zero;
  memcpy(pointer, &TotalNodes, sizeof(int));
  pointer += sizeof(int);
  int TotalEntries = zero;
  memcpy(pointer, &TotalEntries, sizeof(int));
  pointer += sizeof(int);
  int level = zero;
  memcpy(pointer, &level, sizeof(int));
  int Next =   len;
  pointer += sizeof(int);
  memcpy(pointer, &Next, sizeof(int));
  writeBlock(zero, fh_handle, start);
  free(start);
  closePageFile(fh_handle);          
  free(fh_handle);
  return RC_OK;
}
RC openBtree (BTreeHandle **binary_tree, char *idxId){
  BTreeHandle *bin_tree= new(BTreeHandle);
  BM_BufferPool *bm= new(BM_BufferPool);
  initBufferPool(bm, idxId, TABLE_BUFFER_LENGTH, RS_LRU, NULL);
  BM_PageHandle *pg_handle = new(BM_PageHandle);         
  pinPage(bm, pg_handle, zero);
  char *pointer= pg_handle->data;
  
  bin_tree->idxId = idxId;
  bin_tree->mgmtData = bm;
  memcpy(&bin_tree->capacity, pointer, sizeof(int));
  pointer += sizeof(int);
  memcpy(&bin_tree->keyType, pointer, sizeof(int));
  pointer += sizeof(int);
  memcpy(&bin_tree->Root_Location, pointer, sizeof(int));
  pointer += sizeof(int);
  memcpy(&bin_tree->TotalNodes, pointer, sizeof(int));
  pointer += sizeof(int);
  memcpy(&bin_tree->TotalEntries, pointer, sizeof(int));
  pointer += sizeof(int);
  memcpy(&bin_tree->level, pointer, sizeof(int));
  pointer += sizeof(int);
  memcpy(&bin_tree->Next, pointer, sizeof(int));
  unpinPage(bm, pg_handle);
  MakeEmpty(len, pg_handle);
  Load_Btree(bin_tree);
  *binary_tree = bin_tree;
  return RC_OK;
}

RC getNumNodes (BTreeHandle *binary_tree, int *output) {
  *output = binary_tree->TotalNodes;
  return RC_OK;
}


RC getNumEntries (BTreeHandle *binary_tree, int *output) {
  *output = binary_tree->TotalEntries;
  return RC_OK;
}


RC getKeyType (BTreeHandle *binary_tree, DataType *output) {
  *output = binary_tree->keyType;
  return RC_OK;
}
RC closeBtree (BTreeHandle *binary_tree){
  shutdownBufferPool(binary_tree->mgmtData);                  
  Remove_Btree_Nodes(binary_tree->root);
  MakeEmpty(2, binary_tree->mgmtData, binary_tree);
  return RC_OK;                                           
}


RC deleteBtree (char *idxId) {
  unlink(idxId);
  return RC_OK;                               
}

RC findKey (BTreeHandle *binary_tree, Value *key, RID *output) {
  int place;
  btree *leaf_node;
  leaf_node = Locate_Node(binary_tree, key->v.intV);
  int index = Sort_Find(leaf_node->values, key->v.intV, &place);
  if (index >= zero) {
    output->page = leaf_node->Leaf_Page->Item[index];
    output->slot = leaf_node->Leaf_Spot->Item[index];
    return RC_OK;
  }
  return RC_IM_KEY_NOT_FOUND;
}


RC insertKey (BTreeHandle *binary_tree, Value *key, RID rid) {
  btree *leaf_node= Locate_Node(binary_tree, key->v.intV); 
  // leaf_node                            
  if (!leaf_node) {
    leaf_node = Build_Btree_Node(binary_tree->capacity, len, binary_tree->Next);
    binary_tree->Next +=  len;
    binary_tree->Root_Location = leaf_node->PageNumber;
    binary_tree->TotalNodes +=  len;
    binary_tree->level +=  len;
    binary_tree->root = leaf_node;
    Output_Btree(binary_tree);
  }
  int place;
  Sort_Find(leaf_node->values, key->v.intV, &place);
  int index = Sort_Place(leaf_node->values, key->v.intV);
  if (index < zero) {
    btree * overflowed = Build_Btree_Node(binary_tree->capacity +   len, len, minus_1);
    overflowed = copy_leaf_node(overflowed,leaf_node,zero);
    overflowed->Leaf_Page->store = leaf_node->Leaf_Page->store;
    overflowed->values->store = leaf_node->values->store;
    overflowed->Leaf_Spot->store = leaf_node->Leaf_Spot->store;
    index = Sort_Place(overflowed->values, key->v.intV);
    Sort_Place_Pos(overflowed->Leaf_Page, rid.page, index);
    Sort_Place_Pos(overflowed->Leaf_Spot, rid.slot, index);
    int left_sibling = ceil((float) overflowed->values->store / 2);
    int right_sibling = overflowed->values->store - left_sibling;
    btree *rleaf = Build_Btree_Node(binary_tree->capacity, len, binary_tree->Next);
    binary_tree->Next +=  len;
    binary_tree->TotalNodes +=  len;
    leaf_node->values->store = leaf_node->Leaf_Page->store = leaf_node->Leaf_Spot->store = left_sibling;
    leaf_node=copy_leaf_node(leaf_node,overflowed,zero);
    
    rleaf->values->store = rleaf->Leaf_Page->store = rleaf->Leaf_Spot->store = right_sibling;
    rleaf = copy_leaf_node(rleaf,overflowed,left_sibling);
    Remove_Btree_Node(overflowed);
    rleaf->Right_Node = leaf_node->Right_Node;
    if (rleaf->Right_Node) {
      rleaf->Right_Node->Left_Node = rleaf;
    }
    leaf_node->Right_Node = rleaf;
    rleaf->Left_Node = leaf_node;
    Write_Btree_Node(rleaf, binary_tree);
    Write_Btree_Node(leaf_node, binary_tree);
    Move_Btree_Parent(binary_tree, leaf_node, rleaf, rleaf->values->Item[zero]);
  }else {
    Sort_Place_Pos(leaf_node->Leaf_Page, rid.page, index);
    Sort_Place_Pos(leaf_node->Leaf_Spot, rid.slot, index);
  }
  binary_tree->TotalEntries +=  len;
  Output_Btree(binary_tree);
  return RC_OK;
}


RC deleteKey (BTreeHandle *binary_tree, Value *key) {
  btree *leaf_node;
  leaf_node = Locate_Node(binary_tree, key->v.intV);                
  if (leaf_node) {
    int index, _unused;
    index = Sort_Find(leaf_node->values, key->v.intV, &_unused);
    if (index >= zero) {
      leaf_node->values->store = leaf_node->values->store -   len;
      for(int i = index;i < leaf_node->values->store;i +=  len) {
        leaf_node->values->Item[i] = leaf_node->values->Item[i + len];
      }
      Sort_Remove_Pos(leaf_node->Leaf_Page, index, len);
      Sort_Remove_Pos(leaf_node->Leaf_Spot, index, len);
      binary_tree->TotalEntries--;
      Write_Btree_Node(leaf_node, binary_tree);
      Output_Btree(binary_tree);
    }
  }
  return RC_OK;
}


RC openTreeScan (BTreeHandle *binary_tree, BT_ScanHandle **handle){
  BT_ScanHandle *scan_handle= new(BT_ScanHandle);                        
  ScanInfo *data=new(ScanInfo);
  scan_handle->tree = binary_tree;
  data->presentNode = binary_tree->root;
  while(!data->presentNode->IsLeaf) {
    data->presentNode = data->presentNode->Child_Node[zero];
  }
  data->pos = zero;
  scan_handle->mgmtData = data;
  *handle = scan_handle;
  return RC_OK;
}


RC nextEntry (BT_ScanHandle *handle, RID *output){
  ScanInfo *data = handle->mgmtData; 
  if(data->pos >= data->presentNode->Leaf_Page->store) {
    if(data->pos == data->presentNode->values->store && !data->presentNode->Right_Node){
      return RC_IM_NO_MORE_ENTRIES;
    }else{
      data->presentNode = data->presentNode->Right_Node;
      data->pos = zero;
    }
  }
  output->page = data->presentNode->Leaf_Page->Item[data->pos];
  output->slot = data->presentNode->Leaf_Spot->Item[data->pos];
  data->pos +=  len;
  return RC_OK;
}


RC closeTreeScan (BT_ScanHandle *handle){
  MakeEmpty(2, handle->mgmtData, handle);
  return RC_OK;
}


char *printTree (BTreeHandle *binary_tree){
  char *output;
  return output;
}
