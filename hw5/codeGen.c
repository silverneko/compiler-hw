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
void emitLocalDeclarations(AST *);
int emitLocalDeclaration(AST *, int);
void emitSaveArgs(int parametersCount);
void emitPrologue();
void emitEpilogue();
void emitStatementList(AST *);
void emitStatement(AST *);
void emitAssignmentStmt(AST_NODE* assignmentNode);
int emitExprRelatedNode(AST_NODE* exprRelatedNode);

void emitAlignment();
char * idName(AST * node);
int getReg();
void freeReg(int);

FILE * adotout;

int _const;
int _regs[32];

int getReg(){
  for(int i = 19; i <= 29; ++i){
    if(!_regs[i]){
      _regs[i] = 1;
      return i;
    }
  }
  fprintf(stderr, "Out of Regs\n");
  exit(1);
}

void freeReg(int reg){
  _regs[reg] = 0;
}

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

void emitLocalDeclarations(AST * root){
  AST * decls = root->child;
  int offset = 0;
  while(decls){
    offset += emitLocalDeclaration(decls, offset + 96);
    decls = decls->rightSibling;
  }
  fprintf(adotout, "\tadd sp, sp, #%d\n", -offset);
}

int emitLocalDeclaration(AST * decl, int _offset){
  AST * typeNode = decl->child;
  processTypeNode(typeNode);
  AST * idlist = typeNode->rightSibling;
  int offset = 0;
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
            offset += 4;
          }else{
            int arraySize = 4;
            int arrayDim = attribute->attr.typeDescriptor->properties.arrayProperties.dimension;
            int * arrayDims = attribute->attr.typeDescriptor->properties.arrayProperties.sizeInEachDimension;
            for(int i = 0; i < arrayDim; ++i){
              arraySize *= arrayDims[i];
            }
            offset += arraySize;
          }
        }
        attribute->global = 0;
        attribute->offset = _offset + offset;
        idlist->semantic_value.identifierSemanticValue.symbolTableEntry =
          enterSymbol(idlist->semantic_value.identifierSemanticValue.identifierName, attribute);
        idlist = idlist->rightSibling;
      }
      break;
    default:
      fprintf(stderr, "emitLocalDeclaration: switch case exhaust\n");
      exit(1);
  }
  return offset;
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
        attribute->global = 1;
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

void emitStatementList(AST * node){
  node = node->child;
  while(node){
    emitStatement(node);
    node = node->rightSibling;
  }
}

void emitStatement(AST * stmtNode){
  if(stmtNode->nodeType == NUL_NODE){
    return;
  }else if(stmtNode->nodeType == BLOCK_NODE){
    /* TODO */
    processBlockNode(stmtNode);
  }else{
    switch(stmtNode->semantic_value.stmtSemanticValue.kind){
      case WHILE_STMT:
        checkWhileStmt(stmtNode);
        break;
      case FOR_STMT:
        checkForStmt(stmtNode);
        break;
      case ASSIGN_STMT:
        emitAssignmentStmt(stmtNode);
        break;
      case IF_STMT:
        checkIfStmt(stmtNode);
        break;
      case FUNCTION_CALL_STMT:
        checkFunctionCall(stmtNode);
        break;
      case RETURN_STMT:
        checkReturnStmt(stmtNode);
        break;
      default:
        printf("Unhandle case in void processStmtNode(AST_NODE* stmtNode)\n");
        stmtNode->dataType = ERROR_TYPE;
        break;
    }
  }
}

void emitAssignmentStmt(AST_NODE* assignmentNode)
{
    AST_NODE* idNode = assignmentNode->child;
    AST_NODE* rightOp = idNode->rightSibling;
    int resultReg = emitExprRelatedNode(rightOp);
    // processVariableLValue(leftOp);
    SymbolTableEntry *symbolTableEntry = retrieveSymbol(idNode->semantic_value.identifierSemanticValue.identifierName);
    idNode->semantic_value.identifierSemanticValue.symbolTableEntry = symbolTableEntry;
    TypeDescriptor *typeDescriptor = idNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor;
    if(idNode->semantic_value.identifierSemanticValue.kind == NORMAL_ID){
      idNode->dataType = typeDescriptor->properties.dataType;
      if(symbolTableEntry->attribute->global){
        int reg = getReg();
        fprintf(adotout, "\tldr x%d, =_%s\n", reg, idName(idNode));
        fprintf(adotout, "\tstr w%d, [x%d, #0]\n", resultReg, reg);
        freeReg(reg);
      }else{
        int offset = symbolTableEntry->attribute->offset;
        fprintf(adotout, "\tstr w%d, [x29, #%d]\n", resultReg, -offset);
      }
    }else{
        idNode->dataType = typeDescriptor->properties.arrayProperties.elementType;
        int dimension = 0;
        int * arrayDims = typeDescriptor->properties.arrayProperties.sizeInEachDimension;
        AST_NODE *traverseDimList = idNode->child;
        int reg = getReg();
        fprintf(adotout, "\tmov x%d, #0\n", reg);
        while(traverseDimList){
          int indexReg = emitExprRelatedNode(traverseDimList);
          int _reg = getReg();
          fprintf(adotout, "\tldr x%d, =%d\n", _reg, arrayDims[dimension]);
          fprintf(adotout, "\tmul x%d, x%d, x%d\n", reg, reg, _reg);
          fprintf(adotout, "\tadd x%d, x%d, x%d\n", reg, reg, indexReg);
          freeReg(indexReg);
          freeReg(_reg);
          traverseDimList = traverseDimList->rightSibling;
          ++dimension;
        }
        int offset = symbolTableEntry->attribute->offset;
        int _reg = getReg();
        fprintf(adotout, "\tadd x%d, x%d, x%d\n", reg, reg, 29);
        fprintf(adotout, "\tldr x%d, =%d\n", _reg, -offset);
        fprintf(adotout, "\tadd x%d, x%d, x%d\n", reg, reg, _reg);
        fprintf(adotout, "\tstr w%d, [x%d, #0]\n", resultReg, reg);
        freeReg(_reg);
        freeReg(reg);
    }
    freeReg(resultReg);
    assignmentNode->dataType = getBiggerType(idNode->dataType, rightOp->dataType);
}

