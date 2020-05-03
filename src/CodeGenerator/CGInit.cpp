/*
    This file contains the main functions of the code generator - the ones that call the entire codeGen process.
    This includes the main Code Generator's entry point: startCodeGen()
*/
#include "CodeGenerator.h"

void CodeGenerator::setup(bool isLib, bool noGC)
{
    _isLib = isLib;
    _noGC = noGC;

    _namedValues.push({});

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    // _dbuilder = new llvm::DIBuilder(*_module);
    // TODO!
    // _compileUnit = _dbuilder->createCompileUnit(llvm::dwarf::DW_LANG_C, _dbuilder->createFile("fib.ks", "."),"Kaleidoscope Compiler", 0, "", 0);

    // @.interface_vtable = type { i32, i8** } (interface id, array of function pointers)
    _interfaceVtableType = llvm::StructType::create(_context, { _builder.getInt32Ty(), _builder.getInt8Ty()->getPointerTo()->getPointerTo() }, ".interface_vtable");

    // @.vtable_type = type { i32, @.interface_vtable* } (interface count, array of vtables for each interface)
    _objVtableType = llvm::StructType::create(_context, { _builder.getInt32Ty(), _interfaceVtableType->getPointerTo() }, ".vtable_type");

    // @.interface_type = type { i8*, @.vtable_type* } (object ptr, vtable ptr)
    _interfaceType = llvm::StructType::create(_context, { _builder.getInt8Ty()->getPointerTo(), _objVtableType->getPointerTo() }, ".interface_type");
}

// Returns a pointer to the Main function
llvm::Function *CodeGenerator::startCodeGen(DST::Program *node) 
{
    llvm::Function *ret = NULL;

    for (auto i : node->getNamespaces())
    {
        auto currentNs = _namespaces[i.first] = new NamespaceMembers();
        currentNs->decl = i.second;
        _currentNamespace.push_back(currentNs);
        declareNamespaceTypes(i.second);
        _currentNamespace.pop_back();
    }

    for (auto i : node->getNamespaces())
    {
        _currentNamespace.push_back(_namespaces[i.first]);
        if (auto var = declareNamespaceMembers(i.second))
            ret = var;
        _currentNamespace.pop_back();
    }
    
    for (auto i : node->getNamespaces())    // Needs to be after declaring interfaces because vtable indexing happends then
    {
        _currentNamespace.push_back(_namespaces[i.first]);
        declareNamespaceTypesContent(i.second);
        _currentNamespace.pop_back();
    }

    for (auto i : node->getNamespaces())
    {
        _currentNamespace.push_back(_namespaces[i.first]);
        defineNamespaceMembers(i.second);
        _currentNamespace.pop_back();
    }
    return ret;
}

void CodeGenerator::writeBitcodeToFile(DST::Program *prog, string fileName) 
{
    llvm::Linker linker(*_module.get());
    for (string s : prog->getBcFileImports())
    {
        llvm::SMDiagnostic err;
        auto m = llvm::parseIRFile(s, err, _context);
        linker.linkInModule(std::move(m));
    }

    fstream file(fileName);
    std::error_code ec;
    llvm::raw_fd_ostream out(fileName, ec, llvm::sys::fs::F_None);
    llvm::WriteBitcodeToFile(_module.get(), out);
}

void CodeGenerator::declareNamespaceTypes(DST::NamespaceDeclaration *node)
{
    for (auto p : node->getMembers())
    {
        auto member = p.second.first;
        switch (member->getStatementType())
        {
            case ST_NAMESPACE_DECLARATION:
            {
                auto currentNs = _currentNamespace.back()->namespaces[p.first] = new NamespaceMembers();
                currentNs->decl = node;
                _currentNamespace.push_back(currentNs);
                declareNamespaceTypesContent((DST::NamespaceDeclaration*)member);
                _currentNamespace.pop_back();
                break;
            }
            case ST_TYPE_DECLARATION:
                declareType((DST::TypeDeclaration*)member);
                break;
            default: break;
        }
    }
}

