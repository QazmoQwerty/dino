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
    static std::unordered_map<DST::InterfaceDeclaration*, std::unordered_map<unicode_string, InterfaceFuncInfo, UnicodeHasherFunction>> _interfaceVtableFuncInfo;
    
    typedef struct TypeDefinition {
        llvm::StructType *structType;
        std::unordered_map<unicode_string, unsigned int, UnicodeHasherFunction> variableIndexes;
        std::unordered_map<unicode_string, llvm::Function*, UnicodeHasherFunction> functions;
        std::unordered_map<llvm::Function*, unsigned int> vtableFuncIndexes;
        llvm::Value *vtable;
    } TypeDefinition;

    /* Maps DST::TypeDeclarations to useful codeGen info */
    static std::unordered_map<DST::TypeDeclaration*, TypeDefinition*> _types;

    typedef struct NamespaceMembers {
        std::unordered_map<unicode_string, llvm::Value*, UnicodeHasherFunction> values;
        std::unordered_map<unicode_string, TypeDefinition*, UnicodeHasherFunction> types;
        std::unordered_map<unicode_string, NamespaceMembers*, UnicodeHasherFunction> namespaces;
        DST::NamespaceDeclaration *decl = NULL;
    } NamespaceMembers;

    /* Maps a namespace's id to all values, types, and namespaces contained in it */
    static std::unordered_map<unicode_string, NamespaceMembers*, UnicodeHasherFunction> _namespaces;

    /* Are we compiling a library or a regular file? */
    static bool _isLib;

    /* Needed for any codeGen */
    static llvm::LLVMContext _context;
    
    /* The module we're generating IR to */
    static std::unique_ptr<llvm::Module> _module(new llvm::Module("test", _context));

    /* Makes IR generation much simpler */
    static llvm::IRBuilder<> _builder(_context);

    /* Used to find allocation sizes of types */
    static llvm::DataLayout *_dataLayout(new llvm::DataLayout(_module.get()));

    /* All the values existing in the local scope */
    static std::unordered_map<std::string, Value*> _namedValues;

    /* Since multi-return functions are not supported in LLVM we have to return them by reference (through arguments) */
    static vector<Value*> _funcReturns;

    /*
        %.interface_type = type { i8*, %.vtable_type* }
        First member is a ptr to the object, second is a ptr to the object's vtable.
    */
    static llvm::StructType *_interfaceType;

    /*
        %.vtable_type = type { i32, %.interface_vtable* }
        Fist member is the type's unique ID.
        Second member is an array containing a vtable corresponding to each interface the type implements.
        Each interface's vtable contains that interface's ID and all the function the type implements for that interface.
    */
    static llvm::StructType *_objVtableType;

    /*
        %.interface_vtable = type { i32, i8** }
        First value is the interface's unique ID, second is an array of functions.
    */
    static llvm::StructType *_interfaceVtableType;

    /* Maps all types to their vtables */
    static unordered_map<llvm::Type*, llvm::Value*> _vtables;

    /* The global variable in we store a pointer to an exception we are throwing */
    static llvm::GlobalVariable *_globCaughtErr;

    static llvm::BasicBlock *_currCatchBlock = NULL;

    /* The block we should jump to should we hit a 'break' statement */
    static llvm::BasicBlock *_currBreakJmp = NULL;

    /* The block we should jump to should we hit a 'continue' statement */
    static llvm::BasicBlock *_currContinueJmp = NULL;

    /* A struct type representing a string, defined in Bootstrap.dino */
    static llvm::StructType *_stringTy = NULL;

    /* A struct type representing a Null Pointer Error */
    static llvm::StructType *_nullPtrErrorTy = NULL;

    /* The namespaces we are currently in */
    static std::vector<NamespaceMembers*> _currentNamespace;
}