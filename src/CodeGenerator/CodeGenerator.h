#pragma once

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
// #include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/IR/Verifier.h"
// #include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
// #include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
// #include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/ConstantRange.h"
#include "llvm/Support/FileSystem.h"
//#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Bitcode/BitcodeWriter.h"

#include "llvm/Linker/Linker.h"

#include "../Decorator/DstNode.h"
#include <string>
#include <fstream>

#define CONDITION_TYPE unicode_string("bool")

using llvm::Value;
using llvm::AllocaInst;
using std::fstream;

namespace CodeGenerator 
{
    static bool _isLib;
    static llvm::LLVMContext _context;
    static llvm::IRBuilder<> _builder(_context);
    static std::unique_ptr<llvm::Module> _module(new llvm::Module("test", _context));
    static llvm::DataLayout *_dataLayout(new llvm::DataLayout(_module.get()));

    static std::unordered_map<std::string, Value*> _namedValues;
    static vector<Value*> _funcReturns; // by-reference arguments for multi-return functions
    //static std::unordered_map<std::string, llvm::GlobalVariable*> _globalValues;
    static llvm::AllocaInst *_currRetPtr;
    static llvm::BasicBlock *_currFuncExit;
    static llvm::StructType *_interfaceVtableType;
    static llvm::StructType *_objVtableType;
    static llvm::StructType *_interfaceType;
    static llvm::GlobalVariable *_globJmpBuf;
    static llvm::GlobalVariable *_globCaughtErr;
    static llvm::BasicBlock *_currCatchBlock = NULL;
    static llvm::BasicBlock *_currBreakJmp = NULL;
    static llvm::BasicBlock *_currContinueJmp = NULL;
    static llvm::StructType *_stringTy = NULL;
    static unordered_map<llvm::Type*, llvm::Value*> _vtables;

    typedef struct InterfaceFuncInfo {
        unsigned int index;
        llvm::FunctionType *type;
    } InterfaceFuncInfo;

    static std::unordered_map<DST::InterfaceDeclaration*, std::unordered_map<unicode_string, InterfaceFuncInfo, UnicodeHasherFunction>> _interfaceVtableFuncInfo;
    static llvm::Function *_vtableInterfaceLookupFunc = NULL;

    typedef struct TypeDefinition {
        llvm::StructType *structType;
        std::unordered_map<unicode_string, unsigned int, UnicodeHasherFunction> variableIndexes;
        std::unordered_map<unicode_string, llvm::Function*, UnicodeHasherFunction> functions;
        std::unordered_map<llvm::Function*, unsigned int> vtableFuncIndexes;
        llvm::Value *vtable;
    } TypeDefinition;

    // typedef struct InterfaceDefinition {
        
    // } InterfaceDefinition;

    llvm::Value *getFuncFromVtable(llvm::Value *vtable, DST::InterfaceDeclaration *interface, unicode_string &funcName);

    void declareInterfaceMembers(DST::InterfaceDeclaration *node);
    llvm::FunctionType *getInterfaceFuncType(DST::FunctionDeclaration *node);

    llvm::Value *createCallOrInvoke(llvm::Value *callee, llvm::ArrayRef<llvm::Value*> args);

    typedef struct NamespaceMembers {
        std::unordered_map<unicode_string, llvm::Value*, UnicodeHasherFunction> values;
        std::unordered_map<unicode_string, TypeDefinition*, UnicodeHasherFunction> types;
        // std::unordered_map<unicode_string, InterfaceDefinition*, UnicodeHasherFunction> interfaces;
        std::unordered_map<unicode_string, NamespaceMembers*, UnicodeHasherFunction> namespaces;
        DST::NamespaceDeclaration *decl = NULL;
    } NamespaceMembers;

    static std::unordered_map<unicode_string, NamespaceMembers*, UnicodeHasherFunction> _namespaces;

    static std::vector<NamespaceMembers*> _currentNamespace;

    static std::unordered_map<DST::TypeDeclaration*, TypeDefinition*> _types;

    static llvm::AllocaInst *_currThisPtr = NULL;

    void setup(bool isLib = false);

    void writeBitcodeToFile(DST::Program *prog, string fileName);
    // void execute(llvm::Function *func);

    // Returns a pointer to the Main function
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

    llvm::Function *createVtableInterfaceLookupFunction();

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