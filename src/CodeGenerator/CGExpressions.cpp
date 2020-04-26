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
        default: throw ErrorReporter::report("Unimplemented codegen for expression", ERR_CODEGEN, node->getPosition());
    }
}

Value *CodeGenerator::codeGen(DST::Literal *node) 
{
    switch (node->getLiteralType())
    {
    case LT_BOOLEAN:
        return llvm::ConstantInt::get(_context, llvm::APInt( /* 1 bit width */ 1, ((AST::Boolean*)node->getBase())->getValue()));
    case LT_CHARACTER:
        return llvm::ConstantInt::get(_context, llvm::APInt( /* 8 bit width */ 8, (char)((AST::Character*)node->getBase())->getValue().getValue()));   // TODO - unicode characters
    case LT_INTEGER:
        return llvm::ConstantInt::get(_context, llvm::APInt( /* 32 bit width */ 32, ((AST::Integer*)node->getBase())->getValue(), /* signed */ true));
    case LT_FRACTION:
        return llvm::ConstantFP::get(_context, llvm::APFloat(((AST::Fraction*)node->getBase())->getValue()));
    case LT_NULL:
        return llvm::Constant::getNullValue(_builder.getInt8Ty()->getPointerTo());  // 'void*' is invalid in llvm IR
    case LT_STRING:
    {
        return llvm::ConstantStruct::get(_stringTy, llvm::ConstantStruct::get(llvm::cast<llvm::StructType>(_stringTy->getElementType(0)), {
            _builder.getInt32(((AST::String*)node->getBase())->getValue().size()),
            (llvm::Constant*)_builder.CreateBitCast(_builder.CreateGlobalString(((AST::String*)node->getBase())->getValue(), ".stringLit"), _builder.getInt8PtrTy())
        }));
    }
    default:
        throw ErrorReporter::report("Unimplemented literal type", ERR_CODEGEN, node->getPosition());
    }
}

llvm::Function *CodeGenerator::codeGen(DST::FunctionLiteral *node) 
{
    llvm::errs() << "1\n";
    vector<llvm::Type*> types;
    auto params = node->getParameters();
llvm::errs() << "1\n";
    llvm::Type *returnType = NULL; 
llvm::errs() << "1\n";
    // functions that return multiple values return them based on pointers they get as arguments
    bool isMultiReturnFunc = 1 < node->getType()->getReturns()->size();
    if (isMultiReturnFunc)
    {
        for (auto i : node->getType()->getReturns()->getTypes())
            types.push_back(evalType(i)->getPointerTo());
        returnType = _builder.getVoidTy();
    }
    else returnType = evalType(node->getType()->getReturns());
    llvm::errs() << "1\n";
    for (auto i : params) 
        types.push_back(evalType(i->getType()));
    
    auto funcType = llvm::FunctionType::get(returnType, types, false);

    llvm::Function *func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, ".anonFunc", _module.get());
llvm::errs() << "1\n";
    // Set names for all arguments.
    unsigned idx = 0;
    unsigned idx2 = 0;
    for (auto &arg : func->args())
    {
        llvm::errs() << std::to_string(idx);
        if (isMultiReturnFunc && idx < node->getType()->getReturns()->size())
            arg.setName(".ret" + std::to_string(idx++));
        else arg.setName(params[idx2++]->getVarId().to_string());
    }
llvm::errs() << "1\n";

    if (node->getContent() == NULL)
        throw ErrorReporter::report("function literal with no body", ERR_CODEGEN, node->getPosition());

    // Create a new basic block to start insertion into.
    llvm::BasicBlock *bb = llvm::BasicBlock::Create(_context, "entry", func);
    auto savedInsertPoint = _builder.GetInsertBlock();
    _builder.SetInsertPoint(bb);
llvm::errs() << "1\n";
    // Record the function arguments in the NamedValues map.
    _namedValues.push({});
    bool isFirst = true;
    idx = 0;
    if (isMultiReturnFunc)  // breaks this too :)
        _funcReturns.clear();
    llvm::errs() << "1\n";
    for (llvm::Argument &arg : func->args())
    {
        if (isMultiReturnFunc && idx < ((DST::TypeList*)node->getType()->getReturns())->size())
        {
            _funcReturns.push_back(&arg);
            idx++;
        }
        else 
        {
            AllocaInst *alloca = CreateEntryBlockAlloca(func, arg.getType(), arg.getName());    // Create an alloca for this variable.
            _builder.CreateStore(&arg, alloca);     // Store the initial value into the alloca.
            _namedValues.top()[arg.getName()] = alloca;   // Add arguments to variable symbol table.
        }    
        isFirst = false;
    }