void CodeGenerator::declareNamespaceTypesContent(DST::NamespaceDeclaration *node)
{
    for (auto p : node->getMembers())
    {
        auto member = p.second.first;
        switch (member->getStatementType())
        {
            case ST_NAMESPACE_DECLARATION:
            {
                auto currentNs = _currentNamespace.back()->namespaces[p.first] = new NamespaceMembers();
                currentNs->decl = node;
                _currentNamespace.push_back(currentNs);
                declareNamespaceTypesContent((DST::NamespaceDeclaration*)member);
                _currentNamespace.pop_back();
                break;
            }
            case ST_TYPE_DECLARATION:
                declareTypeContent((DST::TypeDeclaration*)member);
                break;
            default: break;
        }
    }
}

llvm::Function * CodeGenerator::declareNamespaceMembers(DST::NamespaceDeclaration *node)
{
    llvm::Function *ret = NULL;
    for (auto p : node->getMembers())
    {
        auto member = p.second.first;
        switch (member->getStatementType())
        {
            case ST_NAMESPACE_DECLARATION:
            {
                auto currentNs = _currentNamespace.back()->namespaces[p.first];
                _currentNamespace.push_back(currentNs);
                if (auto var = declareNamespaceMembers((DST::NamespaceDeclaration*)member))
                    ret = var;
                _currentNamespace.pop_back();
                break;
            }
            case ST_INTERFACE_DECLARATION:
                declareInterfaceMembers((DST::InterfaceDeclaration*)member);
                break;
            case ST_FUNCTION_DECLARATION:
                if (((DST::FunctionDeclaration*)member)->getVarDecl()->getVarId().to_string() == "Main")
                    ret = declareFunction((DST::FunctionDeclaration*)member);
                else declareFunction((DST::FunctionDeclaration*)member);
                break;
            case ST_PROPERTY_DECLARATION:
                declareProperty((DST::PropertyDeclaration*)member);
                break;
            case ST_VARIABLE_DECLARATION:
                createGlobalVariable((DST::VariableDeclaration*)member);
                break;
            case ST_CONST_DECLARATION:
            {
                auto decl = (DST::ConstDeclaration*)member;
                _currentNamespace.back()->values[decl->getName()] = codeGen(decl->getExpression());
                break;
            }
            default: break;
        }
    }
    return ret;
}

void CodeGenerator::defineNamespaceMembers(DST::NamespaceDeclaration *node)
{
    for (auto p : node->getMembers())
    {
        auto member = p.second.first;
        switch (member->getStatementType())
        {
            case ST_NAMESPACE_DECLARATION:
                _currentNamespace.push_back(_currentNamespace.back()->namespaces[p.first]);
                defineNamespaceMembers((DST::NamespaceDeclaration*)member);
                _currentNamespace.pop_back();
                break;
            case ST_PROPERTY_DECLARATION:
                codegenProperty((DST::PropertyDeclaration*)member);
                break;
            case ST_FUNCTION_DECLARATION:
                codegenFunction((DST::FunctionDeclaration*)member);
                break;
            case ST_TYPE_DECLARATION:
                codegenTypeMembers((DST::TypeDeclaration*)member);
                break;
            default: break;
        }
    }
}

void CodeGenerator::declareInterfaceMembers(DST::InterfaceDeclaration *node)
{
    auto &vtableIndexes = _interfaceVtableFuncInfo[node];
    int idx = 0;

    for (auto i : node->getDeclarations())
    {
        auto &name = i.first;
        auto stmnt = i.second.first;
        if (stmnt->getStatementType() == ST_FUNCTION_DECLARATION)
        {
            auto decl = (DST::FunctionDeclaration*)stmnt;
            vtableIndexes[name].index = idx++;
            vtableIndexes[name].type = getInterfaceFuncType(decl);
        }
        else if (stmnt->getStatementType() == ST_PROPERTY_DECLARATION)
        {
            auto decl = (DST::PropertyDeclaration*)stmnt;
            
            if (decl->getReturnType()->as<DST::PropertyType>()->hasGet())
            {
                auto funcId = name;
                funcId += ".get";
                vtableIndexes[funcId].index = idx++;
                vtableIndexes[funcId].type = llvm::FunctionType::get(evalType(decl->getReturnType()), _builder.getInt8PtrTy(), false);
            }
            if (decl->getReturnType()->as<DST::PropertyType>()->hasSet())
            {
                auto funcId = name;
                funcId += ".set";
                vtableIndexes[funcId].index = idx++;
                vtableIndexes[funcId].type = llvm::FunctionType::get(_builder.getVoidTy(), { _builder.getInt8PtrTy(), evalType(decl->getReturnType()) }, false);
            }
        }
        else UNREACHABLE
    }
}

