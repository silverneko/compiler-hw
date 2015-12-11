#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "header.h"
#include "symbolTable.h"
// This file is for reference only, you are not required to follow the implementation. //
// You only need to check for errors stated in the hw4 assignment document. //
int g_anyErrorOccur = 0;

DATA_TYPE getBiggerType(DATA_TYPE dataType1, DATA_TYPE dataType2);
void processProgramNode(AST_NODE *programNode);
void processDeclarationNode(AST_NODE* declarationNode);
void declareIdList(AST_NODE* typeNode, SymbolAttributeKind isVariableOrTypeAttribute, int ignoreArrayFirstDimSize);
void declareFunction(AST_NODE* returnTypeNode);
int processDeclDimList(AST_NODE* id_node, DATA_TYPE datatype, TypeDescriptor* desc, int ignoreFirstDimSize);
void processTypeNode(AST_NODE* typeNode);
void processBlockNode(AST_NODE* blockNode);
void processStmtNode(AST_NODE* stmtNode);
void processGeneralNode(AST_NODE *node);
DATA_TYPE checkAssignOrExpr(AST_NODE* assignOrExprRelatedNode);
void checkWhileStmt(AST_NODE* whileNode);
void checkForStmt(AST_NODE* forNode);
void checkAssignmentStmt(AST_NODE* assignmentNode);
void checkIfStmt(AST_NODE* ifNode);
void checkWriteFunction(AST_NODE* functionCallNode);
DATA_TYPE checkFunctionCall(AST_NODE* functionCallNode);
void processExprRelatedNode(AST_NODE* exprRelatedNode);
void checkParameterPassing(Parameter* formalParameter, AST_NODE* actualParameter);
void checkReturnStmt(AST_NODE* returnNode);
void processExprNode(AST_NODE* exprNode);
void processVariableLValue(AST_NODE* idNode);
void processVariableRValue(AST_NODE* idNode);
void processConstValueNode(AST_NODE* constValueNode);
void getExprOrConstValue(AST_NODE* exprOrConstNode, int* iValue, float* fValue);
void evaluateExprValue(AST_NODE* exprNode);

#define AST_ITER_CHILD(parent, pos) \
    for (pos = (parent)->child; pos != NULL; pos = pos->rightSibling)
#define AST_ITER_SIBLING(head, pos) \
    for (pos = (head); pos != NULL; pos = pos->rightSibling)

typedef enum ErrorMsgKind
{
    SYMBOL_IS_NOT_TYPE,
    SYMBOL_REDECLARE,
    SYMBOL_UNDECLARED,
    NOT_FUNCTION_NAME,
    TRY_TO_INIT_ARRAY,
    EXCESSIVE_ARRAY_DIM_DECLARATION,
    RETURN_ARRAY,
    VOID_VARIABLE,
    TYPEDEF_VOID_ARRAY,
    PARAMETER_TYPE_UNMATCH,
    TOO_FEW_ARGUMENTS,
    TOO_MANY_ARGUMENTS,
    RETURN_TYPE_UNMATCH,
    INCOMPATIBLE_ARRAY_DIMENSION,
    NOT_ASSIGNABLE,
    NOT_ARRAY,
    IS_TYPE_NOT_VARIABLE,
    IS_FUNCTION_NOT_VARIABLE,
    STRING_OPERATION,
    ARRAY_SIZE_NOT_INT,
    ARRAY_SIZE_NEGATIVE,
    ARRAY_SUBSCRIPT_NOT_INT,
    PASS_ARRAY_TO_SCALAR,
    PASS_SCALAR_TO_ARRAY
} ErrorMsgKind;

const char * idName(AST_NODE * node){
  return node->semantic_value.identifierSemanticValue.identifierName;
}

void printErrorMsgSpecial(AST_NODE* node1, const char* name2, ErrorMsgKind errorMsgKind)
{
    g_anyErrorOccur = 1;
    printf("Error found in line %d\n", node1->linenumber);
    switch(errorMsgKind){
      case PASS_ARRAY_TO_SCALAR:
        printf("Array %s passed to scalar parameter %s.\n", idName(node1), name2);
        break;
      case PASS_SCALAR_TO_ARRAY:
        printf("Scalar %s passed to array parameter %s.\n", idName(node1), name2);
        break;
      default:
        printf("Unhandled case in void printErrorMsg(AST_NODE* node, ERROR_MSG_KIND* errorMsgKind)\n");
        break;
    }
}