llvm::errs() << "1\n";
    for (auto i : node->getContent()->getStatements()) 
    {
        auto val = codeGen(i);
        if (val == nullptr)
            throw ErrorReporter::report("Error while generating IR for statement", ERR_CODEGEN, i->getPosition());
    }
llvm::errs() << "1\n";
    if (!_builder.GetInsertBlock()->getTerminator())
        _builder.CreateRetVoid();
llvm::errs() << "1\n";
    llvm::verifyFunction(*func, &llvm::errs());
    _namedValues.pop(); // leave block
    _builder.SetInsertPoint(savedInsertPoint);
    return func;
}

Value *CodeGenerator::codeGen(DST::BinaryOperation* node)
{
    Value *left, *right;
    
    if (node->getOperator()._type == OT_IS)
    {
        auto right = evalType((DST::Type*)node->getRight());
        auto left = codeGenLval(node->getLeft());
        if (right == _interfaceType)
        {
            if (left->getType() != _interfaceType)
            {
                // should be known at compile-time
                throw "TODO";
            }

            auto vtablePtr = _builder.CreateInBoundsGEP(left, { _builder.getInt32(0), _builder.getInt32(1) });
            auto vtableLoad = _builder.CreateLoad(vtablePtr);
            auto interface = ((DST::BasicType*)node->getRight())->getTypeSpecifier()->getInterfaceDecl();
            auto interfaceVtable = _builder.CreateCall(getVtableInterfaceLookupFunction(), { vtableLoad, _builder.getInt32((unsigned long)interface) }); 
            auto diff = _builder.CreatePtrDiff(interfaceVtable, llvm::ConstantPointerNull::get(_interfaceVtableType->getPointerTo()));
            return _builder.CreateICmpEQ(diff, _builder.getInt64(0), "isTmp");
        }
        else if (((DST::Type*)node->getRight())->getExactType() == EXACT_BASIC)
        {
            if (left->getType()->getPointerElementType() != _interfaceType)
                return _builder.getInt1(false);
            auto vtablePtr = _builder.CreateInBoundsGEP(left, { _builder.getInt32(0), _builder.getInt32(1) });
            auto vtableLoad = _builder.CreateLoad(vtablePtr);
            // auto decl = ((DST::BasicType*)node->getRight())->getTypeSpecifier()->getTypeDecl();
            // llvm::Value *vtable = NULL;
            // if (auto def = _types[decl])
            //     vtable = def->vtable;
            // else vtable = createEmptyVtable(evalType(((DST::Type*)node->getRight())));
            auto vtable = getVtable(evalType(((DST::Type*)node->getRight())));
            auto diff = _builder.CreatePtrDiff(vtableLoad, vtable);
            return _builder.CreateICmpEQ(diff, _builder.getInt64(0), "isTmp");
        }
        else throw ErrorReporter::report("The \"is\" operator is currently only implemented for basic types!", ERR_CODEGEN, node->getPosition());

        // auto interfaceVtable = _builder.CreateCall(_vtableInterfaceLookupFunc, { vtable, _builder.getInt32((unsigned long)interface) });

        // funcPtr = getFuncFromVtable(vtable, interfaceDecl, funcId);
        // funcTy = _interfaceVtableFuncInfo[interfaceDecl][funcId].type;
        // thisPtr = _builder.CreateLoad(_builder.CreateInBoundsGEP(thisPtr, { _builder.getInt32(0), _builder.getInt32(0) }));
        return NULL;
    }

    if (node->getOperator()._type != OT_SQUARE_BRACKETS_OPEN)
    {
        left = codeGen(node->getLeft());
        right = codeGen(node->getRight());
    }
    
    switch (node->getOperator()._type)
    {
        case OT_ADD:
            return _builder.CreateAdd(left, right, "addtmp");
        case OT_SUBTRACT:
            return _builder.CreateSub(left, right, "subtmp");
        case OT_MULTIPLY:
            return _builder.CreateMul(left, right, "multmp");
        case OT_DIVIDE:
            return _builder.CreateSDiv(left, right, "divtmp");
        case OT_MODULUS:
            return _builder.CreateSRem(left, right, "divtmp");
        case OT_SMALLER:
            return _builder.CreateICmpSLT(left, right, "cmptmp");
        case OT_SMALLER_EQUAL:
            return _builder.CreateICmpSLE(left, right, "cmptmp");
        case OT_GREATER:
            return _builder.CreateICmpSGT(left, right, "cmptmp");
        case OT_GREATER_EQUAL:
            return _builder.CreateICmpSGE(left, right, "cmptmp");
        case OT_EQUAL:
        {
            if (left->getType() == _interfaceType)
                left = _builder.CreateExtractValue(left, 0);

            if (right->getType() == _interfaceType)
                right = _builder.CreateExtractValue(right, 0);

            if (left->getType() != right->getType())
                return _builder.CreateICmpEQ(_builder.CreateBitCast(left, right->getType()), right, "cmptmp");
            return _builder.CreateICmpEQ(left, right, "cmptmp");
        }
        case OT_NOT_EQUAL:
        {
            if (left->getType() == _interfaceType)
                left = _builder.CreateExtractValue(left, 0);

            if (right->getType() == _interfaceType)
                right = _builder.CreateExtractValue(right, 0);

            if (left->getType() != right->getType())
                return _builder.CreateICmpNE(_builder.CreateBitCast(left, right->getType()), right, "cmptmp");
            return _builder.CreateICmpNE(left, right, "cmptmp");
        }
        case OT_LOGICAL_AND:
            return _builder.CreateAnd(left, right, "andtmp");
        case OT_LOGICAL_OR:
            return _builder.CreateOr(left, right, "ortmp");

        case OT_SQUARE_BRACKETS_OPEN:
            return _builder.CreateLoad(codeGenLval(node));
        default:
            throw ErrorReporter::report("Unimplemented Binary operation!", ERR_CODEGEN, node->getPosition());
    }
    
}

