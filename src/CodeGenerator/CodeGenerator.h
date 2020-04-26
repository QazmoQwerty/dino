/*
    The Code Generator gets a DST from the decorator (see Decorator.h for more info) and 
    emmits LLVM IR based on it. This IR is then compiles by LLVM into an executable.
*/
#pragma once

#include "CodeGenContext.h"

#define CONDITION_TYPE unicode_string("bool")

namespace CodeGenerator 
{ 
    /*
        Initializes LLVM and some variables.
        If the optional flag isLib is activated the code generator will output a library directory.
        IMPORTANT: Function MUST be called once before using any other functions of this namespace.
	*/
    void setup(bool isLib = false, bool noGC = false);

    /* 
        Main entry point to the Code Generator, invokes the entire code generation process.
        Returns a pointer to the Main function (this feature can be removed since we're not executing the IR anymore). 
    */
    llvm::Function *startCodeGen(DST::Program *node);

    /* 
        Handles linkage of imports + writes the Module to a .bc file 
    */
    void writeBitcodeToFile(DST::Program *prog, string fileName);

    /********************************************************************************
    *                    HERE ENDS THE EXTERNAL CODEGENERATOR API                   *
    *                       the rest is implementation details.                     *
    ********************************************************************************/

    /* 
        Recursively declares opaque llvm structs for every type.
        Calls declareType().
    */
    void declareNamespaceTypes(DST::NamespaceDeclaration *node);

    /* Declare an opaque struct type */
    void declareType(DST::TypeDeclaration *node);

    /* 
        Recursively declares headers for all non-type members of the namespace.
        Calls: declareInterfaceMembers()
               declareFunction()
               declareProperty()
    */
    llvm::Function * declareNamespaceMembers(DST::NamespaceDeclaration *node);

    /* Creates vtable information for an interface */
    void declareInterfaceMembers(DST::InterfaceDeclaration *node);

    /* 
        Declares a function and sets its parameter/return types 
        If typeDef is set the function will declare a member function of said type.
    */
    llvm::Function * declareFunction(DST::FunctionDeclaration *node, TypeDefinition *typeDef = NULL);

    /* 
        Declares functions for getters/setters and sets its parameter/return types. 
        If typeDef is set the function will declare the properties as members of said type.
        Returns the functions as a pair: first is the setter and second is the getter.
    */
    std::pair<llvm::Function*, llvm::Function*> declareProperty(DST::PropertyDeclaration *node, TypeDefinition *typeDef = NULL);

    /* 
        Recursively declares headers for all types of the namespace.
        Calls: declareTypeContent().
    */
    void declareNamespaceTypesContent(DST::NamespaceDeclaration *node);
    
    /* 
        - Declares headers for all member functions of a type.
        - Sets the llvm struct type to a valid non-opaque struct.
        - Creates a vtable for the type.
    */
    void declareTypeContent(DST::TypeDeclaration *node);

    /* 
        Generates code for function/property/type contents of all members of the namespace.
        Calls: codegenTypeMembers()
               codegenFunction()
               codegenProperty()
    */
    void defineNamespaceMembers(DST::NamespaceDeclaration *node);

    /*
        Generates code for function/property contents of the type.
        Calls: codegenFunction()
               codegenProperty()
    */
    void codegenTypeMembers(DST::TypeDeclaration *node);

    /*
        Generates code for contents of a function.
        If typeDef is set the function will be generated a member of said type.
    */
    void codegenFunction(DST::FunctionDeclaration *node, TypeDefinition *typeDef = NULL);

    /*
        Generates code for contents of a property.
        If typeDef is set the properties will be generated a member of said type.
    */  
    void codegenProperty(DST::PropertyDeclaration *node, TypeDefinition *typeDef = NULL);

    /************************************************************************
    *                               Interfaces                              *
    ************************************************************************/

    /* Boxes a pointer value into an interface value */
    llvm::Value* convertToInterface(llvm::Value *ptr);

    /* Boxes a pointer value into an interface alloca */
    llvm::Value* convertToInterfaceLval(llvm::Value *ptr);

    /* Finds which interface a property belongs to. */
    DST::InterfaceDeclaration *getPropertyInterface(DST::TypeDeclaration *typeDecl, DST::PropertyDeclaration *propDecl);

    /* Finds which interface a property belongs to. */
    DST::InterfaceDeclaration *getPropertyInterface(DST::InterfaceDeclaration *interfaceDecl, DST::PropertyDeclaration *propDecl);

    /* Finds which interface a function belongs to. */
    DST::InterfaceDeclaration *getFunctionInterface(DST::TypeDeclaration *typeDecl, DST::FunctionDeclaration *funcDecl);

    /* Finds which interface a function belongs to. */
    DST::InterfaceDeclaration *getFunctionInterface(DST::InterfaceDeclaration *interfaceDecl, DST::FunctionDeclaration *funcDecl);

    /************************************************************************
    *                                Vtables                                *
    ************************************************************************/

    /* 
        Create an empty vtable for a type that doesn't have one.
        Needed for type checks which are basen on vtable pointers.
    */
    llvm::Value* createEmptyVtable(llvm::Type *type);

    /* Gets a vtable for a type. If one doesn't exists creates an empty vtable. */
    llvm::Value* getVtable(llvm::Type *type);

    /* 
        Gets a pointer to a vtable, a function name, and the interface where the function is declared 
        Generates code to look up the function in the vtable and returns a ptr value to the requested function.
    */
    llvm::Value *getFuncFromVtable(llvm::Value *vtable, DST::InterfaceDeclaration *interface, unicode_string &funcName);