void printErrorMsg(AST_NODE* node, ErrorMsgKind errorMsgKind)
{
    g_anyErrorOccur = 1;
    printf("Error found in line %d\n", node->linenumber);
    switch(errorMsgKind){
      case SYMBOL_IS_NOT_TYPE:
        printf("ID %s is not type.\n", idName(node));
        break;
      case SYMBOL_REDECLARE:
        printf("ID %s redeclared.\n", idName(node));
        break;
      case SYMBOL_UNDECLARED:
        printf("ID %s undeclared.\n", idName(node));
        break;
      case TOO_FEW_ARGUMENTS:
        printf("Too few arguments to function %s.\n", idName(node));
        break;
      case TOO_MANY_ARGUMENTS:
        printf("Too many arguments to function %s.\n", idName(node));
        break;
      case RETURN_TYPE_UNMATCH:
        printf("Incompatible return type.\n");
        break;
      case INCOMPATIBLE_ARRAY_DIMENSION:
        printf("Incompatible array dimensions.\n");
        break;
      case ARRAY_SUBSCRIPT_NOT_INT:
        printf("Array subscript is not an integer.\n");
        break;
      case NOT_FUNCTION_NAME:
        printf("ID %s is not function name.\n", idName(node));
        break;
      default:
        printf("Unhandled case in void printErrorMsg(AST_NODE* node, ERROR_MSG_KIND* errorMsgKind)\n");
        break;
    }
}

TypeDescriptor* createTypeDesc(TypeDescriptorKind kind) {
    TypeDescriptor *desc;
    desc = (TypeDescriptor*)malloc(sizeof(*desc));
    desc->kind = kind;
    return desc;
}
SymbolAttribute* createSymAttr(SymbolAttributeKind attributeKind) {
    SymbolAttribute *attr;
    attr = (SymbolAttribute*)malloc(sizeof(*attr));
    attr->attributeKind = attributeKind;
    return attr;
}
SymbolTableEntry * insertTypeDef(const char *name, DATA_TYPE type) {
    SymbolAttribute *attr;
    TypeDescriptor *desc;

    desc = createTypeDesc(SCALAR_TYPE_DESCRIPTOR);
    desc->dataType = type;
    attr = createSymAttr(TYPE_ATTRIBUTE);
    attr->typeDescriptor = desc;
    return enterSymbol(name, attr);
}

void semanticAnalysis(AST_NODE *root)
{
    insertTypeDef(SYMBOL_TABLE_INT_NAME, INT_TYPE);
    insertTypeDef(SYMBOL_TABLE_FLOAT_NAME, FLOAT_TYPE);
    insertTypeDef(SYMBOL_TABLE_VOID_NAME, VOID_TYPE);

    // TODO read() fread() write(int or float or strlit)

    processProgramNode(root);
}


DATA_TYPE getBiggerType(DATA_TYPE dataType1, DATA_TYPE dataType2)
{
    if(dataType1 == FLOAT_TYPE || dataType2 == FLOAT_TYPE) {
        return FLOAT_TYPE;
    } else {
        return INT_TYPE;
    }
}

DATA_TYPE findTypeDecl(char *name) {
    SymbolTableEntry* entry;
    if((entry = retrieveSymbol(name)) == NULL) {
        return ERROR_TYPE;
    }
    if(entry->symbolAttribute->attributeKind != TYPE_ATTRIBUTE) {
        return ERROR_TYPE;
    }
    return entry->symbolAttribute->typeDescriptor->dataType;
}

