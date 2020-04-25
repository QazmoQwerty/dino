#include "CodeGenerator.h"

void CodeGenerator::setup(bool isLib)
{
    _isLib = isLib;

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    /* 
        @.interface_vtable = type { i32, i8** } (interface id, array of function pointers)
        @.vtable_type = type { i32, @.interface_vtable* } (interface count, array of vtables for each interface)
        @.interface_type = type { i8*, @.vtable_type* } (object ptr, vtable ptr)    
    */

    _interfaceVtableType = llvm::StructType::create(_context, { _builder.getInt32Ty(), _builder.getInt8Ty()->getPointerTo()->getPointerTo() }, ".interface_vtable");
    _objVtableType = llvm::StructType::create(_context, { _builder.getInt32Ty(), _interfaceVtableType->getPointerTo() }, ".vtable_type");
    _interfaceType = llvm::StructType::create(_context, { _builder.getInt8Ty()->getPointerTo(), _objVtableType->getPointerTo() }, ".interface_type");

    getVtableInterfaceLookupFunction();
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
            // case ST_TYPE_DECLARATION:
            //     declareTypeContent((DST::TypeDeclaration*)member);
            //     break;
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

llvm::GlobalVariable * CodeGenerator::createGlobalVariable(DST::VariableDeclaration *node)
{
    auto type = evalType(node->getType());
    if (type->isVoidTy())
        throw ErrorReporter::report("Cannot create instance of type \"void\"", ERR_CODEGEN, node->getPosition());
    auto name = node->getVarId().to_string();
    auto glob = new llvm::GlobalVariable(*_module, type, false, llvm::GlobalVariable::CommonLinkage, llvm::Constant::getNullValue(type), name);
    _currentNamespace.back()->values[node->getVarId()] = glob;
    return glob;
}

Value *CodeGenerator::codeGen(DST::Node *node) 
{
    if (node == nullptr) 
        return nullptr;
    if (node->isExpression())
        return codeGen(dynamic_cast<DST::Expression*>(node));
    else return codeGen(dynamic_cast<DST::Statement*>(node));
}

Value *CodeGenerator::codeGen(DST::Statement *node) 
{
    if (node == nullptr) 
        return nullptr;
    switch (node->getStatementType()) 
    {
        case ST_ASSIGNMENT: return codeGen((DST::Assignment*)node);
        case ST_FUNCTION_DECLARATION: return codeGen((DST::FunctionDeclaration*)node);
        case ST_STATEMENT_BLOCK: return codeGen((DST::StatementBlock*)node);
        case ST_UNARY_OPERATION: return codeGen((DST::UnaryOperationStatement*)node);
        case ST_VARIABLE_DECLARATION: return codeGen((DST::VariableDeclaration*)node);
        case ST_CONST_DECLARATION: return codeGen((DST::ConstDeclaration*)node);
        case ST_IF_THEN_ELSE: return codeGen((DST::IfThenElse*)node);
        case ST_SWITCH: return codeGen((DST::SwitchCase*)node);
        case ST_WHILE_LOOP: return codeGen((DST::WhileLoop*)node);
        case ST_DO_WHILE_LOOP: return codeGen((DST::DoWhileLoop*)node);
        case ST_FOR_LOOP: return codeGen((DST::ForLoop*)node);
        case ST_FUNCTION_CALL: return codeGen((DST::FunctionCall*)node);
        case ST_INCREMENT: return codeGen((DST::Increment*)node);
        case ST_TRY_CATCH: return codeGen((DST::TryCatch*)node);
        default: throw ErrorReporter::report("Unimplemented codegen for statement", ERR_CODEGEN, node->getPosition());;
    }
}

Value *CodeGenerator::codeGen(DST::Expression *node) 
{
    if (node == nullptr)
        return nullptr;
    switch (node->getExpressionType()) 
    {
        case ET_BINARY_OPERATION: return codeGen((DST::BinaryOperation*)node);
        case ET_UNARY_OPERATION: return codeGen((DST::UnaryOperation*)node);
        case ET_LITERAL: return codeGen((DST::Literal*)node);
        case ET_ASSIGNMENT: return codeGen((DST::Assignment*)node);
        case ET_IDENTIFIER: return codeGen((DST::Variable*)node);
        case ET_VARIABLE_DECLARATION: return codeGen((DST::VariableDeclaration*)node);
        case ET_FUNCTION_CALL: return codeGen((DST::FunctionCall*)node);
        case ET_MEMBER_ACCESS: return codeGen((DST::MemberAccess*)node);
        case ET_ARRAY: return codeGen((DST::ArrayLiteral*)node);
        case ET_CONVERSION: return codeGen((DST::Conversion*)node);
        case ET_INCREMENT: return codeGen((DST::Increment*)node);
        case ET_CONDITIONAL_EXPRESSION: return codeGen((DST::ConditionalExpression*)node);
        default: throw ErrorReporter::report("Unimplemented codegen for expression", ERR_CODEGEN, node->getPosition());
    }
}

llvm::BasicBlock *CodeGenerator::codeGen(DST::StatementBlock *node, const llvm::Twine &blockName)
{
    // Get parent function
    auto parent = getParentFunction();

    // Create BasicBlock
    llvm::BasicBlock *bb = llvm::BasicBlock::Create(_context, blockName, parent);

    _builder.SetInsertPoint(bb);
    if (node) for (auto i : node->getStatements()) 
    {
        auto val = codeGen(i);
        if (val == nullptr)
            throw ErrorReporter::report("Error while generating ir for statementblock", ERR_CODEGEN, node->getPosition());
    }
    return bb;
}

llvm::Value *CodeGenerator::assertNotNull(llvm::Value *val)
{
    return createCallOrInvoke(getNullCheckFunc(), _builder.CreateBitCast(val, _builder.getInt8PtrTy()));
}

/*
    // get a pointer to a function like this:
    void assertNotNull(void@ ptr) {
        if ptr = null:
            throw new Std.NullPtrError
    }
*/
llvm::Function *CodeGenerator::getNullCheckFunc()
{
    static llvm::Function *func = NULL;
    if (func)
        return func;
    
    auto funcTy = llvm::FunctionType::get(_builder.getVoidTy(), _builder.getInt8PtrTy(), false);
    func = llvm::Function::Create(funcTy, llvm::Function::ExternalLinkage, ".assertNotNull", _module.get());

    if (_isLib)
        return func;

    auto savedInsertPoint = _builder.GetInsertBlock();

    llvm::BasicBlock *bb = llvm::BasicBlock::Create(_context, "entry", func);
    _builder.SetInsertPoint(bb);

    llvm::Argument *ptr = func->args().begin();

    auto isNull = _builder.CreateIsNull(ptr, "isNull");

    llvm::BasicBlock *thenBB = llvm::BasicBlock::Create(_context, "then");
    llvm::BasicBlock *elseBB = llvm::BasicBlock::Create(_context, "else");
    _builder.CreateCondBr(isNull, thenBB, elseBB);
    func->getBasicBlockList().push_back(thenBB);
    func->getBasicBlockList().push_back(elseBB);

    _builder.SetInsertPoint(thenBB);

    auto errAlloc = _builder.CreateCall(getMallocFunc(), { _builder.getInt32(_dataLayout->getTypeAllocSize(_nullPtrErrorTy)) });
    auto alloca = CreateEntryBlockAlloca(func, _interfaceType, "errAlloc");
    auto objPtr = _builder.CreateGEP(alloca, {_builder.getInt32(0), _builder.getInt32(0) }, "objPtr");
    auto vtablePtr = _builder.CreateGEP(alloca, {_builder.getInt32(0), _builder.getInt32(1) }, "vtablePtr");
    _builder.CreateStore(errAlloc, objPtr);
    _builder.CreateStore(getVtable(_nullPtrErrorTy), vtablePtr);

    createThrow(_builder.CreateLoad(alloca), true);

    _builder.CreateBr(elseBB);

    _builder.SetInsertPoint(elseBB);
    _builder.CreateRetVoid();

    _builder.SetInsertPoint(savedInsertPoint);
    return func;
}

llvm::Function *CodeGenerator::getVtableInterfaceLookupFunction()
{
    static llvm::Function *func = NULL;
    if (func)   
        return func;

    auto funcTy = llvm::FunctionType::get(_interfaceVtableType->getPointerTo(), {_objVtableType->getPointerTo(), _builder.getInt32Ty() }, false);
    func = llvm::Function::Create(funcTy, llvm::Function::ExternalLinkage, ".getInterfaceVtable", _module.get());

    if (_isLib)
        return func;

    auto savedInsertPoint = _builder.GetInsertBlock();

    llvm::BasicBlock *bb = llvm::BasicBlock::Create(_context, "entry", func);
    _builder.SetInsertPoint(bb);

    llvm::Argument *objVtable = func->args().begin(), *interfaceId = func->args().begin() + 1;
    auto objVtableSizePtr = _builder.CreateInBoundsGEP(objVtable, { _builder.getInt32(0), _builder.getInt32(0) });
    auto objVtableSize = _builder.CreateLoad(objVtableSizePtr, "interfaceCount");

    auto idx = CreateEntryBlockAlloca(func, _builder.getInt32Ty(), "idx");
    _builder.CreateStore(_builder.getInt32(0), idx);

    llvm::BasicBlock *loopCondBB = llvm::BasicBlock::Create(_context, "loopCond");
    llvm::BasicBlock *loopBB = llvm::BasicBlock::Create(_context, "loop");
    llvm::BasicBlock *foundBB = llvm::BasicBlock::Create(_context, "found");
    llvm::BasicBlock *incBB = llvm::BasicBlock::Create(_context, "inc");
    llvm::BasicBlock *exitBB = llvm::BasicBlock::Create(_context, "exit");

    func->getBasicBlockList().push_back(loopCondBB);
    func->getBasicBlockList().push_back(loopBB);
    func->getBasicBlockList().push_back(foundBB);
    func->getBasicBlockList().push_back(incBB);
    func->getBasicBlockList().push_back(exitBB);

    _builder.CreateBr(loopCondBB);

    _builder.SetInsertPoint(loopCondBB);
    auto idxLoad = _builder.CreateLoad(idx, "idxLoad");
    auto loopCond = _builder.CreateICmpULT(idxLoad, objVtableSize, "loopCond");
    _builder.CreateCondBr(loopCond, loopBB, exitBB);

    _builder.SetInsertPoint(loopBB);
    auto interfaceVtableArrPtr = _builder.CreateGEP(objVtable, { _builder.getInt32(0), _builder.getInt32(1) });
    auto currInterfaceVtable = _builder.CreateGEP(_builder.CreateLoad(interfaceVtableArrPtr), idxLoad);
    auto currInterfaceIdPtr = _builder.CreateGEP(currInterfaceVtable, { _builder.getInt32(0), _builder.getInt32(0) });
    auto currInterfaceId = _builder.CreateLoad(currInterfaceIdPtr);
    auto cond = _builder.CreateICmpEQ(currInterfaceId, interfaceId);
    _builder.CreateCondBr(cond, foundBB, incBB);
    
    _builder.SetInsertPoint(foundBB);
    _builder.CreateRet(currInterfaceVtable);

    _builder.SetInsertPoint(incBB);
    auto addVal = _builder.CreateAdd(idxLoad, _builder.getInt32(1));
    _builder.CreateStore(addVal, idx);
    _builder.CreateBr(loopCondBB);

    _builder.SetInsertPoint(exitBB);
    _builder.CreateRet(llvm::Constant::getNullValue(_interfaceVtableType->getPointerTo()));

    _builder.SetInsertPoint(savedInsertPoint);

    return func;
}

llvm::Value *CodeGenerator::getFuncFromVtable(llvm::Value *vtable, DST::InterfaceDeclaration *interface, unicode_string &funcName) 
{
    auto interfaceVtable = _builder.CreateCall(getVtableInterfaceLookupFunction(), { vtable, _builder.getInt32((unsigned long)interface) });

    auto idx = _builder.getInt32(_interfaceVtableFuncInfo[interface][funcName].index);

    auto arrPtr = _builder.CreateInBoundsGEP(interfaceVtable, { _builder.getInt32(0), _builder.getInt32(1) });
    auto funcsArr = _builder.CreateLoad(arrPtr);

    auto funcPtr = _builder.CreateInBoundsGEP(funcsArr, idx);

    return _builder.CreateLoad(funcPtr, "funcPtr");
}

Value *CodeGenerator::codeGen(DST::Literal *node) 
{
    switch (node->getLiteralType())
    {
    case LT_BOOLEAN:
        return llvm::ConstantInt::get(_context, llvm::APInt( /* 1 bit width */ 1, ((AST::Boolean*)node->getBase())->getValue()));
    case LT_CHARACTER:
        return llvm::ConstantInt::get(_context, llvm::APInt( /* 8 bit width */ 8, (char)((AST::Character*)node->getBase())->getValue().getValue()));   // TODO - unicode characters
    case LT_INTEGER:
        return llvm::ConstantInt::get(_context, llvm::APInt( /* 32 bit width */ 32, ((AST::Integer*)node->getBase())->getValue(), /* signed */ true));
    case LT_FRACTION:
        return llvm::ConstantFP::get(_context, llvm::APFloat(((AST::Fraction*)node->getBase())->getValue()));
    case LT_NULL:
        return llvm::Constant::getNullValue(_builder.getInt8Ty()->getPointerTo());  // 'void*' is invalid in llvm IR
    case LT_STRING:
    {
        return llvm::ConstantStruct::get(_stringTy, llvm::ConstantStruct::get(llvm::cast<llvm::StructType>(_stringTy->getElementType(0)), {
            _builder.getInt32(((AST::String*)node->getBase())->getValue().size()),
            (llvm::Constant*)_builder.CreateBitCast(_builder.CreateGlobalString(((AST::String*)node->getBase())->getValue(), ".stringLit"), _builder.getInt8PtrTy())
        }));
    }
    default:
        throw ErrorReporter::report("Unimplemented literal type", ERR_CODEGEN, node->getPosition());
    }
}

CodeGenerator::NamespaceMembers *CodeGenerator::getNamespaceMembers(DST::Expression *node)
{
    if (node->getExpressionType() == ET_MEMBER_ACCESS)
    {
        auto members = getNamespaceMembers(((DST::MemberAccess*)node)->getLeft());
        return members->namespaces[((DST::MemberAccess*)node)->getRight()];
    }
    if (node->getExpressionType() == ET_IDENTIFIER) {
        if (_currentNamespace.back())
            if (auto ns = _currentNamespace.back()->namespaces[((DST::Variable*)node)->getVarId()])
                return ns;
        return _namespaces[((DST::Variable*)node)->getVarId()];
    }
    else throw "TODO - write a proper error";
}

Value *CodeGenerator::codeGenLval(DST::MemberAccess *node)
{
    auto leftType = node->getLeft()->getType();
    if (leftType->getExactType() == EXACT_TYPELIST && ((DST::TypeList*)leftType)->size() == 1)
        leftType = ((DST::TypeList*)leftType)->getTypes()[0];

    if (leftType->getExactType() == EXACT_NAMESPACE)
    {
        auto members = getNamespaceMembers(node->getLeft());
        if (!members)
            throw "TODO - Error message";
        return members->values[node->getRight()];
    }
    else if (leftType->getExactType() == EXACT_BASIC)
    {
        if (auto interfaceDecl = ((DST::BasicType*)leftType)->getTypeSpecifier()->getInterfaceDecl())
        {
            auto lval = codeGenLval(node->getLeft());
            auto vtablePtr = _builder.CreateInBoundsGEP(lval, { _builder.getInt32(0), _builder.getInt32(1) });
            auto vtable = _builder.CreateLoad(vtablePtr);
            auto funcPtr = getFuncFromVtable(vtable, interfaceDecl, node->getRight());
            node->getRight() += ".get";
            auto funcTy = _interfaceVtableFuncInfo[interfaceDecl][node->getRight()].type;
            auto thisPtr = _builder.CreateLoad(_builder.CreateInBoundsGEP(lval, { _builder.getInt32(0), _builder.getInt32(0) }));
            auto func = _builder.CreateBitCast(funcPtr, funcTy->getPointerTo());
            auto val = createCallOrInvoke(func, thisPtr);
            auto alloca = _builder.CreateAlloca(funcTy->getReturnType());
            _builder.CreateStore(val, alloca);
            return alloca;
        }

        auto bt = (DST::BasicType*)leftType;
        auto typeDef = _types[bt->getTypeSpecifier()->getTypeDecl()];
        auto lval = codeGenLval(node->getLeft());
        assertNotNull(lval);
        return _builder.CreateInBoundsGEP(
            typeDef->structType, 
            lval, 
            { _builder.getInt32(0), _builder.getInt32(typeDef->variableIndexes[node->getRight()]) }, 
            node->getRight().to_string()
        );
    }
    else if (leftType->getExactType() == EXACT_POINTER && 
            ((DST::PointerType*)leftType)->getPtrType()->getExactType() == EXACT_BASIC)
    {
        auto bt = (DST::BasicType*)((DST::PointerType*)leftType)->getPtrType();
        auto typeDef = _types[bt->getTypeSpecifier()->getTypeDecl()];
        auto lval = _builder.CreateLoad(codeGenLval(node->getLeft()));
        assertNotNull(lval);
        return _builder.CreateInBoundsGEP(
            typeDef->structType, 
            lval, 
            { _builder.getInt32(0), _builder.getInt32(typeDef->variableIndexes[node->getRight()]) }, 
            node->getRight().to_string()
        );
    }
    else 
    {
        std::cout << leftType->toShortString() << '\n';
        std::cout << (leftType->getExactType()) << '\n';
        std::cout << (((DST::PointerType*)leftType)->getPtrType()->getExactType() == EXACT_BASIC) << '\n';
        throw ErrorReporter::report("Expression must be of class or namespace type", ERR_CODEGEN, node->getPosition());
    }
}

llvm::Value *CodeGenerator::createCallOrInvoke(llvm::Value *callee, llvm::ArrayRef<llvm::Value*> args)
{
    if (_currCatchBlock)
    {
        auto continueBB = llvm::BasicBlock::Create(_context, "invokeCont");
        auto invoke = _builder.CreateInvoke(callee, continueBB, _currCatchBlock, args);

        getParentFunction()->getBasicBlockList().push_back(continueBB);
        _builder.SetInsertPoint(continueBB);
        return invoke;
    }
    else return _builder.CreateCall(callee, args);
}

Value *CodeGenerator::codeGen(DST::MemberAccess *node)
{
    if (node->getType()->getExactType() == EXACT_PROPERTY)
    {
        node->getRight() += ".get";
        auto leftTy = node->getLeft()->getType();
        if (leftTy->getExactType() == EXACT_TYPELIST && ((DST::TypeList*)leftTy)->getTypes().size() == 1)
            leftTy = ((DST::TypeList*)leftTy)->getTypes()[0];
        
        if (leftTy->getExactType() == EXACT_PROPERTY)
            leftTy = ((DST::PropertyType*)leftTy)->getReturn();
        switch (leftTy->getExactType())
        {
            case EXACT_NAMESPACE:   // Static getter property
            {
                auto func = codeGenLval(node);
                if (!isFunc(func) || ((llvm::Function*)func)->arg_size() != 0)
                    throw ErrorReporter::report("expression is not a getter property", ERR_CODEGEN, node->getPosition());

                return createCallOrInvoke((llvm::Function*)func, {});
            }
            case EXACT_BASIC:       // Member getter property of basic type
            {
                auto lval = codeGenLval(node->getLeft());

                if (auto interfaceDecl = ((DST::BasicType*)leftTy)->getTypeSpecifier()->getInterfaceDecl())
                {
                    auto vtablePtr = _builder.CreateInBoundsGEP(lval, { _builder.getInt32(0), _builder.getInt32(1) });
                    auto vtable = _builder.CreateLoad(vtablePtr);
                    auto funcPtr = getFuncFromVtable(vtable, interfaceDecl, node->getRight());
                    auto funcTy = _interfaceVtableFuncInfo[interfaceDecl][node->getRight()].type;
                    auto thisPtr = _builder.CreateLoad(_builder.CreateInBoundsGEP(lval, { _builder.getInt32(0), _builder.getInt32(0) }));
                    auto func = _builder.CreateBitCast(funcPtr, funcTy->getPointerTo());
                    return createCallOrInvoke(func, thisPtr);
                }

                auto typeDef = _types[((DST::BasicType*)leftTy)->getTypeSpecifier()->getTypeDecl()];
                auto func = typeDef->functions[node->getRight()];
                if (func->arg_size() != 1)
                    throw ErrorReporter::report("expression is not a getter property", ERR_CODEGEN, node->getPosition());
                return createCallOrInvoke(func, lval);
            }
            case EXACT_POINTER:     // Member getter property of pointer to basic type
            {
                auto ptrTy = ((DST::PointerType*)leftTy)->getPtrType();
                if (ptrTy->getExactType() != EXACT_BASIC)
                    throw "TODO";
                auto lval = codeGen(node->getLeft());
                auto typeDef = _types[((DST::BasicType*)ptrTy)->getTypeSpecifier()->getTypeDecl()];
                auto func = typeDef->functions[node->getRight()];
                if (!isFunc(func) || ((llvm::Function*)func)->arg_size() != 1)
                    throw ErrorReporter::report("expression is not a getter property", ERR_CODEGEN, node->getPosition());
                return createCallOrInvoke((llvm::Function*)func, lval);
            }
            case EXACT_ARRAY:       // Member property of array
                if (node->getRight() == unicode_string("Size.get")) {
                    if (((DST::ArrayType*)leftTy)->getLength() == DST::UNKNOWN_ARRAY_LENGTH)
                        return _builder.CreateLoad(_builder.CreateInBoundsGEP(
                            codeGenLval(node->getLeft()),
                            { _builder.getInt32(0), _builder.getInt32(0) },
                            "sizePtrTmp")
                        );
                    return _builder.getInt32(codeGen(node->getLeft())->getType()->getArrayNumElements());
                }
                throw ErrorReporter::report("Unimplemented Error no.2", ERR_CODEGEN, node->getPosition());   // TODO
            default:
                throw ErrorReporter::report("Unimplemented Error no.3 | " + 
                leftTy->toShortString() + std::to_string(leftTy->getExactType()), ERR_CODEGEN, node->getPosition());   // TODO
        }
    }
    if (node->getType()->isConst())
        return codeGenLval(node);
    auto val = codeGenLval(node);
    assertNotNull(val);
    return _builder.CreateLoad(val, "accesstmp");
}

llvm::Value* CodeGenerator::getVtable(llvm::Type *type) {
    if (auto vtable = _vtables[type])
        return vtable;
    else return createEmptyVtable(type);
}


llvm::Value* CodeGenerator::createEmptyVtable(llvm::Type *type) 
{
    auto llvmVtable = llvm::ConstantStruct::get(_objVtableType, { _builder.getInt32(0), _builder.getInt32(0) });
    return _vtables[type] = new llvm::GlobalVariable(*_module, llvmVtable->getType(), true, llvm::GlobalVariable::PrivateLinkage, 
                                            llvmVtable, "empty.vtable");
}

Value *CodeGenerator::codeGen(DST::BinaryOperation* node)
{
    Value *left, *right;
    
    if (node->getOperator()._type == OT_IS)
    {
        auto right = evalType((DST::Type*)node->getRight());
        auto left = codeGenLval(node->getLeft());
        if (right == _interfaceType)
        {
            if (left->getType() != _interfaceType)
            {
                // should be known at compile-time
                throw "TODO";
            }

            auto vtablePtr = _builder.CreateInBoundsGEP(left, { _builder.getInt32(0), _builder.getInt32(1) });
            auto vtableLoad = _builder.CreateLoad(vtablePtr);
            auto interface = ((DST::BasicType*)node->getRight())->getTypeSpecifier()->getInterfaceDecl();
            auto interfaceVtable = _builder.CreateCall(getVtableInterfaceLookupFunction(), { vtableLoad, _builder.getInt32((unsigned long)interface) }); 
            auto diff = _builder.CreatePtrDiff(interfaceVtable, llvm::ConstantPointerNull::get(_interfaceVtableType->getPointerTo()));
            return _builder.CreateICmpEQ(diff, _builder.getInt64(0), "isTmp");
        }
        else if (((DST::Type*)node->getRight())->getExactType() == EXACT_BASIC)
        {
            if (left->getType()->getPointerElementType() != _interfaceType)
                return _builder.getInt1(false);
            auto vtablePtr = _builder.CreateInBoundsGEP(left, { _builder.getInt32(0), _builder.getInt32(1) });
            auto vtableLoad = _builder.CreateLoad(vtablePtr);
            // auto decl = ((DST::BasicType*)node->getRight())->getTypeSpecifier()->getTypeDecl();
            // llvm::Value *vtable = NULL;
            // if (auto def = _types[decl])
            //     vtable = def->vtable;
            // else vtable = createEmptyVtable(evalType(((DST::Type*)node->getRight())));
            auto vtable = getVtable(evalType(((DST::Type*)node->getRight())));
            auto diff = _builder.CreatePtrDiff(vtableLoad, vtable);
            return _builder.CreateICmpEQ(diff, _builder.getInt64(0), "isTmp");
        }
        else throw ErrorReporter::report("The \"is\" operator is currently only implemented for basic types!", ERR_CODEGEN, node->getPosition());

        // auto interfaceVtable = _builder.CreateCall(_vtableInterfaceLookupFunc, { vtable, _builder.getInt32((unsigned long)interface) });

        // funcPtr = getFuncFromVtable(vtable, interfaceDecl, funcId);
        // funcTy = _interfaceVtableFuncInfo[interfaceDecl][funcId].type;
        // thisPtr = _builder.CreateLoad(_builder.CreateInBoundsGEP(thisPtr, { _builder.getInt32(0), _builder.getInt32(0) }));
        return NULL;
    }

    if(node->getOperator()._type != OT_SQUARE_BRACKETS_OPEN)
    {
        left = codeGen(node->getLeft());
        right = codeGen(node->getRight());
    }
    
    switch (node->getOperator()._type)
    {
        case OT_ADD:
            return _builder.CreateAdd(left, right, "addtmp");
        case OT_SUBTRACT:
            return _builder.CreateSub(left, right, "subtmp");
        case OT_MULTIPLY:
            return _builder.CreateMul(left, right, "multmp");
        case OT_DIVIDE:
            return _builder.CreateSDiv(left, right, "divtmp");
        case OT_MODULUS:
            return _builder.CreateSRem(left, right, "divtmp");
        case OT_SMALLER:
            return _builder.CreateICmpSLT(left, right, "cmptmp");
        case OT_SMALLER_EQUAL:
            return _builder.CreateICmpSLE(left, right, "cmptmp");
        case OT_GREATER:
            return _builder.CreateICmpSGT(left, right, "cmptmp");
        case OT_GREATER_EQUAL:
            return _builder.CreateICmpSGE(left, right, "cmptmp");
        case OT_EQUAL:
        {
            if (left->getType() == _interfaceType)
                left = _builder.CreateExtractValue(left, 0);

            if (right->getType() == _interfaceType)
                right = _builder.CreateExtractValue(right, 0);

            if (left->getType() != right->getType())
                return _builder.CreateICmpEQ(_builder.CreateBitCast(left, right->getType()), right, "cmptmp");
            return _builder.CreateICmpEQ(left, right, "cmptmp");
        }
        case OT_NOT_EQUAL:
        {
            if (left->getType() == _interfaceType)
                left = _builder.CreateExtractValue(left, 0);

            if (right->getType() == _interfaceType)
                right = _builder.CreateExtractValue(right, 0);

            if (left->getType() != right->getType())
                return _builder.CreateICmpNE(_builder.CreateBitCast(left, right->getType()), right, "cmptmp");
            return _builder.CreateICmpNE(left, right, "cmptmp");
        }
        case OT_LOGICAL_AND:
            return _builder.CreateAnd(left, right, "andtmp");
        case OT_LOGICAL_OR:
            return _builder.CreateOr(left, right, "ortmp");

        case OT_SQUARE_BRACKETS_OPEN:
            return _builder.CreateLoad(codeGenLval(node));
        default:
            throw ErrorReporter::report("Unimplemented Binary operation!", ERR_CODEGEN, node->getPosition());
    }
    
}

Value *CodeGenerator::codeGen(DST::ConditionalExpression *node)
{
    return _builder.CreateSelect(
        codeGen(node->getCondition()),
        codeGen(node->getThenBranch()),
        codeGen(node->getElseBranch())
    );
}

llvm::Function *CodeGenerator::getMallocFunc()
{
    static llvm::Function *malloc = NULL;
    if (malloc == NULL)
    {
        auto type = llvm::FunctionType::get(llvm::Type::getInt8Ty(_context)->getPointerTo(), { llvm::Type::getInt32Ty(_context) }, false);
        // malloc = llvm::Function::Create(type, llvm::Function::ExternalLinkage, "malloc", _module.get());
        malloc = llvm::Function::Create(type, llvm::Function::ExternalLinkage, "GC_malloc", _module.get());
    }
    return malloc;
}

Value *CodeGenerator::codeGen(DST::UnaryOperation* node)
{
    switch (node->getOperator()._type)
    {
        case OT_NEW:
        {
            if (((DST::Type*)node->getExpression())->getExactType() == EXACT_ARRAY && ((DST::ArrayType*)node->getExpression())->getLenExp())
            {
                auto type = evalType((DST::Type*)node->getExpression());
                int size = _dataLayout->getTypeAllocSize(evalType(((DST::ArrayType*)node->getExpression())->getElementType()));
                auto len = codeGen(((DST::ArrayType*)node->getExpression())->getLenExp());
                auto allocSize = _builder.CreateMul(_builder.getInt32(size), len, "allocSize");
                return _builder.CreateBitCast( createCallOrInvoke(getMallocFunc(), { allocSize }), type, "newTmp");
            }
            auto type = evalType((DST::Type*)node->getExpression());
            if (type->isVoidTy())
                throw ErrorReporter::report("Cannot create instance of 'void'", ERR_CODEGEN, node->getPosition());
            int size = _dataLayout->getTypeAllocSize(type);
            return _builder.CreateBitCast( createCallOrInvoke(getMallocFunc(), { _builder.getInt32(size) }), type->getPointerTo(), "newTmp");
        }
        case OT_ADD:
            return codeGen(node->getExpression());
        case OT_SUBTRACT:
            throw ErrorReporter::report("Unimplemented literal type", ERR_CODEGEN, node->getPosition());
        case OT_AT:
        {
            auto val = codeGen(node->getExpression());
            assertNotNull(val);
            return _builder.CreateLoad(val);
        }
        case OT_BITWISE_AND:
            return _builder.CreateGEP(codeGenLval(node->getExpression()), _builder.getInt32(0));
        case OT_LOGICAL_NOT:
            return _builder.CreateNot(codeGen(node->getExpression()), "nottmp");
        default:
            throw ErrorReporter::report("Unimplemented unary operation", ERR_CODEGEN, node->getPosition());
    }
    
}

Value *CodeGenerator::codeGen(DST::ArrayLiteral *node)
{
    vector<llvm::Constant*> IRvalues;
    llvm::Type *atype = evalType(node->getType());

    for(auto val : node->getArray())
    {
        IRvalues.push_back((llvm::Constant*)codeGen(val));
    }
    return llvm::ConstantArray::get((llvm::ArrayType*)atype, IRvalues);
}

Value *CodeGenerator::codeGen(DST::Increment *node)
{
    llvm::Value *lvalue = codeGenLval(node->getExpression());
    int varSize = lvalue->getType()->getPointerElementType()->getPrimitiveSizeInBits();
    
    llvm::Value *inc;
    if(node->isIncrement())
        inc = _builder.CreateAdd(_builder.CreateLoad(lvalue), _builder.getIntN(varSize, 1));
    else inc = _builder.CreateSub(_builder.CreateLoad(lvalue), _builder.getIntN(varSize, 1));
    
    _builder.CreateStore(inc, lvalue);
    
    return inc;
}

Value *CodeGenerator::codeGenLval(DST::UnaryOperation* node)
{
    Value *val = codeGenLval(node->getExpression());
    switch (node->getOperator()._type)
    {
        case OT_AT:
            assertNotNull(val);
            return _builder.CreateLoad(val);
        case OT_BITWISE_AND:
            return _builder.CreateGEP(val, _builder.getInt32(0));
        default:
            throw ErrorReporter::report("Unimplemented lval unary operation", ERR_CODEGEN, node->getPosition());
    }    
}

Value *CodeGenerator::codeGenLval(DST::BinaryOperation *node)
{
    Value *left = codeGenLval(node->getLeft());
    switch (node->getOperator()._type)
    {
        case OT_SQUARE_BRACKETS_OPEN:
            if (((DST::ArrayType*)node->getLeft()->getType())->getLength() == DST::UNKNOWN_ARRAY_LENGTH)
            {
                auto arrPtr = _builder.CreateInBoundsGEP(left, { _builder.getInt32(0), _builder.getInt32(1) } );
                return _builder.CreateGEP(_builder.CreateLoad(arrPtr), codeGen(node->getRight()) );
            }
            else {
                // TODO - array literal access
                return _builder.CreateInBoundsGEP(left, { _builder.getInt32(0), codeGen(node->getRight()) } );
            }
        default:
            throw ErrorReporter::report("Unimplemented lval Binary operation", ERR_CODEGEN, node->getPosition());
    }
}

Value *CodeGenerator::codeGenLval(DST::Expression *node)
{
    if (node == nullptr)
        return nullptr;
    switch (node->getExpressionType()) 
    {
        case ET_IDENTIFIER: return codeGenLval((DST::Variable*)node);
        case ET_VARIABLE_DECLARATION: return codeGenLval((DST::VariableDeclaration*)node);
        case ET_MEMBER_ACCESS: return codeGenLval((DST::MemberAccess*)node);
        case ET_UNARY_OPERATION: return codeGenLval((DST::UnaryOperation*)node);
        case ET_BINARY_OPERATION: return codeGenLval((DST::BinaryOperation*)node);
        case ET_CONVERSION: return codeGenLval((DST::Conversion*)node);
        default: throw ErrorReporter::report("unimplemented lval expression type.", ERR_CODEGEN, node->getPosition());
    }
}

Value *CodeGenerator::codeGenLval(DST::Conversion* node) 
{
    auto type = evalType(node->getType());
    auto exp = codeGenLval(node->getExpression());
    if (type->getPointerTo() == exp->getType() || type == _interfaceType)
        return exp;
    if (type != _interfaceType && exp->getType() == _interfaceType->getPointerTo()) // Interface to non-interface
    {
        // TODO - throw exception incase of invalid conversion
        auto ptr = _builder.CreateGEP(exp, {_builder.getInt32(0), _builder.getInt32(0)}, "accessTmp");
        return _builder.CreateBitCast(ptr, type->getPointerTo(), "cnvrttmp");
    }
    throw "TODO";
}

Value *CodeGenerator::codeGen(DST::Conversion* node)
{
    auto type = evalType(node->getType());
    auto exp = codeGen(node->getExpression());
    if (type == exp->getType() || type == _interfaceType)
        return exp;
    if (type != _interfaceType && exp->getType() == _interfaceType) // Interface to non-interface
    {
        // TODO - throw exception incase of invalid conversion
        auto ptr = _builder.CreateExtractValue(exp, 0, "accessTmp");
        return _builder.CreateBitCast(ptr, type, "cnvrttmp");
    }
    unsigned int targetSz = type->getPrimitiveSizeInBits();
    unsigned int currSz = exp->getType()->getPrimitiveSizeInBits();
    if (targetSz < currSz)
        return _builder.CreateTrunc(exp, type, "cnvrttmp");
    else if (targetSz > currSz)
        return _builder.CreateZExt(exp, type, "cnvrttmp");  // TODO - exending cast for unsigned intergers
    else return _builder.CreateBitCast(exp, type, "cnvrttmp");
}

Value *CodeGenerator::convertToInterface(Value* node) 
{
    if (node == NULL) 
        return llvm::ConstantStruct::get(_interfaceType, { 
            llvm::ConstantPointerNull::get(_builder.getInt8PtrTy()), 
            llvm::ConstantPointerNull::get(_objVtableType->getPointerTo())
        });
    // node->print(llvm::errs());
    // llvm::errs() << "\n";
    // node->getType()->print(llvm::errs());
    // llvm::errs() << "\n";
    if (node->getType() == _interfaceType)
        return node;
    // llvm::errs() << "aaa\n";
    return _builder.CreateLoad(convertToInterfaceLval(node));
}

Value *CodeGenerator::convertToInterfaceLval(Value* node)
{
    auto alloca = CreateEntryBlockAlloca(getParentFunction(), _interfaceType, "cnvrttmp");
    if (node == NULL || node->getType() == _interfaceType) 
    {
        _builder.CreateStore(convertToInterface(node), alloca);
        return alloca;
    }

    if (!node->getType()->isPointerTy())
        throw "cannot convert non pointer to interface";
    
    auto objPtr = _builder.CreateInBoundsGEP(alloca, { _builder.getInt32(0), _builder.getInt32(0) }, "objPtrTmp");
    auto vtablePtr = _builder.CreateInBoundsGEP(alloca, { _builder.getInt32(0), _builder.getInt32(1) }, "vtableTmp");
    _builder.CreateStore(_builder.CreateBitCast(node, _builder.getInt8PtrTy()), objPtr);
    _builder.CreateStore(getVtable(node->getType()->getPointerElementType()), vtablePtr);
    return alloca;
}

Value *CodeGenerator::codeGen(DST::Assignment* node)
{
    Value *left = NULL;
    Value *right = NULL;

    if (node->getLeft()->getType()->getExactType() == EXACT_PROPERTY)
    {
        if (node->getLeft()->getExpressionType() == ET_IDENTIFIER)
            ((DST::Variable*)node->getLeft())->getVarId() += ".set";
        else if (node->getLeft()->getExpressionType() == ET_MEMBER_ACCESS)
            ((DST::MemberAccess*)node->getLeft())->getRight() += ".set";

        if (node->getLeft()->getExpressionType() == ET_MEMBER_ACCESS) 
        {
            auto ac = (DST::MemberAccess*)node->getLeft();
            switch (ac->getLeft()->getType()->getExactType())
            {
                case EXACT_NAMESPACE:   // Static setter property
                {
                    left = codeGenLval(node->getLeft());
                    if (!isFunc(left))
                        throw ErrorReporter::report("expression is not a setter property", ERR_CODEGEN, node->getPosition());
                    auto right = codeGen(node->getRight());
                    createCallOrInvoke((llvm::Function*)left, right);
                    return right;
                }
                case EXACT_BASIC:       // Member setter property of basic type
                {
                    left = codeGenLval(ac->getLeft());
                    right = codeGen(node->getRight());

                    if (auto interfaceDecl = ((DST::BasicType*)ac->getLeft()->getType())->getTypeSpecifier()->getInterfaceDecl())
                    {
                        auto vtablePtr = _builder.CreateInBoundsGEP(left, { _builder.getInt32(0), _builder.getInt32(1) });
                        auto vtable = _builder.CreateLoad(vtablePtr);
                        auto funcPtr = getFuncFromVtable(vtable, interfaceDecl, ac->getRight());
                        auto funcTy = _interfaceVtableFuncInfo[interfaceDecl][ac->getRight()].type;
                        auto thisPtr = _builder.CreateLoad(_builder.CreateInBoundsGEP(left, { _builder.getInt32(0), _builder.getInt32(0) }));
                        auto func = _builder.CreateBitCast(funcPtr, funcTy->getPointerTo());
                        createCallOrInvoke(func, { thisPtr, right });
                    }
                    else 
                    {
                        auto typeDef = _types[((DST::BasicType*)ac->getLeft()->getType())->getTypeSpecifier()->getTypeDecl()];
                        auto func = typeDef->functions[ac->getRight()];
                        if (!isFunc(func) || ((llvm::Function*)func)->arg_size() != 2)
                            throw ErrorReporter::report("expression is not a setter property", ERR_CODEGEN, node->getPosition());
                        createCallOrInvoke((llvm::Function*)func, { left, right });
                    }
                    return right;
                }
                case EXACT_POINTER:     // Member setter property of pointer to basic type
                {
                    auto ptrTy = ((DST::PointerType*)ac->getLeft()->getType())->getPtrType();
                    if (ptrTy->getExactType() != EXACT_BASIC)
                        throw "TODO";
                    auto thisPtr = codeGen(ac->getLeft());
                    auto typeDef = _types[((DST::BasicType*)ptrTy)->getTypeSpecifier()->getTypeDecl()];
                    auto func = typeDef->functions[ac->getRight()];
                    if (!isFunc(func) || ((llvm::Function*)func)->arg_size() != 2)
                        throw ErrorReporter::report("expression is not a setter property", ERR_CODEGEN, node->getPosition());
                    return createCallOrInvoke((llvm::Function*)func, { thisPtr, codeGen(node->getRight()) });
                }
                case EXACT_ARRAY:       // Member property of array
                    throw ErrorReporter::report("Unimplemented Error no.2", ERR_CODEGEN, node->getPosition());   // TODO
                default:
                    throw ErrorReporter::report("Unimplemented Error no.3 | " + ac->getLeft()->getType()->toShortString(), ERR_CODEGEN, node->getPosition());   // TODO
            }
        }

        left = codeGenLval(node->getLeft());
        if (!isFunc(left))
            throw ErrorReporter::report("expression is not a setter property", ERR_CODEGEN, node->getPosition());

        createCallOrInvoke((llvm::Function*)left, codeGen(node->getRight()));
        return right;
    }

    if (node->getLeft()->getType()->getExactType() == EXACT_TYPELIST) 
    {
        if (node->getRight()->getExpressionType() == ET_FUNCTION_CALL)
        {
            vector<Value*> retPtrs;
            for (auto i : ((DST::ExpressionList*)node->getLeft())->getExpressions())
                retPtrs.push_back(codeGenLval(i));
            return codeGen(((DST::FunctionCall*)node->getRight()), retPtrs);
        }

        if (node->getRight()->getExpressionType() != ET_LIST)
            throw ErrorReporter::report("Multi-return functions are not implemented yet", ERR_CODEGEN, node->getPosition());
        vector<Value*> lefts, rights;
        for (auto i : ((DST::ExpressionList*)node->getLeft())->getExpressions())
            lefts.push_back(codeGenLval(i));
        for (auto i : ((DST::ExpressionList*)node->getRight())->getExpressions())
            rights.push_back(codeGen(i));
        Value *lastStore = NULL;
        for (unsigned int i = 0; i < lefts.size(); i++)
            lastStore = _builder.CreateStore(rights[i], lefts[i]);
        return lastStore;   // Temporary fix.
    }

    if (node->getLeft()->getType()->getExactType() == EXACT_ARRAY && 
        ((DST::ArrayType*)node->getLeft()->getType())->getLength() == DST::UNKNOWN_ARRAY_LENGTH)
    {
        left = codeGenLval(node->getLeft());
        right = codeGen(node->getRight());
        if (right->getType()->isPointerTy())
        {
            auto sizePtr = _builder.CreateInBoundsGEP(left, { _builder.getInt32(0), _builder.getInt32(0) }, "sizePtrTmp");
            auto arrPtr = _builder.CreateInBoundsGEP(left, { _builder.getInt32(0), _builder.getInt32(1) }, "arrPtrTmp");
            _builder.CreateStore(_builder.CreateBitOrPointerCast(right, arrPtr->getType()->getPointerElementType()), arrPtr);

            if (node->getRight()->getType()->getExactType() == EXACT_ARRAY && ((DST::ArrayType*)node->getRight()->getType())->getLenExp()) 
                _builder.CreateStore(codeGen(((DST::ArrayType*)node->getRight()->getType())->getLenExp()), sizePtr);
            else _builder.CreateStore(_builder.getInt32(right->getType()->getPointerElementType()->getArrayNumElements()), sizePtr);
            return right;
        }
        if (right->getType() == left->getType()->getPointerElementType())
        {
            _builder.CreateStore(right, left);
            return right;
        }
        else throw ErrorReporter::report("You shouldn't have gotten here...", ERR_CODEGEN, node->getPosition());
    }

    if (node->getOperator()._type != OT_ASSIGN_EQUAL)
    {
        left = codeGenLval(node->getLeft());
        right = codeGen(node->getRight());
    }

    switch (node->getOperator()._type) 
    {
        case OT_ASSIGN_EQUAL:
        {
            left = codeGenLval(node->getLeft());
            if (left->getType() == _interfaceType->getPointerTo())
            {
                right = codeGen(node->getRight());
                _builder.CreateStore(convertToInterface(right), left);
                return right;
            }
            
            if (left->getType()->getPointerElementType()->isPointerTy() &&
                left->getType()->getPointerElementType()->getPointerElementType()->isFunctionTy())
            {
                right = codeGenLval(node->getRight());
                _builder.CreateStore(right, left);
                return right;    
            }
            right = codeGen(node->getRight());
            _builder.CreateStore(right, left);
            return right;
        }
        case OT_ASSIGN_ADD:
        {
            auto res = _builder.CreateAdd(_builder.CreateLoad(left), right, "addtmp");
            _builder.CreateStore(res, left);
            return res;
        }
        case OT_ASSIGN_SUBTRACT:
        {
            auto res = _builder.CreateSub(_builder.CreateLoad(left), right, "subtmp");
            _builder.CreateStore(res, left);
            return res;
        }
        case OT_ASSIGN_MULTIPLY:
        {
            auto res = _builder.CreateMul(_builder.CreateLoad(left), right, "multmp");
            _builder.CreateStore(res, left);
            return res;
        }
        case OT_ASSIGN_DIVIDE:
        {
            auto res = _builder.CreateSDiv(_builder.CreateLoad(left), right, "divtmp");
            _builder.CreateStore(res, left);
            return res;
        }
        case OT_ASSIGN_MODULUS:
        {
            auto res = _builder.CreateSRem(_builder.CreateLoad(left), right, "modtmp");
            _builder.CreateStore(res, left);
            return res;
        }
        default: throw ErrorReporter::report("Unimplemented assignment operator", ERR_CODEGEN, node->getPosition());
    }
}

/// CreateEntryBlockAlloca - Create an alloca instruction in the entry block of
/// the function.  This is used for mutable variables etc.
AllocaInst *CodeGenerator::CreateEntryBlockAlloca(llvm::Function *func, llvm::Type *type, const string &varName) 
{ 
    llvm::IRBuilder<> TmpB(&func->getEntryBlock(), func->getEntryBlock().begin());
    return TmpB.CreateAlloca(type, nullptr, varName);
}

bool CodeGenerator::isFunc(llvm::Value *funcPtr)
{
    if (funcPtr == nullptr)
        return false;
    if (funcPtr->getType()->isFunctionTy())
        return true;
    else if (funcPtr->getType()->isPointerTy() && ((llvm::PointerType*)funcPtr->getType())->getElementType()->isFunctionTy())
        return true;
    else return false;
}

bool CodeGenerator::isFuncPtr(llvm::Value *funcPtr)
{
    if (funcPtr == nullptr || !funcPtr->getType()->isPointerTy())
        return false;
    if (funcPtr->getType()->getPointerElementType()->isFunctionTy())
        return true;
    if (funcPtr->getType()->getPointerElementType()->isPointerTy() && funcPtr->getType()->getPointerElementType()->getPointerElementType()->isFunctionTy())
        return true;
    return false;
}

Value *CodeGenerator::codeGen(DST::FunctionCall *node, vector<Value*> retPtrs)
{
    if (node->getFunctionId()->getExpressionType() == ET_MEMBER_ACCESS)
    {
        auto ty = ((DST::MemberAccess*)node->getFunctionId())->getLeft()->getType();
        if (ty->getExactType() == EXACT_TYPELIST && ((DST::TypeList*)ty)->size() == 1)
            ty = ((DST::TypeList*)ty)->getTypes()[0];
        if (ty->getExactType() == EXACT_BASIC || ty->getExactType() == EXACT_POINTER)
        {
            auto &funcId = ((DST::MemberAccess*)node->getFunctionId())->getRight();
            TypeDefinition *typeDef = NULL;
            llvm::Value *funcPtr = NULL;
            llvm::Value *thisPtr = NULL;
            llvm::FunctionType *funcTy = NULL;

            if (ty->getExactType() == EXACT_POINTER)
                thisPtr = codeGen(((DST::MemberAccess*)node->getFunctionId())->getLeft());
            else thisPtr = codeGenLval(((DST::MemberAccess*)node->getFunctionId())->getLeft());

            if (ty->getExactType() == EXACT_BASIC)
            {
                auto bt = ((DST::BasicType*)ty);
                if (auto interfaceDecl = bt->getTypeSpecifier()->getInterfaceDecl())
                {
                    auto vtablePtr = _builder.CreateInBoundsGEP(thisPtr, { _builder.getInt32(0), _builder.getInt32(1) });
                    auto vtable = _builder.CreateLoad(vtablePtr);
                    funcPtr = getFuncFromVtable(vtable, interfaceDecl, funcId);
                    funcTy = _interfaceVtableFuncInfo[interfaceDecl][funcId].type;
                    thisPtr = _builder.CreateLoad(_builder.CreateInBoundsGEP(thisPtr, { _builder.getInt32(0), _builder.getInt32(0) }));
                }
                else typeDef = _types[((DST::BasicType*)ty)->getTypeSpecifier()->getTypeDecl()];
            }
                
            else 
            {
                if (((DST::PointerType*)ty)->getPtrType()->getExactType() != EXACT_BASIC)
                    throw ErrorReporter::report("Internal decorator error?", ERR_CODEGEN, node->getPosition());
                typeDef = _types[((DST::BasicType*)((DST::PointerType*)ty)->getPtrType())->getTypeSpecifier()->getTypeDecl()];
            }

            if (typeDef && !funcPtr)
            {
                auto func = typeDef->functions[funcId];
                funcTy = llvm::dyn_cast<llvm::FunctionType>(func->getType()->getElementType());
                funcPtr = func;
            }

            if (funcPtr->getType() != funcTy)
                funcPtr = _builder.CreateBitCast(funcPtr, funcTy->getPointerTo());

            if (funcTy->getNumParams() != node->getArguments()->getExpressions().size() + 1 + retPtrs.size()) // + 1 since we are also passing a "this" ptr
                throw ErrorReporter::report(string("Incorrect # arguments passed (needed ") + 
                    std::to_string(funcTy->getNumParams()) + ", got " + std::to_string(node->getArguments()->getExpressions().size()) + ")"
                    , ERR_CODEGEN, node->getPosition());

            std::vector<Value*> args;
            args.push_back(thisPtr);
        
            int i = -1;
            unsigned int i2 = 0;
            for (auto &argTy : funcTy->params())
            {
                if (i == -1) { i++; continue; }

                if (i2 < retPtrs.size())
                    args.push_back(retPtrs[i2++]);
                else 
                {
                    auto gen = codeGen(node->getArguments()->getExpressions()[i++]);        
                    if (gen->getType() != argTy) {
                        if (argTy == _interfaceType)
                            args.push_back(convertToInterface(gen));
                        else args.push_back(_builder.CreateBitCast(gen, argTy, "castTmp"));
                    }
                    else args.push_back(gen);
                }
            }

            return createCallOrInvoke(funcPtr, args);

        }
    }

    llvm::Value *funcPtr = codeGenLval(node->getFunctionId());
    
    if (!isFunc(funcPtr))
    {
        if (isFuncPtr(funcPtr))
            funcPtr = _builder.CreateLoad(funcPtr);
        else throw ErrorReporter::report("expression is not a function!", ERR_CODEGEN, node->getPosition());
    }

    auto funcTy = llvm::dyn_cast<llvm::FunctionType>(funcPtr->getType()->getPointerElementType());
    if (!funcTy)
        throw ErrorReporter::report("Internal error while generating IR for function call", ERR_CODEGEN, node->getPosition());
    
    if (funcTy->getNumParams() != node->getArguments()->getExpressions().size() + retPtrs.size())
        throw ErrorReporter::report(string("Incorrect # arguments passed (needed ") + 
            std::to_string(funcTy->getNumParams()) + ", got " + std::to_string(node->getArguments()->getExpressions().size()) + ")"
            , ERR_CODEGEN, node->getPosition());
        
    std::vector<Value *> args;

    int i = 0;
    unsigned int i2 = 0;
    for (auto &argTy : funcTy->params())
    {
        if (i2 < retPtrs.size())
            args.push_back(retPtrs[i2++]);
        else
        {
            auto gen = codeGen(node->getArguments()->getExpressions()[i++]);       
            
            if (gen->getType() != argTy)
            {
                if (argTy == _interfaceType)
                    args.push_back(convertToInterface(gen));
                else args.push_back(_builder.CreateBitCast(gen, argTy, "castTmp"));
            }
            else args.push_back(gen);   
        }
    }
    return createCallOrInvoke(funcPtr, args);
}

Value *CodeGenerator::createThrow(llvm::Value *exceptionVal, bool dontInvoke)
{
    static llvm::Function *throwFunc = NULL;
    static llvm::Function *allocExceptionFunc = NULL;

    if (throwFunc == NULL)
    {
        auto throwFuncTy = llvm::FunctionType::get(_builder.getVoidTy(), { _builder.getInt8PtrTy(), _builder.getInt8PtrTy(), _builder.getInt8PtrTy() }, false);
        throwFunc = llvm::Function::Create(throwFuncTy, llvm::Function::ExternalLinkage, "__cxa_throw", _module.get());

        auto allocExceptionFuncTy = llvm::FunctionType::get(_builder.getInt8PtrTy(), _builder.getInt64Ty(), false);
        allocExceptionFunc = llvm::Function::Create(allocExceptionFuncTy, llvm::Function::ExternalLinkage, "__cxa_allocate_exception", _module.get());
    }

    auto alloc = _builder.CreateCall(allocExceptionFunc, _builder.getInt64(_dataLayout->getTypeAllocSize(_builder.getInt8PtrTy())));
    auto cast = _builder.CreateBitCast(alloc, _interfaceType->getPointerTo());
    _builder.CreateStore(exceptionVal, cast);
    auto nullPtr = llvm::ConstantPointerNull::get(_builder.getInt8PtrTy());

    if (dontInvoke)
        return _builder.CreateCall(throwFunc, { alloc, nullPtr, nullPtr });

    return createCallOrInvoke(throwFunc, { alloc, nullPtr, nullPtr });
}

Value *CodeGenerator::codeGen(DST::UnaryOperationStatement *node)
{
    static llvm::Function *free = NULL;

    switch (node->getOperator()._type)
    {
        case OT_BREAK:
            return _builder.CreateBr(_currBreakJmp);
        case OT_CONTINUE:
            return _builder.CreateBr(_currContinueJmp);
        case OT_THROW: 
            return createThrow(codeGen(node->getExpression()));
        case OT_RETURN:
        {
            if (node->getExpression() == NULL) 
            {
                if (!getParentFunction()->getReturnType()->isVoidTy())
                    throw ErrorReporter::report("cannot return void in non void function", ERR_DECORATOR, node->getPosition());
                return _builder.CreateRetVoid();
            }
            if (node->getExpression()->getExpressionType() == ET_LIST)
            {
                auto expList = (DST::ExpressionList*)node->getExpression();
                for (unsigned int i = 0; i < expList->size(); i++)
                    _builder.CreateStore(codeGen(expList->getExpressions()[i]), _funcReturns[i]);
                return _builder.CreateRetVoid();
            }
            auto val = codeGen(node->getExpression());
            auto returnTy = getParentFunction()->getReturnType();
            if (val->getType() != returnTy)
            {
                if (returnTy == _interfaceType)
                    return _builder.CreateRet(convertToInterface(val));
                return _builder.CreateRet(_builder.CreateBitCast(val, returnTy));
            }
            return _builder.CreateRet(val);
        }

        

        case OT_DELETE:
        {
            if (free == NULL)
            {
                auto type = llvm::FunctionType::get(_builder.getVoidTy(), _builder.getInt8PtrTy(), false);
                free = llvm::Function::Create(type, llvm::Function::ExternalLinkage, "free", _module.get());
            }
            Value *ptr = codeGenLval(node->getExpression());
            if (node->getExpression()->getType()->getExactType() == EXACT_ARRAY && 
                ((DST::ArrayType*)node->getExpression()->getType())->getLength() == DST::UNKNOWN_ARRAY_LENGTH)
            {
                auto sizePtr = _builder.CreateInBoundsGEP(ptr, { _builder.getInt32(0), _builder.getInt32(0) }, "sizePtrTmp");
                _builder.CreateStore(_builder.getInt32(0), sizePtr);
                ptr = _builder.CreateInBoundsGEP(ptr, { _builder.getInt32(0), _builder.getInt32(1) } );
            }
            
            if (ptr->getType()->getPointerElementType() == _interfaceType) 
            {
                auto objPtr = _builder.CreateInBoundsGEP(ptr, { _builder.getInt32(0), _builder.getInt32(0) }, "objPtrTmp");
                auto vtablePtr = _builder.CreateInBoundsGEP(ptr, { _builder.getInt32(0), _builder.getInt32(1) }, "objPtrTmp");
                auto cast = _builder.CreateBitCast(_builder.CreateLoad(objPtr), llvm::Type::getInt8Ty(_context)->getPointerTo(), "castTmp");
                createCallOrInvoke(free, cast);
                _builder.CreateStore(llvm::Constant::getNullValue(vtablePtr->getType()->getPointerElementType()), vtablePtr);
                return _builder.CreateStore(llvm::Constant::getNullValue(objPtr->getType()->getPointerElementType()), objPtr);
            }

            auto cast = _builder.CreateBitCast(_builder.CreateLoad(ptr), llvm::Type::getInt8Ty(_context)->getPointerTo(), "castTmp");

            createCallOrInvoke(free, cast);
            return _builder.CreateStore(llvm::Constant::getNullValue(ptr->getType()->getPointerElementType()), ptr);
        }
        default: throw ErrorReporter::report("Unimplemented unary operation statement!", ERR_CODEGEN, node->getPosition());
    }
}

Value *CodeGenerator::codeGenLval(DST::Variable *node)
{
    if (auto var = _namedValues[node->getVarId().to_string()])
        return var;
    else for (int i = _currentNamespace.size() - 1; i >= 0; i--)
        if (auto var = _currentNamespace[i]->values[node->getVarId().to_string()])
            return var;
    throw "could not find variable";
}

Value *CodeGenerator::codeGen(DST::Variable *node)
{
    if (node->getType()->getExactType() == EXACT_PROPERTY)
    {
        node->getVarId() += ".get";
        auto lval = codeGenLval(node);
        if (!isFunc(lval))
            throw ErrorReporter::report("expression is not a getter property", ERR_CODEGEN, node->getPosition());
        auto func = (llvm::Function*)lval;
        if (func->arg_size() != 0)
            throw ErrorReporter::report("expression is not a getter property", ERR_CODEGEN, node->getPosition());
        return createCallOrInvoke(func, {});
    }
    auto t = codeGenLval(node);
    if (node->getType()->isConst())
        return t;
    if (t->getType()->getPointerElementType()->isFunctionTy())
        return t;
    auto str = node->getVarId().to_string().c_str();
    return _builder.CreateLoad(t, str);
}

AllocaInst *CodeGenerator::codeGenLval(DST::VariableDeclaration *node) { return codeGen(node); }

Value *CodeGenerator::codeGen(DST::ConstDeclaration *node) 
{
    return _namedValues[node->getName().to_string()] = codeGen(node->getExpression());
}

AllocaInst *CodeGenerator::codeGen(DST::VariableDeclaration *node) 
{
    auto type = evalType(node->getType());
    if (type->isVoidTy())
        throw ErrorReporter::report("Cannot create instance of type \"void\"", ERR_CODEGEN, node->getPosition());

    auto name = node->getVarId().to_string();
    if (!_builder.GetInsertBlock())
        throw "will this ever happen? idk...";
    auto func = _builder.GetInsertBlock()->getParent();

    // Create an alloca for the variable in the entry block.
    AllocaInst *alloca = CreateEntryBlockAlloca(func, type, name);
    _namedValues[name] = alloca;

    /*if(type->isArrayTy())
    {
        _builder.CreateStore(llvm::ConstantAggregateZero::get(type), alloca); 
    }*/
    return alloca;
}

llvm::Type *CodeGenerator::evalType(DST::Type *node) 
{
    switch (node->getExactType())
    {
        case EXACT_BASIC:
            if (((DST::BasicType*)node)->getTypeSpecifier()->getInterfaceDecl())
                return _interfaceType;
            else if (((DST::BasicType*)node)->getTypeId() == CONDITION_TYPE)
                return llvm::Type::getInt1Ty(_context);
            else if (((DST::BasicType*)node)->getTypeId() == unicode_string("char"))
                return llvm::Type::getInt8Ty(_context);
            else if (((DST::BasicType*)node)->getTypeId() == unicode_string("int"))
                return llvm::Type::getInt32Ty(_context);
            // else if (((DST::BasicType*)node)->getTypeId() == unicode_string("float"))
            //     return _builder.getFloatTy();
            else if (((DST::BasicType*)node)->getTypeId() == unicode_string("void"))
                return llvm::Type::getVoidTy(_context);
            else if (((DST::BasicType*)node)->getTypeId() == unicode_string("string"))
                return _stringTy;
            //     return llvm::Type::getVoidTy(_context);
            else 
            {
                auto bt = (DST::BasicType*)node;
                if (bt->getTypeSpecifier() && bt->getTypeSpecifier()->getTypeDecl())
                    return _types[bt->getTypeSpecifier()->getTypeDecl()]->structType;
                throw ErrorReporter::report("Type " + node->toShortString() + " does not exist", ERR_CODEGEN, node->getPosition());
            }
        case EXACT_ARRAY:
            if (((DST::ArrayType*)node)->getLength() == DST::UNKNOWN_ARRAY_LENGTH) 
            {
                if (auto exp = ((DST::ArrayType*)node)->getLenExp()) 
                {
                    auto v = codeGen(exp);
                    if (auto len = llvm::dyn_cast<llvm::ConstantInt>(v))
                        return llvm::ArrayType::get(evalType(((DST::ArrayType*)node)->getElementType()), len->getLimitedValue());
                    else return evalType(((DST::ArrayType*)node)->getElementType())->getPointerTo();
                }
                return llvm::StructType::get(_context, {
                    llvm::Type::getInt32Ty(_context), 
                    evalType(((DST::ArrayType*)node)->getElementType())->getPointerTo()
                });
            }
            return llvm::ArrayType::get(evalType(((DST::ArrayType*)node)->getElementType()), ((DST::ArrayType*)node)->getLength());
        case EXACT_PROPERTY:
            return evalType(((DST::PropertyType*)node)->getReturn());
        case EXACT_POINTER:
            if (((DST::PointerType*)node)->getPtrType()->getExactType() == EXACT_BASIC)
            {
                if (((DST::BasicType*)(((DST::PointerType*)node)->getPtrType()))->getTypeSpecifier()->getInterfaceDecl())
                    return _interfaceType;
                if (((DST::BasicType*)(((DST::PointerType*)node)->getPtrType()))->getTypeId() == unicode_string("void"))   // void* is invalid in llvm IR
                    return llvm::Type::getInt8Ty(_context)->getPointerTo();
            }
            return evalType(((DST::PointerType*)node)->getPtrType())->getPointerTo();
        case EXACT_FUNCTION:
        {
            auto ft = (DST::FunctionType*)node;
            vector<llvm::Type*> params;
            for(auto i : ft->getParameters()->getTypes())
                params.push_back(evalType(i));
            return llvm::FunctionType::get(evalType(ft->getReturns()), params, /*isVarArgs=*/false)->getPointerTo();
        }
        case EXACT_TYPELIST:
        {
            auto tl = (DST::TypeList*)node;
            if (tl->size() == 1)
                return evalType(tl->getTypes()[0]);
            else 
            {
                vector<llvm::Type*> types;
                for (auto i : tl->getTypes())
                    types.push_back(evalType(i));
                return llvm::StructType::get(_context, types);    
            }
        }    
        default: throw ErrorReporter::report("Specified type is not currently supported in code generation.", ERR_CODEGEN, node->getPosition());
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
            
            if (((DST::PropertyType*)decl->getReturnType())->hasGet())
            {
                auto funcId = name;
                funcId += ".get";
                vtableIndexes[funcId].index = idx++;
                vtableIndexes[funcId].type = llvm::FunctionType::get(evalType(decl->getReturnType()), _builder.getInt8PtrTy(), false);
            }
            if (((DST::PropertyType*)decl->getReturnType())->hasSet())
            {
                auto funcId = name;
                funcId += ".set";
                vtableIndexes[funcId].index = idx++;
                vtableIndexes[funcId].type = llvm::FunctionType::get(_builder.getVoidTy(), { _builder.getInt8PtrTy(), evalType(decl->getReturnType()) }, false);
            }
        }
        else throw "Getting here should not be possible!";
    }
}

