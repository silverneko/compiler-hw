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
void emitFunctionCall(AST_NODE* functionCallNode);
void emitWriteFunction(AST *);
void emitGeneralNode(AST *);
int emitExprNode(AST_NODE* exprNode);
void emitWhileStmt(AST * stmtNode);
void emitIfStmt(AST_NODE* ifNode);
void emitReturnStmt(AST *);
int emitIntLiteral(int);
int emitFloatLiteral(float);
void emitBlockNode(AST_NODE* blockNode);

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

int emitIntLiteral(int i){
  fprintf(adotout, ".data\n");
  fprintf(adotout, "_const_%d: .word %d\n", _const, i);
  emitAlignment();
  fprintf(adotout, ".text\n");
  return _const++;
}

int emitFloatLiteral(float f){
  fprintf(adotout, ".data\n");
  fprintf(adotout, "_const_%d: .float %f\n", _const, f);
  emitAlignment();
  fprintf(adotout, ".text\n");
  return _const++;
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
  fprintf(adotout, ".data\n");
  fprintf(adotout, "\t_integer_const_%d: .word %d\n", _const, offset);
  emitAlignment();
  fprintf(adotout, ".text\n");
  int reg = getReg();
  fprintf(adotout, "\tldr w%d, _integer_const_%d\n", reg, _const);
  ++_const;
  freeReg(reg);
  fprintf(adotout, "\tsub sp, sp, w%d\n", reg);
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
            fprintf(adotout, "\t_%s: .word 0\n", idName(idlist));
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

/*
   void emitGeneralNode(AST_NODE *node){
   AST_NODE *traverseChildren = node->child;
   switch(node->nodeType){
   case VARIABLE_DECL_LIST_NODE:
   while(traverseChildren)
   {
   processDeclarationNode(traverseChildren);
   if(traverseChildren->dataType == ERROR_TYPE)
   {
   node->dataType = ERROR_TYPE;
   }
   traverseChildren = traverseChildren->rightSibling;
   }
   break;
   case STMT_LIST_NODE:
   while(traverseChildren)
   {
   processStmtNode(traverseChildren);
   if(traverseChildren->dataType == ERROR_TYPE)
   {
   node->dataType = ERROR_TYPE;
   }
   traverseChildren = traverseChildren->rightSibling;
   }
   break;
   case NONEMPTY_ASSIGN_EXPR_LIST_NODE:
   while(traverseChildren)
   {
   checkAssignOrExpr(traverseChildren);
   if(traverseChildren->dataType == ERROR_TYPE)
   {
   node->dataType = ERROR_TYPE;
   }
   traverseChildren = traverseChildren->rightSibling;
   }
   break;
   case NONEMPTY_RELOP_EXPR_LIST_NODE:
   while(traverseChildren)
   {
   processExprRelatedNode(traverseChildren);
   if(traverseChildren->dataType == ERROR_TYPE)
   {
   node->dataType = ERROR_TYPE;
   }
   traverseChildren = traverseChildren->rightSibling;
   }
   break;
   case NUL_NODE:
   break;
   }
   }
   */

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
    emitBlockNode(stmtNode);
  }else{
    switch(stmtNode->semantic_value.stmtSemanticValue.kind){
      case WHILE_STMT:
        emitWhileStmt(stmtNode);
        break;
      case FOR_STMT:
        checkForStmt(stmtNode);
        break;
      case ASSIGN_STMT:
        emitAssignmentStmt(stmtNode);
        break;
      case IF_STMT:
        emitIfStmt(stmtNode);
        break;
      case FUNCTION_CALL_STMT:
        emitFunctionCall(stmtNode);
        break;
      case RETURN_STMT:
        emitReturnStmt(stmtNode);
        break;
    }
  }
}