void processVariableDeclList(AST_NODE *listNode) {
    AST_NODE *child;

    AST_ITER_CHILD(listNode, child) {
        assert(child->nodeType == DECLARATION_NODE);
        switch(child->semantic_value.declSemanticValue.kind) {
            case TYPE_DECL:
            {
                AST_NODE *type_node, *id_node;
                DATA_TYPE datatype;

                type_node = child->child;
                datatype = findTypeDecl(type_node->semantic_value.identifierSemanticValue.identifierName);
                if(datatype == ERROR_TYPE){
                  printf("type decl error\n");
                  exit(1);
                }
                AST_ITER_SIBLING(type_node->rightSibling, id_node) {
                    if(insertTypeDef(idName(id_node), datatype) == NULL){
                      printErrorMsg(id_node, SYMBOL_REDECLARE);
                    }
                }

                break;
            }
            case VARIABLE_DECL:
            {
                AST_NODE *type_node, *id_node;
                DATA_TYPE datatype;

                type_node = child->child;
                datatype = findTypeDecl(type_node->semantic_value.identifierSemanticValue.identifierName);
                if(datatype == ERROR_TYPE){
                  printErrorMsg(type_node, SYMBOL_IS_NOT_TYPE);
                  break;
                }
                AST_ITER_SIBLING(type_node->rightSibling, id_node) {
                    switch(id_node->semantic_value.identifierSemanticValue.kind) {
                        case NORMAL_ID:
                        case WITH_INIT_ID:
                        {
                            if(id_node->semantic_value.identifierSemanticValue.kind == WITH_INIT_ID) {
                                //TODO Check
                            }

                            TypeDescriptor *desc;
                            SymbolAttribute *attr;

                            desc = createTypeDesc(SCALAR_TYPE_DESCRIPTOR);
                            desc->dataType = datatype;
                            attr = createSymAttr(VARIABLE_ATTRIBUTE);
                            attr->typeDescriptor = desc;
                            if(enterSymbol(idName(id_node), attr) == NULL){
                              printErrorMsg(id_node, SYMBOL_REDECLARE);
                            }

                            break;
                        }
                        case ARRAY_ID:
                        {
                            TypeDescriptor *desc;
                            SymbolAttribute *attr;

                            desc = createTypeDesc(ARRAY_TYPE_DESCRIPTOR);
                            processDeclDimList(id_node, datatype, desc, 0);

                            attr = createSymAttr(VARIABLE_ATTRIBUTE);
                            attr->typeDescriptor = desc;
                            if(enterSymbol(idName(id_node), attr) == NULL){
                              printErrorMsg(id_node, SYMBOL_REDECLARE);
                            }

                            break;
                        }
                        default:
                            puts("unexpected kind");
                            abort();
                            break;
                    }
                }
                break;
            }
            default:
                puts("unexpected type");
                abort();
                break;
        }
    }
}

void processProgramNode(AST_NODE *programNode)
{
    AST_NODE *child;

    AST_ITER_CHILD(programNode, child) {
        switch(child->nodeType) {
            case VARIABLE_DECL_LIST_NODE:
                processVariableDeclList(child);
                break;
            case DECLARATION_NODE:
                declareFunction(child);
                break;
            default:
                puts("unexpected type");
                abort();
                break;
        }
    }
}

void processDeclarationNode(AST_NODE* declarationNode)
{
}


void processTypeNode(AST_NODE* idNodeAsType)
{
}


void declareIdList(AST_NODE* declarationNode, SymbolAttributeKind isVariableOrTypeAttribute, int ignoreArrayFirstDimSize)
{
}

