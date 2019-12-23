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


#include "../Decorator/Decorator.h"

using llvm::Value;
using llvm::AllocaInst;

namespace CodeGenerator 
{
    static llvm::LLVMContext _context;
    static llvm::IRBuilder<> _builder(_context);
    static std::unique_ptr<llvm::Module> _module(new llvm::Module("test", _context));
    static std::unordered_map<std::string, AllocaInst*> _namedValues;

    static void setup() 
    {
	    // Nothing yet
        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmPrinter();
        llvm::InitializeNativeTargetAsmParser();
    }

    static void execute(llvm::Function *func)
    {
        // Create JIT
        llvm::Module *M = _module.get();
        std::string errStr;
        llvm::ExecutionEngine *EE = llvm::EngineBuilder(std::move(_module)).setErrorStr(&errStr).setEngineKind(llvm::EngineKind::Interpreter).create();

        if (!EE) {
            llvm::errs() << ": Failed to construct ExecutionEngine: " << errStr << "\n";
            return;
        }

        llvm::errs() << "verifying... ";
        if (llvm::verifyModule(*M)) {
            llvm::errs() << ": Error constructing function!\n";
            return;
        }

        llvm::errs() << "OK\n";
        llvm::errs() << "We just constructed this LLVM module:\n\n---------\n" << *M;
        llvm::errs() << "---------\nstarting with Interpreter...\n";

        std::vector<llvm::GenericValue> noargs;
        llvm::GenericValue GV = EE->runFunction(func, noargs);

        llvm::errs() << "done !!! \n";
        llvm::outs() << "Result: " << GV.IntVal << "\n";
    }

    llvm::Function *getParentFunction(); 

    static AllocaInst *CreateEntryBlockAlloca(llvm::Function *func, llvm::Type *type, const string &varName) ; 

    Value *codeGen(DST::Node *node);
    Value *codeGen(DST::Statement *node);
    Value *codeGen(DST::Expression *node);
    Value *codeGen(DST::Literal *node);
    Value *codeGen(DST::BinaryOperation* node);
    Value *codeGen(DST::Assignment* node);

    llvm::BasicBlock *codeGen(DST::StatementBlock *node, const llvm::Twine &blockName = "entry");
    Value *codeGen(DST::Variable *node);
    AllocaInst *codeGen(DST::VariableDeclaration *node);
    Value *codeGen(DST::UnaryOperationStatement *node);

    llvm::Function *codeGen(DST::FunctionDeclaration *node);
    llvm::Value *codeGen(DST::IfThenElse *node);
    llvm::Value *codeGen(DST::WhileLoop *node);

    llvm::Type *evalType(DST::Type *node);
}