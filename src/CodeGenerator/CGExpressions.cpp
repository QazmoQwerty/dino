/*
    Code generation for expression.
*/
#include "CodeGenerator.h"

Value *CodeGenerator::codeGen(DST::Expression *node) 
{
    if (node == nullptr)
        return nullptr;
    switch (node->getExpressionType()) 
    {
        case ET_BINARY_OPERATION: return codeGen((DST::BinaryOperation*)node);
        case ET_UNARY_OPERATION: return codeGen((DST::UnaryOperation*)node);
        case ET_LITERAL: return codeGen((DST::Literal*)node);
        case ET_ASSIGNMENT: return codeGen((DST::Assignment*)node);
        case ET_IDENTIFIER: return codeGen((DST::Variable*)node);
        case ET_VARIABLE_DECLARATION: return codeGen((DST::VariableDeclaration*)node);
        case ET_FUNCTION_CALL: return codeGen((DST::FunctionCall*)node);
        case ET_MEMBER_ACCESS: return codeGen((DST::MemberAccess*)node);
        case ET_ARRAY: return codeGen((DST::ArrayLiteral*)node);
        case ET_CONVERSION: return codeGen((DST::Conversion*)node);
        case ET_INCREMENT: return codeGen((DST::Increment*)node);
        case ET_CONDITIONAL_EXPRESSION: return codeGen((DST::ConditionalExpression*)node);
        case ET_FUNCTION_LITERAL: return codeGen((DST::FunctionLiteral*)node);
        case ET_COMPARISON: return codeGen((DST::Comparison*)node);
        case ET_LIST: return codeGen((DST::ExpressionList*)node);
        default: UNREACHABLE
    }
}

Value *CodeGenerator::codeGen(DST::Literal *node) 
{
    switch (node->getLiteralType())
    {
    case LT_BOOLEAN:
        return llvm::ConstantInt::get(_context, llvm::APInt( /* 1 bit width */ 1, ((AST::Boolean*)node->getBase())->getValue()));
    case LT_CHARACTER:
        return llvm::ConstantInt::get(_context, llvm::APInt( /* 8 bit width */ 8, (char)((AST::Character*)node->getBase())->getValue().getValue()));
    case LT_INTEGER:
        return llvm::ConstantInt::get(_context, llvm::APInt( /* 32 bit width */ 32, ((AST::Integer*)node->getBase())->getValue(), /* signed */ true));
    case LT_FRACTION:
        return llvm::ConstantFP::get(_context, llvm::APFloat(((AST::Fraction*)node->getBase())->getValue()));
    case LT_NULL:
        return llvm::Constant::getNullValue(_builder.getInt8Ty()->getPointerTo());  // 'void*' is invalid in llvm IR
    case LT_ENUM:
        return llvm::ConstantInt::get(evalType(node->getType()), node->getIntValue(), node->getType()->isSigned());
    case LT_STRING:
    {
        return llvm::ConstantStruct::get(_stringTy, llvm::ConstantStruct::get(llvm::cast<llvm::StructType>(_stringTy->getElementType(0)), {
            _builder.getInt32(((AST::String*)node->getBase())->getValue().size()),
            (llvm::Constant*)_builder.CreateBitCast(_builder.CreateGlobalString(((AST::String*)node->getBase())->getValue(), ".stringLit"), _builder.getInt8PtrTy())
        }));
    }
    default: UNREACHABLE
    }
}

llvm::Value *CodeGenerator::codeGen(DST::ExpressionList *node)
{
    auto ty = evalType(node->getType());
    llvm::Value* ret = llvm::UndefValue::get(ty);
    int idx = 0;
    for (auto i : node->getExpressions())
        ret = _builder.CreateInsertValue(ret, codeGen(i), idx++);
    return ret;
}