DATA_TYPE checkEvalConst(AST_NODE *node, int *intval, float *floatval) {
    if(node->nodeType == CONST_VALUE_NODE) {
        if(node->semantic_value.const1->const_type == INTEGERC) {
            *intval = node->semantic_value.const1->const_u.intval;
            return INT_TYPE;
        } else if(node->semantic_value.const1->const_type == FLOATC) {
            *floatval = node->semantic_value.const1->const_u.fval;
            return FLOAT_TYPE;
        }
        return ERROR_TYPE;
    } else if(node->nodeType == EXPR_NODE && node->semantic_value.exprSemanticValue.isConstEval == 1) {
        if(node->semantic_value.exprSemanticValue.dataType == INT_TYPE) {
            *intval = node->semantic_value.exprSemanticValue.constEvalValue.iValue;
            return INT_TYPE;
        } else if(node->semantic_value.exprSemanticValue.dataType == FLOAT_TYPE) {
            *floatval = node->semantic_value.exprSemanticValue.constEvalValue.fValue;
            return FLOAT_TYPE;
        }
        return ERROR_TYPE;
    }
    return ERROR_TYPE;
}
void constFolding(AST_NODE *expr_node, DATA_TYPE datatype) {
    if(expr_node->semantic_value.exprSemanticValue.kind == UNARY_OPERATION) {
        AST_NODE *lexpr;
        DATA_TYPE ldatatype;
        int lint;
        float lfloat;

        lexpr = expr_node->child;
        ldatatype = checkEvalConst(lexpr, &lint, &lfloat);
        if(ldatatype == ERROR_TYPE) {
            return;
        }

        if(datatype == INT_TYPE) {
            assert(ldatatype == INT_TYPE);

            int val;

            switch(expr_node->semantic_value.exprSemanticValue.op.unaryOp) {
                case UNARY_OP_POSITIVE:
                    val = lint;
                    break;
                case UNARY_OP_NEGATIVE:
                    val = -lint;
                    break;
                case UNARY_OP_LOGICAL_NEGATION:
                    val = (~lint);
                    break;
            }

            expr_node->semantic_value.exprSemanticValue.isConstEval = 1;
            expr_node->semantic_value.exprSemanticValue.constEvalValue.iValue = val;

        } else if(datatype == FLOAT_TYPE) {
            if(ldatatype == INT_TYPE) {
                lfloat = (float)lint;
            }

            float val;

            switch(expr_node->semantic_value.exprSemanticValue.op.unaryOp) {
                case UNARY_OP_POSITIVE:
                    val = lfloat;
                    break;
                case UNARY_OP_NEGATIVE:
                    val = -lfloat;
                    break;
            }

            expr_node->semantic_value.exprSemanticValue.isConstEval = 1;
            expr_node->semantic_value.exprSemanticValue.constEvalValue.fValue = val;
        }
    } else if(expr_node->semantic_value.exprSemanticValue.kind == BINARY_OPERATION) {
        AST_NODE *lexpr, *rexpr;
        int lint, rint;
        float lfloat, rfloat;
        DATA_TYPE ldatatype, rdatatype;

        lexpr = expr_node->child;
        rexpr = lexpr->rightSibling;
        
        ldatatype = checkEvalConst(lexpr, &lint, &lfloat);
        rdatatype = checkEvalConst(rexpr, &rint, &rfloat);
        if(ldatatype == ERROR_TYPE || rdatatype == ERROR_TYPE) {
            return;
        }

        if(datatype == INT_TYPE) {
            assert(ldatatype == INT_TYPE);
            assert(rdatatype == INT_TYPE);

            int val;
            
            switch(expr_node->semantic_value.exprSemanticValue.op.binaryOp) {
                case BINARY_OP_ADD:
                    val = lint + rint;
                    break;
                case BINARY_OP_SUB:
                    val = lint - rint;
                    break;
                case BINARY_OP_MUL:
                    val = lint * rint;
                    break;
                case BINARY_OP_DIV:
                    val = lint / rint;
                    break;
                case BINARY_OP_EQ:
                    val = (lint == rint);
                    break;
                case BINARY_OP_GE:
                    val = (lint >= rint);
                    break;
                case BINARY_OP_LE:
                    val = (lint <= rint);
                    break;
                case BINARY_OP_NE:
                    val = (lint != rint);
                    break;
                case BINARY_OP_GT:
                    val = (lint > rint);
                    break;
                case BINARY_OP_LT:
                    val = (lint < rint);
                    break;
                case BINARY_OP_AND:
                    val = (lint && rint);
                    break;
                case BINARY_OP_OR:
                    val = (lint || rint);
                    break;
            }

            expr_node->semantic_value.exprSemanticValue.isConstEval = 1;
            expr_node->semantic_value.exprSemanticValue.constEvalValue.iValue = val;

        } else if(datatype == FLOAT_TYPE) {
            if(ldatatype == INT_TYPE) {
                lfloat = (float)lint;
            }
            if(rdatatype == INT_TYPE) {
                rfloat = (float)rint;
            }

            float val;
            
            switch(expr_node->semantic_value.exprSemanticValue.op.binaryOp) {
                case BINARY_OP_ADD:
                    val = lfloat + rfloat;
                    break;
                case BINARY_OP_SUB:
                    val = lfloat - rfloat;
                    break;
                case BINARY_OP_MUL:
                    val = lfloat * rfloat;
                    break;
                case BINARY_OP_DIV:
                    val = lfloat / rfloat;
                    break;
                case BINARY_OP_EQ:
                    val = (lfloat == rfloat);
                    break;
                case BINARY_OP_GE:
                    val = (lfloat >= rfloat);
                    break;
                case BINARY_OP_LE:
                    val = (lfloat <= rfloat);
                    break;
                case BINARY_OP_NE:
                    val = (lfloat != rfloat);
                    break;
                case BINARY_OP_GT:
                    val = (lfloat > rfloat);
                    break;
                case BINARY_OP_LT:
                    val = (lfloat < rfloat);
                    break;
                case BINARY_OP_AND:
                    val = (lfloat && rfloat);
                    break;
                case BINARY_OP_OR:
                    val = (lfloat || rfloat);
                    break;
            }

            expr_node->semantic_value.exprSemanticValue.isConstEval = 1;
            expr_node->semantic_value.exprSemanticValue.constEvalValue.fValue = val;
        }
    }
}
DATA_TYPE checkAssignOrExpr(AST_NODE* assignOrExprRelatedNode)
{
    if(assignOrExprRelatedNode->nodeType == CONST_VALUE_NODE) {
	CON_Type *const1 = assignOrExprRelatedNode->semantic_value.const1;
        switch(const1->const_type) {
            case INTEGERC:
                return INT_TYPE;
            case FLOATC:
                return FLOAT_TYPE;
            case STRINGC:
                return CONST_STRING_TYPE;
        }
    } else if(assignOrExprRelatedNode->nodeType == IDENTIFIER_NODE) {
        SymbolTableEntry *id_sym = retrieveSymbol(idName(assignOrExprRelatedNode));
        if(id_sym == NULL){
            printErrorMsg(assignOrExprRelatedNode, SYMBOL_UNDECLARED);
            return ERROR_TYPE;
        }

        SymbolAttribute *attr = id_sym->symbolAttribute;
        if(attr->attributeKind != VARIABLE_ATTRIBUTE) {
            //TODO print error message
            return ERROR_TYPE;
        }

        TypeDescriptor *desc;
        desc = attr->typeDescriptor;

        if(desc->kind == SCALAR_TYPE_DESCRIPTOR) {
            if(assignOrExprRelatedNode->semantic_value.identifierSemanticValue.kind != NORMAL_ID) {
                //TODO print error message
                return ERROR_TYPE;
            }
            return desc->dataType;
        } else if(desc->kind == ARRAY_TYPE_DESCRIPTOR) {
            if(assignOrExprRelatedNode->semantic_value.identifierSemanticValue.kind != ARRAY_ID) {
                //TODO print error message
                return ERROR_TYPE;
            }
            if(processDeclDimList(assignOrExprRelatedNode, NONE_TYPE, NULL, 0) != desc->arrayProperties.dimension) {
                //TODO print error message
                return ERROR_TYPE;
            }
            return desc->arrayProperties.elementType;
        }
    } else if(assignOrExprRelatedNode->nodeType == STMT_NODE) {
        if(assignOrExprRelatedNode->semantic_value.stmtSemanticValue.kind == FUNCTION_CALL_STMT) {
            return checkFunctionCall(assignOrExprRelatedNode);
        } else if(assignOrExprRelatedNode->semantic_value.stmtSemanticValue.kind == ASSIGN_STMT) {
            AST_NODE *lvalue_node;
            AST_NODE *expr_node;
            DATA_TYPE datatype;

            lvalue_node = assignOrExprRelatedNode->child;
            datatype = checkAssignOrExpr(lvalue_node);

            expr_node = lvalue_node->rightSibling;
            checkAssignOrExpr(expr_node);

            //TODO is this correct ?
            return datatype;
        }
    } else if(assignOrExprRelatedNode->nodeType == EXPR_NODE) {
        AST_NODE *expr_node;
        DATA_TYPE datatype;

        expr_node = assignOrExprRelatedNode;
        datatype = ERROR_TYPE;

        switch(expr_node->semantic_value.exprSemanticValue.kind) {
            case BINARY_OPERATION:
            {
                DATA_TYPE ldatatype, rdatatype;

                ldatatype = checkAssignOrExpr(expr_node->child);
                rdatatype = checkAssignOrExpr(expr_node->child->rightSibling);

                if(ldatatype != INT_TYPE && ldatatype != FLOAT_TYPE) {
                    //TODO type error
                    datatype = ERROR_TYPE;
                    break;
                }
                if(rdatatype != INT_TYPE && rdatatype != FLOAT_TYPE) {
                    //TODO type error
                    datatype = ERROR_TYPE;
                    break;
                }

                switch(expr_node->semantic_value.exprSemanticValue.op.binaryOp) {
                    case BINARY_OP_ADD:
                    case BINARY_OP_SUB:
                    case BINARY_OP_MUL:
                    case BINARY_OP_DIV:
                        datatype = ldatatype;
                        if(ldatatype == FLOAT_TYPE || rdatatype == FLOAT_TYPE) {
                            datatype = FLOAT_TYPE;
                        }
                        constFolding(expr_node, datatype);
                        break;
                    case BINARY_OP_EQ:
                    case BINARY_OP_GE:
                    case BINARY_OP_LE:
                    case BINARY_OP_NE:
                    case BINARY_OP_GT:
                    case BINARY_OP_LT:
                    case BINARY_OP_AND:
                    case BINARY_OP_OR:
                        datatype = INT_TYPE;
                        constFolding(expr_node, datatype);
                        break;
                }
                break;
            }
            case UNARY_OPERATION:
            {
                datatype = checkAssignOrExpr(expr_node->child);
                if(datatype != INT_TYPE && datatype != FLOAT_TYPE) {
                    //TODO type error
                    datatype = ERROR_TYPE;
                    break;
                }
                if(expr_node->semantic_value.exprSemanticValue.op.unaryOp == UNARY_OP_LOGICAL_NEGATION) {
                    datatype = INT_TYPE;
                }
                constFolding(expr_node, datatype);
                break;
            }
            default:
                puts("unexpected kind");
                abort();
                break;
        }
        
        expr_node->semantic_value.exprSemanticValue.dataType = datatype;

        return datatype;
    }

    puts("unexpected case");
    abort();
    return ERROR_TYPE;
}

