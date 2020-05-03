/*
    General utility functions for the Code Generator.
*/
#include "CodeGenerator.h"

llvm::Type *CodeGenerator::evalType(DST::Type *node) 
{
    switch (node->getExactType())
    {
        case EXACT_BASIC:
            if (node->isInterfaceTy())      return _interfaceType;
            if (node == DST::getBoolTy())   return llvm::Type::getInt1Ty(_context);
            if (node == DST::getc8Ty())     return llvm::Type::getInt8Ty(_context);
            if (node == DST::getc32Ty())    return llvm::Type::getInt32Ty(_context);

            if (node == DST::geti8Ty())     return llvm::Type::getInt8Ty(_context);
            if (node == DST::geti16Ty())    return llvm::Type::getInt16Ty(_context);
            if (node == DST::geti32Ty())    return llvm::Type::getInt32Ty(_context);
            if (node == DST::geti64Ty())    return llvm::Type::getInt64Ty(_context);
            if (node == DST::geti128Ty())   return llvm::Type::getInt128Ty(_context);

            if (node == DST::getu8Ty())     return llvm::Type::getInt8Ty(_context);
            if (node == DST::getu16Ty())    return llvm::Type::getInt16Ty(_context);
            if (node == DST::getu32Ty())    return llvm::Type::getInt32Ty(_context);
            if (node == DST::getu64Ty())    return llvm::Type::getInt64Ty(_context);
            if (node == DST::getu128Ty())   return llvm::Type::getInt128Ty(_context);

            if (node == DST::getf16Ty())    return llvm::Type::getHalfTy(_context);
            if (node == DST::getf32Ty())    return llvm::Type::getFloatTy(_context);
            if (node == DST::getf64Ty())    return llvm::Type::getDoubleTy(_context);
            if (node == DST::getf128Ty())   return llvm::Type::getFP128Ty(_context);

            if (node == DST::getVoidTy())   return llvm::Type::getVoidTy(_context);
            if (node == DST::getStringTy()) return _stringTy;
            else 
            {
                auto bt = (DST::BasicType*)node;
                if (bt->isValueTy())
                    return _types[bt->getTypeDecl()]->structType;
                throw ErrorReporter::report("Type " + node->toShortString() + " does not exist", ERR_CODEGEN, node->getPosition());
            }
        case EXACT_ARRAY:
            if (((DST::ArrayType*)node)->getLength() == DST::UNKNOWN_ARRAY_LENGTH) 
            {
                if (auto exp = ((DST::ArrayType*)node)->getLenExp()) 
                {
                    auto v = codeGen(exp);
                    if (auto len = llvm::dyn_cast<llvm::ConstantInt>(v))
                        return llvm::ArrayType::get(evalType(((DST::ArrayType*)node)->getElementType()), len->getLimitedValue());
                    else return evalType(((DST::ArrayType*)node)->getElementType())->getPointerTo();
                }
                return llvm::StructType::get(_context, {
                    llvm::Type::getInt32Ty(_context), 
                    evalType(((DST::ArrayType*)node)->getElementType())->getPointerTo()
                });
            }
            return llvm::ArrayType::get(evalType(((DST::ArrayType*)node)->getElementType()), ((DST::ArrayType*)node)->getLength());
        case EXACT_PROPERTY:
            return evalType(((DST::PropertyType*)node)->getReturn());
        case EXACT_POINTER:
            if (((DST::PointerType*)node)->getPtrType()->isBasicTy())
            {
                if (((DST::BasicType*)(((DST::PointerType*)node)->getPtrType()))->getTypeName() == unicode_string("void"))   // void* is invalid in llvm IR
                    return llvm::Type::getInt8Ty(_context)->getPointerTo();
            }
            return evalType(((DST::PointerType*)node)->getPtrType())->getPointerTo();
        case EXACT_FUNCTION: 
        {
            auto ft = (DST::FunctionType*)node;
            vector<llvm::Type*> params;
            auto retTy = evalType(ft->getReturn());
            for (auto i : ft->getParameters())
                params.push_back(evalType(i));
            return llvm::FunctionType::get(retTy, params, /*isVarArgs=*/false)->getPointerTo();
        }
        case EXACT_CONST: return evalType(((DST::ConstType*)node)->getNonConstOf());
        case EXACT_TYPELIST:
        {
            auto tl = node->as<DST::TypeList>();
            if (tl->size() == 1)
                return evalType(tl->getTypes()[0]);
            else 
            {
                vector<llvm::Type*> types;
                for (auto i : tl->getTypes())
                    types.push_back(evalType(i));
                return llvm::StructType::get(_context, types);    
            }
        }    
        default: throw ErrorReporter::report("Specified type is not currently supported in code generation.", ERR_CODEGEN, node->getPosition());
    }
}