    /* TODO - what does this function do? */
    llvm::FunctionType *getInterfaceFuncType(DST::FunctionDeclaration *node);

    /*
        IR Function which gets an object's vtable and searches it for an interface it gets as a parameter.
        It the interface is found, that interface's specific vtable is returned, otherwise the function will return null.

        define %.interface_vtable* @.getInterfaceVtable(%.vtable_type*, i32) { ... }
                                                        ^ vtable ptr    ^ interface ID
    */
    llvm::Function *getVtableInterfaceLookupFunction();

    /************************************************************************
    *                                Utility                                *
    ************************************************************************/

    /* Gets a DST::Type and returns its corresponding LLVM type */
    llvm::Type *evalType(DST::Type *node);

    /* Returns the parent function of the BasicBlock the IRBuilder is currently on */
    llvm::Function *getParentFunction();

    /* Create an alloca at the entry block of 'func'*/
    AllocaInst *CreateEntryBlockAlloca(llvm::Function *func, llvm::Type *type, const string &varName) ; 

    /* Handles creation of global variables */
    llvm::GlobalVariable *createGlobalVariable(DST::VariableDeclaration *node);

    /* Gets a value and return's whether that value's type is a function or ptr to function */
    bool isFunc(llvm::Value *funcPtr);

    /* Gets a value and return's whether that value's type is a ptr to function or ptr to ptr to function */
    bool isFuncPtr(llvm::Value *funcPtr);

    /* Utility function for dealing with namespace Member Accesses */
    NamespaceMembers *getNamespaceMembers(DST::Expression *node);

    /* 
        Creates a call instruction, or an invoke instruction if we are in a catch block.
        Use this instead of _builder.CreateCall.
        NOTE: function may only be called when within a function, otherwise it will crash.
    */
    llvm::Value *createCallOrInvoke(llvm::Value *callee, llvm::ArrayRef<llvm::Value*> args);

    /*
        Generate IR for throw statements.
        
        The 'dontInvoke' flag can be turned on in order to manually stop the 
        function from calling createCallOrInvoke() - this is needed because 
        createCallOrInvoke() causes a crash when run outside of a function.
    */
    Value *createThrow(llvm::Value *exception, bool dontInvoke = false);

    /* generate a runtime null check - if the pointer is null a runtime error will be thrown */
    llvm::Value *assertNotNull(llvm::Value *val);

    /*
        create and get an IR function similar to this:
        void assertNotNull(void@ ptr) {
            if ptr = null:
                throw new Std.NullPtrError
        }

        define void @.assertNotNull(i8*) { ... }
    */
    llvm::Function *getNullCheckFunc();

    /* Gets and returns the IR ptr to the "malloc" function for dynamic allocation */
    llvm::Function *getMallocFunc();

    /************************************************************************
    *                       codeGen() + codeGenLval()                       *
    ************************************************************************/

    /////////////////////// Code Gen Lval ///////////////////////
    // these return a pointer to a value rather than a value   //

    Value       *codeGenLval(DST::Expression            *node);
    Value       *codeGenLval(DST::Variable              *node);
    Value       *codeGenLval(DST::MemberAccess          *node);
    Value       *codeGenLval(DST::UnaryOperation        *node);
    Value       *codeGenLval(DST::BinaryOperation       *node);
    Value       *codeGenLval(DST::Conversion            *node);
    AllocaInst  *codeGenLval(DST::VariableDeclaration   *node);

    ///////////////////// Code Gen Functions /////////////////////

    llvm::Value      *codeGen(DST::Node                    *node);

    // --------------------- Expressions ---------------------- //

    llvm::Value      *codeGen(DST::Expression              *node);
    llvm::Value      *codeGen(DST::Literal                 *node);
    llvm::Value      *codeGen(DST::BinaryOperation         *node);
    llvm::Value      *codeGen(DST::UnaryOperation          *node);
    llvm::Value      *codeGen(DST::Conversion              *node);
    llvm::Value      *codeGen(DST::MemberAccess            *node);
    llvm::Value      *codeGen(DST::ArrayLiteral            *node);
    llvm::Value      *codeGen(DST::ConditionalExpression   *node);
    llvm::Value      *codeGen(DST::Variable                *node);

    // ---------------------- Statements ---------------------- //

    llvm::Value      *codeGen(DST::Statement               *node);
    llvm::BasicBlock *codeGen(DST::StatementBlock          *node, const llvm::Twine &blockName = "entry");
    llvm::Value      *codeGen(DST::ConstDeclaration        *node);
    llvm::Value      *codeGen(DST::UnaryOperationStatement *node);
    llvm::Value      *codeGen(DST::TryCatch                *node);
    llvm::Value      *codeGen(DST::SwitchCase              *node);
    llvm::Value      *codeGen(DST::IfThenElse              *node);
    llvm::Value      *codeGen(DST::WhileLoop               *node);
    llvm::Value      *codeGen(DST::DoWhileLoop             *node);
    llvm::Value      *codeGen(DST::ForLoop                 *node);

    // ----------------- ExpressionStatements ----------------- //

    llvm::AllocaInst *codeGen(DST::VariableDeclaration     *node);
    llvm::Value      *codeGen(DST::Assignment              *node);
    llvm::Value      *codeGen(DST::Increment               *node);
    llvm::Value      *codeGen(DST::FunctionCall            *node, vector<Value*> retPtrs = {});
}