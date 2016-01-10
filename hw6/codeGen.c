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

int _offset;

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
  int __offset = _offset;
  while(decls){
    _offset += emitLocalDeclaration(decls, _offset + 88);
    decls = decls->rightSibling;
  }
  fprintf(adotout, ".data\n");
  fprintf(adotout, "_integer_const_%d: .word %d\n", _const, _offset - __offset);
  emitAlignment();
  fprintf(adotout, ".text\n");
  int reg = getReg();
  fprintf(adotout, "ldr w%d, _integer_const_%d\n", reg, _const);
  ++_const;
  freeReg(reg);
  fprintf(adotout, "sub sp, sp, w%d\n", reg);
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
            fprintf(adotout, "_%s: .word 0\n", idName(idlist));
          }else{
            int arraySize = 4;
            int arrayDim = attribute->attr.typeDescriptor->properties.arrayProperties.dimension;
            int * arrayDims = attribute->attr.typeDescriptor->properties.arrayProperties.sizeInEachDimension;
            for(int i = 0; i < arrayDim; ++i){
              arraySize *= arrayDims[i];
            }
            fprintf(adotout, "_%s: .skip %d\n", idName(idlist), arraySize);
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
  int wn = _const++;
  fprintf(adotout, "_IF_%d:\n", wn);
  if(boolExpression->nodeType == STMT_NODE
      && boolExpression->semantic_value.stmtSemanticValue.kind == ASSIGN_STMT){
    emitAssignmentStmt(boolExpression);
    boolExpression = boolExpression->child;
  }
  int reg = emitExprRelatedNode(boolExpression);
  freeReg(reg);
  if(boolExpression->dataType == FLOAT_TYPE){
    fprintf(adotout, "fcvtzs w%d, s%d\n", reg, reg);
  }
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
  int wn = _const++;
  fprintf(adotout, "_WHILE_%d:\n", wn);
  if(boolExpression->nodeType == STMT_NODE
      && boolExpression->semantic_value.stmtSemanticValue.kind == ASSIGN_STMT){
    emitAssignmentStmt(boolExpression);
    boolExpression = boolExpression->child;
  }
  int reg = emitExprRelatedNode(boolExpression);
  freeReg(reg);
  if(boolExpression->dataType == FLOAT_TYPE){
    fprintf(adotout, "fcvtzs w%d, s%d\n", reg, reg);
  }
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
      fprintf(adotout, "mov w0, w%d\n", reg);
      fprintf(adotout, "bl _write_int\n");
      break;
    case FLOAT_TYPE:
      fprintf(adotout, "fmov s0, s%d\n", reg);
      fprintf(adotout, "bl _write_float\n");
      break;
    case CONST_STRING_TYPE:
      fprintf(adotout, "mov x0, x%d\n", reg);
      fprintf(adotout, "bl _write_str\n");
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

  /* Only parameter-less proc call for this homework */
  SymbolTableEntry* symbolTableEntry = retrieveSymbol(functionIDNode->semantic_value.identifierSemanticValue.identifierName);
  functionIDNode->semantic_value.identifierSemanticValue.symbolTableEntry = symbolTableEntry;

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
    int reg = getReg();
    freeReg(reg);
    if(symbolTableEntry->attribute->global){
      fprintf(adotout, "ldr x%d, =_%s\n", reg, idName(idNode));
    }else{
      int offset = symbolTableEntry->attribute->offset;
      int id = emitIntLiteral(offset);
      fprintf(adotout, "ldr w%d, _const_%d\n", reg, id);
      fprintf(adotout, "sub x%d, x29, x%d\n", reg, reg);
    }
    if(idNode->dataType == INT_TYPE){
      if(rightOp->dataType == FLOAT_TYPE){
        fprintf(adotout, "fcvtzs w%d, s%d\n", resultReg, resultReg);
      }
      fprintf(adotout, "str w%d, [x%d, #0]\n", resultReg, reg);
    }else{
      if(rightOp->dataType == INT_TYPE){
        fprintf(adotout, "scvtf s%d, w%d\n", resultReg, resultReg);
      }
      fprintf(adotout, "str s%d, [x%d, #0]\n", resultReg, reg);
    }
  }else{
    idNode->dataType = typeDescriptor->properties.arrayProperties.elementType;
    int dimension = 0;
    int * arrayDims = typeDescriptor->properties.arrayProperties.sizeInEachDimension;
    AST_NODE *traverseDimList = idNode->child;
    int reg = getReg();
    fprintf(adotout, "mov x%d, #0\n", reg);
    while(traverseDimList){
      int indexReg = emitExprRelatedNode(traverseDimList);
      int _reg = getReg();
      int id = emitIntLiteral(arrayDims[dimension]);
      fprintf(adotout, "ldr x%d, _const_%d\n", _reg, id);
      fprintf(adotout, "mul x%d, x%d, x%d\n", reg, reg, _reg);
      fprintf(adotout, "lsl x%d, x%d, #2\n", indexReg, indexReg);
      fprintf(adotout, "add x%d, x%d, x%d\n", reg, reg, indexReg);
      freeReg(indexReg);
      freeReg(_reg);
      traverseDimList = traverseDimList->rightSibling;
      ++dimension;
    }
    int offset = symbolTableEntry->attribute->offset;
    int _reg = getReg();
    if(symbolTableEntry->attribute->global){
      fprintf(adotout, "ldr x%d, =_%s\n", _reg, idName(idNode));
    }else{
      fprintf(adotout, "add x%d, x%d, x%d\n", reg, reg, 29);
      int id = emitIntLiteral(offset);
      fprintf(adotout, "ldr x%d, _const_%d\n", _reg, id);
      fprintf(adotout, "neg x%d, x%d\n", _reg, _reg);
    }
    fprintf(adotout, "add x%d, x%d, x%d\n", reg, reg, _reg);
    if(idNode->dataType == INT_TYPE){
      if(rightOp->dataType == FLOAT_TYPE){
        fprintf(adotout, "fcvtzs w%d, s%d\n", resultReg, resultReg);
      }
      fprintf(adotout, "str w%d, [x%d, #0]\n", resultReg, reg);
    }else{
      if(rightOp->dataType == INT_TYPE){
        fprintf(adotout, "scvtf s%d, w%d\n", resultReg, resultReg);
      }
      fprintf(adotout, "str s%d, [x%d, #0]\n", resultReg, reg);
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
              fprintf(adotout, "ldr w%d, _%s\n", reg, idName(idNode));
            }else{
              fprintf(adotout, "ldr s%d, _%s\n", reg, idName(idNode));
            }
          }else{
            int offset = symbolTableEntry->attribute->offset;
            int id = emitIntLiteral(offset);
            fprintf(adotout, "ldr w%d, _const_%d\n", reg, id);
            fprintf(adotout, "sub x%d, x29, x%d\n", reg, reg);
            if(idNode->dataType == INT_TYPE){
              fprintf(adotout, "ldr w%d, [x%d, #0]\n", reg, reg);
            }else{
              fprintf(adotout, "ldr s%d, [x%d, #0]\n", reg, reg);
            }
          }
        }else{
          idNode->dataType = typeDescriptor->properties.arrayProperties.elementType;
          int dimension = 0;
          int * arrayDims = typeDescriptor->properties.arrayProperties.sizeInEachDimension;
          AST_NODE *traverseDimList = idNode->child;
          fprintf(adotout, "mov x%d, #0\n", reg);
          while(traverseDimList){
            int id = emitIntLiteral(arrayDims[dimension]);
            int _reg = getReg();
            fprintf(adotout, "ldr x%d, _const_%d\n", _reg, id);
            fprintf(adotout, "mul x%d, x%d, x%d\n", reg, reg, _reg);
            freeReg(_reg);
            int indexReg = emitExprRelatedNode(traverseDimList);
            fprintf(adotout, "lsl x%d, x%d, #2\n", indexReg, indexReg);
            fprintf(adotout, "add x%d, x%d, x%d\n", reg, reg, indexReg);
            freeReg(indexReg);
            traverseDimList = traverseDimList->rightSibling;
            ++dimension;
          }
          int offset = symbolTableEntry->attribute->offset;
          int _reg = getReg();
          if(symbolTableEntry->attribute->global){
            fprintf(adotout, "ldr x%d, =_%s\n", _reg, idName(idNode));
          }else{
            fprintf(adotout, "add x%d, x%d, x%d\n", reg, reg, 29);
            int id = emitIntLiteral(offset);
            fprintf(adotout, "ldr x%d, _const_%d\n", _reg, id);
            fprintf(adotout, "neg x%d, x%d\n", _reg, _reg);
          }
          freeReg(_reg);
          fprintf(adotout, "add x%d, x%d, x%d\n", reg, reg, _reg);
          if(idNode->dataType == INT_TYPE){
            fprintf(adotout, "ldr w%d, [x%d, #0]\n", reg, reg);
          }else{
            fprintf(adotout, "ldr s%d, [x%d, #0]\n", reg, reg);
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
          fprintf(adotout, "_string_const_%d: .asciz %s\n", _const, exprRelatedNode->semantic_value.const1->const_u.sc);
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
      int reg = getReg();
      fprintf(adotout, ".data\n");
      if(exprNode->dataType == INT_TYPE){
        fprintf(adotout, "_integer_const_%d: .word %d\n", _const, exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue);
        emitAlignment();
        fprintf(adotout, ".text\n");
        fprintf(adotout, "ldr w%d, _integer_const_%d\n", reg, _const);
      }else{
        fprintf(adotout, "_float_const_%d: .float %f\n", _const, exprNode->semantic_value.exprSemanticValue.constEvalValue.fValue);
        emitAlignment();
        fprintf(adotout, ".text\n");
        fprintf(adotout, "ldr s%d, _float_const_%d\n", reg, _const);
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
        exprNode->dataType = INT_TYPE;
        switch(exprNode->semantic_value.exprSemanticValue.op.binaryOp){
          case BINARY_OP_ADD:
            fprintf(adotout, "add w%d, w%d, w%d\n", reg1, reg1, reg2);
            break;
          case BINARY_OP_SUB:
            fprintf(adotout, "sub w%d, w%d, w%d\n", reg1, reg1, reg2);
            break;
          case BINARY_OP_MUL:
            fprintf(adotout, "mul w%d, w%d, w%d\n", reg1, reg1, reg2);
            break;
          case BINARY_OP_DIV:
            fprintf(adotout, "sdiv w%d, w%d, w%d\n", reg1, reg1, reg2);
            break;
          case BINARY_OP_EQ:
            fprintf(adotout, "cmp w%d, w%d\n", reg1, reg2);
            fprintf(adotout, "beq _ELSE_%d\n", _const);
            fprintf(adotout, "mov w%d, #0\n", reg1);
            fprintf(adotout, "b _END_%d\n", _const);
            fprintf(adotout, "_ELSE_%d:\n", _const);
            fprintf(adotout, "mov w%d, #1\n", reg1);
            fprintf(adotout, "_END_%d:\n", _const);
            ++_const;
            break;
          case BINARY_OP_GE:
            fprintf(adotout, "cmp w%d, w%d\n", reg1, reg2);
            fprintf(adotout, "bge _ELSE_%d\n", _const);
            fprintf(adotout, "mov w%d, #0\n", reg1);
            fprintf(adotout, "b _END_%d\n", _const);
            fprintf(adotout, "_ELSE_%d:\n", _const);
            fprintf(adotout, "mov w%d, #1\n", reg1);
            fprintf(adotout, "_END_%d:\n", _const);
            ++_const;
            break;
          case BINARY_OP_LE:
            fprintf(adotout, "cmp w%d, w%d\n", reg1, reg2);
            fprintf(adotout, "ble _ELSE_%d\n", _const);
            fprintf(adotout, "mov w%d, #0\n", reg1);
            fprintf(adotout, "b _END_%d\n", _const);
            fprintf(adotout, "_ELSE_%d:\n", _const);
            fprintf(adotout, "mov w%d, #1\n", reg1);
            fprintf(adotout, "_END_%d:\n", _const);
            ++_const;
            break;
          case BINARY_OP_NE:
            fprintf(adotout, "cmp w%d, w%d\n", reg1, reg2);
            fprintf(adotout, "bne _ELSE_%d\n", _const);
            fprintf(adotout, "mov w%d, #0\n", reg1);
            fprintf(adotout, "b _END_%d\n", _const);
            fprintf(adotout, "_ELSE_%d:\n", _const);
            fprintf(adotout, "mov w%d, #1\n", reg1);
            fprintf(adotout, "_END_%d:\n", _const);
            ++_const;
            break;
          case BINARY_OP_GT:
            fprintf(adotout, "cmp w%d, w%d\n", reg1, reg2);
            fprintf(adotout, "bgt _ELSE_%d\n", _const);
            fprintf(adotout, "mov w%d, #0\n", reg1);
            fprintf(adotout, "b _END_%d\n", _const);
            fprintf(adotout, "_ELSE_%d:\n", _const);
            fprintf(adotout, "mov w%d, #1\n", reg1);
            fprintf(adotout, "_END_%d:\n", _const);
            ++_const;
            break;
          case BINARY_OP_LT:
            fprintf(adotout, "cmp w%d, w%d\n", reg1, reg2);
            fprintf(adotout, "blt _ELSE_%d\n", _const);
            fprintf(adotout, "mov w%d, #0\n", reg1);
            fprintf(adotout, "b _END_%d\n", _const);
            fprintf(adotout, "_ELSE_%d:\n", _const);
            fprintf(adotout, "mov w%d, #1\n", reg1);
            fprintf(adotout, "_END_%d:\n", _const);
            ++_const;
            break;
          case BINARY_OP_AND:
            fprintf(adotout, "cmp w%d, #0\n", reg1);
            fprintf(adotout, "beq _ELSE_%d\n", _const);
            fprintf(adotout, "cmp w%d, #0\n", reg2);
            fprintf(adotout, "beq _ELSE_%d\n", _const);
            fprintf(adotout, "mov w%d, #1\n", reg1);
            fprintf(adotout, "b _END_%d\n", _const);
            fprintf(adotout, "_ELSE_%d:\n", _const);
            fprintf(adotout, "mov w%d, #0\n", reg1);
            fprintf(adotout, "_END_%d:\n", _const);
            ++_const;
            break;
          case BINARY_OP_OR:
            fprintf(adotout, "cmp w%d, #0\n", reg1);
            fprintf(adotout, "bne _ELSE_%d\n", _const);
            fprintf(adotout, "cmp w%d, #0\n", reg2);
            fprintf(adotout, "bne _ELSE_%d\n", _const);
            fprintf(adotout, "mov w%d, #0\n", reg1);
            fprintf(adotout, "b _END_%d\n", _const);
            fprintf(adotout, "_ELSE_%d:\n", _const);
            fprintf(adotout, "mov w%d, #1\n", reg1);
            fprintf(adotout, "_END_%d:\n", _const);
            ++_const;
            break;
        }
        freeReg(reg2);
        return reg1;
      }else{
        exprNode->dataType = FLOAT_TYPE;
        if(leftOp->dataType == INT_TYPE){
          fprintf(adotout, "scvtf s%d, w%d\n", reg1, reg1);
        }
        if(rightOp->dataType == INT_TYPE){
          fprintf(adotout, "scvtf s%d, w%d\n", reg2, reg2);
        }
        switch(exprNode->semantic_value.exprSemanticValue.op.binaryOp){
          case BINARY_OP_ADD:
            fprintf(adotout, "fadd s%d, s%d, s%d\n", reg1, reg1, reg2);
            break;
          case BINARY_OP_SUB:
            fprintf(adotout, "fsub s%d, s%d, s%d\n", reg1, reg1, reg2);
            break;
          case BINARY_OP_MUL:
            fprintf(adotout, "fmul s%d, s%d, s%d\n", reg1, reg1, reg2);
            break;
          case BINARY_OP_DIV:
            fprintf(adotout, "fdiv s%d, s%d, s%d\n", reg1, reg1, reg2);
            break;
          case BINARY_OP_EQ:
            exprNode->dataType = INT_TYPE;
            fprintf(adotout, "fcmp s%d, s%d\n", reg1, reg2);
            fprintf(adotout, "beq _ELSE_%d\n", _const);
            fprintf(adotout, "mov w%d, #0\n", reg1);
            fprintf(adotout, "b _END_%d\n", _const);
            fprintf(adotout, "_ELSE_%d:\n", _const);
            fprintf(adotout, "mov w%d, #1\n", reg1);
            fprintf(adotout, "_END_%d:\n", _const);
            ++_const;
            break;
          case BINARY_OP_GE:
            exprNode->dataType = INT_TYPE;
            fprintf(adotout, "fcmp s%d, s%d\n", reg1, reg2);
            fprintf(adotout, "bge _ELSE_%d\n", _const);
            fprintf(adotout, "mov w%d, #0\n", reg1);
            fprintf(adotout, "b _END_%d\n", _const);
            fprintf(adotout, "_ELSE_%d:\n", _const);
            fprintf(adotout, "mov w%d, #1\n", reg1);
            fprintf(adotout, "_END_%d:\n", _const);
            ++_const;
            break;
          case BINARY_OP_LE:
            exprNode->dataType = INT_TYPE;
            fprintf(adotout, "fcmp s%d, s%d\n", reg1, reg2);
            fprintf(adotout, "ble _ELSE_%d\n", _const);
            fprintf(adotout, "mov w%d, #0\n", reg1);
            fprintf(adotout, "b _END_%d\n", _const);
            fprintf(adotout, "_ELSE_%d:\n", _const);
            fprintf(adotout, "mov w%d, #1\n", reg1);
            fprintf(adotout, "_END_%d:\n", _const);
            ++_const;
            break;
          case BINARY_OP_NE:
            exprNode->dataType = INT_TYPE;
            fprintf(adotout, "fcmp s%d, s%d\n", reg1, reg2);
            fprintf(adotout, "bne _ELSE_%d\n", _const);
            fprintf(adotout, "mov w%d, #0\n", reg1);
            fprintf(adotout, "b _END_%d\n", _const);
            fprintf(adotout, "_ELSE_%d:\n", _const);
            fprintf(adotout, "mov w%d, #1\n", reg1);
            fprintf(adotout, "_END_%d:\n", _const);
            ++_const;
            break;
          case BINARY_OP_GT:
            exprNode->dataType = INT_TYPE;
            fprintf(adotout, "fcmp s%d, s%d\n", reg1, reg2);
            fprintf(adotout, "bgt _ELSE_%d\n", _const);
            fprintf(adotout, "mov w%d, #0\n", reg1);
            fprintf(adotout, "b _END_%d\n", _const);
            fprintf(adotout, "_ELSE_%d:\n", _const);
            fprintf(adotout, "mov w%d, #1\n", reg1);
            fprintf(adotout, "_END_%d:\n", _const);
            ++_const;
            break;
          case BINARY_OP_LT:
            exprNode->dataType = INT_TYPE;
            fprintf(adotout, "fcmp s%d, s%d\n", reg1, reg2);
            fprintf(adotout, "blt _ELSE_%d\n", _const);
            fprintf(adotout, "mov w%d, #0\n", reg1);
            fprintf(adotout, "b _END_%d\n", _const);
            fprintf(adotout, "_ELSE_%d:\n", _const);
            fprintf(adotout, "mov w%d, #1\n", reg1);
            fprintf(adotout, "_END_%d:\n", _const);
            ++_const;
            break;
          case BINARY_OP_AND:
            exprNode->dataType = INT_TYPE;
            fprintf(adotout, "fcmp s%d, #0\n", reg1);
            fprintf(adotout, "beq _ELSE_%d\n", _const);
            fprintf(adotout, "cmp s%d, #0\n", reg2);
            fprintf(adotout, "beq _ELSE_%d\n", _const);
            fprintf(adotout, "mov w%d, #1\n", reg1);
            fprintf(adotout, "b _END_%d\n", _const);
            fprintf(adotout, "_ELSE_%d:\n", _const);
            fprintf(adotout, "mov w%d, #0\n", reg1);
            fprintf(adotout, "_END_%d:\n", _const);
            ++_const;
            break;
          case BINARY_OP_OR:
            exprNode->dataType = INT_TYPE;
            fprintf(adotout, "fcmp s%d, #0\n", reg1);
            fprintf(adotout, "bne _ELSE_%d\n", _const);
            fprintf(adotout, "cmp s%d, #0\n", reg2);
            fprintf(adotout, "bne _ELSE_%d\n", _const);
            fprintf(adotout, "mov w%d, #0\n", reg1);
            fprintf(adotout, "b _END_%d\n", _const);
            fprintf(adotout, "_ELSE_%d:\n", _const);
            fprintf(adotout, "mov w%d, #1\n", reg1);
            fprintf(adotout, "_END_%d:\n", _const);
            ++_const;
            break;
        }
        freeReg(reg2);
        return reg1;
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
      fprintf(adotout, ".data\n");
      if(exprNode->dataType == INT_TYPE){
        fprintf(adotout, "_integer_const_%d: .word %d\n", _const, exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue);
      }else{
        fprintf(adotout, "_float_const_%d: .float %f\n", _const, exprNode->semantic_value.exprSemanticValue.constEvalValue.fValue);
      }
      emitAlignment();
      fprintf(adotout, ".text\n");
      int reg = getReg();
      if(exprNode->dataType == INT_TYPE){
        fprintf(adotout, "ldr w%d, _integer_const_%d\n", reg, _const);
      }else{
        fprintf(adotout, "ldr s%d, _float_const_%d\n", reg, _const);
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
            fprintf(adotout, "neg w%d, w%d", reg, reg);
            break;
          case UNARY_OP_LOGICAL_NEGATION:
            fprintf(adotout, "cmp w%d, #0\n", reg);
            fprintf(adotout, "beq _ELSE_%d\n", _const);
            fprintf(adotout, "mov w%d, #0\n", reg);
            fprintf(adotout, "b _END_%d\n", _const);
            fprintf(adotout, "_ELSE_%d:\n", _const);
            fprintf(adotout, "mov w%d, #1\n", reg);
            fprintf(adotout, "_END_%d:\n", _const);
            ++_const;
            break;
        }
      }else{
        exprNode->dataType = FLOAT_TYPE;
        switch(exprNode->semantic_value.exprSemanticValue.op.unaryOp){
          case UNARY_OP_POSITIVE:
            break;
          case UNARY_OP_NEGATIVE:
            fprintf(adotout, "fneg s%d, s%d", reg, reg);
            break;
          case UNARY_OP_LOGICAL_NEGATION:
            exprNode->dataType = INT_TYPE;
            fprintf(adotout, "fcvtzs w%d, s%d\n", reg, reg);
            fprintf(adotout, "cmp w%d, #0\n", reg);
            fprintf(adotout, "beq _ELSE_%d\n", _const);
            fprintf(adotout, "mov w%d, #0\n", reg);
            fprintf(adotout, "b _END_%d\n", _const);
            fprintf(adotout, "_ELSE_%d:\n", _const);
            fprintf(adotout, "mov w%d, #1\n", reg);
            fprintf(adotout, "_END_%d:\n", _const);
            ++_const;
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
  /* hw5 only need to implement parameter less function */
}

void emitPrologue(){
  fprintf(adotout, "str x30, [sp, #0]\n");
  fprintf(adotout, "str x29, [sp, #-8]\n");
  fprintf(adotout, "add x29, sp, #-8\n");
  fprintf(adotout, "add sp, sp, #-16\n");
  int offset = 0;
  for(int i = 19; i <= 29; ++i){
    offset += 8;
    fprintf(adotout, "str x%d, [x29, #%d]\n", i, -offset);
  }
  fprintf(adotout, ".data\n");
  fprintf(adotout, "_AR_SIZE_%d: .word %d\n", _const, offset);
  emitAlignment();
  fprintf(adotout, ".text\n");
  fprintf(adotout, "ldr w19, _AR_SIZE_%d\n", _const);
  fprintf(adotout, "sub sp, sp, w19\n");
  ++_const;
  _offset = 0;
}

void emitEpilogue(){
  int offset = 0;
  for(int i = 19; i <= 29; ++i){
    offset += 8;
    fprintf(adotout, "ldr x%d, [x29, #%d]\n", i, -offset);
  }
  fprintf(adotout, "ldr x30, [x29, #8]\n");
  fprintf(adotout, "add sp, x29, #8\n");
  fprintf(adotout, "ldr x29, [x29, #0]\n");
  fprintf(adotout, "ret x30\n");
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
        fprintf(adotout, "fmov s0, s%d\n", reg);
        fprintf(adotout, "fcvtzs w0, s0\n");
      }
      break;
    case FLOAT_TYPE:
      if(returnNode->child->dataType == INT_TYPE){
        fprintf(adotout, "mov w0, w%d\n", reg);
        fprintf(adotout, "scvtf s0, w0\n");
      }else{
        fprintf(adotout, "fmov s0, s%d\n", reg);
      }
      break;
  }
  fprintf(adotout, "b _end_%s\n", idName(parentNode->child->rightSibling));
}