void CodeGenerator::declareType(DST::TypeDeclaration *node)
{
    auto def = new TypeDefinition();

    // Todo - add file name as well
    string typeName = "type.";
    for (auto i : _currentNamespace)
        typeName += i->decl->getName().to_string() + ".";
    typeName += node->getName().to_string();

    def->structType = llvm::StructType::create(_context, typeName);

    if (!_nullPtrErrorTy && typeName == "type.Std.NullPointerError")
        _nullPtrErrorTy = def->structType;

    if (!_stringTy && typeName == "type.Std.String")
        _stringTy = def->structType;

    _types[node] = _currentNamespace.back()->types[node->getName()] = def;
}

void CodeGenerator::declareTypeContent(DST::TypeDeclaration *node)
{
    auto def = _types[node];

    std::vector<llvm::Type*> members;
    int count = 0;

    unordered_map<DST::InterfaceDeclaration*, vector<llvm::Function*>> vtable;

    for (auto i : node->getInterfaces())
        vtable[i].resize(_interfaceVtableFuncInfo[i].size());

    for (auto i : node->getMembers())
    {
        if (i.second.first) switch (i.second.first->getStatementType())
        {
            case ST_VARIABLE_DECLARATION:
            {
                auto decl = (DST::VariableDeclaration*)i.second.first;
                def->variableIndexes[decl->getVarId()] = count++;
                members.push_back(evalType(decl->getType()));
                break;
            }
            case ST_FUNCTION_DECLARATION:
            {
                auto decl = (DST::FunctionDeclaration*)i.second.first;
                auto func = declareFunction(decl, def);
                auto inter = getFunctionInterface(node, decl);
                if (inter) 
                {
                    auto idx = _interfaceVtableFuncInfo[inter][i.first].index;
                    vtable[inter][idx] = func;
                }
                break;
            }
            case ST_PROPERTY_DECLARATION:
            {
                auto decl = (DST::PropertyDeclaration*)i.second.first;
                auto funcs = declareProperty(decl, def);
                auto inter = getPropertyInterface(node, decl);
                if (inter) 
                {
                    auto &vec = vtable[inter];
                    if (funcs.first)
                    {
                        auto setFuncName = i.first;
                        setFuncName += ".set";
                        auto idx = _interfaceVtableFuncInfo[inter][setFuncName].index;
                        vec[idx] = funcs.first;
                    }
                    if (funcs.second)
                    {
                        auto getFuncName = i.first;
                        getFuncName += ".get";
                        auto idx = _interfaceVtableFuncInfo[inter][getFuncName].index;
                        vec[idx] = funcs.second;
                    }
                }
                break;
            }
            default: break;
        }
    }
    def->structType->setBody(members);

    vector<llvm::Constant*> interfaceVtables;

    for (auto i : vtable)
    {
        vector<llvm::Constant*> vec;
        auto interfaceId = _builder.getInt32((unsigned long int)i.first);
        
        for (auto j : i.second)
        {
            auto cast = _builder.CreateBitCast(j, _builder.getInt8PtrTy());
            vec.push_back((llvm::Constant*)cast);
        }
        
        auto arrTy = llvm::ArrayType::get(_builder.getInt8Ty()->getPointerTo(), vec.size());
        auto arr = llvm::ConstantArray::get(arrTy, vec);

        auto arrPtr = new llvm::GlobalVariable(*_module, arr->getType(), true, llvm::GlobalVariable::PrivateLinkage, 
                                            arr, node->getName().to_string() + "." + i.first->getName().to_string() + ".vtable.arr");

        auto arrCast = (llvm::Constant*)_builder.CreateBitCast(arrPtr, _builder.getInt8PtrTy()->getPointerTo());
        auto interfaceVtable = llvm::ConstantStruct::get(_interfaceVtableType, { interfaceId, arrCast });
        interfaceVtables.push_back(interfaceVtable);
    }

    llvm::Constant *llvmVtable = NULL;

    if (interfaceVtables.size() == 0)
        llvmVtable = llvm::ConstantStruct::get(_objVtableType, { _builder.getInt32(0), _builder.getInt32(0) });
    else 
    {        
        auto numInterfaces = _builder.getInt32(interfaceVtables.size());
        auto arrTy = llvm::ArrayType::get(_interfaceVtableType, interfaceVtables.size());
        auto arr = llvm::ConstantArray::get(arrTy, interfaceVtables);

        auto arrPtr = new llvm::GlobalVariable(*_module, arr->getType(), true, llvm::GlobalVariable::PrivateLinkage, 
                                            arr, node->getName().to_string() + ".interfacesArr");

        auto arrCast = (llvm::Constant*)_builder.CreateBitCast(arrPtr, _interfaceVtableType->getPointerTo());
        llvmVtable = llvm::ConstantStruct::get(_objVtableType, { numInterfaces, arrCast });
    }
    def->vtable = new llvm::GlobalVariable(*_module, llvmVtable->getType(), true, llvm::GlobalVariable::PrivateLinkage, 
                                            llvmVtable, node->getName().to_string() + ".vtable");
    _vtables[DST::BasicType::get(node)] = def->vtable;
}