llvm::Function *CodeGenerator::getParentFunction() 
{
    return _builder.GetInsertBlock() ? _builder.GetInsertBlock()->getParent() : nullptr;
}

AllocaInst *CodeGenerator::CreateEntryBlockAlloca(llvm::Function *func, llvm::Type *type, const string &varName) 
{ 
    llvm::IRBuilder<> TmpB(&func->getEntryBlock(), func->getEntryBlock().begin());
    return TmpB.CreateAlloca(type, nullptr, varName);
}

llvm::GlobalVariable * CodeGenerator::createGlobalVariable(DST::VariableDeclaration *node)
{
    auto type = evalType(node->getType());
    if (type->isVoidTy())
        throw ErrorReporter::report("Cannot create instance of type \"void\"", ERR_CODEGEN, node->getPosition());
    auto name = node->getVarId().to_string();
    auto glob = new llvm::GlobalVariable(*_module, type, false, llvm::GlobalVariable::CommonLinkage, llvm::Constant::getNullValue(type), name);
    _currentNamespace.back()->values[node->getVarId()] = glob;
    return glob;
}

bool CodeGenerator::isFunc(llvm::Value *funcPtr)
{
    if (funcPtr == nullptr)
        return false;
    if (funcPtr->getType()->isFunctionTy())
        return true;
    else if (funcPtr->getType()->isPointerTy() && ((llvm::PointerType*)funcPtr->getType())->getElementType()->isFunctionTy())
        return true;
    else return false;
}

bool CodeGenerator::isFuncPtr(llvm::Value *funcPtr)
{
    if (funcPtr == nullptr || !funcPtr->getType()->isPointerTy())
        return false;
    if (funcPtr->getType()->getPointerElementType()->isFunctionTy())
        return true;
    if (funcPtr->getType()->getPointerElementType()->isPointerTy() && funcPtr->getType()->getPointerElementType()->getPointerElementType()->isFunctionTy())
        return true;
    return false;
}

CodeGenerator::NamespaceMembers *CodeGenerator::getNamespaceMembers(DST::Expression *node)
{
    if (node->getExpressionType() == ET_MEMBER_ACCESS)
    {
        auto members = getNamespaceMembers(((DST::MemberAccess*)node)->getLeft());
        return members->namespaces[((DST::MemberAccess*)node)->getRight()];
    }
    if (node->getExpressionType() == ET_IDENTIFIER) {
        if (_currentNamespace.back())
            if (auto ns = _currentNamespace.back()->namespaces[((DST::Variable*)node)->getVarId()])
                return ns;
        return _namespaces[((DST::Variable*)node)->getVarId()];
    }
    UNREACHABLE
}

llvm::Value *CodeGenerator::createCallOrInvoke(llvm::Value *callee, llvm::ArrayRef<llvm::Value*> args)
{
    if (_currCatchBlock)
    {
        auto continueBB = llvm::BasicBlock::Create(_context, "invokeCont");
        auto invoke = _builder.CreateInvoke(callee, continueBB, _currCatchBlock, args);

        getParentFunction()->getBasicBlockList().push_back(continueBB);
        _builder.SetInsertPoint(continueBB);
        return invoke;
    }
    else return _builder.CreateCall(callee, args);
}

Value *CodeGenerator::createThrow(llvm::Value *exceptionVal, bool dontInvoke)
{
    static llvm::Function *throwFunc = NULL;
    static llvm::Function *allocExceptionFunc = NULL;

    if (throwFunc == NULL)
    {
        auto throwFuncTy = llvm::FunctionType::get(_builder.getVoidTy(), { _builder.getInt8PtrTy(), _builder.getInt8PtrTy(), _builder.getInt8PtrTy() }, false);
        throwFunc = llvm::Function::Create(throwFuncTy, llvm::Function::ExternalLinkage, "__cxa_throw", _module.get());

        auto allocExceptionFuncTy = llvm::FunctionType::get(_builder.getInt8PtrTy(), _builder.getInt64Ty(), false);
        allocExceptionFunc = llvm::Function::Create(allocExceptionFuncTy, llvm::Function::ExternalLinkage, "__cxa_allocate_exception", _module.get());
    }

    auto alloc = _builder.CreateCall(allocExceptionFunc, _builder.getInt64(_dataLayout->getTypeAllocSize(_builder.getInt8PtrTy())));
    auto cast = _builder.CreateBitCast(alloc, _interfaceType->getPointerTo());
    _builder.CreateStore(exceptionVal, cast);
    auto nullPtr = llvm::ConstantPointerNull::get(_builder.getInt8PtrTy());

    if (dontInvoke)
        return _builder.CreateCall(throwFunc, { alloc, nullPtr, nullPtr });

    return createCallOrInvoke(throwFunc, { alloc, nullPtr, nullPtr });
}

