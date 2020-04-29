/*
    All code generation functions which yeild an lvalue.
*/
#include "CodeGenerator.h"

Value *CodeGenerator::codeGenLval(DST::Expression *node)
{
    if (node == nullptr)
        return nullptr;
    switch (node->getExpressionType()) 
    {
        case ET_IDENTIFIER: return codeGenLval((DST::Variable*)node);
        case ET_VARIABLE_DECLARATION: return codeGenLval((DST::VariableDeclaration*)node);
        case ET_MEMBER_ACCESS: return codeGenLval((DST::MemberAccess*)node);
        case ET_UNARY_OPERATION: return codeGenLval((DST::UnaryOperation*)node);
        case ET_BINARY_OPERATION: return codeGenLval((DST::BinaryOperation*)node);
        case ET_CONVERSION: return codeGenLval((DST::Conversion*)node);
        case ET_FUNCTION_LITERAL: return codeGenLval((DST::FunctionLiteral*)node);
        default: throw ErrorReporter::report("unimplemented lval expression type.", ERR_CODEGEN, node->getPosition());
    }
}

llvm::Function *CodeGenerator::codeGenLval(DST::FunctionLiteral *node) { 
    // FIXME: function literals aren't actually literals, even though they ARE references.
    return codeGen(node);  
}

AllocaInst *CodeGenerator::codeGenLval(DST::VariableDeclaration *node) { 
    // FIXME: codeGen(DST::Vardecl*) should return a load of codeGenLval()
    return codeGen(node);  
}

Value *CodeGenerator::codeGenLval(DST::Conversion* node) 
{
    auto type = evalType(node->getType());
    auto exp = codeGenLval(node->getExpression());
    if (type->getPointerTo() == exp->getType() || type == _interfaceType)
        return exp;
    if (type != _interfaceType && exp->getType() == _interfaceType->getPointerTo()) // Interface to non-interface
    {
        // TODO - throw exception incase of invalid conversion
        auto ptr = _builder.CreateGEP(exp, {_builder.getInt32(0), _builder.getInt32(0)}, "accessTmp");
        return _builder.CreateBitCast(ptr, type->getPointerTo(), "cnvrttmp");
    }
    throw "unreachable";
}

Value *CodeGenerator::codeGenLval(DST::UnaryOperation* node)
{
    Value *val = codeGenLval(node->getExpression());
    switch (node->getOperator()._type)
    {
        case OT_AT:
            assertNotNull(val);
            return _builder.CreateLoad(val);
        case OT_BITWISE_AND:
            return _builder.CreateGEP(val, _builder.getInt32(0));
        default:
            throw ErrorReporter::report("Unimplemented lval unary operation", ERR_CODEGEN, node->getPosition());
    }    
}

Value *CodeGenerator::codeGenLval(DST::BinaryOperation *node)
{
    Value *left = codeGenLval(node->getLeft());
    switch (node->getOperator()._type)
    {
        case OT_SQUARE_BRACKETS_OPEN:
            if (node->getLeft()->getType()->isArrayTy())
            {
                if (node->getLeft()->getType()->as<DST::ArrayType>()->getLength() == DST::UNKNOWN_ARRAY_LENGTH)
                {
                    auto arrPtr = _builder.CreateInBoundsGEP(left, { _builder.getInt32(0), _builder.getInt32(1) } );
                    return _builder.CreateGEP(_builder.CreateLoad(arrPtr), codeGen(node->getRight()) );
                }
                else // TODO - array literal access
                    return _builder.CreateInBoundsGEP(left, { _builder.getInt32(0), codeGen(node->getRight()) } ); 
            }
            if (node->getLeft()->getType()->isListTy())
            {
                int idx = ((DST::Literal*)node->getRight())->getIntValue();
                return _builder.CreateInBoundsGEP(left, { _builder.getInt32(0), _builder.getInt32(idx) });
            }
            else throw "unreachable";
        default:
            throw ErrorReporter::report("Unimplemented lval Binary operation", ERR_CODEGEN, node->getPosition());
    }
}

Value *CodeGenerator::codeGenLval(DST::Variable *node)
{
    if (auto var = _namedValues.top()[node->getVarId().to_string()])
        return var;
    else for (int i = _currentNamespace.size() - 1; i >= 0; i--)
        if (auto var = _currentNamespace[i]->values[node->getVarId().to_string()])
            return var;
    throw "could not find variable";
}

Value *CodeGenerator::codeGenLval(DST::MemberAccess *node)
{
    auto leftType = node->getLeft()->getType();

    if (leftType->isNamespaceTy())
    {
        auto members = getNamespaceMembers(node->getLeft());
        if (!members)
            throw "TODO - Error message";
        return members->values[node->getRight()];
    }
    else if (leftType->isBasicTy())
    {
        if (auto interfaceDecl = leftType->as<DST::BasicType>()->getTypeSpecifier()->getInterfaceDecl())
        {
            auto lval = codeGenLval(node->getLeft());
            auto vtablePtr = _builder.CreateInBoundsGEP(lval, { _builder.getInt32(0), _builder.getInt32(1) });
            auto vtable = _builder.CreateLoad(vtablePtr);
            auto funcPtr = getFuncFromVtable(vtable, interfaceDecl, node->getRight());
            node->getRight() += ".get";
            auto funcTy = _interfaceVtableFuncInfo[interfaceDecl][node->getRight()].type;
            auto thisPtr = _builder.CreateLoad(_builder.CreateInBoundsGEP(lval, { _builder.getInt32(0), _builder.getInt32(0) }));
            auto func = _builder.CreateBitCast(funcPtr, funcTy->getPointerTo());
            auto val = createCallOrInvoke(func, thisPtr);
            auto alloca = _builder.CreateAlloca(funcTy->getReturnType());
            _builder.CreateStore(val, alloca);
            return alloca;
        }

        auto bt = (DST::BasicType*)leftType;
        auto typeDef = _types[bt->getTypeSpecifier()->getTypeDecl()];
        auto lval = codeGenLval(node->getLeft());
        assertNotNull(lval);
        return _builder.CreateInBoundsGEP(
            typeDef->structType, 
            lval, 
            { _builder.getInt32(0), _builder.getInt32(typeDef->variableIndexes[node->getRight()]) }, 
            node->getRight().to_string()
        );
    }
    else if (leftType->isPtrTy() && 
            leftType->as<DST::PointerType>()->getPtrType()->isBasicTy())
    {
        auto bt = leftType->as<DST::PointerType>()->getPtrType()->as<DST::BasicType>();
        auto typeDef = _types[bt->getTypeSpecifier()->getTypeDecl()];
        auto lval = _builder.CreateLoad(codeGenLval(node->getLeft()));
        assertNotNull(lval);
        return _builder.CreateInBoundsGEP(
            typeDef->structType, 
            lval, 
            { _builder.getInt32(0), _builder.getInt32(typeDef->variableIndexes[node->getRight()]) }, 
            node->getRight().to_string()
        );
    }
    else throw ErrorReporter::report("Expression must be of class or namespace type", ERR_CODEGEN, node->getPosition());
}