void emitIfStmt(AST_NODE* ifNode){
  AST_NODE* boolExpression = ifNode->child;
  AST_NODE* ifBodyNode = boolExpression->rightSibling;
  AST_NODE* elsePartNode = ifBodyNode->rightSibling;
  // checkAssignOrExpr(boolExpression);
  int wn = _const++;
  fprintf(adotout, "_IF_%d:\n", wn);
  int reg = emitExprRelatedNode(boolExpression);
  freeReg(reg);
  /* TODO if boolExpression is a Assignment statement, it shall break*/
  fprintf(adotout, "cmp w%d, #0\n", reg);
  fprintf(adotout, "beq _ELSE_%d\n", wn);
  emitStatement(ifBodyNode);
  fprintf(adotout, "b _END_IF_%d\n", wn);
  fprintf(adotout, "_ELSE_%d:\n", wn);
  emitStatement(elsePartNode);
  fprintf(adotout, "_END_IF_%d:\n", wn);
}

void emitWhileStmt(AST * whileNode){
  AST_NODE* boolExpression = whileNode->child;
  AST_NODE* bodyNode = boolExpression->rightSibling;
  // checkAssignOrExpr(boolExpression);
  int wn = _const++;
  fprintf(adotout, "_WHILE_%d:\n", wn);
  int reg = emitExprRelatedNode(boolExpression);
  /* TODO if boolExpression is a Assignment statement, it shall break*/
  freeReg(reg);
  fprintf(adotout, "cmp w%d, #0\n", reg);
  fprintf(adotout, "beq _END_WHILE_%d\n", wn);
  emitStatement(bodyNode);
  fprintf(adotout, "b _WHILE_%d\n", wn);
  fprintf(adotout, "_END_WHILE_%d:\n", wn);
}

void emitWriteFunction(AST_NODE* functionCallNode){
  AST_NODE* functionIDNode = functionCallNode->child;
  AST_NODE* actualParameterList = functionIDNode->rightSibling;
  AST_NODE* actualParameter = actualParameterList->child;

  int reg = emitExprRelatedNode(actualParameter);
  switch(actualParameter->dataType){
    case INT_TYPE:
      fprintf(adotout, "\tmov w0, w%d\n", reg);
      fprintf(adotout, "\tbl _write_int\n");
      break;
    case FLOAT_TYPE:
      fprintf(adotout, "\tfmov s0, s%d\n", reg);
      fprintf(adotout, "\tbl _write_float\n");
      break;
    case CONST_STRING_TYPE:
      fprintf(adotout, "\tmov x0, x%d\n", reg);
      fprintf(adotout, "\tbl _write_str\n");
      break;
  }
  freeReg(reg);

  functionCallNode->dataType = VOID_TYPE;
}

void emitFunctionCall(AST_NODE* functionCallNode){
  AST_NODE* functionIDNode = functionCallNode->child;

  //special case
  if(strcmp(functionIDNode->semantic_value.identifierSemanticValue.identifierName, "write") == 0){
    emitWriteFunction(functionCallNode);
    return;
  }

  /* TODO below */

  SymbolTableEntry* symbolTableEntry = retrieveSymbol(functionIDNode->semantic_value.identifierSemanticValue.identifierName);
  functionIDNode->semantic_value.identifierSemanticValue.symbolTableEntry = symbolTableEntry;

  /* Only parameter-less proc call for this homework */
  if(strcmp(idName(functionIDNode), "read") == 0){
    // Another special case
    fprintf(adotout, "bl _read_int\n");
  }else if(strcmp(idName(functionIDNode), "fread") == 0){
    // Yet Another speCial Case
    fprintf(adotout, "bl _read_float\n");
  }else{
    fprintf(adotout, "bl _start_%s\n", idName(functionIDNode));
  }

  functionCallNode->dataType = symbolTableEntry->attribute->attr.functionSignature->returnType;
}

