/*
    A ton of global variables to be used for context whithin the Code Generator.
*/
#pragma once

#include "LlvmInclude.h"

using llvm::Value;
using llvm::AllocaInst;

#include "../Decorator/DstNode.h"
#include <string>
#include <fstream>
using std::fstream;

namespace CodeGenerator 
{
    typedef struct InterfaceFuncInfo {
        unsigned int index;
        llvm::FunctionType *type;
    } InterfaceFuncInfo;

    /* Maps vtable function index information to interface's member's name. */
    extern std::unordered_map<DST::InterfaceDeclaration*, std::unordered_map<unicode_string, InterfaceFuncInfo, UnicodeHasherFunction>> _interfaceVtableFuncInfo;
    
    typedef struct TypeDefinition {
        llvm::StructType *structType;
        std::unordered_map<unicode_string, unsigned int, UnicodeHasherFunction> variableIndexes;
        std::unordered_map<unicode_string, llvm::Function*, UnicodeHasherFunction> functions;
        std::unordered_map<llvm::Function*, unsigned int> vtableFuncIndexes;
        llvm::Value *vtable;
    } TypeDefinition;

    /* Maps DST::TypeDeclarations to useful codeGen info */
    extern std::unordered_map<DST::TypeDeclaration*, TypeDefinition*> _types;

    typedef struct NamespaceMembers {
        std::unordered_map<unicode_string, llvm::Value*, UnicodeHasherFunction> values;
        std::unordered_map<unicode_string, TypeDefinition*, UnicodeHasherFunction> types;
        std::unordered_map<unicode_string, NamespaceMembers*, UnicodeHasherFunction> namespaces;
        DST::NamespaceDeclaration *decl = NULL;
    } NamespaceMembers;

    /* Maps a namespace's id to all values, types, and namespaces contained in it */
    extern std::unordered_map<unicode_string, NamespaceMembers*, UnicodeHasherFunction> _namespaces;

    /* Are we compiling a library or a regular file? */
    extern bool _isLib;

    /* Turn off garbage collection */
    extern bool _noGC;

    /* Needed for any codeGen */
    extern llvm::LLVMContext _context;
    
    /* The module we're generating IR to */
    extern std::unique_ptr<llvm::Module> _module;

    /* Makes IR generation much simpler */
    extern llvm::IRBuilder<> _builder;

    /* Used to find allocation sizes of types */
    extern llvm::DataLayout *_dataLayout;

    /* All the values existing in the local scope */
    extern std::unordered_map<std::string, Value*> _namedValues;

    /* Since multi-return functions are not supported in LLVM we have to return them by reference (through arguments) */
    extern vector<Value*> _funcReturns;

    /*
        %.interface_type = type { i8*, %.vtable_type* }
        First member is a ptr to the object, second is a ptr to the object's vtable.
    */
    extern llvm::StructType *_interfaceType;

    /*
        %.vtable_type = type { i32, %.interface_vtable* }
        Fist member is the type's unique ID.
        Second member is an array containing a vtable corresponding to each interface the type implements.
        Each interface's vtable contains that interface's ID and all the function the type implements for that interface.
    */
    extern llvm::StructType *_objVtableType;

    /*
        %.interface_vtable = type { i32, i8** }
        First value is the interface's unique ID, second is an array of functions.
    */
    extern llvm::StructType *_interfaceVtableType;

    /* Maps all types to their vtables */
    extern unordered_map<llvm::Type*, llvm::Value*> _vtables;

    /* The global variable in we store a pointer to an exception we are throwing */
    extern llvm::GlobalVariable *_globCaughtErr;

    extern llvm::BasicBlock *_currCatchBlock;

    /* The block we should jump to should we hit a 'break' statement */
    extern llvm::BasicBlock *_currBreakJmp;

    /* The block we should jump to should we hit a 'continue' statement */
    extern llvm::BasicBlock *_currContinueJmp;

    /* A struct type representing a string, defined in Bootstrap.dino */
    extern llvm::StructType *_stringTy;

    /* A struct type representing a Null Pointer Error */
    extern llvm::StructType *_nullPtrErrorTy;

    /* The namespaces we are currently in */
    extern std::vector<NamespaceMembers*> _currentNamespace;
}