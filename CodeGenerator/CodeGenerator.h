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
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/IR/Verifier.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/ConstantRange.h"
#include "llvm/Support/FileSystem.h"
//#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Bitcode/BitcodeWriter.h"

#include "../Decorator/DstNode.h"
#include <string>
#include <fstream>

#define CONDITION_TYPE unicode_string("bool")

using llvm::Value;
using llvm::AllocaInst;
using std::fstream;

namespace CodeGenerator 
{
    static llvm::LLVMContext _context;
    static llvm::IRBuilder<> _builder(_context);
    static std::unique_ptr<llvm::Module> _module(new llvm::Module("test", _context));
    static llvm::DataLayout *_dataLayout(new llvm::DataLayout(_module.get()));

    static std::unordered_map<std::string, AllocaInst*> _namedValues;
    //static std::unordered_map<std::string, llvm::GlobalVariable*> _globalValues;
    static llvm::AllocaInst *_currRetPtr;
    static llvm::BasicBlock *_currFuncExit;

    typedef struct TypeDefinition {
        llvm::StructType *structType;
        std::unordered_map<unicode_string, unsigned int, UnicodeHasherFunction> variableIndexes;
        std::unordered_map<unicode_string, llvm::Function*, UnicodeHasherFunction> functions;
    } TypeDefinition;

    typedef struct NamespaceMembers {
        std::unordered_map<unicode_string, llvm::Value*, UnicodeHasherFunction> values;
        std::unordered_map<unicode_string, TypeDefinition*, UnicodeHasherFunction> types;
        std::unordered_map<unicode_string, NamespaceMembers*, UnicodeHasherFunction> namespaces;
    } NamespaceMembers;

    static std::unordered_map<unicode_string, NamespaceMembers*, UnicodeHasherFunction> _namespaces;

    static std::vector<NamespaceMembers*> _currentNamespace;

    static std::unordered_map<DST::TypeDeclaration*, TypeDefinition*> _types;

    static llvm::AllocaInst *_currThisPtr = NULL;

    void setup();

    void writeBitcodeToFile(string fileName);
    void execute(llvm::Function *func);

    // Returns a pointer to the Main function
    llvm::Function *startCodeGen(DST::Program *node);

    void declareNamespaceTypes(DST::NamespaceDeclaration *node);
    llvm::Function * declareNamespaceMembers(DST::NamespaceDeclaration *node);
    void defineNamespaceMembers(DST::NamespaceDeclaration *node);

    llvm::Function * declareFunction(DST::FunctionDeclaration *node, TypeDefinition *typeDef = NULL);
    void codegenFunction(DST::FunctionDeclaration *node, TypeDefinition *typeDef = NULL);

    void declareProperty(DST::PropertyDeclaration *node, TypeDefinition *typeDef = NULL);
    void codegenProperty(DST::PropertyDeclaration *node, TypeDefinition *typeDef = NULL);

    void declareType(DST::TypeDeclaration *node);
    void declareTypeContent(DST::TypeDeclaration *node);
    void codegenTypeMembers(DST::TypeDeclaration *node);


    bool isFunc(llvm::Value *funcPtr);

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
    Value *codeGen(DST::FunctionCall *node);
    Value *codeGen(DST::MemberAccess *node);
    Value *codeGen(DST::ArrayLiteral *node);


    llvm::BasicBlock *codeGen(DST::StatementBlock *node, const llvm::Twine &blockName = "entry");
    Value *codeGen(DST::Variable *node);
    AllocaInst *codeGen(DST::VariableDeclaration *node);
    Value *codeGen(DST::UnaryOperationStatement *node);

    llvm::Function *codeGen(DST::FunctionDeclaration *node);
    llvm::Value *codeGen(DST::IfThenElse *node);
    llvm::Value *codeGen(DST::WhileLoop *node);
    llvm::Value *codeGen(DST::DoWhileLoop *node);
    llvm::Value *codeGen(DST::ForLoop *node);

    llvm::Type *evalType(DST::Type *node);
}