void CodeGenerator::codegenTypeMembers(DST::TypeDeclaration *node)
{
    auto def = _types[node];
    for (auto i : node->getMembers())
    {
        if (i.second.first) switch (i.second.first->getStatementType())
        {
            case ST_FUNCTION_DECLARATION:
                codegenFunction((DST::FunctionDeclaration*)i.second.first, def);
                break;
            case ST_PROPERTY_DECLARATION:
                codegenProperty((DST::PropertyDeclaration*)i.second.first, def);
                break;
            default: break;
        }
    }
}

// first return value is the 'set' property, second is 'get'
std::pair<llvm::Function*, llvm::Function*> CodeGenerator::declareProperty(DST::PropertyDeclaration *node, CodeGenerator::TypeDefinition *typeDef)
{
    auto propType = evalType(node->getReturnType());
    if (propType->isVoidTy())
        throw ErrorReporter::report("Property type may not be \"void\"", ERR_CODEGEN, node->getPosition());

    auto ret = std::make_pair<llvm::Function*, llvm::Function*>(NULL, NULL);

    if (node->getSet())
    {
        vector<llvm::Type*> setParams;
        if (typeDef)
            setParams.push_back(typeDef->structType->getPointerTo());
        setParams.push_back(propType);  // Add "value" parameter to set function
        auto setFuncType = llvm::FunctionType::get(llvm::Type::getVoidTy(_context), setParams, false);
        unicode_string setFuncName = node->getName();
        setFuncName += ".set";

        string llvmFuncId = "";

        if (node->getSet()->getStatements().size() == 1 && node->getSet()->getStatements()[0]->getStatementType() == ST_UNARY_OPERATION
            && ((DST::UnaryOperationStatement*)node->getSet()->getStatements()[0])->getOperator()._type == OT_EXTERN
            && ((DST::UnaryOperationStatement*)node->getSet()->getStatements()[0])->getExpression() != NULL)
        {
            // externally defined function with a func name argument
            auto opStmnt = ((DST::UnaryOperationStatement*)node->getSet()->getStatements()[0]);
            if (opStmnt->getExpression()->getExpressionType() != ET_LITERAL)
                UNREACHABLE
            if (((DST::Literal*)opStmnt->getExpression())->getBase()->getLiteralType() != LT_STRING)
                UNREACHABLE
            auto strlit = (AST::String*)((DST::Literal*)opStmnt->getExpression())->getBase();
            llvmFuncId = strlit->getValue();
        }
        else 
        {
            for (auto i : _currentNamespace)
                llvmFuncId += i->decl->getName().to_string() + ".";
            if (typeDef) llvmFuncId += typeDef->structType->getName().str() + ".";
            llvmFuncId += setFuncName.to_string();
        }


        llvm::Function *setFunc = llvm::Function::Create(setFuncType, llvm::Function::ExternalLinkage, llvmFuncId, _module.get());
        _currentNamespace.back()->values[setFuncName] = setFunc;
        bool b = true;
        for (auto &arg : setFunc->args())
        {
            if (typeDef && b) 
            {
                b = false;
                arg.setName("this");    
            }
            else arg.setName("value");
        }     
        if (typeDef)
            typeDef->functions[setFuncName] = setFunc;
        node->_llvmSetFuncId = setFunc->getName();
        ret.first = setFunc;
    }

    if (node->getGet())
    {
        vector<llvm::Type*> getParams;
        if (typeDef)
            getParams.push_back(typeDef->structType->getPointerTo());
        auto getFuncType = llvm::FunctionType::get(propType, getParams, false);
        unicode_string getFuncName = node->getName();
        getFuncName += ".get";

        string llvmFuncId = ""; // Todo - add file name as well

        if (node->getGet()->getStatements().size() == 1 && node->getGet()->getStatements()[0]->getStatementType() == ST_UNARY_OPERATION
            && ((DST::UnaryOperationStatement*)node->getGet()->getStatements()[0])->getOperator()._type == OT_EXTERN
            && ((DST::UnaryOperationStatement*)node->getGet()->getStatements()[0])->getExpression() != NULL)
        {
            // externally defined function with a func name argument
            auto opStmnt = ((DST::UnaryOperationStatement*)node->getGet()->getStatements()[0]);
            if (opStmnt->getExpression()->getExpressionType() != ET_LITERAL)
                UNREACHABLE
            if (((DST::Literal*)opStmnt->getExpression())->getBase()->getLiteralType() != LT_STRING)
                UNREACHABLE
            auto strlit = (AST::String*)((DST::Literal*)opStmnt->getExpression())->getBase();
            llvmFuncId = strlit->getValue();
        }
        else 
        {
            for (auto i : _currentNamespace)
                llvmFuncId += i->decl->getName().to_string() + ".";
            if (typeDef) llvmFuncId += typeDef->structType->getName().str() + ".";
            llvmFuncId += getFuncName.to_string();
        }

        llvm::Function *getFunc = llvm::Function::Create(getFuncType, llvm::Function::ExternalLinkage, llvmFuncId, _module.get());
        _currentNamespace.back()->values[getFuncName] = getFunc;
        for (auto &arg : getFunc->args())
            arg.setName("this");
        if (typeDef)
            typeDef->functions[getFuncName] = getFunc;
        node->_llvmGetFuncId = getFunc->getName();
        ret.second = getFunc;
    }
    return ret;
}

