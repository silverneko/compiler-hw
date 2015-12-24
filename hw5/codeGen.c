#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"
#include "symbolTable.h"

typedef AST_NODE AST;

void emitCode(AST_NODE *);
void emitProgramNode(AST_NODE *);
void emitGlobalDeclarations(AST *);
void emitGlobalDeclaration(AST *);
void emitFunctionDeclaration(AST *);
void emitAlignment();

char * idName(AST * node);

FILE * adotout;

void emitAlignment(){
  fprintf(adotout, ".align 3\n");
}

char * idName(AST * node){
  return node->semantic_value.identifierSemanticValue.identifierName;
}

void emitCode(AST_NODE * root){
  adotout = fopen("output.s", "w");
  emitProgramNode(root);
  fclose(adotout);
}

void emitProgramNode(AST_NODE * root){
  AST * decls = root->child;
  while(decls != NULL){
    if(decls->nodeType == VARIABLE_DECL_LIST_NODE){
      emitGlobalDeclarations(decls);
    }else{
      // function decl
      emitFunctionDeclaration(decls);
    }
    decls = decls->rightSibling;
  }
}

void emitGlobalDeclarations(AST * root){
  AST * decls = root->child;
  fprintf(adotout, ".data\n");
  while(decls){
    emitGlobalDeclaration(decls);
    decls = decls->rightSibling;
  }
  emitAlignment();
}

void emitGlobalDeclaration(AST * decl){
  AST * typeNode = decl->child;
  processTypeNode(typeNode);
  AST * idlist = typeNode->rightSibling;
  switch(decl->semantic_value.declSemanticValue.kind){
    case VARIABLE_DECL:
    case TYPE_DECL:
      while(idlist){
        SymbolAttribute * attribute = malloc(sizeof(SymbolAttribute));
        if(decl->semantic_value.declSemanticValue.kind == VARIABLE_DECL){
          attribute->attributeKind = VARIABLE_ATTRIBUTE;
        }else{
          attribute->attributeKind = TYPE_ATTRIBUTE;
        }
        switch(idlist->semantic_value.identifierSemanticValue.kind){
          case NORMAL_ID:
            attribute->attr.typeDescriptor = typeNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor;
            break;
          case WITH_INIT_ID:
            attribute->attr.typeDescriptor = typeNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor;
            break;
          case ARRAY_ID:
            attribute->attr.typeDescriptor = malloc(sizeof(TypeDescriptor));
            processDeclDimList(idlist, attribute->attr.typeDescriptor, 0);
            if(typeNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor->kind == SCALAR_TYPE_DESCRIPTOR){
              attribute->attr.typeDescriptor->properties.arrayProperties.elementType =
                typeNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor->properties.dataType;
            }else if(typeNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor->kind == ARRAY_TYPE_DESCRIPTOR){
              int typeArrayDimension = typeNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor->properties.arrayProperties.dimension;
              int idArrayDimension = attribute->attr.typeDescriptor->properties.arrayProperties.dimension;
              attribute->attr.typeDescriptor->properties.arrayProperties.elementType = 
                typeNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor->properties.arrayProperties.elementType;
              attribute->attr.typeDescriptor->properties.arrayProperties.dimension = 
                typeArrayDimension + idArrayDimension;
              int indexType = 0;
              int indexId = 0;
              for(indexType = 0, indexId = idArrayDimension; indexId < idArrayDimension + typeArrayDimension; ++indexType, ++indexId)
              {
                attribute->attr.typeDescriptor->properties.arrayProperties.sizeInEachDimension[indexId] = 
                  typeNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor->properties.arrayProperties.sizeInEachDimension[indexType];
              }
            }
            break;
        }
        if(decl->semantic_value.declSemanticValue.kind == VARIABLE_DECL){
          if(attribute->attr.typeDescriptor->kind == SCALAR_TYPE_DESCRIPTOR){
            fprintf(adotout, "\t_%s: .word\n", idName(idlist));
          }else{
            int arraySize = 4;
            int arrayDim = attribute->attr.typeDescriptor->properties.arrayProperties.dimension;
            int * arrayDims = attribute->attr.typeDescriptor->properties.arrayProperties.sizeInEachDimension;
            for(int i = 0; i < arrayDim; ++i){
              arraySize *= arrayDims[i];
            }
            fprintf(adotout, "\t_%s: .skip %d\n", idName(idlist), arraySize);
          }
        }
        idlist->semantic_value.identifierSemanticValue.symbolTableEntry =
          enterSymbol(idlist->semantic_value.identifierSemanticValue.identifierName, attribute);
        idlist = idlist->rightSibling;
      }
      break;
    default:
      fprintf(stderr, "emitGlobalDeclaration: switch case exhaust\n");
      exit(1);
  }
}

void emitFunctionDeclaration(AST * node){
  AST * returnTypeNode = node->child;
  AST * functionNameID = returnTypeNode->rightSibling;
  SymbolAttribute * attribute = NULL;
  attribute = (SymbolAttribute*)malloc(sizeof(SymbolAttribute));
  attribute->attributeKind = FUNCTION_SIGNATURE;
  attribute->attr.functionSignature = (FunctionSignature*)malloc(sizeof(FunctionSignature));
  attribute->attr.functionSignature->returnType = returnTypeNode->dataType;
  attribute->attr.functionSignature->parameterList = NULL;

  enterSymbol(idName(functionNameID), attribute);

  openScope();

  AST_NODE * parameterListNode = functionNameID->rightSibling;
  AST_NODE * traverseParameter = parameterListNode->child;
  int parametersCount = 0;
  if(traverseParameter)
  {
    ++parametersCount;
    processDeclarationNode(traverseParameter);
    AST_NODE *parameterID = traverseParameter->child->rightSibling;
    Parameter *parameter = (Parameter*)malloc(sizeof(Parameter));
    parameter->next = NULL;
    parameter->parameterName = parameterID->semantic_value.identifierSemanticValue.identifierName;
    parameter->type = parameterID->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor;
    attribute->attr.functionSignature->parameterList = parameter;
    traverseParameter = traverseParameter->rightSibling;
  }

  Parameter *parameterListTail = attribute->attr.functionSignature->parameterList;

  while(traverseParameter)
  {
    ++parametersCount;
    processDeclarationNode(traverseParameter);
    AST_NODE *parameterID = traverseParameter->child->rightSibling;
    Parameter *parameter = (Parameter*)malloc(sizeof(Parameter));
    parameter->next = NULL;
    parameter->parameterName = parameterID->semantic_value.identifierSemanticValue.identifierName;
    parameter->type = parameterID->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor;
    parameterListTail->next = parameter;
    parameterListTail = parameter;
    traverseParameter = traverseParameter->rightSibling;
  }
  attribute->attr.functionSignature->parametersCount = parametersCount;

  /* Prologue start */
  fprintf(adotout, ".text\n");
  fprintf(adotout, "_start_%s:\n", idName(functionNameID));

  AST_NODE *blockNode = parameterListNode->rightSibling;
  AST_NODE *traverseListNode = blockNode->child;
  while(traverseListNode)
  {
    // TODO
    // processGeneralNode(traverseListNode);
    traverseListNode = traverseListNode->rightSibling;
  }

  closeScope();
}