llvm::FunctionType *CodeGenerator::getInterfaceFuncType(DST::FunctionDeclaration *node)
{
    vector<llvm::Type*> types;
    auto params = node->getParameters();
    llvm::Type *returnType = NULL; 
    types.push_back(_builder.getInt8PtrTy());

    // functions that return multiple values return them based on pointers they get as arguments
    bool isMultiReturnFunc = node->getReturnType()->getExactType() == EXACT_TYPELIST && ((DST::TypeList*)node->getReturnType())->size() > 1;
    if (isMultiReturnFunc)
    {
        for (auto i : ((DST::TypeList*)node->getReturnType())->getTypes())
            types.push_back(evalType(i)->getPointerTo());
        returnType = _builder.getVoidTy();
    }
    else returnType = evalType(node->getReturnType());
    
    for (auto i : params) 
        types.push_back(evalType(i->getType()));
    return llvm::FunctionType::get(returnType, types, false);
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
    _vtables[def->structType] = def->vtable;
}

DST::InterfaceDeclaration *CodeGenerator::getPropertyInterface(DST::TypeDeclaration *typeDecl, DST::PropertyDeclaration *propDecl) 
{
    for (auto i : typeDecl->getInterfaces())
        if (auto decl = getPropertyInterface(i, propDecl))
            return decl;
    return NULL;
}

