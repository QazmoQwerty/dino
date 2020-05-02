/*
    Code generation for expression statements.
*/
#include "CodeGenerator.h"

Value *CodeGenerator::codeGen(DST::FunctionCall *node, vector<Value*> retPtrs)
{
    if (node->getFunctionId()->getExpressionType() == ET_MEMBER_ACCESS)
    {
        auto ty = ((DST::MemberAccess*)node->getFunctionId())->getLeft()->getType();
        if (ty->isBasicTy() || ty->isPtrTy())
        {
            auto &funcId = ((DST::MemberAccess*)node->getFunctionId())->getRight();
            TypeDefinition *typeDef = NULL;
            llvm::Value *funcPtr = NULL;
            llvm::Value *thisPtr = NULL;
            llvm::FunctionType *funcTy = NULL;

            if (ty->isPtrTy())
                thisPtr = codeGen(((DST::MemberAccess*)node->getFunctionId())->getLeft());
            else thisPtr = codeGenLval(((DST::MemberAccess*)node->getFunctionId())->getLeft());

            if (ty->isBasicTy())
            {
                if (ty->isInterfaceTy())
                {
                    auto interfaceDecl = ty->as<DST::InterfaceType>()->getInterfaceDecl();
                    auto vtablePtr = _builder.CreateInBoundsGEP(thisPtr, { _builder.getInt32(0), _builder.getInt32(1) });
                    auto vtable = _builder.CreateLoad(vtablePtr);
                    funcPtr = getFuncFromVtable(vtable, interfaceDecl, funcId);
                    funcTy = _interfaceVtableFuncInfo[interfaceDecl][funcId].type;
                    thisPtr = _builder.CreateLoad(_builder.CreateInBoundsGEP(thisPtr, { _builder.getInt32(0), _builder.getInt32(0) }));
                }
                else typeDef = _types[ty->as<DST::ValueType>()->getTypeDecl()];
            }
                
            else 
            {
                if (!ty->as<DST::PointerType>()->getPtrType()->isBasicTy())
                    throw ErrorReporter::reportInternal("Internal decorator error?", ERR_CODEGEN, node->getPosition());
                typeDef = _types[ty->as<DST::PointerType>()->getPtrType()->as<DST::ValueType>()->getTypeDecl()];
            }

            if (typeDef && !funcPtr)
            {
                auto func = typeDef->functions[funcId];
                funcTy = llvm::dyn_cast<llvm::FunctionType>(func->getType()->getElementType());
                funcPtr = func;
            }

            if (funcPtr->getType() != funcTy)
                funcPtr = _builder.CreateBitCast(funcPtr, funcTy->getPointerTo());

            if (funcTy->getNumParams() != node->getArguments().size() + 1 + retPtrs.size()) // + 1 since we are also passing a "this" ptr
                throw ErrorReporter::report(string("Incorrect # arguments passed (needed ") + 
                    std::to_string(funcTy->getNumParams()) + ", got " + std::to_string(node->getArguments().size()) + ")"
                    , ERR_CODEGEN, node->getPosition());

            std::vector<Value*> args;
            args.push_back(thisPtr);
        
            int i = -1;
            unsigned int i2 = 0;
            for (auto &argTy : funcTy->params())
            {
                if (i == -1) { i++; continue; }

                if (i2 < retPtrs.size())
                    args.push_back(retPtrs[i2++]);
                else 
                {
                    auto currArg = node->getArguments()[i++];
                    auto gen = codeGen(currArg);        
                    if (gen->getType() != argTy) {
                        if (argTy == _interfaceType)
                            args.push_back(convertToInterface(gen, currArg->getType()));
                        else args.push_back(_builder.CreateBitCast(gen, argTy, "castTmp"));
                    }
                    else args.push_back(gen);
                }
            }

            return createCallOrInvoke(funcPtr, args);

        }
    }

    llvm::Value *funcPtr = codeGenLval(node->getFunctionId());
    
    if (!isFunc(funcPtr))
    {
        if (isFuncPtr(funcPtr))
            funcPtr = _builder.CreateLoad(funcPtr);
        else throw ErrorReporter::report("expression is not a function!", ERR_CODEGEN, node->getPosition());
    }

    auto funcTy = llvm::dyn_cast<llvm::FunctionType>(funcPtr->getType()->getPointerElementType());
    if (!funcTy)
        throw ErrorReporter::report("Internal error while generating IR for function call", ERR_CODEGEN, node->getPosition());
    
    if (funcTy->getNumParams() != node->getArguments().size() + retPtrs.size())
        throw ErrorReporter::report(string("Incorrect # arguments passed (needed ") + 
            std::to_string(funcTy->getNumParams()) + ", got " + std::to_string(node->getArguments().size()) + ")"
            , ERR_CODEGEN, node->getPosition());
        
    std::vector<Value *> args;

    int i = 0;
    unsigned int i2 = 0;
    for (auto &argTy : funcTy->params())
    {
        if (i2 < retPtrs.size())
            args.push_back(retPtrs[i2++]);
        else
        {
            auto currArg = node->getArguments()[i++];
            auto gen = codeGen(currArg);       
            
            if (gen->getType() != argTy)
            {
                if (argTy == _interfaceType)
                    args.push_back(convertToInterface(gen, currArg->getType()));
                else args.push_back(_builder.CreateBitCast(gen, argTy, "castTmp"));
            }
            else args.push_back(gen);   
        }
    }
    return createCallOrInvoke(funcPtr, args);
}