void checkWhileStmt(AST_NODE* whileNode)
{
    checkAssignOrExpr(whileNode->child);
    processStmtNode(whileNode->child->rightSibling);
}


void checkForStmt(AST_NODE* forNode)
{
    AST_NODE *cur;
    AST_NODE *child;

    cur = forNode->child;
    if(cur->nodeType == NONEMPTY_ASSIGN_EXPR_LIST_NODE) {
        AST_ITER_CHILD(cur, child) {
            checkAssignOrExpr(child);
        }
    }
    cur = cur->rightSibling;
    if(cur->nodeType == NONEMPTY_RELOP_EXPR_LIST_NODE) {
        AST_ITER_CHILD(cur, child) {
            checkAssignOrExpr(child);
        }
    }
    cur = cur->rightSibling;
    if(cur->nodeType == NONEMPTY_ASSIGN_EXPR_LIST_NODE) {
        AST_ITER_CHILD(cur, child) {
            checkAssignOrExpr(child);
        }
    }
    cur = cur->rightSibling;
    processStmtNode(cur);
}

void checkIfStmt(AST_NODE* ifNode)
{
    AST_NODE *cur;

    cur = ifNode->child;
    checkAssignOrExpr(cur);
    cur = cur->rightSibling;
    processStmtNode(cur);
    cur = cur->rightSibling;
    if(cur->nodeType != NUL_NODE) {
        processStmtNode(cur);
    }
}

