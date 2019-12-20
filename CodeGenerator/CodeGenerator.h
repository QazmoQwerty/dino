#pragma once

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

#include "../Decorator/Decorator.h"

using llvm::Value;

namespace CodeGenerator 
{
    static llvm::LLVMContext _context;
    static llvm::IRBuilder<> _builder(_context);
    static std::unique_ptr<llvm::Module> _module(new llvm::Module("test", _context));
    static std::map<std::string, Value *> _namedValues;

    static void setup() 
    {
	    
    }

    static void show() 
    {
        //llvm::Module *m = _module.get();
        //std::cout << *m << std::endl;
    }

    Value *codeGen(DST::Node *node);
    Value *codeGen(DST::Statement *node);
    Value *codeGen(DST::Expression *node);
    Value *codeGen(DST::Literal *node);
    Value *codeGen(DST::BinaryOperation* node);
}