Value *CodeGenerator::codeGen(DST::Assignment* node)
{
    Value *left = NULL;
    Value *right = NULL;

    if (node->getLeft()->getType()->isPropertyTy())
    {
        if (node->getLeft()->getExpressionType() == ET_IDENTIFIER)
            ((DST::Variable*)node->getLeft())->getVarId() += ".set";
        else if (node->getLeft()->getExpressionType() == ET_MEMBER_ACCESS)
            ((DST::MemberAccess*)node->getLeft())->getRight() += ".set";

        if (node->getLeft()->getExpressionType() == ET_MEMBER_ACCESS) 
        {
            auto ac = (DST::MemberAccess*)node->getLeft();
            switch (ac->getLeft()->getType()->getNonConstOf()->getNonPropertyOf()->getExactType())
            {
                case EXACT_NAMESPACE:   // Static setter property
                {
                    left = codeGenLval(node->getLeft());
                    if (!isFunc(left))
                        throw ErrorReporter::report("expression is not a setter property", ERR_CODEGEN, node->getPosition());
                    auto right = codeGen(node->getRight());
                    createCallOrInvoke((llvm::Function*)left, right);
                    return right;
                }
                case EXACT_BASIC:       // Member setter property of basic type
                {
                    left = codeGenLval(ac->getLeft());
                    right = codeGen(node->getRight());

                    if (ac->getLeft()->getType()->isInterfaceTy())
                    {
                        auto interfaceDecl = ac->getLeft()->getType()->as<DST::InterfaceType>()->getInterfaceDecl();
                        auto vtablePtr = _builder.CreateInBoundsGEP(left, { _builder.getInt32(0), _builder.getInt32(1) });
                        auto vtable = _builder.CreateLoad(vtablePtr);
                        auto funcPtr = getFuncFromVtable(vtable, interfaceDecl, ac->getRight());
                        auto funcTy = _interfaceVtableFuncInfo[interfaceDecl][ac->getRight()].type;
                        auto thisPtr = _builder.CreateLoad(_builder.CreateInBoundsGEP(left, { _builder.getInt32(0), _builder.getInt32(0) }));
                        auto func = _builder.CreateBitCast(funcPtr, funcTy->getPointerTo());
                        createCallOrInvoke(func, { thisPtr, right });
                    }
                    else 
                    {
                        auto typeDef = _types[ac->getLeft()->getType()->as<DST::ValueType>()->getTypeDecl()];
                        auto func = typeDef->functions[ac->getRight()];
                        if (!isFunc(func) || ((llvm::Function*)func)->arg_size() != 2)
                            throw ErrorReporter::report("expression is not a setter property", ERR_CODEGEN, node->getPosition());
                        createCallOrInvoke((llvm::Function*)func, { left, right });
                    }
                    return right;
                }
                case EXACT_POINTER:     // Member setter property of pointer to basic type
                {
                    auto ptrTy = ac->getLeft()->getType()->as<DST::PointerType>()->getPtrType();
                    if (!ptrTy->isBasicTy())
                        throw ErrorReporter::report("only pointers and basic types have setter properties", ERR_CODEGEN, node->getPosition());
                    auto thisPtr = codeGen(ac->getLeft());
                    auto typeDef = _types[ptrTy->as<DST::ValueType>()->getTypeDecl()];
                    auto func = typeDef->functions[ac->getRight()];
                    if (!isFunc(func) || ((llvm::Function*)func)->arg_size() != 2)
                        throw ErrorReporter::report("expression is not a setter property", ERR_CODEGEN, node->getPosition());
                    return createCallOrInvoke((llvm::Function*)func, { thisPtr, codeGen(node->getRight()) });
                }
                default:
                    throw ErrorReporter::report("only pointers and basic types have setter properties", ERR_CODEGEN, node->getPosition());
            }
        }

        left = codeGenLval(node->getLeft());
        if (!isFunc(left))
            throw ErrorReporter::report("expression is not a setter property", ERR_CODEGEN, node->getPosition());

        createCallOrInvoke((llvm::Function*)left, codeGen(node->getRight()));
        return right;
    }

    if (node->getLeft()->getExpressionType() == ET_LIST) 
    {
        right = codeGen(node->getRight());
        auto list = (DST::ExpressionList*)node->getLeft();
        for (unsigned int i = 0; i < list->size(); i++) 
            _builder.CreateStore(
                _builder.CreateExtractValue(right, i), 
                codeGenLval(list->getExpressions()[i])
            );
        return right;
    }

    /*if (node->getLeft()->getType()->isListTy()) 
    {
        // if (node->getRight()->getExpressionType() == ET_FUNCTION_CALL)
        // {
        //     vector<Value*> retPtrs;
        //     for (auto i : ((DST::ExpressionList*)node->getLeft())->getExpressions())
        //         retPtrs.push_back(codeGenLval(i));
        //     return codeGen(((DST::FunctionCall*)node->getRight()), retPtrs);
        // }

        // if (node->getRight()->getExpressionType() != ET_LIST)
        //     throw ErrorReporter::report("Multi-return functions are not implemented yet", ERR_CODEGEN, node->getPosition());
        // vector<Value*> lefts, rights;
        // for (auto i : ((DST::ExpressionList*)node->getLeft())->getExpressions())
        //     lefts.push_back(codeGenLval(i));
        // for (auto i : ((DST::ExpressionList*)node->getRight())->getExpressions())
        //     rights.push_back(codeGen(i));
        // Value *lastStore = NULL;
        // for (unsigned int i = 0; i < lefts.size(); i++)
        //     lastStore = _builder.CreateStore(rights[i], lefts[i]);
        // return lastStore;   // Temporary fix.
    }*/

    if (node->getLeft()->getType()->isArrayTy() && 
        node->getLeft()->getType()->as<DST::ArrayType>()->getLength() == DST::UNKNOWN_ARRAY_LENGTH)
    {
        left = codeGenLval(node->getLeft());
        right = codeGen(node->getRight());
        if (right->getType()->isPointerTy())
        {
            auto sizePtr = _builder.CreateInBoundsGEP(left, { _builder.getInt32(0), _builder.getInt32(0) }, "sizePtrTmp");
            auto arrPtr = _builder.CreateInBoundsGEP(left, { _builder.getInt32(0), _builder.getInt32(1) }, "arrPtrTmp");
            _builder.CreateStore(_builder.CreateBitOrPointerCast(right, arrPtr->getType()->getPointerElementType()), arrPtr);

            if (node->getRight()->getType()->isArrayTy() && node->getRight()->getType()->as<DST::ArrayType>()->getLenExp()) 
                _builder.CreateStore(codeGen(node->getRight()->getType()->as<DST::ArrayType>()->getLenExp()), sizePtr);
            else _builder.CreateStore(_builder.getInt32(right->getType()->getPointerElementType()->getArrayNumElements()), sizePtr);
            return right;
        }
        if (right->getType() == left->getType()->getPointerElementType())
        {
            _builder.CreateStore(right, left);
            return right;
        }
        else throw ErrorReporter::report("You shouldn't have gotten here...", ERR_CODEGEN, node->getPosition());
    }

    if (node->getOperator()._type != OT_ASSIGN_EQUAL)
    {
        left = codeGenLval(node->getLeft());
        right = codeGen(node->getRight());
    }

    switch (node->getOperator()._type) 
    {
        case OT_ASSIGN_EQUAL:
        {
            left = codeGenLval(node->getLeft());
            if (left->getType() == _interfaceType->getPointerTo())
            {
                right = codeGen(node->getRight());
                _builder.CreateStore(convertToInterface(right, node->getRight()->getType()), left);
                return right;
            }
            
            if (left->getType()->getPointerElementType()->isPointerTy() &&
                left->getType()->getPointerElementType()->getPointerElementType()->isFunctionTy())
            {
                right = codeGenLval(node->getRight());
                _builder.CreateStore(right, left);
                return right;    
            }
            right = codeGen(node->getRight());
            _builder.CreateStore(right, left);
            return right;
        }
        case OT_ASSIGN_ADD:
        {
            auto res = _builder.CreateAdd(_builder.CreateLoad(left), right, "addtmp");
            _builder.CreateStore(res, left);
            return res;
        }
        case OT_ASSIGN_SUBTRACT:
        {
            auto res = _builder.CreateSub(_builder.CreateLoad(left), right, "subtmp");
            _builder.CreateStore(res, left);
            return res;
        }
        case OT_ASSIGN_MULTIPLY:
        {
            auto res = _builder.CreateMul(_builder.CreateLoad(left), right, "multmp");
            _builder.CreateStore(res, left);
            return res;
        }
        case OT_ASSIGN_DIVIDE:
        {
            auto res = _builder.CreateSDiv(_builder.CreateLoad(left), right, "divtmp");
            _builder.CreateStore(res, left);
            return res;
        }
        case OT_ASSIGN_MODULUS:
        {
            auto res = _builder.CreateSRem(_builder.CreateLoad(left), right, "modtmp");
            _builder.CreateStore(res, left);
            return res;
        }
        default: throw ErrorReporter::report("Unimplemented assignment operator", ERR_CODEGEN, node->getPosition());
    }
}