void checkWriteFunction(AST_NODE* functionCallNode)
{
}

TypeDescriptor * exprType(AST_NODE * node){
  // TODO : return the kind of the expression, scalar or array
  TypeDescriptor * typedesc = malloc(sizeof(TypeDescriptor));
  typedesc->kind = SCALAR_TYPE_DESCRIPTOR;
  if(node->nodeType == IDENTIFIER_NODE){
    switch(node->semantic_value.identifierSemanticValue.kind){
      case ARRAY_ID:
      case NORMAL_ID:
      {
        SymbolTableEntry * arr = retrieveSymbol(idName(node));
        if(arr == NULL){
          return NULL;
        }
        if(arr->symbolAttribute->typeDescriptor->kind == SCALAR_TYPE_DESCRIPTOR){
          break;
        }
        // array type
        typedesc->kind = ARRAY_TYPE_DESCRIPTOR;
        AST_NODE * dims = node->child;
        int dimsn = 0;
        while(dims != NULL){
          ++dimsn;
          dims = dims->rightSibling;
        }
        typedesc->arrayProperties.dimension = arr->symbolAttribute->typeDescriptor->arrayProperties.dimension - dimsn;
        if(typedesc->arrayProperties.dimension == 0){
          typedesc->kind = SCALAR_TYPE_DESCRIPTOR;
          break;
        }else if(typedesc->arrayProperties.dimension < 0){
          typedesc->kind = SCALAR_TYPE_DESCRIPTOR;
          break;
        }
        for(int i = 0; i < typedesc->arrayProperties.dimension; ++i){
          typedesc->arrayProperties.sizeInEachDimension[i] = 
            arr->symbolAttribute->typeDescriptor->arrayProperties.sizeInEachDimension[dimsn + i];
        }
        break;
      }
      default:
        break;
    }
  }
  return typedesc;
}

DATA_TYPE checkFunctionCall(AST_NODE* functionCallNode){
  AST_NODE * _function = functionCallNode->child;
  AST_NODE * args      = _function->rightSibling;
  SymbolTableEntry * function = retrieveSymbol(idName(_function));
  if(function == NULL){
    printErrorMsg(_function, SYMBOL_UNDECLARED);
    return ERROR_TYPE;
  }else if(function->symbolAttribute->attributeKind != FUNCTION_SIGNATURE){
    printErrorMsg(_function, NOT_FUNCTION_NAME);
    return ERROR_TYPE;
  }
  FunctionSignature * functionSig = function->symbolAttribute->functionSignature;

  if(functionSig->parametersCount == 0 && args->nodeType == NUL_NODE){
    return functionSig->returnType;
  }
  if(functionSig->parametersCount != 0 && args->nodeType == NUL_NODE){
    printErrorMsg(_function, TOO_FEW_ARGUMENTS);
    return functionSig->returnType;
  }
  if(args->nodeType != NUL_NODE){
    args = args->child;
  }
  for(Parameter * param = functionSig->parameterList; param != NULL; param = param->next){
    if(args == NULL){
      printErrorMsg(_function, TOO_FEW_ARGUMENTS);
      break;
    }
    checkParameterPassing(param, args);
    args = args->rightSibling;
  }
  if(args != NULL){
    printErrorMsg(_function, TOO_MANY_ARGUMENTS);
  }

  return functionSig->returnType;
}

