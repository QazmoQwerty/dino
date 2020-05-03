#include "CodeGenerator.h"

namespace CodeGenerator 
{
    void genDISetup();
    void diGenVarDecl(DST::VariableDeclaration *decl, llvm::Value *val);
    void diGenFuncStart(DST::FunctionDeclaration* decl, llvm::Function *func);
    void diGenFuncParam(DST::FunctionDeclaration* decl, llvm::Function *func, uint idx);
    void diGenFuncEnd(DST::FunctionDeclaration* decl, llvm::Function *func);
}