Value *CodeGenerator::codeGen(DST::Increment *node)
{
    llvm::Value *lvalue = codeGenLval(node->getExpression());
    int varSize = lvalue->getType()->getPointerElementType()->getPrimitiveSizeInBits();
    
    llvm::Value *inc;
    if (node->isIncrement())
        inc = _builder.CreateAdd(_builder.CreateLoad(lvalue), _builder.getIntN(varSize, 1));
    else inc = _builder.CreateSub(_builder.CreateLoad(lvalue), _builder.getIntN(varSize, 1));
    
    _builder.CreateStore(inc, lvalue);
    
    return inc;
}

AllocaInst *CodeGenerator::codeGen(DST::VariableDeclaration *node) 
{
    auto type = evalType(node->getType());
    if (type->isVoidTy())
        throw ErrorReporter::report("Cannot create instance of type \"void\"", ERR_CODEGEN, node->getPosition());

    auto name = node->getVarId().to_string();
    if (!_builder.GetInsertBlock())
        throw "will this ever happen? idk...";
    auto func = _builder.GetInsertBlock()->getParent();

    // Create an alloca for the variable in the entry block.
    AllocaInst *alloca = CreateEntryBlockAlloca(func, type, name);
    _namedValues.top()[name] = alloca;

    /*if (type->isArrayTy())
    {
        _builder.CreateStore(llvm::ConstantAggregateZero::get(type), alloca); 
    }*/
    return alloca;
}