llvm::Function *CodeGenerator::codeGen(DST::FunctionLiteral *node) 
{
    vector<llvm::Type*> types;
    auto returnType = evalType(node->getType()->getReturn());
    auto params = node->getParameters();
    for (auto i : params) 
        types.push_back(evalType(i->getType()));
    auto funcType = llvm::FunctionType::get(returnType, types, false);

    llvm::Function *func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, ".anonFunc", _module.get());

    // Set names for all arguments.
    unsigned idx = 0;
    for (auto &arg : func->args())
        arg.setName(params[idx++]->getVarId().to_string());

    if (node->getContent() == NULL)
        throw ErrorReporter::report("function literal with no body", ErrorReporter::GENERAL_ERROR, node->getPosition());

    // Create a new basic block to start insertion into.
    llvm::BasicBlock *bb = llvm::BasicBlock::Create(_context, "entry", func);
    auto savedInsertPoint = _builder.GetInsertBlock();
    _builder.SetInsertPoint(bb);

    // Record the function arguments in the NamedValues map.
    _namedValues.push({});
    bool isFirst = true;
    
    for (llvm::Argument &arg : func->args())
    {
        AllocaInst *alloca = CreateEntryBlockAlloca(func, arg.getType(), arg.getName());    // Create an alloca for this variable.
        _builder.CreateStore(&arg, alloca);     // Store the initial value into the alloca.
        _namedValues.top()[arg.getName()] = alloca;   // Add arguments to variable symbol table. 
        isFirst = false;
    }

    for (auto i : node->getContent()->getStatements()) 
    {
        auto val = codeGen(i);
        if (val == nullptr)
            throw ErrorReporter::report("Error while generating IR for statement", ErrorReporter::GENERAL_ERROR, i->getPosition());
    }

    if (!_builder.GetInsertBlock()->getTerminator())
        _builder.CreateRetVoid();

    llvm::verifyFunction(*func, &llvm::errs());
    _namedValues.pop(); // leave block
    _builder.SetInsertPoint(savedInsertPoint);
    return func;
}

Value *CodeGenerator::createIsOperation(DST::BinaryOperation *node)
{
    if (!node->getLeft()->getType()->isInterfaceTy()) // known at compile time
        return _builder.getInt1(node->getLeft()->getType() == node->getRight());
    
    auto right = evalType((DST::Type*)node->getRight());
    auto left = codeGenLval(node->getLeft());
    if (right == _interfaceType)
    {
        if (left->getType() != _interfaceType)
            UNREACHABLE

        auto vtablePtr = _builder.CreateInBoundsGEP(left, { _builder.getInt32(0), _builder.getInt32(1) });
        auto vtableLoad = _builder.CreateLoad(vtablePtr);
        auto interface = ((DST::Type*)node->getRight())->as<DST::InterfaceType>()->getInterfaceDecl();
        auto interfaceVtable = _builder.CreateCall(getVtableInterfaceLookupFunction(), { vtableLoad, _builder.getInt32((unsigned long)interface) }); 
        auto diff = _builder.CreatePtrDiff(interfaceVtable, llvm::ConstantPointerNull::get(_interfaceVtableType->getPointerTo()));
        return _builder.CreateICmpEQ(diff, _builder.getInt64(0), "isTmp");
    }
    else if (((DST::Type*)node->getRight())->isBasicTy())
    {
        if (left->getType()->getPointerElementType() != _interfaceType)
            UNREACHABLE
        
        auto vtablePtr = _builder.CreateInBoundsGEP(left, { _builder.getInt32(0), _builder.getInt32(1) });
        auto vtableLoad = _builder.CreateLoad(vtablePtr);
        auto vtable = getVtable(((DST::Type*)node->getRight()));    // ptrTo since all types stored in interfaces are pointers currently
        auto diff = _builder.CreatePtrDiff(vtableLoad, vtable);
        return _builder.CreateICmpEQ(diff, _builder.getInt64(0), "isTmp");
    }
    else throw ErrorReporter::report("The \"is\" operator is currently only implemented for basic types!", ErrorReporter::GENERAL_ERROR, node->getPosition());
}

Value *CodeGenerator::createLogicalOr(DST::BinaryOperation *node)
{
    auto parent = getParentFunction();
    auto left = codeGen(node->getLeft());
    llvm::BasicBlock *isTrue = llvm::BasicBlock::Create(_context, "isTrue", parent);
    llvm::BasicBlock *isFalse = llvm::BasicBlock::Create(_context, "isFalse", parent);
    llvm::BasicBlock *merge = llvm::BasicBlock::Create(_context, "merge", parent);
    _builder.CreateCondBr(left, isTrue, isFalse);

    _builder.SetInsertPoint(isTrue);
    _builder.CreateBr(merge);

    _builder.SetInsertPoint(isFalse);
    auto right = codeGen(node->getRight());
    _builder.CreateBr(merge);

    _builder.SetInsertPoint(merge);
    auto phi = _builder.CreatePHI(_builder.getInt1Ty(), 2);
    phi->addIncoming(_builder.getInt1(true), isTrue);
    phi->addIncoming(right, isFalse);
    return phi;
}

