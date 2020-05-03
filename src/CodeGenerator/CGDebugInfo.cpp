#include "CodeGenerator.h"

llvm::DIType* CodeGenerator::evalDIType(DST::Type *node)
{
    if (node->_backendDIType)
        return node->_backendDIType;
    switch (node->getExactType())
    {
        case EXACT_BASIC:
        {
            if (node->isInterfaceTy())      return NULL;
            if (node == DST::getBoolTy())   return node->_backendDIType = _dbuilder->createBasicType("bool", 1, llvm::dwarf::DW_ATE_boolean);
            if (node == DST::getc8Ty())     return node->_backendDIType = _dbuilder->createBasicType("char", 8, llvm::dwarf::DW_ATE_unsigned_char);
            if (node == DST::getc32Ty())    return node->_backendDIType = _dbuilder->createBasicType("uchar", 32, llvm::dwarf::DW_ATE_unsigned_char);

            if (node == DST::geti8Ty())     return node->_backendDIType = _dbuilder->createBasicType("i8", 8, llvm::dwarf::DW_ATE_signed);
            if (node == DST::geti16Ty())    return node->_backendDIType = _dbuilder->createBasicType("i16", 16, llvm::dwarf::DW_ATE_signed);
            if (node == DST::geti32Ty())    return node->_backendDIType = _dbuilder->createBasicType("int", 32, llvm::dwarf::DW_ATE_signed);
            if (node == DST::geti64Ty())    return node->_backendDIType = _dbuilder->createBasicType("i64", 64, llvm::dwarf::DW_ATE_signed);
            if (node == DST::geti128Ty())   return node->_backendDIType = _dbuilder->createBasicType("i128", 128, llvm::dwarf::DW_ATE_signed);

            if (node == DST::getu8Ty())     return node->_backendDIType = _dbuilder->createBasicType("u8", 8, llvm::dwarf::DW_ATE_unsigned);
            if (node == DST::getu16Ty())    return node->_backendDIType = _dbuilder->createBasicType("u16", 16, llvm::dwarf::DW_ATE_unsigned);
            if (node == DST::getu32Ty())    return node->_backendDIType = _dbuilder->createBasicType("uint", 32, llvm::dwarf::DW_ATE_unsigned);
            if (node == DST::getu64Ty())    return node->_backendDIType = _dbuilder->createBasicType("u64", 64, llvm::dwarf::DW_ATE_unsigned);
            if (node == DST::getu128Ty())   return node->_backendDIType = _dbuilder->createBasicType("u128", 128, llvm::dwarf::DW_ATE_unsigned);

            if (node == DST::getf16Ty())    return node->_backendDIType = _dbuilder->createBasicType("f16", 16, llvm::dwarf::DW_ATE_float);
            if (node == DST::getf32Ty())    return node->_backendDIType = _dbuilder->createBasicType("float", 32, llvm::dwarf::DW_ATE_float);
            if (node == DST::getf64Ty())    return node->_backendDIType = _dbuilder->createBasicType("f64", 64, llvm::dwarf::DW_ATE_float);
            if (node == DST::getf128Ty())   return node->_backendDIType = _dbuilder->createBasicType("f128", 128, llvm::dwarf::DW_ATE_float);

            if (node == DST::getVoidTy())   return NULL;
            if (node == DST::getStringTy()) TODO

            if (node->isValueTy())
                TODO
            UNREACHABLE
        }
        case EXACT_FUNCTION: 
        {
            vector<llvm::Metadata*> types;
            auto funcTy = node->as<DST::FunctionType>();
            types.push_back(evalDIType(funcTy));
            for (auto ty : funcTy->getParameters())
                types.push_back(evalDIType(ty));
            return node->_backendDIType = _dbuilder->createSubroutineType(_dbuilder->getOrCreateTypeArray(types));
        }
        case EXACT_POINTER: 
            return node->_backendDIType = _dbuilder->createPointerType(
                evalDIType(node->as<DST::PointerType>()->getPtrType()),
                _builder.getInt8PtrTy()->getPrimitiveSizeInBits()
            );
        case EXACT_ARRAY:  TODO
        case EXACT_TYPELIST: TODO
        case EXACT_CONST: return evalDIType(((DST::ConstType*)node)->getNonConstOf());
        case EXACT_PROPERTY: return evalDIType(((DST::PropertyType*)node)->getReturn());
        default: UNREACHABLE
    }
}

llvm::DIFile *CodeGenerator::getDIFile(SourceFile *file)
{
    if (!file->_backendDIFile)
        file->_backendDIFile = _dbuilder->createFile(file->getName(), file->getPath());
    return file->_backendDIFile;
}