DST::InterfaceDeclaration *CodeGenerator::getPropertyInterface(DST::InterfaceDeclaration *interfaceDecl, DST::PropertyDeclaration *propDecl)
{
    if (interfaceDecl == NULL)
        return NULL;
    if (interfaceDecl->getMemberType(propDecl->getName()))
        return interfaceDecl;
    for (auto i : interfaceDecl->getImplements())
    {
        if (auto decl = getPropertyInterface(i, propDecl))
            return decl;
    }
    return NULL;
} 

DST::InterfaceDeclaration *CodeGenerator::getFunctionInterface(DST::TypeDeclaration *typeDecl, DST::FunctionDeclaration *funcDecl) 
{
    for (auto i : typeDecl->getInterfaces())
        if (auto decl = getFunctionInterface(i, funcDecl))
            return decl;
    return NULL;
}

DST::InterfaceDeclaration *CodeGenerator::getFunctionInterface(DST::InterfaceDeclaration *interfaceDecl, DST::FunctionDeclaration *funcDecl)
{
    if (interfaceDecl == NULL)
        return NULL;
    if (interfaceDecl->getMemberType(funcDecl->getVarDecl()->getVarId()))
        return interfaceDecl;
    for (auto i : interfaceDecl->getImplements())
        if (auto decl = getFunctionInterface(i, funcDecl))
            return decl;
    return NULL;
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
                throw "umm";
            if (((DST::Literal*)opStmnt->getExpression())->getBase()->getLiteralType() != LT_STRING)
                throw "umm2";
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
                throw "umm";
            if (((DST::Literal*)opStmnt->getExpression())->getBase()->getLiteralType() != LT_STRING)
                throw "umm2";
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
                _namedValues[arg.getName()] = alloca;   // Add arguments to variable symbol table.
                // _currThisPtr = alloca;
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
        _namedValues.clear();

        bool isFirst = false;
        for (llvm::Argument &arg : setFunc->args())
        {
            AllocaInst *alloca = CreateEntryBlockAlloca(setFunc, arg.getType(), arg.getName());    // Create an alloca for this variable.
            _builder.CreateStore(&arg, alloca);     // Store the initial value into the alloca.
            _namedValues[arg.getName()] = alloca;   // Add arguments to variable symbol table.

            // if (typeDef && isFirst)
            //     _currThisPtr = alloca;
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
    }
}