Value *CodeGenerator::createLogicalAnd(DST::BinaryOperation *node)
{
    auto parent = getParentFunction();
    auto left = codeGen(node->getLeft());
    llvm::BasicBlock *isTrue = llvm::BasicBlock::Create(_context, "isTrue", parent);
    llvm::BasicBlock *isFalse = llvm::BasicBlock::Create(_context, "isFalse", parent);
    llvm::BasicBlock *merge = llvm::BasicBlock::Create(_context, "merge", parent);
    _builder.CreateCondBr(left, isTrue, isFalse);

    _builder.SetInsertPoint(isFalse);
    _builder.CreateBr(merge);

    _builder.SetInsertPoint(isTrue);
    auto right = codeGen(node->getRight());
    _builder.CreateBr(merge);

    _builder.SetInsertPoint(merge);
    auto phi = _builder.CreatePHI(_builder.getInt1Ty(), 2);
    phi->addIncoming(_builder.getInt1(false), isFalse);
    phi->addIncoming(right, isTrue);
    return phi;
}

llvm::Value *CodeGenerator::createCompare(Value *left, Value *right, OperatorType op, DST::Type *leftTy, DST::Type *rightTy, ErrorReporter::Position pos)
{
    if (op == OT_EQUAL || op == OT_NOT_EQUAL)
    {
        if (leftTy->isNullTy())
            left = _builder.CreateBitCast(left, right->getType());
        if (rightTy->isNullTy())
            right = _builder.CreateBitCast(right, left->getType());

        if (left->getType() == _interfaceType)
            left = _builder.CreateExtractValue(left, 0);
        if (right->getType() == _interfaceType)
            right = _builder.CreateExtractValue(right, 0);
            
        if (left->getType() != right->getType())
            left = _builder.CreateBitCast(left, right->getType());

        if (op == OT_EQUAL)
            return _builder.CreateICmpEQ(left, right, "cmptmp");
        else return _builder.CreateICmpNE(left, right, "cmptmp");
    }
    if (leftTy->isFloatTy())
        switch (op) // TODO - shoud these be "ordered" or "unordered" comparisons?
        {
            case OT_SMALLER: return _builder.CreateFCmpOLT(left, right, "cmptmp");
            case OT_SMALLER_EQUAL: return _builder.CreateFCmpOLE(left, right, "cmptmp");
            case OT_GREATER: return _builder.CreateFCmpOGT(left, right, "cmptmp");
            case OT_GREATER_EQUAL: return _builder.CreateFCmpOGE(left, right, "cmptmp");
            default: UNREACHABLE
        }
    if (leftTy->isSigned())
        switch (op)
        {
            case OT_SMALLER: return _builder.CreateICmpSLT(left, right, "cmptmp");
            case OT_SMALLER_EQUAL: return _builder.CreateICmpSLE(left, right, "cmptmp");
            case OT_GREATER: return _builder.CreateICmpSGT(left, right, "cmptmp");
            case OT_GREATER_EQUAL: return _builder.CreateICmpSGE(left, right, "cmptmp");
            default: UNREACHABLE
        }
    switch (op)
    {
        case OT_SMALLER: return _builder.CreateICmpULT(left, right, "cmptmp");
        case OT_SMALLER_EQUAL: return _builder.CreateICmpULE(left, right, "cmptmp");
        case OT_GREATER: return _builder.CreateICmpUGT(left, right, "cmptmp");
        case OT_GREATER_EQUAL: return _builder.CreateICmpUGE(left, right, "cmptmp");
        default: UNREACHABLE
    }
}

Value *CodeGenerator::codeGen(DST::Comparison* node)
{
    Value *ret = NULL;
    Value *left = codeGen(node->getExpressions()[0]);
    for (uint i = 0; i < node->getOperators().size(); i++)
    {
        auto right = codeGen(node->getExpressions()[i + 1]);
        Value *newComp = createCompare(
            left, right, 
            node->getOperators()[i]._type, 
            node->getExpressions()[i]->getType(), 
            node->getExpressions()[i+1]->getType(), 
            node->getPosition()
        );
        if (ret) 
            ret = _builder.CreateAnd(ret, newComp);
        else ret = newComp;
        left = right;
    }
    return ret;
}