int emitExprRelatedNode(AST_NODE* exprRelatedNode){
  int reg = getReg();
  switch(exprRelatedNode->nodeType){
    case EXPR_NODE:
      processExprNode(exprRelatedNode);
      break;
    case STMT_NODE:
      //function call
      checkFunctionCall(exprRelatedNode);
      break;
    case IDENTIFIER_NODE:
      processVariableRValue(exprRelatedNode);
      break;
    case CONST_VALUE_NODE:
      switch(exprRelatedNode->semantic_value.const1->const_type){
        case INTEGERC:
          exprRelatedNode->dataType = INT_TYPE;
          exprRelatedNode->semantic_value.exprSemanticValue.constEvalValue.iValue =
            exprRelatedNode->semantic_value.const1->const_u.intval;
          fprintf(adotout, "ldr x%d, =%d\n", reg, exprRelatedNode->semantic_value.const1->const_u.intval);
          break;
        case FLOATC:
          exprRelatedNode->dataType = FLOAT_TYPE;
          exprRelatedNode->semantic_value.exprSemanticValue.constEvalValue.fValue =
            exprRelatedNode->semantic_value.const1->const_u.fval;
          fprintf(adotout, ".data\n");
          fprintf(adotout, "_float_const_%d: .float %f\n", _const, exprRelatedNode->semantic_value.const1->const_u.fval);
          emitAlignment();
          fprintf(adotout, ".text\n");
          fprintf(adotout, "ldr s%d, _float_const_%d\n", reg, _const);
          _const++;
          break;
        case STRINGC:
          /* TODO */
          exprRelatedNode->dataType = CONST_STRING_TYPE;
          break;
      }
      break;
  }
  return reg;
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
  emitPrologue();
  // emitSaveArgs(parametersCount);

  AST_NODE *blockNode = parameterListNode->rightSibling;
  AST_NODE *traverseListNode = blockNode->child;
  int offset;
  while(traverseListNode)
  {
    // TODO
    // processGeneralNode(traverseListNode);
    switch(traverseListNode->nodeType){
      case VARIABLE_DECL_LIST_NODE:
        emitLocalDeclarations(traverseListNode);
        break;
      case STMT_LIST_NODE:
        emitStatementList(traverseListNode);
        break;
      default:
        fprintf(stderr, "emitFunctionDeclaration: switch pattern exhaust\n");
        exit(1);
    }
    traverseListNode = traverseListNode->rightSibling;
  }
  fprintf(adotout, "_end_%s:\n", idName(functionNameID));
  emitEpilogue();

  closeScope();
}

void emitSaveArgs(int parametersCount){
  /* TODO */
  /* hw5 only need to implement parameter less function */
}

void emitPrologue(){
  fprintf(adotout, "\tstr x30, [sp, #0]\n");
  fprintf(adotout, "\tstr x29, [sp, #-8]\n");
  fprintf(adotout, "\tadd x29, sp, #-8\n");
  fprintf(adotout, "\tadd sp, sp, #-16\n");
  int offset = 0;
  for(int i = 19; i <= 29; ++i){
    fprintf(adotout, "\tstr x%d, [x29, #%d]\n", i, -8 - offset);
    offset += 8;
  }
  fprintf(adotout, "\tadd sp, sp, #%d\n", -offset);
}

void emitEpilogue(){
  int offset = 0;
  for(int i = 19; i <= 29; ++i){
    fprintf(adotout, "\tldr x%d, [x29, #%d]\n", i, -8 - offset);
    offset += 8;
  }
  fprintf(adotout, "\tldr x30, [x29, #8]\n");
  fprintf(adotout, "\tadd sp, x29, #8\n");
  fprintf(adotout, "\tldr x29, [x29, #0]\n");
  fprintf(adotout, "\tRET x30\n");
}