Value *CodeGenerator::codeGen(DST::ConditionalExpression *node)
{
    // FIXME: currently both branches are evaluated, should only eval the needed branch.
    return _builder.CreateSelect(
        codeGen(node->getCondition()),
        codeGen(node->getThenBranch()),
        codeGen(node->getElseBranch())
    );
}

Value *CodeGenerator::codeGen(DST::UnaryOperation* node)
{
    switch (node->getOperator()._type)
    {
        case OT_NEW:
        {
            if (((DST::Type*)node->getExpression())->getExactType() == EXACT_ARRAY && ((DST::ArrayType*)node->getExpression())->getLenExp())
            {
                auto type = evalType((DST::Type*)node->getExpression());
                int size = _dataLayout->getTypeAllocSize(evalType(((DST::ArrayType*)node->getExpression())->getElementType()));
                auto len = codeGen(((DST::ArrayType*)node->getExpression())->getLenExp());
                auto allocSize = _builder.CreateMul(_builder.getInt32(size), len, "allocSize");
                return _builder.CreateBitCast( createCallOrInvoke(getMallocFunc(), { allocSize }), type, "newTmp");
            }
            auto type = evalType((DST::Type*)node->getExpression());
            if (type->isVoidTy())
                throw ErrorReporter::report("Cannot create instance of 'void'", ERR_CODEGEN, node->getPosition());
            int size = _dataLayout->getTypeAllocSize(type);
            return _builder.CreateBitCast( createCallOrInvoke(getMallocFunc(), { _builder.getInt32(size) }), type->getPointerTo(), "newTmp");
        }
        case OT_ADD:
            return codeGen(node->getExpression());
        case OT_SUBTRACT:
            throw ErrorReporter::report("Unimplemented literal type", ERR_CODEGEN, node->getPosition());
        case OT_AT:
        {
            auto val = codeGen(node->getExpression());
            assertNotNull(val);
            return _builder.CreateLoad(val);
        }
        case OT_BITWISE_AND:
            return _builder.CreateGEP(codeGenLval(node->getExpression()), _builder.getInt32(0));
        case OT_LOGICAL_NOT:
            return _builder.CreateNot(codeGen(node->getExpression()), "nottmp");
        default:
            throw ErrorReporter::report("Unimplemented unary operation", ERR_CODEGEN, node->getPosition());
    }
    
}

Value *CodeGenerator::codeGen(DST::Conversion* node)
{
    auto type = evalType(node->getType());
    auto exp = codeGen(node->getExpression());
    if (type == exp->getType() || type == _interfaceType)
        return exp;
    if (type != _interfaceType && exp->getType() == _interfaceType) // Interface to non-interface
    {
        // TODO - throw exception incase of invalid conversion
        auto ptr = _builder.CreateExtractValue(exp, 0, "accessTmp");
        return _builder.CreateBitCast(ptr, type, "cnvrttmp");
    }
    unsigned int targetSz = type->getPrimitiveSizeInBits();
    unsigned int currSz = exp->getType()->getPrimitiveSizeInBits();
    if (targetSz < currSz)
        return _builder.CreateTrunc(exp, type, "cnvrttmp");
    else if (targetSz > currSz)
        return _builder.CreateZExt(exp, type, "cnvrttmp");  // TODO - exending cast for unsigned intergers
    else return _builder.CreateBitCast(exp, type, "cnvrttmp");
}

