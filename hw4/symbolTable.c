#include "symbolTable.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
// This file is for reference only, you are not required to follow the implementation. //

int HASH(const char *);

SymbolTable symbolTable;

SymbolTableEntry* newSymbolTableEntry(){
    SymbolTableEntry* symbolTableEntry = malloc(sizeof(SymbolTableEntry));
    memset(symbolTableEntry, 0, sizeof(SymbolTableEntry));
    return symbolTableEntry;
}

void initializeSymbolTable(){
  symbolTable.tables = NULL;
  openScope();
}

void symbolTableEnd(){
  while(symbolTable.tables != NULL){
    closeScope();
  }
}

void openScope(){
  SymbolTableScope * scope = malloc(sizeof(SymbolTableScope));
  memset(scope, 0, sizeof(SymbolTableScope));
  scope->prev = symbolTable.tables;
  symbolTable.tables = scope;
}

void closeScope(){
  /* free the memory, no memory leak is good */
  SymbolTableScope * scope = symbolTable.tables;
  symbolTable.tables = scope->prev;
  for(int i = 0; i < HASH_TABLE_SIZE; ++i){
    SymbolTableEntry * entry = scope->hashTable[i], * temp;
    while(entry != NULL){
      temp = entry->next;
      free(entry);
      entry = temp;
    }
  }
  free(scope);
}

SymbolTableEntry* retrieveSymbol(const char * symbolName){
  int hashv = HASH(symbolName);
  SymbolTableScope * scope = symbolTable.tables;
  while(scope != NULL){
    SymbolTableEntry * entry = scope->hashTable[hashv];
    while(entry != NULL){
      if(strcmp(symbolName, entry->name) == 0){
        // found !!
        return entry;
      }
      entry = entry->next;
    }
    // not found in this scope, go up one level then
    scope = scope->prev;
  }
  // cannot find symbol
  return NULL;
}

SymbolTableEntry* enterSymbol(const char * symbolName, SymbolAttribute* attribute){
  if(declaredLocally(symbolName)){
    return NULL;
  }
  int hashv = HASH(symbolName);
  SymbolTableEntry * entry = newSymbolTableEntry();
  entry->next = symbolTable.tables->hashTable[hashv];
  symbolTable.tables->hashTable[hashv] = entry;
  if(entry->next != NULL){
    entry->next->prev = entry;
  }
  entry->prev = NULL;
  entry->name = symbolName;
  entry->symbolAttribute = attribute;
  return entry;
}

//remove the symbol from the current scope
void removeSymbol(const char * symbolName){
  int hashv = HASH(symbolName);
  SymbolTableEntry * entry = symbolTable.tables->hashTable[hashv];
  while(entry != NULL){
    if(strcmp(symbolName, entry->name) == 0){
      // found !!
      if(entry->prev != NULL){
        entry->prev->next = entry->next;
      }
      if(entry->next != NULL){
        entry->next->prev = entry->prev;
      }
      free(entry);
      break;
    }
    entry = entry->next;
  }
}

int declaredLocally(const char * symbolName){
  int hashv = HASH(symbolName);
  SymbolTableEntry * entry = symbolTable.tables->hashTable[hashv];
  while(entry != NULL){
    if(strcmp(symbolName, entry->name) == 0){
      // symbol already exists in current scope
      return 1;
    }
    entry = entry->next;
  }
  return 0;
}

int HASH(const char * str) {
  int idx=0;
  while (*str){
    idx = idx << 1;
    idx+=*str;
    str++;
  }
  return (idx & (HASH_TABLE_SIZE-1));
}

