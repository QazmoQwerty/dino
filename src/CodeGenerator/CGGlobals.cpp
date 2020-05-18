/*
    Definitions for all the globals declared in "CodeGenContext.h"
*/
#include "CodeGenerator.h"

namespace CodeGenerator 
{
    /* Maps vtable function index information to interface's member's name. */
    unordered_map<DST::InterfaceDeclaration*, unordered_map<unicode_string, InterfaceFuncInfo, UnicodeHasherFunction>> _interfaceVtableFuncInfo;

    /* Maps DST::TypeDeclarations to useful codeGen info */
    unordered_map<DST::TypeDeclaration*, TypeDefinition*> _types;

    /* Maps a namespace's id to all values, types, and namespaces contained in it */
    // unordered_map<unicode_string, NamespaceMembers*, UnicodeHasherFunction> _namespaces;

    /* Maps DST::NamespaceDeclarations to useful codeGen info */
    unordered_map<DST::NamespaceDeclaration*, NamespaceMembers*> _namespaces;

    /* Are we compiling a library or a regular file? */
    bool _isLib;

    /* Turn off garbage collection */
    bool _noGC = false;

    /* turn on emission of debug information */
    bool _emitDebugInfo = false;

    /* Needed for any codeGen */
    llvm::LLVMContext _context;
    
    /* The module we're generating IR to */
    std::unique_ptr<llvm::Module> _module(new llvm::Module("test", _context));

    /* Makes IR generation much simpler */
    llvm::IRBuilder<> _builder(_context);

    /* Used to find allocation sizes of types */
    llvm::DataLayout *_dataLayout(new llvm::DataLayout(_module.get()));

    /* All the values existing in the local scope */
    stack<unordered_map<std::string, Value*>> _namedValues;

    /*
        %.interface_type = type { i8*, %.vtable_type* }
        First member is a ptr to the object, second is a ptr to the object's vtable.
    */
    llvm::StructType *_interfaceType;

    /*
        %.vtable_type = type { i32, %.interface_vtable* }
        Fist member is the type's unique ID.
        Second member is an array containing a vtable corresponding to each interface the type implements.
        Each interface's vtable contains that interface's ID and all the function the type implements for that interface.
    */
    llvm::StructType *_objVtableType;

    /*
        %.interface_vtable = type { i32, i8** }
        First value is the interface's unique ID, second is an array of functions.
    */
    llvm::StructType *_interfaceVtableType;

    /* Maps all types to their vtables */
    unordered_map<DST::Type*, llvm::Value*> _vtables;

    /* The global variable in we store a pointer to an exception we are throwing */
    llvm::GlobalVariable *_globCaughtErr;

    llvm::BasicBlock *_currCatchBlock = NULL;

    /* The block we should jump to should we hit a 'break' statement */
    llvm::BasicBlock *_currBreakJmp = NULL;

    /* The block we should jump to should we hit a 'continue' statement */
    llvm::BasicBlock *_currContinueJmp = NULL;

    /* A struct type representing a string, defined in Bootstrap.dino */
    llvm::StructType *_stringTy = NULL;

    /* A struct type representing a Null Pointer Error */
    llvm::StructType *_nullPtrErrorTy = NULL;

    /* The namespaces we are currently in */
    std::vector<NamespaceMembers*> _currentNamespace;

    /* Helper for debug info generation */
    llvm::DIBuilder *_dbuilder = NULL;

    /* IDK what this does yet D:' */
    llvm::DICompileUnit *_compileUnit = NULL;
}