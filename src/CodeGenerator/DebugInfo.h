#include "CodeGenerator.h"

namespace CodeGenerator 
{
    extern vector<llvm::DIScope*> _currDIScope;
    extern llvm::DISubprogram *_currDISubProgram;
    llvm::DIScope *getCurrDIScope(DST::Node *node);
    void diEmitLocation(DST::Node *node);
    void diGenNamespace(NamespaceMembers *node);
    void diEnterNamespace(NamespaceMembers *ns);
    void diLeaveNamespace();
    void diGenSetup();
    void diGenFinalize();
    void diGenVarDecl(DST::VariableDeclaration *decl, llvm::Value *val);
    void diGenFuncStart(DST::FunctionDeclaration* decl, llvm::Function *func);
    void diGenFuncParam(DST::FunctionDeclaration* decl, llvm::Function *func, uint idx, llvm::AllocaInst *alloca);
    void diGenFuncEnd(DST::FunctionDeclaration* decl, llvm::Function *func);
}