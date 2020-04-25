#pragma once

#include "CodeGenContext.h"

#define CONDITION_TYPE unicode_string("bool")


namespace CodeGenerator 
{
    llvm::Value *getFuncFromVtable(llvm::Value *vtable, DST::InterfaceDeclaration *interface, unicode_string &funcName);

    void declareInterfaceMembers(DST::InterfaceDeclaration *node);
    llvm::FunctionType *getInterfaceFuncType(DST::FunctionDeclaration *node);

    llvm::Value *createCallOrInvoke(llvm::Value *callee, llvm::ArrayRef<llvm::Value*> args);


    void setup(bool isLib = false);

    void writeBitcodeToFile(DST::Program *prog, string fileName);

    /* Returns a pointer to the Main function */
    llvm::Function *startCodeGen(DST::Program *node);

    void declareNamespaceTypes(DST::NamespaceDeclaration *node);
    void declareNamespaceTypesContent(DST::NamespaceDeclaration *node);
    llvm::Function * declareNamespaceMembers(DST::NamespaceDeclaration *node);
    void defineNamespaceMembers(DST::NamespaceDeclaration *node);

    llvm::Function * declareFunction(DST::FunctionDeclaration *node, TypeDefinition *typeDef = NULL);
    void codegenFunction(DST::FunctionDeclaration *node, TypeDefinition *typeDef = NULL);

    std::pair<llvm::Function*, llvm::Function*> declareProperty(DST::PropertyDeclaration *node, TypeDefinition *typeDef = NULL);
    void codegenProperty(DST::PropertyDeclaration *node, TypeDefinition *typeDef = NULL);

    DST::InterfaceDeclaration *getPropertyInterface(DST::TypeDeclaration *typeDecl, DST::PropertyDeclaration *propDecl);
    DST::InterfaceDeclaration *getPropertyInterface(DST::InterfaceDeclaration *interfaceDecl, DST::PropertyDeclaration *propDecl);
    DST::InterfaceDeclaration *getFunctionInterface(DST::TypeDeclaration *typeDecl, DST::FunctionDeclaration *funcDecl);
    DST::InterfaceDeclaration *getFunctionInterface(DST::InterfaceDeclaration *interfaceDecl, DST::FunctionDeclaration *funcDecl);

    llvm::Function *getVtableInterfaceLookupFunction();
    llvm::Function *getNullCheckFunc();
    llvm::Value *assertNotNull(llvm::Value *val);
    llvm::Function *getMallocFunc();
    Value *createThrow(llvm::Value *exception, bool dontInvoke = false);

    void declareType(DST::TypeDeclaration *node);
    void declareTypeContent(DST::TypeDeclaration *node);
    void codegenTypeMembers(DST::TypeDeclaration *node);

    bool isFunc(llvm::Value *funcPtr);
    bool isFuncPtr(llvm::Value *funcPtr);

    llvm::Value* convertToInterface(llvm::Value *ptr);
    llvm::Value* convertToInterfaceLval(llvm::Value *ptr);

    llvm::Value* createEmptyVtable(llvm::Type *type);
    llvm::Value* getVtable(llvm::Type *type);

    NamespaceMembers *getNamespaceMembers(DST::Expression *node);

    llvm::Function *getParentFunction(); 

    AllocaInst *CreateEntryBlockAlloca(llvm::Function *func, llvm::Type *type, const string &varName) ; 
    llvm::GlobalVariable *createGlobalVariable(DST::VariableDeclaration *node);

    Value *codeGenLval(DST::Expression *node);
    Value *codeGenLval(DST::Variable *node);
    Value *codeGenLval(DST::MemberAccess *node);
    AllocaInst *codeGenLval(DST::VariableDeclaration *node);
    Value *codeGenLval(DST::UnaryOperation* node);
    Value *codeGenLval(DST::BinaryOperation* node);
    Value *codeGenLval(DST::Conversion* node);


    Value *codeGen(DST::Node *node);
    Value *codeGen(DST::Statement *node);
    Value *codeGen(DST::Expression *node);
    Value *codeGen(DST::Literal *node);
    Value *codeGen(DST::BinaryOperation *node);
    Value *codeGen(DST::UnaryOperation *node);
    Value *codeGen(DST::Assignment *node);
    Value *codeGen(DST::Conversion *node);
    Value *codeGen(DST::FunctionCall *node, vector<Value*> retPtrs = {});
    Value *codeGen(DST::MemberAccess *node);
    Value *codeGen(DST::ArrayLiteral *node);
    Value *codeGen(DST::Increment *node);
    Value *codeGen(DST::ConditionalExpression *node);

    llvm::BasicBlock *codeGen(DST::StatementBlock *node, const llvm::Twine &blockName = "entry");
    Value *codeGen(DST::Variable *node);
    Value *codeGen(DST::ConstDeclaration *node);
    AllocaInst *codeGen(DST::VariableDeclaration *node);
    Value *codeGen(DST::UnaryOperationStatement *node);

    llvm::Function *codeGen(DST::FunctionDeclaration *node);
    llvm::Value *codeGen(DST::TryCatch *node);
    llvm::Value *codeGen(DST::SwitchCase *node);
    llvm::Value *codeGen(DST::IfThenElse *node);
    llvm::Value *codeGen(DST::WhileLoop *node);
    llvm::Value *codeGen(DST::DoWhileLoop *node);
    llvm::Value *codeGen(DST::ForLoop *node);

    llvm::Type *evalType(DST::Type *node);
}