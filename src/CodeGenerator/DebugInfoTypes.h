#include "CodeGenContext.h"

namespace CodeGenerator 
{

    llvm::DIType* createDIBasicTy(DST::Type *type);

    llvm::DIType* getDIBoolTy();

    llvm::DIType* getDIInt8Ty();
    llvm::DIType* getDIInt16Ty();
    llvm::DIType* getDIInt32Ty();
    llvm::DIType* getDIInt64Ty();
    llvm::DIType* getDIInt128Ty();

    llvm::DIType* getDIUnsigned8Ty();
    llvm::DIType* getDIUnsigned16Ty();
    llvm::DIType* getDIUnsigned32Ty();
    llvm::DIType* getDIUnsigned64Ty();
    llvm::DIType* getDIUnsigned128Ty();

    llvm::DIType* getDIFloat16Ty();
    llvm::DIType* getDIFloat32Ty();
    llvm::DIType* getDIFloat64Ty();
    llvm::DIType* getDIFloat128Ty();

    llvm::DIType* getDIVoidTy();

    llvm::DIType* getDIStringTy();
}