Value *CodeGenerator::codeGen(DST::BinaryOperation* node)
{
    /* special cases that might not eval both sides of the operation */
    switch (node->getOperator()._type)
    {
        case OT_IS:
            return createIsOperation(node);
        case OT_IS_NOT:
            return _builder.CreateNot(createIsOperation(node));
        case OT_SQUARE_BRACKETS_OPEN:
            if (node->getLeft()->getType()->isConstTy() || node->getLeft()->getType()->isPropertyTy())
            {
                auto arrVal = codeGen(node->getLeft());
                auto idx = codeGen(node->getRight());
                auto alloca = _builder.CreateAlloca(arrVal->getType());
                _builder.CreateStore(arrVal, alloca);
                return _builder.CreateLoad(_builder.CreateGEP(alloca, {_builder.getInt32(0), idx}));
            }
            return _builder.CreateLoad(codeGenLval(node));
        case OT_LOGICAL_OR:
            return createLogicalOr(node);   
        case OT_LOGICAL_AND:
            return createLogicalAnd(node);   
        default: break;
    }

    auto left = codeGen(node->getLeft());
    auto right = codeGen(node->getRight());

    switch (node->getOperator()._type)
    {
        case OT_ADD:
            if (node->getType()->isFloatTy())
                return _builder.CreateFAdd(left, right, "addtmp");    
            return _builder.CreateAdd(left, right, "addtmp");
        case OT_SUBTRACT:
            if (node->getType()->isFloatTy())
                return _builder.CreateFSub(left, right, "subtmp");
            return _builder.CreateSub(left, right, "subtmp");
        case OT_MULTIPLY:
            if (node->getType()->isFloatTy())
                return _builder.CreateFMul(left, right, "multmp");
            return _builder.CreateMul(left, right, "multmp");
        case OT_DIVIDE:
            if (node->getType()->isFloatTy())
                return _builder.CreateFDiv(left, right, "divtmp");    
            if (node->getType()->isSigned())
                return _builder.CreateSDiv(left, right, "divtmp");
            return _builder.CreateUDiv(left, right, "divtmp");
        case OT_MODULUS:
            if (node->getType()->isFloatTy())
                return _builder.CreateFRem(left, right, "remtmp");    
            if (node->getType()->isSigned())
                return _builder.CreateSRem(left, right, "dremtmp");
            return _builder.CreateURem(left, right, "remtmp");
        case OT_BITWISE_AND:
            return _builder.CreateAnd(left, right, "bAndtmp");
        case OT_BITWISE_OR:
            return _builder.CreateOr(left, right, "bOrtmp");
        case OT_BITWISE_XOR:
            return _builder.CreateOr(left, right, "bXortmp");
        default: TODO
    }
}

Value *CodeGenerator::codeGen(DST::ConditionalExpression *node)
{
    auto parent = getParentFunction();
    llvm::BasicBlock *isTrue = llvm::BasicBlock::Create(_context, "then", parent);
    llvm::BasicBlock *isFalse = llvm::BasicBlock::Create(_context, "then", parent);
    llvm::BasicBlock *merge = llvm::BasicBlock::Create(_context, "then", parent);
    _builder.CreateCondBr(codeGen(node->getCondition()), isTrue, isFalse);

    _builder.SetInsertPoint(isTrue);
    auto thenBranch = codeGen(node->getThenBranch());
    _builder.CreateBr(merge);

    _builder.SetInsertPoint(isFalse);
    auto elseBranch = codeGen(node->getElseBranch());
    _builder.CreateBr(merge);

    _builder.SetInsertPoint(merge);
    auto phi = _builder.CreatePHI(evalType(node->getType()), 2);
    phi->addIncoming(thenBranch, isTrue);
    phi->addIncoming(elseBranch, isFalse);
    return phi;
}