void emitAssignmentStmt(AST_NODE* assignmentNode)
{
  AST_NODE* idNode = assignmentNode->child;
  AST_NODE* rightOp = idNode->rightSibling;
  int resultReg = emitExprRelatedNode(rightOp);
  SymbolTableEntry *symbolTableEntry = retrieveSymbol(idNode->semantic_value.identifierSemanticValue.identifierName);
  idNode->semantic_value.identifierSemanticValue.symbolTableEntry = symbolTableEntry;
  TypeDescriptor *typeDescriptor = idNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor;
  if(idNode->semantic_value.identifierSemanticValue.kind == NORMAL_ID){
    idNode->dataType = typeDescriptor->properties.dataType;
    if(symbolTableEntry->attribute->global){
      int reg = getReg();
      fprintf(adotout, "\tldr x%d, =_%s\n", reg, idName(idNode));
      if(rightOp->dataType == INT_TYPE){
        fprintf(adotout, "\tstr w%d, [x%d, #0]\n", resultReg, reg);
      }else{
        fprintf(adotout, "\tstr s%d, [x%d, #0]\n", resultReg, reg);
      }
      freeReg(reg);
    }else{
      int offset = symbolTableEntry->attribute->offset;
      if(rightOp->dataType == INT_TYPE){
        fprintf(adotout, "\tstr w%d, [x29, #%d]\n", resultReg, -offset);
      }else{
        fprintf(adotout, "\tstr s%d, [x29, #%d]\n", resultReg, -offset);
      }
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
      int id = emitIntLiteral(arrayDims[dimension]);
      fprintf(adotout, "\tldr x%d, _const_%d\n", _reg, id);
      fprintf(adotout, "\tmul x%d, x%d, x%d\n", reg, reg, _reg);
      fprintf(adotout, "\tlsl x%d, x%d, #2\n", indexReg, indexReg);
      fprintf(adotout, "\tadd x%d, x%d, x%d\n", reg, reg, indexReg);
      freeReg(indexReg);
      freeReg(_reg);
      traverseDimList = traverseDimList->rightSibling;
      ++dimension;
    }
    int offset = symbolTableEntry->attribute->offset;
    int _reg = getReg();
    if(symbolTableEntry->attribute->global){
      fprintf(adotout, "\tldr x%d, =_%s\n", _reg, idName(idNode));
    }else{
      fprintf(adotout, "\tadd x%d, x%d, x%d\n", reg, reg, 29);
      int id = emitIntLiteral(offset);
      fprintf(adotout, "\tldr x%d, _const_%d\n", _reg, id);
      fprintf(adotout, "\tneg x%d, x%d\n", _reg, _reg);
    }
    fprintf(adotout, "\tadd x%d, x%d, x%d\n", reg, reg, _reg);
    if(rightOp->dataType == INT_TYPE){
      fprintf(adotout, "\tstr w%d, [x%d, #0]\n", resultReg, reg);
    }else{
      fprintf(adotout, "\tstr s%d, [x%d, #0]\n", resultReg, reg);
    }
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
      {
        freeReg(reg);
        reg = emitExprNode(exprRelatedNode);
        return reg;
      }
      break;
    case STMT_NODE:
      //function call
      {
        freeReg(reg);
        emitFunctionCall(exprRelatedNode);
        reg = getReg();
        switch(exprRelatedNode->dataType){
          case INT_TYPE:
            fprintf(adotout, "mov w%d, w0\n", reg);
            break;
          case FLOAT_TYPE:
            fprintf(adotout, "fmov s%d, s0\n", reg);
            break;
        }
        return reg;
      }
      break;
    case IDENTIFIER_NODE:
      // emitVariableRValue(exprRelatedNode);
      {
        AST_NODE* idNode = exprRelatedNode;
        SymbolTableEntry *symbolTableEntry = retrieveSymbol(idNode->semantic_value.identifierSemanticValue.identifierName);
        idNode->semantic_value.identifierSemanticValue.symbolTableEntry = symbolTableEntry;
        TypeDescriptor *typeDescriptor = idNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor;
        if(idNode->semantic_value.identifierSemanticValue.kind == NORMAL_ID){
          idNode->dataType = typeDescriptor->properties.dataType;
          if(symbolTableEntry->attribute->global){
            if(idNode->dataType == INT_TYPE){
              fprintf(adotout, "\tldr w%d, _%s\n", reg, idName(idNode));
            }else{
              fprintf(adotout, "\tldr s%d, _%s\n", reg, idName(idNode));
            }
          }else{
            int offset = symbolTableEntry->attribute->offset;
            if(idNode->dataType == INT_TYPE){
              fprintf(adotout, "\tldr w%d, [x29, #%d]\n", reg, -offset);
            }else{
              fprintf(adotout, "\tldr s%d, [x29, #%d]\n", reg, -offset);
            }
          }
        }else{
          idNode->dataType = typeDescriptor->properties.arrayProperties.elementType;
          int dimension = 0;
          int * arrayDims = typeDescriptor->properties.arrayProperties.sizeInEachDimension;
          AST_NODE *traverseDimList = idNode->child;
          fprintf(adotout, "\tmov x%d, #0\n", reg);
          while(traverseDimList){
            int id = emitIntLiteral(arrayDims[dimension]);
            int _reg = getReg();
            fprintf(adotout, "\tldr x%d, _const_%d\n", _reg, id);
            fprintf(adotout, "\tmul x%d, x%d, x%d\n", reg, reg, _reg);
            freeReg(_reg);
            int indexReg = emitExprRelatedNode(traverseDimList);
            fprintf(adotout, "\tlsl x%d, x%d, #2\n", indexReg, indexReg);
            fprintf(adotout, "\tadd x%d, x%d, x%d\n", reg, reg, indexReg);
            freeReg(indexReg);
            traverseDimList = traverseDimList->rightSibling;
            ++dimension;
          }
          int offset = symbolTableEntry->attribute->offset;
          int _reg = getReg();
          if(symbolTableEntry->attribute->global){
            fprintf(adotout, "\tldr x%d, =_%s\n", _reg, idName(idNode));
          }else{
            fprintf(adotout, "\tadd x%d, x%d, x%d\n", reg, reg, 29);
            int id = emitIntLiteral(offset);
            fprintf(adotout, "\tldr x%d, _const_%d\n", _reg, id);
            fprintf(adotout, "\tneg x%d, x%d\n", _reg, _reg);
          }
          freeReg(_reg);
          fprintf(adotout, "\tadd x%d, x%d, x%d\n", reg, reg, _reg);
          if(idNode->dataType == INT_TYPE){
            fprintf(adotout, "\tldr w%d, [x%d, #0]\n", reg, reg);
          }else{
            fprintf(adotout, "\tldr s%d, [x%d, #0]\n", reg, reg);
          }
        }
      }
      break;
    case CONST_VALUE_NODE:
      switch(exprRelatedNode->semantic_value.const1->const_type){
        case INTEGERC:
          exprRelatedNode->dataType = INT_TYPE;
          exprRelatedNode->semantic_value.exprSemanticValue.constEvalValue.iValue =
            exprRelatedNode->semantic_value.const1->const_u.intval;
          fprintf(adotout, ".data\n");
          fprintf(adotout, "_integer_const_%d: .word %d\n", _const, exprRelatedNode->semantic_value.const1->const_u.intval);
          emitAlignment();
          fprintf(adotout, ".text\n");
          fprintf(adotout, "ldr w%d, _integer_const_%d\n", reg, _const);
          _const++;
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
          exprRelatedNode->dataType = CONST_STRING_TYPE;
          fprintf(adotout, ".data\n");
          fprintf(adotout, "_string_const_%d: .ascii %s\n", _const, exprRelatedNode->semantic_value.const1->const_u.sc);
          emitAlignment();
          fprintf(adotout, ".text\n");
          fprintf(adotout, "ldr x%d, =_string_const_%d\n", reg, _const);
          _const++;
          break;
      }
      break;
  }
  return reg;
}