llvm::Function * CodeGenerator::declareFunction(DST::FunctionDeclaration *node, CodeGenerator::TypeDefinition *typeDef)
{
    vector<llvm::Type*> types;
    auto params = node->getParameters();

    llvm::Type *returnType = NULL; 

    if (typeDef)
        types.push_back(typeDef->structType->getPointerTo());

    // functions that return multiple values return them based on pointers they get as arguments
    bool isMultiReturnFunc = node->getReturnType()->getExactType() == EXACT_TYPELIST && ((DST::TypeList*)node->getReturnType())->size() > 1;
    if (isMultiReturnFunc)
    {
        for (auto i : ((DST::TypeList*)node->getReturnType())->getTypes())
            types.push_back(evalType(i)->getPointerTo());
        returnType = _builder.getVoidTy();
    }
    else returnType = evalType(node->getReturnType());
    
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
        if (opStmnt->getExpression()->getExpressionType() != ET_LITERAL)
            throw "umm";
        if (((DST::Literal*)opStmnt->getExpression())->getBase()->getLiteralType() != LT_STRING)
            throw "umm2";
        auto strlit = (AST::String*)((DST::Literal*)opStmnt->getExpression())->getBase();
        funcId = strlit->getValue();
    }
    else if (node->getVarDecl()->getVarId().to_string() == "Main")
        funcId = "main";
    else 
    {
        // Todo - add file name as well
        for (auto i : _currentNamespace)
            funcId += i->decl->getName().to_string() + ".";
        if (typeDef) funcId += typeDef->structType->getName().str() + ".";
        funcId += node->getVarDecl()->getVarId().to_string();
    }

    llvm::Function *func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, funcId, _module.get());
    node->_llvmFuncId = func->getName();

    // Set names for all arguments.
    unsigned idx = 0;
    unsigned idx2 = 0;
    bool b = true;
    for (auto &arg : func->args())
    {
        if (idx == 0 && typeDef != nullptr && b)
        {
            arg.setName("this");
            b = false;
        }
        else if (isMultiReturnFunc && idx2 < ((DST::TypeList*)node->getReturnType())->size())
            arg.setName(".ret" + std::to_string(idx2++));
        else arg.setName(params[idx++]->getVarId().to_string());
    }

    _currentNamespace.back()->values[node->getVarDecl()->getVarId()] = func;
    if (typeDef)
        typeDef->functions[node->getVarDecl()->getVarId()] = func;
    return func;
}