void CodeGenerator::codegenProperty(DST::PropertyDeclaration *node, TypeDefinition *typeDef)
{
    if (node->getGet())
    {
        if (node->getGet()->getStatements().size() == 1 && node->getGet()->getStatements()[0]->getStatementType() == ST_UNARY_OPERATION
        && ((DST::UnaryOperationStatement*)node->getGet()->getStatements()[0])->getOperator()._type == OT_EXTERN)
        {
            // externally defined function
            return;
        }

        unicode_string getFuncName = node->getName();
        getFuncName += ".get";
        llvm::Value *getFuncPtr = NULL;
        if (typeDef)
            getFuncPtr = typeDef->functions[getFuncName];
        else getFuncPtr = _currentNamespace.back()->values[getFuncName];
        llvm::Function *getFunc = NULL;
        if (isFunc(getFuncPtr))
            getFunc = (llvm::Function*)getFuncPtr;
        else throw ErrorReporter::report("\"" + getFuncName.to_string() + "\" is not a function", ERR_CODEGEN, node->getPosition());

        // Create a new basic block to start insertion into.
        llvm::BasicBlock *bb = llvm::BasicBlock::Create(_context, "entry", getFunc);
        _builder.SetInsertPoint(bb);

        if (typeDef)
        {
            for (llvm::Argument &arg : getFunc->args()) // create entry block alloca for 'this' ptr
            {
                AllocaInst *alloca = CreateEntryBlockAlloca(getFunc, arg.getType(), arg.getName());    // Create an alloca for this variable.
                _builder.CreateStore(&arg, alloca);     // Store the initial value into the alloca.
                _namedValues.top()[arg.getName()] = alloca;   // Add arguments to variable symbol table.
            }
        }

        for (auto i : node->getGet()->getStatements()) 
        {
            auto val = codeGen(i);
            if (val == nullptr)
                throw ErrorReporter::report("Error while generating IR for statement", ERR_CODEGEN, i->getPosition());
        }
        if (!_builder.GetInsertBlock()->getTerminator())
            _builder.CreateRetVoid();
        llvm::verifyFunction(*getFunc, &llvm::errs());

        if (!_builder.GetInsertBlock()->getTerminator())
            _builder.CreateRetVoid();
    }

    if (node->getSet())
    {
        if (node->getSet()->getStatements().size() == 1 && node->getSet()->getStatements()[0]->getStatementType() == ST_UNARY_OPERATION
        && ((DST::UnaryOperationStatement*)node->getSet()->getStatements()[0])->getOperator()._type == OT_EXTERN)
        {
            // externally defined function
            return;
        }

        unicode_string setFuncName = node->getName();
        setFuncName += ".set";
        llvm::Value *setFuncPtr = NULL;
        if (typeDef)
            setFuncPtr = typeDef->functions[setFuncName];
        else setFuncPtr = _currentNamespace.back()->values[setFuncName];
        llvm::Function *setFunc = NULL;
        if (isFunc(setFuncPtr))
            setFunc = (llvm::Function*)setFuncPtr;
        else throw ErrorReporter::report("\"" + setFuncName.to_string() + "\" is not a function", ERR_CODEGEN, node->getPosition());

        // Create a new basic block to start insertion into.
        llvm::BasicBlock *bb = llvm::BasicBlock::Create(_context, "entry", setFunc);
        _builder.SetInsertPoint(bb);

        // Record the function arguments in the NamedValues map.
        _namedValues.push({});
        bool isFirst = false;
        for (llvm::Argument &arg : setFunc->args())
        {
            AllocaInst *alloca = CreateEntryBlockAlloca(setFunc, arg.getType(), arg.getName());    // Create an alloca for this variable.
            _builder.CreateStore(&arg, alloca);     // Store the initial value into the alloca.
            _namedValues.top()[arg.getName()] = alloca;   // Add arguments to variable symbol table.
            isFirst = false;
        }

        for (auto i : node->getSet()->getStatements()) 
        {
            auto val = codeGen(i);
            if (val == nullptr)
                throw ErrorReporter::report("Error while generating IR for statement", ERR_CODEGEN, i->getPosition());
        }
        if (!_builder.GetInsertBlock()->getTerminator())
            _builder.CreateRetVoid();
        llvm::verifyFunction(*setFunc, &llvm::errs());
        _namedValues.pop(); // leave block
    }
}

