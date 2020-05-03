#include "CodeGenerator.h"
#include "DebugInfoTypes.h"

llvm::DIType* CodeGenerator::evalDIType(DST::Type *node)
{
    if (node->_backendDIType)
        return node->_backendDIType;
    switch (node->getExactType())
    {
        case EXACT_BASIC:
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
            TODO
        case EXACT_ARRAY: TODO
        case EXACT_POINTER: 
            return node->_backendDIType = _dbuilder->createPointerType(
                evalDIType(node->as<DST::PointerType>()->getPtrType()),
                32 /* todo - what size? */
            );
        case EXACT_FUNCTION:  TODO
        case EXACT_TYPELIST: TODO
        case EXACT_CONST: return evalDIType(((DST::ConstType*)node)->getNonConstOf());
        case EXACT_PROPERTY:
            return evalDIType(((DST::PropertyType*)node)->getReturn());
        default: throw ErrorReporter::report("Specified type is not currently supported in code generation.", ERR_CODEGEN, node->getPosition());
    }
}

llvm::DIType* CodeGenerator::getDIVoidTy() 
{
    static llvm::DIType *ret = NULL;
    if (!ret) TODO;
    return ret;
}

llvm::DIType* CodeGenerator::getDIBoolTy() 
{
    static llvm::DIType *ret = NULL;
    if (!ret) ret = _dbuilder->createBasicType("bool", 1, llvm::dwarf::DW_ATE_boolean);
    return ret;
}

llvm::DIType* CodeGenerator::getDIInt8Ty() 
{
    static llvm::DIType *ret = NULL;
    if (!ret) ret = _dbuilder->createBasicType("i8", 8, llvm::dwarf::DW_ATE_signed);
    return ret;
}

llvm::DIType* CodeGenerator::getDIInt16Ty() 
{
    static llvm::DIType *ret = NULL;
    if (!ret) ret = _dbuilder->createBasicType("i16", 16, llvm::dwarf::DW_ATE_signed);
    return ret;
}

llvm::DIType* CodeGenerator::getDIInt32Ty() 
{
    static llvm::DIType *ret = NULL;
    if (!ret) ret = _dbuilder->createBasicType("int", 32, llvm::dwarf::DW_ATE_signed);
    return ret;
}

llvm::DIType* CodeGenerator::getDIInt64Ty() 
{
    static llvm::DIType *ret = NULL;
    if (!ret) ret = _dbuilder->createBasicType("i64", 64, llvm::dwarf::DW_ATE_signed);
    return ret;
}

llvm::DIType* CodeGenerator::getDIInt128Ty() 
{
    static llvm::DIType *ret = NULL;
    if (!ret) ret = _dbuilder->createBasicType("i128", 128, llvm::dwarf::DW_ATE_signed);
    return ret;
}

llvm::DIType* CodeGenerator::getDIUnsigned8Ty() 
{
    static llvm::DIType *ret = NULL;
    if (!ret) ret = _dbuilder->createBasicType("u8", 8, llvm::dwarf::DW_ATE_unsigned);
    return ret;
}

llvm::DIType* CodeGenerator::getDIUnsigned16Ty() 
{
    static llvm::DIType *ret = NULL;
    if (!ret) ret = _dbuilder->createBasicType("u16", 16, llvm::dwarf::DW_ATE_unsigned);
    return ret;
}

llvm::DIType* CodeGenerator::getDIUnsigned32Ty() 
{
    static llvm::DIType *ret = NULL;
    if (!ret) ret = _dbuilder->createBasicType("uint", 32, llvm::dwarf::DW_ATE_unsigned);
    return ret;
}

llvm::DIType* CodeGenerator::getDIUnsigned64Ty() 
{
    static llvm::DIType *ret = NULL;
    if (!ret) ret = _dbuilder->createBasicType("u64", 64, llvm::dwarf::DW_ATE_unsigned);
    return ret;
}

llvm::DIType* CodeGenerator::getDIUnsigned128Ty() 
{
    static llvm::DIType *ret = NULL;
    if (!ret) ret = _dbuilder->createBasicType("u128", 128, llvm::dwarf::DW_ATE_unsigned);
    return ret;
}

llvm::DIType* CodeGenerator::getDIFloat16Ty() 
{
    static llvm::DIType *ret = NULL;
    if (!ret) ret = _dbuilder->createBasicType("f16", 16, llvm::dwarf::DW_ATE_float);
    return ret;
}

llvm::DIType* CodeGenerator::getDIFloat32Ty() 
{
    static llvm::DIType *ret = NULL;
    if (!ret) ret = _dbuilder->createBasicType("float", 32, llvm::dwarf::DW_ATE_float);
    return ret;
}

llvm::DIType* CodeGenerator::getDIFloat64Ty() 
{
    static llvm::DIType *ret = NULL;
    if (!ret) ret = _dbuilder->createBasicType("f64", 64, llvm::dwarf::DW_ATE_float);
    return ret;
}

llvm::DIType* CodeGenerator::getDIFloat128Ty() 
{
    static llvm::DIType *ret = NULL;
    if (!ret) ret = _dbuilder->createBasicType("f128", 128, llvm::dwarf::DW_ATE_float);
    return ret;
}