void CodeGenerator::codegenFunction(DST::FunctionDeclaration *node, CodeGenerator::TypeDefinition *typeDef)
{
    bool isMultiReturnFunc = node->getReturnType()->getExactType() == EXACT_TYPELIST && ((DST::TypeList*)node->getReturnType())->size() > 1;

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
    _namedValues.clear();
    bool isFirst = true;
    unsigned idx = 0;
    if (isMultiReturnFunc)
        _funcReturns.clear();
    for (llvm::Argument &arg : func->args())
    {
        if (!(isFirst && typeDef) && isMultiReturnFunc && idx < ((DST::TypeList*)node->getReturnType())->size())
        {
            _funcReturns.push_back(&arg);
            idx++;
        }
        else 
        {
            AllocaInst *alloca = CreateEntryBlockAlloca(func, arg.getType(), arg.getName());    // Create an alloca for this variable.
            _builder.CreateStore(&arg, alloca);     // Store the initial value into the alloca.
            _namedValues[arg.getName()] = alloca;   // Add arguments to variable symbol table.

            // if (isFirst && typeDef)
            //     _currThisPtr = alloca;
        }
        
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
}

llvm::Function *CodeGenerator::codeGen(DST::FunctionDeclaration *node)
{
    vector<llvm::Type*> types;
    auto params = node->getParameters();
    for (auto i : params) {
        auto ty = evalType(i->getType());
        if (ty->isVoidTy())
            throw ErrorReporter::report("Cannot create instance of type \"void\"", ERR_CODEGEN, i->getPosition());
        types.push_back(ty);
    }
    auto returnType = evalType(node->getReturnType());
    auto funcType = llvm::FunctionType::get(returnType, types, false);
    auto funcId = node->getVarDecl()->getVarId().to_string();
    llvm::Function *func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, funcId, _module.get());

    // Set names for all arguments.
    unsigned Idx = 0;
    for (auto &arg : func->args())
        arg.setName(params[Idx++]->getVarId().to_string());

    if (node->getContent() == nullptr)
        return func;

    // Create a new basic block to start insertion into.
    llvm::BasicBlock *bb = llvm::BasicBlock::Create(_context, "entry", func);
    _builder.SetInsertPoint(bb);

    // Record the function arguments in the NamedValues map.
    _namedValues.clear();
    for (llvm::Argument &arg : func->args())
    {
        AllocaInst *alloca = CreateEntryBlockAlloca(func, arg.getType(), arg.getName());    // Create an alloca for this variable.
        _builder.CreateStore(&arg, alloca);     // Store the initial value into the alloca.
        _namedValues[arg.getName()] = alloca;   // Add arguments to variable symbol table.
    }

    for (auto i : node->getContent()->getStatements()) 
    {
        auto val = codeGen(i);
        if (val == nullptr)
            throw ErrorReporter::report("Error while generating IR for statement", ERR_CODEGEN, i->getPosition());
    }

    if (!_builder.GetInsertBlock()->getTerminator())
        _builder.CreateBr(_builder.GetInsertBlock());
    
    if (!llvm::verifyFunction(*func, &llvm::errs()))
        llvm::errs() << "Function Vertified!\n---------------------------\n";

    return func;
}