Value *CodeGenerator::codeGen(DST::UnaryOperation* node)
{
    switch (node->getOperator()._type)
    {
        case OT_NEW:
        {
            auto ty = ((DST::Type*)node->getExpression());
            if (ty->isArrayTy() && ty->as<DST::ArrayType>()->getLenExp())
            {
                auto type = evalType(ty);
                int size = _dataLayout->getTypeAllocSize(evalType(ty->as<DST::ArrayType>()->getElementType()));
                auto len = codeGen(ty->as<DST::ArrayType>()->getLenExp());
                auto allocSize = _builder.CreateMul(_builder.getInt32(size), len, "allocSize");
                return _builder.CreateBitCast( createCallOrInvoke(getMallocFunc(), { allocSize }), type, "newTmp");
            }
            auto type = evalType((DST::Type*)node->getExpression());
            if (type->isVoidTy())
                throw ErrorReporter::report("Cannot create instance of 'void'", ErrorReporter::GENERAL_ERROR, node->getPosition());
            int size = _dataLayout->getTypeAllocSize(type);
            return _builder.CreateBitCast( createCallOrInvoke(getMallocFunc(), { _builder.getInt32(size) }), type->getPointerTo(), "newTmp");
        }
        case OT_ADD:
            return codeGen(node->getExpression());
        case OT_SUBTRACT:
        {
            if (node->getType()->isFloatTy())
            {
                auto val = codeGen(node->getExpression());
                auto zero = llvm::ConstantFP::get(val->getType(),  0);
                return _builder.CreateSub(zero, val);    
            }
            auto val = codeGen(node->getExpression());
            auto zero = llvm::ConstantInt::get(val->getType(),  0);
            return _builder.CreateSub(zero, val);
        }
        case OT_AT:
        {
            auto val = codeGen(node->getExpression());
            assertNotNull(val);
            return _builder.CreateLoad(val);
        }
        case OT_BITWISE_AND:
            return _builder.CreateGEP(codeGenLval(node->getExpression()), _builder.getInt32(0));
        case OT_LOGICAL_NOT: case OT_BITWISE_NOT:
            return _builder.CreateNot(codeGen(node->getExpression()), "nottmp");
        default: UNREACHABLE
    }
    
}

Value *CodeGenerator::codeGen(DST::Conversion* node)
{
    auto type = evalType(node->getType());
    auto exp = codeGen(node->getExpression());
    if (type == exp->getType())
        return exp;
    if (type == _interfaceType)
        return convertToInterface(exp, node->getExpression()->getType());
    if (type != _interfaceType && exp->getType() == _interfaceType) // Interface to non-interface
    {
        // TODO - throw exception incase of invalid conversion
        auto ptr = _builder.CreateExtractValue(exp, 0, "accessTmp");
        return _builder.CreateBitCast(ptr, type, "cnvrttmp");
    }
    if (node->getExpression()->getType()->isFloatTy() && node->getType()->isIntTy())
    {
        // float to int
        if (node->getType()->isSigned())
            return _builder.CreateFPToSI(exp, type);
        return _builder.CreateFPToUI(exp, type);
    }

    if (node->getExpression()->getType()->isFloatTy() && node->getType()->isIntTy())
    {   
        // int to float
        if (node->getType()->isSigned())
            return _builder.CreateSIToFP(exp, type);
        return _builder.CreateUIToFP(exp, type);
    }
    uint targetSz = type->getPrimitiveSizeInBits();
    uint currSz = exp->getType()->getPrimitiveSizeInBits();
    if (targetSz < currSz)
        return _builder.CreateTrunc(exp, type, "cnvrttmp");
    else if (targetSz > currSz) 
    {
        if (node->getType()->isSigned())
            return _builder.CreateSExt(exp, type, "cnvrttmp");    
        return _builder.CreateZExt(exp, type, "cnvrttmp");
    }
    else return _builder.CreateBitCast(exp, type, "cnvrttmp");
}