llvm::Value *CodeGenerator::assertNotNull(llvm::Value *val)
{
    return createCallOrInvoke(getNullCheckFunc(), _builder.CreateBitCast(val, _builder.getInt8PtrTy()));
}

llvm::Function *CodeGenerator::getNullCheckFunc()
{
    static llvm::Function *func = NULL;
    if (func)
        return func;
    
    auto funcTy = llvm::FunctionType::get(_builder.getVoidTy(), _builder.getInt8PtrTy(), false);
    func = llvm::Function::Create(funcTy, llvm::Function::ExternalLinkage, ".assertNotNull", _module.get());

    if (_isLib)
        return func;

    auto savedInsertPoint = _builder.GetInsertBlock();

    llvm::BasicBlock *bb = llvm::BasicBlock::Create(_context, "entry", func);
    _builder.SetInsertPoint(bb);

    llvm::Argument *ptr = func->args().begin();

    auto isNull = _builder.CreateIsNull(ptr, "isNull");

    llvm::BasicBlock *thenBB = llvm::BasicBlock::Create(_context, "then");
    llvm::BasicBlock *elseBB = llvm::BasicBlock::Create(_context, "else");
    _builder.CreateCondBr(isNull, thenBB, elseBB);
    func->getBasicBlockList().push_back(thenBB);
    func->getBasicBlockList().push_back(elseBB);

    _builder.SetInsertPoint(thenBB);

    auto errAlloc = _builder.CreateCall(getMallocFunc(), { _builder.getInt32(_dataLayout->getTypeAllocSize(_nullPtrErrorTy)) });
    // auto alloca = CreateEntryBlockAlloca(func, _interfaceType, "errAlloc");
    // auto objPtr = _builder.CreateGEP(alloca, {_builder.getInt32(0), _builder.getInt32(0) }, "objPtr");
    // auto vtablePtr = _builder.CreateGEP(alloca, {_builder.getInt32(0), _builder.getInt32(1) }, "vtablePtr");
    // _builder.CreateStore(errAlloc, objPtr);
    // _builder.CreateStore(getVtable(DST::getNullPtrErrTy()), vtablePtr);
    // createThrow(_builder.CreateLoad(alloca), true);

    auto undef = llvm::UndefValue::get(_interfaceType);
    auto withErr = _builder.CreateInsertValue(undef, errAlloc, 0);
    auto withVtable = _builder.CreateInsertValue(withErr, getVtable(DST::getNullPtrErrTy()), 1);
    createThrow(withVtable, true);

    _builder.CreateBr(elseBB);

    _builder.SetInsertPoint(elseBB);
    _builder.CreateRetVoid();

    _builder.SetInsertPoint(savedInsertPoint);
    return func;
}

llvm::Function *CodeGenerator::getMallocFunc()
{
    static llvm::Function *malloc = NULL;
    if (malloc == NULL)
    {
        auto type = llvm::FunctionType::get(llvm::Type::getInt8Ty(_context)->getPointerTo(), { llvm::Type::getInt32Ty(_context) }, false);
        if (_noGC)
             malloc = llvm::Function::Create(type, llvm::Function::ExternalLinkage, "malloc", _module.get());
        else malloc = llvm::Function::Create(type, llvm::Function::ExternalLinkage, "GC_malloc", _module.get());
    }
    return malloc;
}

Value *CodeGenerator::codeGen(DST::Node *node) 
{
    if (node == nullptr) 
        return nullptr;
    if (node->isExpression())
        return codeGen(dynamic_cast<DST::Expression*>(node));
    else return codeGen(dynamic_cast<DST::Statement*>(node));
}