llvm::Function *CodeGenerator::getParentFunction() 
{
    return _builder.GetInsertBlock() ? _builder.GetInsertBlock()->getParent() : nullptr;
}

llvm::Value *CodeGenerator::codeGen(DST::DoWhileLoop *node)
{
    auto tmpBreakJmp = _currBreakJmp;
    auto tmpContinueJmp = _currContinueJmp;
    auto parent = getParentFunction();
    llvm::BasicBlock *condBB = llvm::BasicBlock::Create(_context, "cond");
    llvm::BasicBlock *loopBB = llvm::BasicBlock::Create(_context, "loop");
    llvm::BasicBlock *exitBB = llvm::BasicBlock::Create(_context, "exitLoop");

    _currContinueJmp = condBB;
    _currBreakJmp = exitBB;

    // first go to the loop statement.
    _builder.CreateBr(loopBB);

    // add the block to the function.
    parent->getBasicBlockList().push_back(loopBB);

    // generate loop statement's code.
    _builder.SetInsertPoint(loopBB);
    if (node->getStatement()->getStatements().size() != 0)
    {
        for (auto i : node->getStatement()->getStatements()) 
        {
            auto val = codeGen(i);
            if (val == nullptr)
                throw ErrorReporter::report("Error while generating IR for statement", ERR_CODEGEN, i->getPosition());
        }
    }
    // at the end of the statements: add a jump statement to the condition.
    _builder.CreateBr(condBB);

    // generate condition block.
    _builder.SetInsertPoint(condBB);
    Value *cond = codeGen(node->getCondition());
    
    parent->getBasicBlockList().push_back(condBB);

    // loop condition statement.
    auto br = _builder.CreateCondBr(cond, loopBB, exitBB);
    
    // added exit_block
    parent->getBasicBlockList().push_back(exitBB);
    _builder.SetInsertPoint(exitBB);
    _currBreakJmp = tmpBreakJmp;
    _currContinueJmp = tmpContinueJmp;
    return br;
}