void checkParameterPassing(Parameter * param, AST_NODE * arg){
  checkAssignOrExpr(arg);
  TypeDescriptor * typeDesc = exprType(arg);
  if(typeDesc == NULL){
    return ;
  }
  switch(param->type->kind){
    case SCALAR_TYPE_DESCRIPTOR:
      if(typeDesc->kind == ARRAY_TYPE_DESCRIPTOR){
        printErrorMsgSpecial(arg, param->parameterName, PASS_ARRAY_TO_SCALAR);
      }else{
        // may need type cast check
        // what if `arg` is of type `void` ?
      }
      break;
    case ARRAY_TYPE_DESCRIPTOR:
      if(typeDesc->kind == SCALAR_TYPE_DESCRIPTOR){
        printErrorMsgSpecial(arg, param->parameterName, PASS_SCALAR_TO_ARRAY);
      }else if(typeDesc->arrayProperties.dimension != param->type->arrayProperties.dimension){
          printErrorMsg(arg, INCOMPATIBLE_ARRAY_DIMENSION);
      }else{
        for(int i = 0; i < typeDesc->arrayProperties.dimension; ++i){
          if(i == 0 && param->type->arrayProperties.sizeInEachDimension[0] == -1){
            // ignore first dimensino
            continue;
          }
          if(typeDesc->arrayProperties.sizeInEachDimension[i] != param->type->arrayProperties.sizeInEachDimension[i]){
            printErrorMsg(arg, INCOMPATIBLE_ARRAY_DIMENSION);
            break;
          }
        }
      }
      break;
    default:
      printf("bad param type\n");
      exit(1);
  }
}


void processExprRelatedNode(AST_NODE* exprRelatedNode)
{
}

void getExprOrConstValue(AST_NODE* exprOrConstNode, int* iValue, float* fValue)
{
}

void evaluateExprValue(AST_NODE* exprNode)
{
}


void processExprNode(AST_NODE* exprNode)
{
}


void processVariableLValue(AST_NODE* idNode)
{
}

void processVariableRValue(AST_NODE* idNode)
{
}


void processConstValueNode(AST_NODE* constValueNode)
{
}


void checkReturnStmt(AST_NODE* returnNode)
{
    if(returnNode->nodeType != NUL_NODE) {
        checkAssignOrExpr(returnNode->child);
    }
}


void processBlockNode(AST_NODE* blockNode)
{
    AST_NODE *child;

    assert(blockNode->nodeType == BLOCK_NODE);

    AST_ITER_CHILD(blockNode, child) {
        switch(child->nodeType) {
            case VARIABLE_DECL_LIST_NODE:
                processVariableDeclList(child);
                break;
            case STMT_LIST_NODE:
            {
                AST_NODE *stmt_child;

                AST_ITER_CHILD(child, stmt_child) {
                    processStmtNode(stmt_child);
                }
                break;
            }
            default:
                puts("unexpected type");
                abort();
                break;
        }
    }
}


void processStmtNode(AST_NODE* stmtNode)
{
    // printf("%d\n", stmtNode->linenumber);

    if(stmtNode->nodeType == BLOCK_NODE) {
        processBlockNode(stmtNode);
        return;
    }
    if(stmtNode->nodeType == NUL_NODE) {
        return;
    }
    switch(stmtNode->semantic_value.stmtSemanticValue.kind) {
        case WHILE_STMT:
            checkWhileStmt(stmtNode);
            break;
        case FOR_STMT:
            checkForStmt(stmtNode);
            break;
        case ASSIGN_STMT:
            checkAssignOrExpr(stmtNode);
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
            puts("unexpected type");
            abort();
            break;
    }
}


void processGeneralNode(AST_NODE *node)
{
}