llvm::Function * CodeGenerator::declareFunction(DST::FunctionDeclaration *node, CodeGenerator::TypeDefinition *typeDef)
{
    vector<llvm::Type*> types;
    if (typeDef)
        types.push_back(typeDef->structType->getPointerTo());
    auto returnType = evalType(node->getReturnType());

    auto params = node->getParameters();
    for (auto i : params) 
        types.push_back(evalType(i->getType()));

    auto funcType = llvm::FunctionType::get(returnType, types, false);

    string funcId = "";

    if (node->getContent()->getStatements().size() == 1 && node->getContent()->getStatements()[0]->getStatementType() == ST_UNARY_OPERATION
        && ((DST::UnaryOperationStatement*)node->getContent()->getStatements()[0])->getOperator()._type == OT_EXTERN
        && ((DST::UnaryOperationStatement*)node->getContent()->getStatements()[0])->getExpression() != NULL)
    {
        // externally defined function with a func name argument
        auto opStmnt = ((DST::UnaryOperationStatement*)node->getContent()->getStatements()[0]);

        ASSERT(opStmnt->getExpression()->getExpressionType() == ET_LITERAL);
        ASSERT(((DST::Literal*)opStmnt->getExpression())->getBase()->getLiteralType() == LT_STRING);

        auto strlit = (AST::String*)((DST::Literal*)opStmnt->getExpression())->getBase();
        funcId = strlit->getValue();
    }
    else if (node->getVarDecl()->getVarId().to_string() == "Main")
        funcId = "main";
    else 
    {
        // TODO - add file name as well
        for (auto i : _currentNamespace)
            funcId += i->decl->getName().to_string() + ".";
        if (typeDef) funcId += typeDef->structType->getName().str() + ".";
        funcId += node->getVarDecl()->getVarId().to_string();
    }

    llvm::Function *func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, funcId, _module.get());
    node->_llvmFuncId = func->getName();

    // Set names for all arguments.
    unsigned idx = 0;
    bool b = true;
    for (auto &arg : func->args())
    {
        if (idx == 0 && typeDef != nullptr && b)
        {
            arg.setName("this");
            b = false;
        }
        else arg.setName(params[idx++]->getVarId().to_string());
    }

    _currentNamespace.back()->values[node->getVarDecl()->getVarId()] = func;
    if (typeDef)
        typeDef->functions[node->getVarDecl()->getVarId()] = func;
    return func;
}