Value *CodeGenerator::codeGen(DST::MemberAccess *node)
{
    if (node->getType()->getExactType() == EXACT_PROPERTY)
    {
        node->getRight() += ".get";
        auto leftTy = node->getLeft()->getType();
        if (leftTy->getExactType() == EXACT_TYPELIST && ((DST::TypeList*)leftTy)->getTypes().size() == 1)
            leftTy = ((DST::TypeList*)leftTy)->getTypes()[0];
        
        if (leftTy->getExactType() == EXACT_PROPERTY)
            leftTy = ((DST::PropertyType*)leftTy)->getReturn();
        switch (leftTy->getExactType())
        {
            case EXACT_NAMESPACE:   // Static getter property
            {
                auto func = codeGenLval(node);
                if (!isFunc(func) || ((llvm::Function*)func)->arg_size() != 0)
                    throw ErrorReporter::report("expression is not a getter property", ERR_CODEGEN, node->getPosition());

                return createCallOrInvoke((llvm::Function*)func, {});
            }
            case EXACT_BASIC:       // Member getter property of basic type
            {
                auto lval = codeGenLval(node->getLeft());

                if (auto interfaceDecl = ((DST::BasicType*)leftTy)->getTypeSpecifier()->getInterfaceDecl())
                {
                    auto vtablePtr = _builder.CreateInBoundsGEP(lval, { _builder.getInt32(0), _builder.getInt32(1) });
                    auto vtable = _builder.CreateLoad(vtablePtr);
                    auto funcPtr = getFuncFromVtable(vtable, interfaceDecl, node->getRight());
                    auto funcTy = _interfaceVtableFuncInfo[interfaceDecl][node->getRight()].type;
                    auto thisPtr = _builder.CreateLoad(_builder.CreateInBoundsGEP(lval, { _builder.getInt32(0), _builder.getInt32(0) }));
                    auto func = _builder.CreateBitCast(funcPtr, funcTy->getPointerTo());
                    return createCallOrInvoke(func, thisPtr);
                }

                auto typeDef = _types[((DST::BasicType*)leftTy)->getTypeSpecifier()->getTypeDecl()];
                auto func = typeDef->functions[node->getRight()];
                if (func->arg_size() != 1)
                    throw ErrorReporter::report("expression is not a getter property", ERR_CODEGEN, node->getPosition());
                return createCallOrInvoke(func, lval);
            }
            case EXACT_POINTER:     // Member getter property of pointer to basic type
            {
                auto ptrTy = ((DST::PointerType*)leftTy)->getPtrType();
                if (ptrTy->getExactType() != EXACT_BASIC)
                    throw "TODO";
                auto lval = codeGen(node->getLeft());
                auto typeDef = _types[((DST::BasicType*)ptrTy)->getTypeSpecifier()->getTypeDecl()];
                auto func = typeDef->functions[node->getRight()];
                if (!isFunc(func) || ((llvm::Function*)func)->arg_size() != 1)
                    throw ErrorReporter::report("expression is not a getter property", ERR_CODEGEN, node->getPosition());
                return createCallOrInvoke((llvm::Function*)func, lval);
            }
            case EXACT_ARRAY:       // Member property of array
                if (node->getRight() == unicode_string("Size.get")) {
                    if (((DST::ArrayType*)leftTy)->getLength() == DST::UNKNOWN_ARRAY_LENGTH)
                        return _builder.CreateLoad(_builder.CreateInBoundsGEP(
                            codeGenLval(node->getLeft()),
                            { _builder.getInt32(0), _builder.getInt32(0) },
                            "sizePtrTmp")
                        );
                    return _builder.getInt32(codeGen(node->getLeft())->getType()->getArrayNumElements());
                }
                throw ErrorReporter::report("Unimplemented Error no.2", ERR_CODEGEN, node->getPosition());   // TODO
            default:
                throw ErrorReporter::report("Unimplemented Error no.3 | " + 
                leftTy->toShortString() + std::to_string(leftTy->getExactType()), ERR_CODEGEN, node->getPosition());   // TODO
        }
    }
    if (node->getType()->isConst())
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
    if (node->getType()->getExactType() == EXACT_PROPERTY)
    {
        node->getVarId() += ".get";
        auto lval = codeGenLval(node);
        if (!isFunc(lval))
            throw ErrorReporter::report("expression is not a getter property", ERR_CODEGEN, node->getPosition());
        auto func = (llvm::Function*)lval;
        if (func->arg_size() != 0)
            throw ErrorReporter::report("expression is not a getter property", ERR_CODEGEN, node->getPosition());
        return createCallOrInvoke(func, {});
    }
    auto t = codeGenLval(node);
    if (node->getType()->isConst())
        return t;
    if (t->getType()->getPointerElementType()->isFunctionTy())
        return t;
    auto str = node->getVarId().to_string().c_str();
    return _builder.CreateLoad(t, str);
}