llvm::Value *CodeGenerator::codeGen(DST::ForLoop *node)
{
    codeGen(node->getBegin());
    auto parent = getParentFunction();
    llvm::BasicBlock *condBB = llvm::BasicBlock::Create(_context, "cond");
    llvm::BasicBlock *loopBB = llvm::BasicBlock::Create(_context, "loop");
    llvm::BasicBlock *exitBB = llvm::BasicBlock::Create(_context, "exitLoop");

    _builder.CreateBr(condBB);
    _builder.SetInsertPoint(condBB);
    Value *cond = codeGen(node->getCondition());
    auto br = _builder.CreateCondBr(cond, loopBB, exitBB);

    parent->getBasicBlockList().push_back(condBB);

    parent->getBasicBlockList().push_back(loopBB);
    _builder.SetInsertPoint(loopBB);
    if (node->getStatement()->getStatements().size() != 0)
    {
        for (auto i : node->getStatement()->getStatements()) 
        {
            auto val = codeGen(i);
            if (val == nullptr)
                throw ErrorReporter::report("Error while generating IR for statement", ERR_CODEGEN, i->getPosition());
        }
    }
    codeGen(node->getIncrement());
    _builder.CreateBr(condBB);
    parent->getBasicBlockList().push_back(exitBB);
    _builder.SetInsertPoint(exitBB);
    return br;
}