int processDeclDimList(AST_NODE* id_node, DATA_TYPE datatype, TypeDescriptor* desc, int ignoreFirstDimSize)
{
    AST_NODE *array_node;
    int dim;

    dim = 0;
    AST_ITER_CHILD(id_node, array_node) {
        if(array_node->nodeType == NUL_NODE) {
            if(ignoreFirstDimSize) {
                if(desc != NULL) {
                    desc->arrayProperties.sizeInEachDimension[dim] = -1;
                }
            } else {
                //TODO Check
            }
        } else {
            int size;

            if(array_node->nodeType == CONST_VALUE_NODE) {
                if(array_node->semantic_value.const1->const_type != INTEGERC) {
                    printErrorMsg(array_node, ARRAY_SUBSCRIPT_NOT_INT);
                    size = -2;
                } else {
                    size = array_node->semantic_value.const1->const_u.intval;
                }
            } else if(array_node->nodeType == EXPR_NODE) {
                if(checkAssignOrExpr(array_node) != INT_TYPE) {
                    printErrorMsg(array_node, ARRAY_SUBSCRIPT_NOT_INT);
                    size = -2;
                } else {
                    size = array_node->semantic_value.exprSemanticValue.constEvalValue.iValue;
                }
            }

            if(desc != NULL) {
                desc->arrayProperties.sizeInEachDimension[dim] = size;
            }
        }

        dim += 1;
        ignoreFirstDimSize = 0;
    }

    if(desc != NULL) {
        desc->arrayProperties.elementType = datatype;
        desc->arrayProperties.dimension = dim;
    }

    return dim;
}

void declareFunction(AST_NODE* declarationNode)
{
    AST_NODE *ret_node, *name_node, *param_node, *block_node; 
    AST_NODE *child;
    FunctionSignature *func;
    SymbolAttribute *attr;
    Parameter *last_param, *new_param;

    assert(declarationNode->nodeType == DECLARATION_NODE);
    assert(declarationNode->semantic_value.declSemanticValue.kind == FUNCTION_DECL);

    ret_node = declarationNode->child;
    name_node = ret_node->rightSibling;
    param_node = name_node->rightSibling;
    block_node = param_node->rightSibling;

    func = (FunctionSignature*)malloc(sizeof(*func));
    func->returnType = findTypeDecl(ret_node->semantic_value.identifierSemanticValue.identifierName);
    if(func->returnType == ERROR_TYPE){
      printErrorMsg(ret_node, SYMBOL_IS_NOT_TYPE);
      return ;
    }

    attr = createSymAttr(FUNCTION_SIGNATURE); 
    attr->functionSignature = func;
    if(enterSymbol(idName(name_node), attr) == NULL){
      printErrorMsg(name_node, SYMBOL_REDECLARE);
      return ;
    }

    
    openScope();

    func->parametersCount = 0;
    last_param = NULL;
    AST_ITER_SIBLING(param_node->child, child) {
        func->parametersCount += 1;
        new_param = (Parameter*)malloc(sizeof(*new_param));

        AST_NODE *type_node, *id_node;
        DATA_TYPE datatype;

        type_node = child->child;
        id_node = type_node->rightSibling;
        
        datatype = findTypeDecl(type_node->semantic_value.identifierSemanticValue.identifierName);
        if(datatype == ERROR_TYPE){
          printErrorMsg(type_node, SYMBOL_IS_NOT_TYPE);
          break;
        }

        switch(id_node->semantic_value.identifierSemanticValue.kind) {
            case NORMAL_ID:
            {
                TypeDescriptor *desc;

                desc = createTypeDesc(SCALAR_TYPE_DESCRIPTOR);
                desc->dataType = datatype;
                attr = createSymAttr(VARIABLE_ATTRIBUTE);
                attr->typeDescriptor = desc;
                if(enterSymbol(idName(id_node), attr) == NULL){
                  printErrorMsg(id_node, SYMBOL_REDECLARE);
                  break ;
                }

                new_param->type = desc;
                new_param->parameterName = id_node->semantic_value.identifierSemanticValue.identifierName;

                break;
            }
            case ARRAY_ID:
            {
                TypeDescriptor *desc;

                desc = createTypeDesc(ARRAY_TYPE_DESCRIPTOR);
                processDeclDimList(id_node, datatype, desc, 1);
                attr = createSymAttr(VARIABLE_ATTRIBUTE);
                attr->typeDescriptor = desc;
                if(enterSymbol(idName(id_node), attr) == NULL){
                  printErrorMsg(id_node, SYMBOL_REDECLARE);
                  break ;
                }


                new_param->type = desc;
                new_param->parameterName = id_node->semantic_value.identifierSemanticValue.identifierName;

                break;
            }
            default:
                puts("unexpected kind");
                abort();
                break;
        }
        new_param->next = NULL;
        if(last_param == NULL) {
            func->parameterList = new_param;
        } else {
            last_param->next =  new_param;
        }
        last_param = new_param;
    }

    processBlockNode(block_node);

    closeScope();

}
