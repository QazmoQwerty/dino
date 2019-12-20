#include "CodeGenerator.h"

/*llvm::LLVMContext CodeGenerator::_context;
llvm::IRBuilder<> CodeGenerator::_builder;
std::unique_ptr<llvm::Module> CodeGenerator::_module;
std::map<std::string, Value *> CodeGenerator::_namedValues;*/

Value *CodeGenerator::codeGen(DST::Node *node) 
{
    if (node == nullptr) 
        return nullptr;
    if (node->isExpression())
        return codeGen(dynamic_cast<DST::Expression*>(node));
    else return codeGen(dynamic_cast<DST::Statement*>(node));
}

Value *CodeGenerator::codeGen(DST::Statement *node) 
{
    return NULL;
}

Value *CodeGenerator::codeGen(DST::Expression *node) 
{
    switch (node->getExpressionType()) 
    {
        case ET_BINARY_OPERATION: return codeGen((DST::BinaryOperation*)node);
        case ET_LITERAL: return codeGen((DST::Literal*)node);
        default: return NULL;
    }
}

Value *CodeGenerator::codeGen(DST::Literal *node) 
{
    switch (node->getLiteralType())
    {
    case LT_BOOLEAN:
        return llvm::ConstantInt::get(_context, llvm::APInt( /* 1 bit width */ 1, ((AST::Boolean*)node->getBase())->getValue()));
    case LT_INTEGER:
        return llvm::ConstantInt::get(_context, llvm::APInt( /* 64 bit width */ 64, ((AST::Integer*)node->getBase())->getValue(), /* signed */ true));
    default:
        return NULL;
    }
}

Value *CodeGenerator::codeGen(DST::BinaryOperation* node)
{
    Value *left = codeGen(node->getLeft());
    Value *right = codeGen(node->getRight());
    switch (node->getOperator()._type)
    {
        case OT_ADD:
            return _builder.CreateAdd(left, right, "addtmp");
        case OT_SUBTRACT:
            return _builder.CreateSub(left, right, "subtmp");
        case OT_SMALLER:
            return _builder.CreateICmpULT(left, right, "cmptmp");
        default:
            return NULL;
    }
    
}