llvm::Value *CodeGenerator::codeGen(DST::WhileLoop *node)
{
    auto tmpBreakJmp = _currBreakJmp;
    auto tmpContinueJmp = _currContinueJmp;
    auto parent = getParentFunction();
    llvm::BasicBlock *condBB = llvm::BasicBlock::Create(_context, "cond");
    llvm::BasicBlock *loopBB = llvm::BasicBlock::Create(_context, "loop");
    llvm::BasicBlock *exitBB = llvm::BasicBlock::Create(_context, "exitLoop");

    _currContinueJmp = condBB;
    _currBreakJmp = exitBB;

    _builder.CreateBr(condBB);
    _builder.SetInsertPoint(condBB);
    Value *cond = codeGen(node->getCondition());
    auto br = _builder.CreateCondBr(cond, loopBB, exitBB);

    parent->getBasicBlockList().push_back(condBB);

    parent->getBasicBlockList().push_back(loopBB);
    _builder.SetInsertPoint(loopBB);
    if (node->getStatement()->getStatements().size() != 0)
    {
        for (auto i : node->getStatement()->getStatements()) 
        {
            auto val = codeGen(i);
            if (val == nullptr)
                throw ErrorReporter::report("Error while generating IR for statement", ERR_CODEGEN, i->getPosition());
        }
    }
    _builder.CreateBr(condBB);
    parent->getBasicBlockList().push_back(exitBB);
    _builder.SetInsertPoint(exitBB);
    _currBreakJmp = tmpBreakJmp;
    _currContinueJmp = tmpContinueJmp;
    return br;
}

llvm::Value *CodeGenerator::codeGen(DST::TryCatch *node)
{

    auto parent = getParentFunction();
    // static llvm::Function *personality = _module.get()->getFunction("__gxx_personality_v0");
    static llvm::Function *personality = NULL;
    static llvm::Function *beginCatchFunc = NULL;
    static llvm::Function *endCatchFunc = NULL;

    if (personality == NULL)
    {
        auto presonalityFuncTy = llvm::FunctionType::get(_builder.getInt32Ty(), {}, true);
        personality = llvm::Function::Create(presonalityFuncTy, llvm::Function::ExternalLinkage, "__gxx_personality_v0", _module.get());

        auto beginCatchTy = llvm::FunctionType::get(_builder.getInt8PtrTy(), _builder.getInt8PtrTy(), true);
        beginCatchFunc = llvm::Function::Create(beginCatchTy, llvm::Function::ExternalLinkage, "__cxa_begin_catch", _module.get());

        auto endCatchTy = llvm::FunctionType::get(_builder.getVoidTy(), {}, true);
        endCatchFunc = llvm::Function::Create(endCatchTy, llvm::Function::ExternalLinkage, "__cxa_end_catch", _module.get());
    }

    parent->setPersonalityFn(personality);

    llvm::BasicBlock *catchBB = llvm::BasicBlock::Create(_context, "catch");
    llvm::BasicBlock *mergeBB = NULL;
    if (!(node->getTryBlock()->hasReturn() && node->getCatchBlock()->hasReturn()))
        mergeBB = llvm::BasicBlock::Create(_context, "tryCont");    // No need to create a continue branch if it's unreachable

    auto lastPad = _currCatchBlock;
    _currCatchBlock = catchBB;
    for (auto i : node->getTryBlock()->getStatements())
        if (!codeGen(i))
            throw ErrorReporter::report("Error while generating IR for statement", ERR_CODEGEN, i->getPosition());
    if (!_builder.GetInsertBlock()->getTerminator())
        _builder.CreateBr(mergeBB);
    _currCatchBlock = lastPad;


    parent->getBasicBlockList().push_back(catchBB);
    _builder.SetInsertPoint(catchBB);
    // auto pad = _builder.CreateLandingPad(_interfaceType->getPointerTo(), 1);
    auto pad = _builder.CreateLandingPad(_builder.getInt8PtrTy(), 1);
    // pad->addClause(llvm::ConstantPointerNull::get(_interfaceType->getPointerTo()) );
    pad->addClause(llvm::ConstantPointerNull::get(_builder.getInt8PtrTy()));

    auto caught = _builder.CreateCall(beginCatchFunc, pad);
    _namedValues["caught"] = _builder.CreateBitCast(caught, _interfaceType->getPointerTo());

    for (auto i : node->getCatchBlock()->getStatements())
        if (!codeGen(i))
            throw ErrorReporter::report("Error while generating IR for statement", ERR_CODEGEN, i->getPosition());

    _builder.CreateCall(endCatchFunc, {});

    if (mergeBB)
    {
        if (!_builder.GetInsertBlock()->getTerminator())
            _builder.CreateBr(mergeBB);
        parent->getBasicBlockList().push_back(mergeBB);
        _builder.SetInsertPoint(mergeBB);
    }

    return catchBB;

    // static llvm::Function *setJmpFunc = NULL;

    // if (setJmpFunc == NULL)
    // {
    //     auto type = llvm::FunctionType::get(_builder.getInt32Ty(), _builder.getInt8PtrTy(), false);
    //     setJmpFunc = llvm::Function::Create(type, llvm::Function::ExternalLinkage, "llvm.eh.sjlj.setjmp", _module.get());
    // }

    // auto jmpBufVal = _builder.CreateLoad(_globJmpBuf);

    // llvm::BasicBlock *tryBB = llvm::BasicBlock::Create(_context, "try");
    // llvm::BasicBlock *catchBB = llvm::BasicBlock::Create(_context, "catch");
    // llvm::BasicBlock *mergeBB = NULL;
    // if (!(node->getTryBlock()->hasReturn() && node->getCatchBlock()->hasReturn()))
    //     mergeBB = llvm::BasicBlock::Create(_context, "tryCont");    // No need to create a continue branch if it's unreachable

    // auto val = _builder.CreateCall(setJmpFunc,  _builder.CreateBitCast(_globJmpBuf, _builder.getInt8PtrTy()));
    // auto cond = _builder.CreateICmpEQ(val, _builder.getInt32(0));
    // _builder.CreateCondBr(cond, tryBB, catchBB);

    // auto parent = getParentFunction();

    // parent->getBasicBlockList().push_back(tryBB);
    // _builder.SetInsertPoint(tryBB);
    // for (auto i : node->getTryBlock()->getStatements()) 
    //     if (codeGen(i) == nullptr) 
    //         throw ErrorReporter::report("Error while generating IR for statement", ERR_CODEGEN, i->getPosition());
    // // tryBB->getInstList().insertBefore(NULL);
    // if (auto tryTerminator = _builder.GetInsertBlock()->getTerminator())
    //     llvm::StoreInst(jmpBufVal, _globJmpBuf, tryTerminator);
    // else 
    // {
    //     _builder.CreateStore(jmpBufVal, _globJmpBuf);
    //     _builder.CreateBr(mergeBB);
    // }

    // parent->getBasicBlockList().push_back(catchBB);
    // _builder.SetInsertPoint(catchBB);
    // _builder.CreateStore(jmpBufVal, _globJmpBuf);
    // for (auto i : node->getCatchBlock()->getStatements()) 
    //     if (codeGen(i) == nullptr) 
    //         throw ErrorReporter::report("Error while generating IR for statement", ERR_CODEGEN, i->getPosition());
    
    

    // if (mergeBB)
    // {
    //     if (!_builder.GetInsertBlock()->getTerminator())
    //         _builder.CreateBr(mergeBB);
    //     parent->getBasicBlockList().push_back(mergeBB);
    //     _builder.SetInsertPoint(mergeBB);
    // }
    // return val;
}

llvm::Value *CodeGenerator::codeGen(DST::SwitchCase *node) 
{
    auto startBlock = _builder.GetInsertBlock();
    auto val = codeGen(node->getExpression());
    bool alwaysReturns = node->getDefault() && node->getDefault()->hasReturn();
    if (alwaysReturns)
        for (auto i : node->getCases())
            if (!i._statement->hasReturn()) 
                { alwaysReturns = false; break; }
    
    llvm::BasicBlock *mergeBB = NULL;
    if (!alwaysReturns)
        mergeBB = llvm::BasicBlock::Create(_context, "switchcont");

    auto defBB = codeGen(node->getDefault(), "default");
    if (mergeBB && !_builder.GetInsertBlock()->getTerminator())
        _builder.CreateBr(mergeBB);
    unsigned int count = 0;
    for (auto i : node->getCases())
        count += i._expressions.size();
    _builder.SetInsertPoint(startBlock);
    auto ret = _builder.CreateSwitch(val, defBB, count);

    for (auto i : node->getCases()) 
    {
        auto block = codeGen(i._statement);
        if (mergeBB && !_builder.GetInsertBlock()->getTerminator())
            _builder.CreateBr(mergeBB);
        for (auto exp : i._expressions) 
        {
            auto v = codeGen(exp);
            if (llvm::isa<llvm::ConstantInt>(v))
                ret->addCase((llvm::ConstantInt*)v, block);
            else throw ErrorReporter::report("case clause must be a constant enumerable value", ERR_CODEGEN, exp->getPosition());
        }
    }
    if (mergeBB) 
    {
        getParentFunction()->getBasicBlockList().push_back(mergeBB);
        _builder.SetInsertPoint(mergeBB);
    }
    // ret->print(llvm::errs());
    return ret;
}

llvm::Value *CodeGenerator::codeGen(DST::IfThenElse *node)
{
    Value *cond = codeGen(node->getCondition());

    auto parent = getParentFunction();

    llvm::BasicBlock *thenBB = llvm::BasicBlock::Create(_context, "then");
    llvm::BasicBlock *elseBB = llvm::BasicBlock::Create(_context, "else");

    llvm::BasicBlock *mergeBB = NULL;
    if (!(node->getThenBranch() && node->getElseBranch() && node->getThenBranch()->hasReturn() && node->getElseBranch()->hasReturn()))
        mergeBB = llvm::BasicBlock::Create(_context, "ifcont"); // No need to create a continue branch if it's unreachable
    llvm::BranchInst *br = _builder.CreateCondBr(cond, thenBB, elseBB);
    parent->getBasicBlockList().push_back(thenBB);
    _builder.SetInsertPoint(thenBB);
    if (node->getThenBranch())
    {
        for (auto i : node->getThenBranch()->getStatements()) 
        {
            auto val = codeGen(i);
            if (val == nullptr)
                throw ErrorReporter::report("Error while generating IR for statement", ERR_CODEGEN, i->getPosition());
        }
    }

    if (mergeBB && !_builder.GetInsertBlock()->getTerminator())
        _builder.CreateBr(mergeBB);

    parent->getBasicBlockList().push_back(elseBB);
    _builder.SetInsertPoint(elseBB);
    if (node->getElseBranch())
    {
        for (auto i : node->getElseBranch()->getStatements()) 
        {
            auto val = codeGen(i);
            if (val == nullptr)
                throw ErrorReporter::report("Error while generating IR for statement", ERR_CODEGEN, i->getPosition());
        }
    } 
    if (mergeBB)
    {
        if (!_builder.GetInsertBlock()->getTerminator())
            _builder.CreateBr(mergeBB);
        parent->getBasicBlockList().push_back(mergeBB);
        _builder.SetInsertPoint(mergeBB);
    }
    return br;
}