int emitExprNode(AST_NODE* exprNode){
  if(exprNode->semantic_value.exprSemanticValue.kind == BINARY_OPERATION){
    AST_NODE* leftOp = exprNode->child;
    AST_NODE* rightOp = leftOp->rightSibling;
    processExprRelatedNode(leftOp);
    processExprRelatedNode(rightOp);
    exprNode->dataType = getBiggerType(leftOp->dataType, rightOp->dataType);
    if((exprNode->dataType != ERROR_TYPE) &&
        (leftOp->nodeType == CONST_VALUE_NODE || (leftOp->nodeType == EXPR_NODE && leftOp->semantic_value.exprSemanticValue.isConstEval)) &&
        (rightOp->nodeType == CONST_VALUE_NODE || (rightOp->nodeType == EXPR_NODE && rightOp->semantic_value.exprSemanticValue.isConstEval))
      ){
      evaluateExprValue(exprNode);
      exprNode->semantic_value.exprSemanticValue.isConstEval = 1;
      fprintf(adotout, "\t.data\n");
      if(exprNode->dataType == INT_TYPE){
        fprintf(adotout, "\t_integer_const_%d: .word %d\n", _const, exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue);
      }else{
        fprintf(adotout, "\t_float_const_%d: .float %f\n", _const, exprNode->semantic_value.exprSemanticValue.constEvalValue.fValue);
      }
      emitAlignment();
      fprintf(adotout, "\t.text\n");
      int reg = getReg();
      if(exprNode->dataType == INT_TYPE){
        fprintf(adotout, "\tldr w%d, _integer_const_%d\n", reg, _const);
      }else{
        fprintf(adotout, "\tldr s%d, _float_const_%d\n", reg, _const);
      }
      ++_const;
      return reg;
    }else{
      int reg1 = emitExprRelatedNode(leftOp);
      freeReg(reg1);
      if(leftOp->dataType == INT_TYPE){
        fprintf(adotout, "str w%d, [sp, #0]\n", reg1);
      }else{
        fprintf(adotout, "str s%d, [sp, #0]\n", reg1);
      }
      fprintf(adotout, "sub sp, sp, #8\n");
      int reg2 = emitExprRelatedNode(rightOp);
      fprintf(adotout, "add sp, sp, #8\n");
      reg1 = getReg();
      if(leftOp->dataType == INT_TYPE){
        fprintf(adotout, "ldr w%d, [sp, #0]\n", reg1);
      }else{
        fprintf(adotout, "ldr s%d, [sp, #0]\n", reg1);
      }
      if(leftOp->dataType == INT_TYPE && rightOp->dataType == INT_TYPE){
        switch(exprNode->semantic_value.exprSemanticValue.op.binaryOp){
          case BINARY_OP_ADD:
            fprintf(adotout, "\tadd w%d, w%d, w%d\n", reg1, reg1, reg2);
            break;
          case BINARY_OP_SUB:
            fprintf(adotout, "\tsub w%d, w%d, w%d\n", reg1, reg1, reg2);
            break;
          case BINARY_OP_MUL:
            fprintf(adotout, "\tmul w%d, w%d, w%d\n", reg1, reg1, reg2);
            break;
          case BINARY_OP_DIV:
            fprintf(adotout, "\tdiv w%d, w%d, w%d\n", reg1, reg1, reg2);
            break;
          case BINARY_OP_EQ:
            fprintf(adotout, "\tcmp w%d, w%d\n", reg1, reg2);
            fprintf(adotout, "\tbeq _ELSE_%d\n", _const);
            fprintf(adotout, "\tmov w%d, #0\n", reg1);
            fprintf(adotout, "\tb _END_%d\n", _const);
            fprintf(adotout, "\t_ELSE_%d:\n", _const);
            fprintf(adotout, "\tmov w%d, #1\n", reg1);
            fprintf(adotout, "\t_END_%d:\n", _const);
            ++_const;
            break;
          case BINARY_OP_GE:
            fprintf(adotout, "\tcmp w%d, w%d\n", reg1, reg2);
            fprintf(adotout, "\tbge _ELSE_%d\n", _const);
            fprintf(adotout, "\tmov w%d, #0\n", reg1);
            fprintf(adotout, "\tb _END_%d\n", _const);
            fprintf(adotout, "\t_ELSE_%d:\n", _const);
            fprintf(adotout, "\tmov w%d, #1\n", reg1);
            fprintf(adotout, "\t_END_%d:\n", _const);
            ++_const;
            break;
          case BINARY_OP_LE:
            fprintf(adotout, "\tcmp w%d, w%d\n", reg1, reg2);
            fprintf(adotout, "\tble _ELSE_%d\n", _const);
            fprintf(adotout, "\tmov w%d, #0\n", reg1);
            fprintf(adotout, "\tb _END_%d\n", _const);
            fprintf(adotout, "\t_ELSE_%d:\n", _const);
            fprintf(adotout, "\tmov w%d, #1\n", reg1);
            fprintf(adotout, "\t_END_%d:\n", _const);
            ++_const;
            break;
          case BINARY_OP_NE:
            fprintf(adotout, "\tcmp w%d, w%d\n", reg1, reg2);
            fprintf(adotout, "\tbne _ELSE_%d\n", _const);
            fprintf(adotout, "\tmov w%d, #0\n", reg1);
            fprintf(adotout, "\tb _END_%d\n", _const);
            fprintf(adotout, "\t_ELSE_%d:\n", _const);
            fprintf(adotout, "\tmov w%d, #1\n", reg1);
            fprintf(adotout, "\t_END_%d:\n", _const);
            ++_const;
            break;
          case BINARY_OP_GT:
            fprintf(adotout, "\tcmp w%d, w%d\n", reg1, reg2);
            fprintf(adotout, "\tbgt _ELSE_%d\n", _const);
            fprintf(adotout, "\tmov w%d, #0\n", reg1);
            fprintf(adotout, "\tb _END_%d\n", _const);
            fprintf(adotout, "\t_ELSE_%d:\n", _const);
            fprintf(adotout, "\tmov w%d, #1\n", reg1);
            fprintf(adotout, "\t_END_%d:\n", _const);
            ++_const;
            break;
          case BINARY_OP_LT:
            fprintf(adotout, "\tcmp w%d, w%d\n", reg1, reg2);
            fprintf(adotout, "\tblt _ELSE_%d\n", _const);
            fprintf(adotout, "\tmov w%d, #0\n", reg1);
            fprintf(adotout, "\tb _END_%d\n", _const);
            fprintf(adotout, "\t_ELSE_%d:\n", _const);
            fprintf(adotout, "\tmov w%d, #1\n", reg1);
            fprintf(adotout, "\t_END_%d:\n", _const);
            ++_const;
            break;
          case BINARY_OP_AND:
            fprintf(adotout, "\tcmp w%d, #0\n", reg1);
            fprintf(adotout, "\tbeq _ELSE_%d\n", _const);
            fprintf(adotout, "\tcmp w%d, #0\n", reg2);
            fprintf(adotout, "\tbeq _ELSE_%d\n", _const);
            fprintf(adotout, "\tmov w%d, #1\n", reg1);
            fprintf(adotout, "\tb _END_%d\n", _const);
            fprintf(adotout, "\t_ELSE_%d:\n", _const);
            fprintf(adotout, "\tmov w%d, #0\n", reg1);
            fprintf(adotout, "\t_END_%d:\n", _const);
            ++_const;
            break;
          case BINARY_OP_OR:
            fprintf(adotout, "\tcmp w%d, #0\n", reg1);
            fprintf(adotout, "\tbne _ELSE_%d\n", _const);
            fprintf(adotout, "\tcmp w%d, #0\n", reg2);
            fprintf(adotout, "\tbne _ELSE_%d\n", _const);
            fprintf(adotout, "\tmov w%d, #0\n", reg1);
            fprintf(adotout, "\tb _END_%d\n", _const);
            fprintf(adotout, "\t_ELSE_%d:\n", _const);
            fprintf(adotout, "\tmov w%d, #1\n", reg1);
            fprintf(adotout, "\t_END_%d:\n", _const);
            ++_const;
            break;
        }
        freeReg(reg2);
        return reg1;
      }else{
        /* TODO insert type conversion instruction */
      }
    }
  }else{
    /* Unary operation */
    AST_NODE* operand = exprNode->child;
    processExprRelatedNode(operand);
    exprNode->dataType = operand->dataType;

    if((exprNode->dataType != ERROR_TYPE) &&
        (operand->nodeType == CONST_VALUE_NODE || (operand->nodeType == EXPR_NODE && operand->semantic_value.exprSemanticValue.isConstEval))
      ){
      evaluateExprValue(exprNode);
      exprNode->semantic_value.exprSemanticValue.isConstEval = 1;
      fprintf(adotout, "\t.data\n");
      if(exprNode->dataType == INT_TYPE){
        fprintf(adotout, "\t_integer_const_%d: .word %d\n", _const, exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue);
      }else{
        fprintf(adotout, "\t_float_const_%d: .float %f\n", _const, exprNode->semantic_value.exprSemanticValue.constEvalValue.fValue);
      }
      emitAlignment();
      fprintf(adotout, "\t.text\n");
      int reg = getReg();
      if(exprNode->dataType == INT_TYPE){
        fprintf(adotout, "\tldr w%d, _integer_const_%d\n", reg, _const);
      }else{
        fprintf(adotout, "\tldr s%d, _float_const_%d\n", reg, _const);
      }
      ++_const;
      return reg;
    }else{
      int reg = emitExprRelatedNode(operand);
      if(operand->dataType == INT_TYPE){
        exprNode->dataType = INT_TYPE;
        switch(exprNode->semantic_value.exprSemanticValue.op.unaryOp){
          case UNARY_OP_POSITIVE:
            break;
          case UNARY_OP_NEGATIVE:
            fprintf(adotout, "\tneg w%d, w%d", reg, reg);
            break;
          case UNARY_OP_LOGICAL_NEGATION:
            fprintf(adotout, "\teor w%d, w%d, #1", reg, reg);
            break;
        }
      }else{
        exprNode->dataType = FLOAT_TYPE;
        switch(exprNode->semantic_value.exprSemanticValue.op.unaryOp){
          case UNARY_OP_POSITIVE:
            break;
          case UNARY_OP_NEGATIVE:
            fprintf(adotout, "\tfneg s%d, s%d", reg, reg);
            break;
          case UNARY_OP_LOGICAL_NEGATION:
            /* TODO type conversion */
            break;
        }
      }
      return reg;
    }

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
  emitPrologue();
  // emitSaveArgs(parametersCount);

  AST_NODE *blockNode = parameterListNode->rightSibling;
  AST_NODE *traverseListNode = blockNode->child;
  int offset;
  while(traverseListNode){
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

void emitBlockNode(AST_NODE* blockNode){
  openScope();

  AST_NODE *traverseListNode = blockNode->child;
  int id = _const++;
  fprintf(adotout, ".data\n");
  fprintf(adotout, "_OLD_SP_%d: .skip 8\n", id);
  fprintf(adotout, ".text\n");
  int reg = getReg(), reg2 = getReg();
  freeReg(reg);
  freeReg(reg2);
  fprintf(adotout, "ldr x%d, =_OLD_SP_%d\n", reg, id);
  fprintf(adotout, "mov x%d, sp\n", reg2);
  fprintf(adotout, "str x%d, [x%d, #0]\n", reg2, reg);
  while(traverseListNode){
    switch(traverseListNode->nodeType){
      case VARIABLE_DECL_LIST_NODE:
        emitLocalDeclarations(traverseListNode);
        break;
      case STMT_LIST_NODE:
        emitStatementList(traverseListNode);
        break;
    }
    traverseListNode = traverseListNode->rightSibling;
  }
  reg = getReg();
  reg2 = getReg();
  freeReg(reg);
  freeReg(reg2);
  fprintf(adotout, "ldr x%d, =_OLD_SP_%d\n", reg, id);
  fprintf(adotout, "ldr x%d, [x%d, #0]\n", reg2, reg);
  fprintf(adotout, "mov sp, x%d\n", reg2);

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
  fprintf(adotout, ".data\n");
  fprintf(adotout, "_AR_SIZE_%d: .word %d\n", _const, offset);
  emitAlignment();
  fprintf(adotout, ".text\n");
  fprintf(adotout, "\tldr w19, _AR_SIZE_%d\n", _const);
  fprintf(adotout, "\tsub sp, sp, w19\n");
  ++_const;
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

void emitReturnStmt(AST_NODE* returnNode){
  AST_NODE* parentNode = returnNode->parent;
  DATA_TYPE returnType = NONE_TYPE;
  while(parentNode){
    if(parentNode->nodeType == DECLARATION_NODE){
      if(parentNode->semantic_value.declSemanticValue.kind == FUNCTION_DECL){
        returnType = parentNode->child->dataType;
      }
      break;
    }
    parentNode = parentNode->parent;
  }
  returnNode->dataType = returnType;
  int reg = emitExprRelatedNode(returnNode->child);
  switch(returnType){
    case VOID_TYPE:
      break;
    case INT_TYPE:
      if(returnNode->child->dataType == INT_TYPE){
        fprintf(adotout, "mov w0, w%d\n", reg);
      }else{
        /* TODO type conversion*/
      }
      break;
    case FLOAT_TYPE:
      if(returnNode->child->dataType == INT_TYPE){
        /* TODO type conversion*/
      }else{
        fprintf(adotout, "fmov s0, s%d\n", reg);
      }
      break;
  }
  fprintf(adotout, "b _end_%s\n", idName(parentNode->child->rightSibling));
}

