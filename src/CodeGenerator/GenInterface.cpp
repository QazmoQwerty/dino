/*
    Utility functions relating to the creation, manipulaton, and analysis of  interface types.
*/
#include "CodeGenerator.h"

Value *CodeGenerator::convertToInterface(Value* node) 
{
    if (node == NULL) 
        return llvm::ConstantStruct::get(_interfaceType, { 
            llvm::ConstantPointerNull::get(_builder.getInt8PtrTy()), 
            llvm::ConstantPointerNull::get(_objVtableType->getPointerTo())
        });
    if (node->getType() == _interfaceType)
        return node;
    return _builder.CreateLoad(convertToInterfaceLval(node));
}

Value *CodeGenerator::convertToInterfaceLval(Value* node)
{
    auto alloca = CreateEntryBlockAlloca(getParentFunction(), _interfaceType, "cnvrttmp");
    if (node == NULL || node->getType() == _interfaceType) 
    {
        _builder.CreateStore(convertToInterface(node), alloca);
        return alloca;
    }

    if (!node->getType()->isPointerTy())
        throw "cannot convert non pointer to interface";
    
    auto objPtr = _builder.CreateInBoundsGEP(alloca, { _builder.getInt32(0), _builder.getInt32(0) }, "objPtrTmp");
    auto vtablePtr = _builder.CreateInBoundsGEP(alloca, { _builder.getInt32(0), _builder.getInt32(1) }, "vtableTmp");
    _builder.CreateStore(_builder.CreateBitCast(node, _builder.getInt8PtrTy()), objPtr);
    _builder.CreateStore(getVtable(node->getType()->getPointerElementType()), vtablePtr);
    return alloca;
}

DST::InterfaceDeclaration *CodeGenerator::getPropertyInterface(DST::TypeDeclaration *typeDecl, DST::PropertyDeclaration *propDecl) 
{
    for (auto i : typeDecl->getInterfaces())
        if (auto decl = getPropertyInterface(i, propDecl))
            return decl;
    return NULL;
}

DST::InterfaceDeclaration *CodeGenerator::getPropertyInterface(DST::InterfaceDeclaration *interfaceDecl, DST::PropertyDeclaration *propDecl)
{
    if (interfaceDecl == NULL)
        return NULL;
    if (interfaceDecl->getMemberType(propDecl->getName()))
        return interfaceDecl;
    for (auto i : interfaceDecl->getImplements())
    {
        if (auto decl = getPropertyInterface(i, propDecl))
            return decl;
    }
    return NULL;
} 

DST::InterfaceDeclaration *CodeGenerator::getFunctionInterface(DST::TypeDeclaration *typeDecl, DST::FunctionDeclaration *funcDecl) 
{
    for (auto i : typeDecl->getInterfaces())
        if (auto decl = getFunctionInterface(i, funcDecl))
            return decl;
    return NULL;
}

DST::InterfaceDeclaration *CodeGenerator::getFunctionInterface(DST::InterfaceDeclaration *interfaceDecl, DST::FunctionDeclaration *funcDecl)
{
    if (interfaceDecl == NULL)
        return NULL;
    if (interfaceDecl->getMemberType(funcDecl->getVarDecl()->getVarId()))
        return interfaceDecl;
    for (auto i : interfaceDecl->getImplements())
        if (auto decl = getFunctionInterface(i, funcDecl))
            return decl;
    return NULL;
} 