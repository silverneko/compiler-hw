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
void processDeclDimList(AST_NODE* id_node, DATA_TYPE datatype, TypeDescriptor* desc, int ignoreFirstDimSize);
void processTypeNode(AST_NODE* typeNode);
void processBlockNode(AST_NODE* blockNode);
void processStmtNode(AST_NODE* stmtNode);
void processGeneralNode(AST_NODE *node);
void checkAssignOrExpr(AST_NODE* assignOrExprRelatedNode);
void checkWhileStmt(AST_NODE* whileNode);
void checkForStmt(AST_NODE* forNode);
void checkAssignmentStmt(AST_NODE* assignmentNode);
void checkIfStmt(AST_NODE* ifNode);
void checkWriteFunction(AST_NODE* functionCallNode);
void checkFunctionCall(AST_NODE* functionCallNode);
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

void printErrorMsgSpecial(AST_NODE* node1, char* name2, ErrorMsgKind errorMsgKind)
{
    g_anyErrorOccur = 1;
    printf("Error found in line %d\n", node1->linenumber);
    /*
    switch(errorMsgKind)
    {
    default:
        printf("Unhandled case in void printErrorMsg(AST_NODE* node, ERROR_MSG_KIND* errorMsgKind)\n");
        break;
    }
    */
}


void printErrorMsg(AST_NODE* node, ErrorMsgKind errorMsgKind)
{
    g_anyErrorOccur = 1;
    printf("Error found in line %d\n", node->linenumber);
    /*
    switch(errorMsgKind)
    {
        printf("Unhandled case in void printErrorMsg(AST_NODE* node, ERROR_MSG_KIND* errorMsgKind)\n");
        break;
    }
    */
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
int insertTypeDef(char *name, DATA_TYPE type) {
    SymbolAttribute *attr;
    TypeDescriptor *desc;

    desc = createTypeDesc(SCALAR_TYPE_DESCRIPTOR);
    desc->dataType = type;
    attr = createSymAttr(TYPE_ATTRIBUTE);
    attr->typeDescriptor = desc;
    enterSymbol(name, attr);

    return 0;
}

void semanticAnalysis(AST_NODE *root)
{
    insertTypeDef(SYMBOL_TABLE_INT_NAME, INT_TYPE);
    insertTypeDef(SYMBOL_TABLE_FLOAT_NAME, FLOAT_TYPE);
    insertTypeDef(SYMBOL_TABLE_VOID_NAME, VOID_TYPE);

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
        //TODO CHECK
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
                AST_ITER_SIBLING(type_node->rightSibling, id_node) {
                    insertTypeDef(id_node->semantic_value.identifierSemanticValue.identifierName, datatype);
                }

                break;
            }
            case VARIABLE_DECL:
            {
                AST_NODE *type_node, *id_node;
                DATA_TYPE datatype;

                type_node = child->child;
                datatype = findTypeDecl(type_node->semantic_value.identifierSemanticValue.identifierName);
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
                            enterSymbol(id_node->semantic_value.identifierSemanticValue.identifierName, attr);

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
                            enterSymbol(id_node->semantic_value.identifierSemanticValue.identifierName, attr);

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
                processDeclarationNode(child);
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
    assert(declarationNode->nodeType == DECLARATION_NODE);
    switch(declarationNode->semantic_value.declSemanticValue.kind) {
        case VARIABLE_DECL:
            //TODO
            break;
        case TYPE_DECL:
            //TODO
            break;
        case FUNCTION_DECL:
            declareFunction(declarationNode);
            break;
    }
}


void processTypeNode(AST_NODE* idNodeAsType)
{
}


void declareIdList(AST_NODE* declarationNode, SymbolAttributeKind isVariableOrTypeAttribute, int ignoreArrayFirstDimSize)
{
}

void checkAssignOrExpr(AST_NODE* assignOrExprRelatedNode)
{
}

void checkWhileStmt(AST_NODE* whileNode)
{
}


void checkForStmt(AST_NODE* forNode)
{
}


void checkAssignmentStmt(AST_NODE* assignmentNode)
{
}


void checkIfStmt(AST_NODE* ifNode)
{
}

void checkWriteFunction(AST_NODE* functionCallNode)
{
}

void checkFunctionCall(AST_NODE* functionCallNode)
{
}

void checkParameterPassing(Parameter* formalParameter, AST_NODE* actualParameter)
{
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
}


void processBlockNode(AST_NODE* blockNode)
{
}


void processStmtNode(AST_NODE* stmtNode)
{
}


void processGeneralNode(AST_NODE *node)
{
}

void processDeclDimList(AST_NODE* id_node, DATA_TYPE datatype, TypeDescriptor* desc, int ignoreFirstDimSize)
{
    AST_NODE *array_node;
    int dim;

    desc->arrayProperties.elementType = datatype;
    dim = 0;
    AST_ITER_CHILD(id_node, array_node) {
        //TODO calculte size

        if(array_node->nodeType = NUL_NODE) {
            if(ignoreFirstDimSize) {
                desc->arrayProperties.sizeInEachDimension[dim] = -1;
            } else {
                //TODO Check
            }
        } else {
            assert(array_node->nodeType == CONST_VALUE_NODE);
            desc->arrayProperties.sizeInEachDimension[dim] = array_node->semantic_value.const1->const_u.intval;
        }

        dim += 1;
        ignoreFirstDimSize = 0;
    }
    desc->arrayProperties.dimension = dim;
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

    attr = createSymAttr(FUNCTION_SIGNATURE); 
    attr->functionSignature = func;
    enterSymbol(name_node->semantic_value.identifierSemanticValue.identifierName, attr);
    
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

        switch(id_node->semantic_value.identifierSemanticValue.kind) {
            case NORMAL_ID:
            {
                TypeDescriptor *desc;

                desc = createTypeDesc(SCALAR_TYPE_DESCRIPTOR);
                desc->dataType = datatype;
                attr = createSymAttr(VARIABLE_ATTRIBUTE);
                attr->typeDescriptor = desc;
                enterSymbol(id_node->semantic_value.identifierSemanticValue.identifierName, attr);

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
                enterSymbol(id_node->semantic_value.identifierSemanticValue.identifierName, attr);

                new_param->type = desc;
                new_param->parameterName = id_node->semantic_value.identifierSemanticValue.identifierName;

                break;
            }
            default:
                puts("unexpected kind");
                abort();
                break;
        }

        if(last_param == NULL) {
            func->parameterList = new_param;
        } else {
            last_param->next =  new_param;
        }
        last_param = new_param;
    }

    closeScope();

}
