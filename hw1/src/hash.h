#ifndef __SILVERLEEK__HASH__
#define __SILVERLEEK__HASH__

#include <stdlib.h>

typedef struct trie{
  int endHere;
  int index;
  struct trie * branches[30];
} trie;

int __index;
trie __root;

int insert(trie *root, char *s){
  if(s[0] == '\0'){
    ++root->endHere;
    root->index = __index++;
    return root->index;
  }else{
    trie *branch = root->branches[(int)s[0] - 'a'];
    if(branch == NULL){
      branch = calloc(1, sizeof(trie));
    }
    int index = insert(branch, s+1);
    root->branches[(int)s[0] - 'a'] = branch;
    return index;
  }
}

int exist(trie *root, char *s){
  if(root == NULL) return -1;
  if(s[0] == '\0'){
    if(! root->endHere) return -1;
    else return root->index;
  }else{
    trie *branch = root->branches[(int)s[0] - 'a'];
    return exist(branch, s+1);
  }
}

int hash(char *s){
  int index = exist(&__root, s);
  if(index != -1){
    return index;
  }else{
    return insert(&__root, s);
  }
}

#endif
