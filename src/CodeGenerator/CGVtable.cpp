/*
    Functions relating to the generation of IR relating to creating, manipulating and analyzing vtables.
*/
#include "CodeGenerator.h"

llvm::Value* CodeGenerator::createEmptyVtable(DST::Type *type) 
{
    auto llvmVtable = llvm::ConstantStruct::get(_objVtableType, { _builder.getInt32(0), _builder.getInt32(0) });
    return _vtables[type] = new llvm::GlobalVariable(*_module, llvmVtable->getType(), true, llvm::GlobalVariable::PrivateLinkage, 
                                            llvmVtable, type->toShortString() + ".vtable");
}

llvm::Value* CodeGenerator::getVtable(DST::Type *type) {
    type = type->getNonPropertyOf()->getNonConstOf();
    if (auto vtable = _vtables[type])
        return vtable;
    else return createEmptyVtable(type);
}

llvm::Value *CodeGenerator::getFuncFromVtable(llvm::Value *vtable, DST::InterfaceDeclaration *interface, unicode_string &funcName) 
{
    auto interfaceVtable = _builder.CreateCall(getVtableInterfaceLookupFunction(), { vtable, _builder.getInt32((unsigned long)interface) });
    auto idx = _builder.getInt32(_interfaceVtableFuncInfo[interface][funcName].index);
    auto arrPtr = _builder.CreateInBoundsGEP(interfaceVtable, { _builder.getInt32(0), _builder.getInt32(1) });
    auto funcsArr = _builder.CreateLoad(arrPtr);
    auto funcPtr = _builder.CreateInBoundsGEP(funcsArr, idx);
    return _builder.CreateLoad(funcPtr, "funcPtr");
}

llvm::FunctionType *CodeGenerator::getInterfaceFuncType(DST::FunctionDeclaration *node)
{
    vector<llvm::Type*> types;
    auto params = node->getParameters();
    types.push_back(_builder.getInt8PtrTy());
    auto returnType = evalType(node->getReturnType());
    for (auto i : params) 
        types.push_back(evalType(i->getType()));
    return llvm::FunctionType::get(returnType, types, false);
}

llvm::Function *CodeGenerator::getVtableInterfaceLookupFunction()
{
    // FIXME: currently using a linear search, which is a pretty dumb way of doing things
    static llvm::Function *func = NULL;
    if (func)   
        return func;

    auto funcTy = llvm::FunctionType::get(_interfaceVtableType->getPointerTo(), {_objVtableType->getPointerTo(), _builder.getInt32Ty() }, false);
    func = llvm::Function::Create(funcTy, llvm::Function::ExternalLinkage, ".getInterfaceVtable", _module.get());

    if (_isLib)
        return func;

    auto savedInsertPoint = _builder.GetInsertBlock();

    llvm::BasicBlock *bb = llvm::BasicBlock::Create(_context, "entry", func);
    _builder.SetInsertPoint(bb);

    llvm::Argument *objVtable = func->args().begin(), *interfaceId = func->args().begin() + 1;
    auto objVtableSizePtr = _builder.CreateInBoundsGEP(objVtable, { _builder.getInt32(0), _builder.getInt32(0) });
    auto objVtableSize = _builder.CreateLoad(objVtableSizePtr, "interfaceCount");

    auto idx = CreateEntryBlockAlloca(func, _builder.getInt32Ty(), "idx");
    _builder.CreateStore(_builder.getInt32(0), idx);

    llvm::BasicBlock *loopCondBB = llvm::BasicBlock::Create(_context, "loopCond");
    llvm::BasicBlock *loopBB = llvm::BasicBlock::Create(_context, "loop");
    llvm::BasicBlock *foundBB = llvm::BasicBlock::Create(_context, "found");
    llvm::BasicBlock *incBB = llvm::BasicBlock::Create(_context, "inc");
    llvm::BasicBlock *exitBB = llvm::BasicBlock::Create(_context, "exit");

    func->getBasicBlockList().push_back(loopCondBB);
    func->getBasicBlockList().push_back(loopBB);
    func->getBasicBlockList().push_back(foundBB);
    func->getBasicBlockList().push_back(incBB);
    func->getBasicBlockList().push_back(exitBB);

    _builder.CreateBr(loopCondBB);

    _builder.SetInsertPoint(loopCondBB);
    auto idxLoad = _builder.CreateLoad(idx, "idxLoad");
    auto loopCond = _builder.CreateICmpULT(idxLoad, objVtableSize, "loopCond");
    _builder.CreateCondBr(loopCond, loopBB, exitBB);

    _builder.SetInsertPoint(loopBB);
    auto interfaceVtableArrPtr = _builder.CreateGEP(objVtable, { _builder.getInt32(0), _builder.getInt32(1) });
    auto currInterfaceVtable = _builder.CreateGEP(_builder.CreateLoad(interfaceVtableArrPtr), idxLoad);
    auto currInterfaceIdPtr = _builder.CreateGEP(currInterfaceVtable, { _builder.getInt32(0), _builder.getInt32(0) });
    auto currInterfaceId = _builder.CreateLoad(currInterfaceIdPtr);
    auto cond = _builder.CreateICmpEQ(currInterfaceId, interfaceId);
    _builder.CreateCondBr(cond, foundBB, incBB);
    
    _builder.SetInsertPoint(foundBB);
    _builder.CreateRet(currInterfaceVtable);

    _builder.SetInsertPoint(incBB);
    auto addVal = _builder.CreateAdd(idxLoad, _builder.getInt32(1));
    _builder.CreateStore(addVal, idx);
    _builder.CreateBr(loopCondBB);

    _builder.SetInsertPoint(exitBB);
    _builder.CreateRet(llvm::Constant::getNullValue(_interfaceVtableType->getPointerTo()));

    _builder.SetInsertPoint(savedInsertPoint);

    return func;
}