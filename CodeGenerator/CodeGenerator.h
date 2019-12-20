#pragma once

#include "llvm/ADT/APFloat.h"
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

using namespace llvm;

namespace CodeGenerator 
{
    static LLVMContext _context;
    static IRBuilder<> _builder(_context);
    static std::unique_ptr<Module> _module(new Module("test", _context));
    static std::map<std::string, Value *> _namedValues;

    static void setup() 
    {
	    
    }
}