Value *CodeGenerator::codeGen(DST::MemberAccess *node)
{
    if (node->getType()->isPropertyTy())
    {
        node->getRight() += ".get";
        auto leftTy = node->getLeft()->getType();
        
        switch (leftTy->getNonPropertyOf()->getNonConstOf()->getExactType())
        {
            case EXACT_NAMESPACE:   // Static getter property
            {
                auto func = codeGenLval(node);
                if (!isFunc(func) || ((llvm::Function*)func)->arg_size() != 0)
                    throw ErrorReporter::report("expression is not a getter property", ErrorReporter::GENERAL_ERROR, node->getPosition());

                return createCallOrInvoke((llvm::Function*)func, {});
            }
            case EXACT_BASIC:       // Member getter property of basic type
            {
                auto lval = codeGenLval(node->getLeft());

                if (leftTy->isInterfaceTy())
                {
                    auto interfaceDecl = leftTy->as<DST::InterfaceType>()->getInterfaceDecl();
                    auto vtablePtr = _builder.CreateInBoundsGEP(lval, { _builder.getInt32(0), _builder.getInt32(1) });
                    auto vtable = _builder.CreateLoad(vtablePtr);
                    auto funcPtr = getFuncFromVtable(vtable, interfaceDecl, node->getRight());
                    auto funcTy = _interfaceVtableFuncInfo[interfaceDecl][node->getRight()].type;
                    auto thisPtr = _builder.CreateLoad(_builder.CreateInBoundsGEP(lval, { _builder.getInt32(0), _builder.getInt32(0) }));
                    auto func = _builder.CreateBitCast(funcPtr, funcTy->getPointerTo());
                    return createCallOrInvoke(func, thisPtr);
                }

                auto typeDef = _types[leftTy->as<DST::ValueType>()->getTypeDecl()];
                auto func = typeDef->functions[node->getRight()];
                if (func->arg_size() != 1)
                    throw ErrorReporter::report("expression is not a getter property", ErrorReporter::GENERAL_ERROR, node->getPosition());
                return createCallOrInvoke(func, lval);
            }
            case EXACT_POINTER:     // Member getter property of pointer to basic type
            {
                auto ptrTy = leftTy->as<DST::PointerType>()->getPtrType();
                if (!ptrTy->isBasicTy())
                {
                    llvm::errs() << ptrTy->toShortString() << "\n";
                    throw ErrorReporter::reportInternal("cannot call getter of ptr to ptr", ErrorReporter::GENERAL_ERROR, node->getPosition());
                }
                auto lval = codeGen(node->getLeft());
                auto typeDef = _types[ptrTy->as<DST::ValueType>()->getTypeDecl()];
                auto func = typeDef->functions[node->getRight()];
                if (!isFunc(func) || ((llvm::Function*)func)->arg_size() != 1)
                    throw ErrorReporter::report("expression is not a getter property", ErrorReporter::GENERAL_ERROR, node->getPosition());
                return createCallOrInvoke((llvm::Function*)func, lval);
            }
            case EXACT_ARRAY:       // Member property of array
                if (node->getRight() == unicode_string("Size.get")) {
                    if (leftTy->as<DST::ArrayType>()->getLength() == DST::UNKNOWN_ARRAY_LENGTH)
                        return _builder.CreateLoad(_builder.CreateInBoundsGEP(
                            codeGenLval(node->getLeft()),
                            { _builder.getInt32(0), _builder.getInt32(0) },
                            "sizePtrTmp")
                        );
                    return _builder.getInt32(codeGen(node->getLeft())->getType()->getArrayNumElements());
                }
                throw ErrorReporter::reportInternal("Arrays only have the Size property rn", ErrorReporter::GENERAL_ERROR, node->getPosition());
            default:
                throw ErrorReporter::report("only arrays, basic types and ptr types can be called upon rn", ErrorReporter::GENERAL_ERROR, node->getPosition());
        }
    }
    if (node->getType()->isConstTy())
        return codeGenLval(node);
    auto val = codeGenLval(node);
    assertNotNull(val);
    return _builder.CreateLoad(val, "accesstmp");
}

Value *CodeGenerator::codeGen(DST::ArrayLiteral *node)
{
    vector<llvm::Constant*> IRvalues;
    llvm::Type *atype = evalType(node->getType());

    for (auto val : node->getArray())
    {
        IRvalues.push_back((llvm::Constant*)codeGen(val));
    }
    return llvm::ConstantArray::get((llvm::ArrayType*)atype, IRvalues);
}

Value *CodeGenerator::codeGen(DST::Variable *node)
{
    if (node->getType()->isPropertyTy())
    {
        if (node->getVarId().to_string().find('.') == node->getVarId().to_string().npos)
            node->getVarId() += ".get";
        auto lval = codeGenLval(node);
        if (!isFunc(lval))
            throw ErrorReporter::report("expression is not a getter property", ErrorReporter::GENERAL_ERROR, node->getPosition());
        auto func = (llvm::Function*)lval;
        if (func->arg_size() != 0)
            throw ErrorReporter::report("expression is not a getter property", ErrorReporter::GENERAL_ERROR, node->getPosition());
        return createCallOrInvoke(func, {});
    }
    auto t = codeGenLval(node);
    if (node->getType()->isConstTy())
        return t;
    if (t->getType()->getPointerElementType()->isFunctionTy())
        return t;
    auto str = node->getVarId().to_string().c_str();
    return _builder.CreateLoad(t, str);
}