void CodeGenerator::codegenFunction(DST::FunctionDeclaration *node, CodeGenerator::TypeDefinition *typeDef)
{
    llvm::Value *funcPtr = NULL;
    if (typeDef)
        funcPtr = typeDef->functions[node->getVarDecl()->getVarId()];
    else funcPtr = _currentNamespace.back()->values[node->getVarDecl()->getVarId()];
    llvm::Function *func = NULL;
    if (isFunc(funcPtr))
        func = (llvm::Function*)funcPtr;
    else throw ErrorReporter::report("\"" + node->getVarDecl()->getVarId().to_string() + "\" is not a function", ERR_CODEGEN, node->getPosition());

    if (node->getContent() == NULL)
        throw ErrorReporter::report("Undefined function", ERR_CODEGEN, node->getPosition());

    if (node->getContent()->getStatements().size() == 1 && node->getContent()->getStatements()[0]->getStatementType() == ST_UNARY_OPERATION
        && ((DST::UnaryOperationStatement*)node->getContent()->getStatements()[0])->getOperator()._type == OT_EXTERN)
    {
        // externally defined function
        llvm::verifyFunction(*func, &llvm::errs());
        return;
    }


    auto params = node->getParameters();

    // Create a new basic block to start insertion into.
    llvm::BasicBlock *bb = llvm::BasicBlock::Create(_context, "entry", func);
    _builder.SetInsertPoint(bb);

    // Record the function arguments in the NamedValues map.
    _namedValues.push({});
    bool isFirst = true;
    for (llvm::Argument &arg : func->args())
    {
        AllocaInst *alloca = CreateEntryBlockAlloca(func, arg.getType(), arg.getName());    // Create an alloca for this variable.
        _builder.CreateStore(&arg, alloca);     // Store the initial value into the alloca.
        _namedValues.top()[arg.getName()] = alloca;   // Add arguments to variable symbol table.
        isFirst = false;
    }

    for (auto i : node->getContent()->getStatements()) 
    {
        auto val = codeGen(i);
        if (val == nullptr)
            throw ErrorReporter::report("Error while generating IR for statement", ERR_CODEGEN, i->getPosition());
    }

    if (!_builder.GetInsertBlock()->getTerminator())
        _builder.CreateRetVoid();
    
    llvm::verifyFunction(*func, &llvm::errs());
    _namedValues.pop(); // leave block
}