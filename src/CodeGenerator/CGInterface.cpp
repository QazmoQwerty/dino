/*
    Utility functions relating to the creation, manipulaton, and analysis of  interface types.
*/
#include "CodeGenerator.h"

Value *CodeGenerator::convertToInterface(Value* node, DST::Type *ty) 
{
    if (node == NULL) 
        return llvm::ConstantStruct::get(_interfaceType, { 
            llvm::ConstantPointerNull::get(_builder.getInt8PtrTy()), 
            llvm::ConstantPointerNull::get(_objVtableType->getPointerTo())
        });
    if (node->getType() == _interfaceType)
        return node;

    if (!node->getType()->isPointerTy())
        throw "cannot convert non pointer to interface";

    auto undef = llvm ::UndefValue::get(_interfaceType);
    auto val = _builder.CreateInsertValue(undef, _builder.CreateBitCast(node, _builder.getInt8PtrTy()), 0);
    return _builder.CreateInsertValue(val, getVtable(ty), 1);
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