#include "CodeGenerator.h"

void CodeGenerator::setup()
{
    // Nothing yet
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
}

void CodeGenerator::writeBitcodeToFile(string fileName) 
{
    //llvm::errs() << *_module.get();
    fstream file(fileName);
    std::error_code ec;
    llvm::raw_fd_ostream out(fileName, ec, llvm::sys::fs::F_None);
    llvm::WriteBitcodeToFile(_module.get(), out);
}

void CodeGenerator::execute(llvm::Function *func)
{
    // Create Interpreter
    llvm::Module *M = _module.get();
    
    std::string errStr;
    llvm::ExecutionEngine *EE = llvm::EngineBuilder(std::move(_module)).setErrorStr(&errStr).setEngineKind(llvm::EngineKind::Interpreter).create();

    if (!EE) {
        llvm::errs() << ": Failed to construct ExecutionEngine: " << errStr << "\n";
        return;
    }

    llvm::errs() << "We are trying to construct this LLVM module:\n\n---------\n" << *M;
    llvm::errs() << "verifying... ";
    if (llvm::verifyModule(*M)) {
        llvm::errs() << ": Error constructing function!\n";
        return;
    }

    llvm::errs() << "OK\n";
    llvm::errs() << "We just constructed this LLVM module:\n\n---------\n" << *M;
    llvm::errs() << "---------\nstarting with Interpreter...\n";

    std::vector<llvm::GenericValue> noargs;
    
    if (llvm::verifyFunction(*func, &llvm::errs()))
        std::cout << "Huh\n";
    llvm::GenericValue GV = EE->runFunction(func, noargs);

    llvm::outs() << "Result: " << GV.IntVal << "\n";
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
                _currentNamespace.push_back(currentNs);
                declareNamespaceTypes((DST::NamespaceDeclaration*)member);
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
            case ST_TYPE_DECLARATION:
                declareTypeContent((DST::TypeDeclaration*)member);
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
            default: break;
        }
    }
}

// Returns a pointer to the Main function
llvm::Function *CodeGenerator::startCodeGen(DST::Program *node) 
{
    llvm::Function *ret = NULL;

    // auto type = llvm::FunctionType::get(llvm::Type::getInt64Ty(_context), { llvm::Type::getInt64Ty(_context), llvm::Type::getInt8Ty(_context)->getPointerTo()->getPointerTo() }, false);
    // auto mainFunc = llvm::Function::Create(type, llvm::Function::ExternalLinkage, "main", _module.get());


    for (auto i : node->getNamespaces())
    {
        auto currentNs = _namespaces[i.first] = new NamespaceMembers();
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
    switch (node->getStatementType()) 
    {
        case ST_ASSIGNMENT: return codeGen((DST::Assignment*)node);
        case ST_FUNCTION_DECLARATION: return codeGen((DST::FunctionDeclaration*)node);
        case ST_STATEMENT_BLOCK: return codeGen((DST::StatementBlock*)node);
        case ST_UNARY_OPERATION: return codeGen((DST::UnaryOperationStatement*)node);
        case ST_VARIABLE_DECLARATION: return codeGen((DST::VariableDeclaration*)node);
        case ST_IF_THEN_ELSE: return codeGen((DST::IfThenElse*)node);
        case ST_WHILE_LOOP: return codeGen((DST::WhileLoop*)node);
        case ST_DO_WHILE_LOOP: return codeGen((DST::DoWhileLoop*)node);
        case ST_FOR_LOOP: return codeGen((DST::ForLoop*)node);
        case ST_FUNCTION_CALL: return codeGen((DST::FunctionCall*)node);
        default: return NULL;
    }
    TypeDefinition t;
}

Value *CodeGenerator::codeGen(DST::Expression *node) 
{
    if (node == nullptr)
        return NULL;
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
        case ET_CONVERSION: return codeGen((DST::Conversion*)node);
        default: return NULL;
    }
}

llvm::BasicBlock *CodeGenerator::codeGen(DST::StatementBlock *node, const llvm::Twine &blockName)
{
    // Get parent function
    auto parent = getParentFunction();

    // Create BasicBlock
    llvm::BasicBlock *bb = llvm::BasicBlock::Create(_context, blockName, parent);

    _builder.SetInsertPoint(bb);
    for (auto i : node->getStatements()) 
    {
        auto val = codeGen(i);
        if (val == nullptr)
            throw DinoException("Error while generating ir for statementblock", EXT_GENERAL, node->getLine());
    }
    return bb;
}

Value *CodeGenerator::codeGen(DST::Literal *node) 
{
    switch (node->getLiteralType())
    {
    case LT_BOOLEAN:
        return llvm::ConstantInt::get(_context, llvm::APInt( /* 1 bit width */ 1, ((AST::Boolean*)node->getBase())->getValue()));
    case LT_CHARACTER:
        return llvm::ConstantInt::get(_context, llvm::APInt( /* 8 bit width */ 8, ((AST::Character*)node->getBase())->getValue()));
    case LT_INTEGER:
        return llvm::ConstantInt::get(_context, llvm::APInt( /* 64 bit width */ 64, ((AST::Integer*)node->getBase())->getValue(), /* signed */ true));
    case LT_FRACTION:
        return llvm::ConstantFP::get(_context, llvm::APFloat(((AST::Fraction*)node->getBase())->getValue()));
    case LT_NULL:
        return llvm::Constant::getNullValue(_builder.getInt8Ty()->getPointerTo());  // 'void*' is invalid in llvm IR
        //return _builder.getInt32(NULL);
    default:
        return NULL;
    }
}

CodeGenerator::NamespaceMembers *CodeGenerator::getNamespaceMembers(DST::Expression *node)
{
    if (node->getExpressionType() == ET_MEMBER_ACCESS)
    {
        auto x = (DST::MemberAccess*)node;
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
        auto bt = (DST::BasicType*)leftType;
        auto typeDef = _types[bt->getTypeSpecifier()->getTypeDecl()];
        return _builder.CreateInBoundsGEP(
            typeDef->structType, 
            codeGenLval(node->getLeft()), 
            { _builder.getInt32(0), _builder.getInt32(typeDef->variableIndexes[node->getRight()]) }, 
            node->getRight().to_string()
        );
    }
    else if (leftType->getExactType() == EXACT_POINTER && 
            ((DST::PointerType*)leftType)->getPtrType()->getExactType() == EXACT_BASIC)
    {
        auto bt = (DST::BasicType*)((DST::PointerType*)leftType)->getPtrType();
        auto typeDef = _types[bt->getTypeSpecifier()->getTypeDecl()];
        return _builder.CreateInBoundsGEP(
            typeDef->structType, 
            _builder.CreateLoad(codeGenLval(node->getLeft())), 
            { _builder.getInt32(0), _builder.getInt32(typeDef->variableIndexes[node->getRight()]) }, 
            node->getRight().to_string()
        );
    }
    else 
    {
        std::cout << leftType->toShortString() << '\n';
        std::cout << (leftType->getExactType()) << '\n';
        std::cout << (((DST::PointerType*)leftType)->getPtrType()->getExactType() == EXACT_BASIC) << '\n';
        throw DinoException("Expression must be of class or namespace type", EXT_GENERAL, node->getLine());
    }
}

Value *CodeGenerator::codeGen(DST::MemberAccess *node)
{
    if (node->getType()->getExactType() == EXACT_PROPERTY)
    {
        node->getRight() += ".get";
        auto leftTy = node->getLeft()->getType();
        if (leftTy->getExactType() == EXACT_TYPELIST && ((DST::TypeList*)leftTy)->getTypes().size() == 1)
            leftTy = ((DST::TypeList*)leftTy)->getTypes()[0];
        switch (leftTy->getExactType())
        {
            case EXACT_NAMESPACE:   // Static getter property
            {
                auto func = codeGenLval(node);
                if (!isFunc(func) || ((llvm::Function*)func)->arg_size() != 0)
                    throw DinoException("expression is not a getter property", EXT_GENERAL, node->getLine());
                return _builder.CreateCall((llvm::Function*)func, {}, "calltmp");
            }
            case EXACT_BASIC:       // Member getter property of basic type
            {
                auto lval = codeGenLval(node->getLeft());
                auto typeDef = _types[((DST::BasicType*)leftTy)->getTypeSpecifier()->getTypeDecl()];
                auto func = typeDef->functions[node->getRight()];
                if (!isFunc(func) || ((llvm::Function*)func)->arg_size() != 1)
                    throw DinoException("expression is not a getter property", EXT_GENERAL, node->getLine());
                return _builder.CreateCall((llvm::Function*)func, { lval }, "calltmp");
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
                    throw DinoException("expression is not a getter property", EXT_GENERAL, node->getLine());
                return _builder.CreateCall((llvm::Function*)func, { lval }, "calltmp");
            }
            case EXACT_ARRAY:       // Member property of array
                throw DinoException("Unimplemented Error no.2", EXT_GENERAL, node->getLine());   // TODO
            default:
                throw DinoException("Unimplemented Error no.3 | " + leftTy->toShortString() + std::to_string(leftTy->getExactType()), EXT_GENERAL, node->getLine());   // TODO
        }
    }
    return _builder.CreateLoad(codeGenLval(node), "accesstmp");
}

Value *CodeGenerator::codeGen(DST::BinaryOperation* node)
{
    Value *left = codeGen(node->getLeft());
    Value *right = codeGen(node->getRight());
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
            return _builder.CreateICmpULT(left, right, "cmptmp");
        case OT_SMALLER_EQUAL:
            return _builder.CreateICmpULE(left, right, "cmptmp");
        case OT_GREATER:
            return _builder.CreateICmpUGT(left, right, "cmptmp");
        case OT_GREATER_EQUAL:
            return _builder.CreateICmpUGE(left, right, "cmptmp");
        case OT_EQUAL:
        {
            if (left->getType() == right->getType())
                return _builder.CreateICmpEQ(left, right, "cmptmp");
            return _builder.CreateICmpEQ(_builder.CreateBitCast(left, right->getType()), right, "cmptmp");
        }
        case OT_NOT_EQUAL:
            return _builder.CreateICmpNE(left, right, "cmptmp");
        case OT_LOGICAL_AND:
            return _builder.CreateAnd(left, right, "andtmp");
        case OT_LOGICAL_OR:
            return _builder.CreateOr(left, right, "ortmp");
        default:
            throw DinoException("Unimplemented Binary operation!", EXT_GENERAL, node->getLine());
    }
    
}

Value *CodeGenerator::codeGen(DST::UnaryOperation* node)
{
    static llvm::Function *malloc = NULL;

    switch (node->getOperator()._type)
    {
        case OT_NEW:
        {
            if (malloc == NULL)
            {
                auto type = llvm::FunctionType::get(llvm::Type::getInt8Ty(_context)->getPointerTo(), { llvm::Type::getInt32Ty(_context) }, false);
                malloc = llvm::Function::Create(type, llvm::Function::ExternalLinkage, "malloc", _module.get());
            }
            auto type = evalType((DST::Type*)node->getExpression());
            int size = _dataLayout->getTypeAllocSize(type);
            return _builder.CreateBitCast( _builder.CreateCall(malloc, { _builder.getInt32(size) }), type->getPointerTo(), "newTmp");
        }
        case OT_ADD:
            return codeGen(node->getExpression());
        case OT_SUBTRACT:
            return NULL;
        case OT_AT:
            return _builder.CreateLoad(codeGen(node->getExpression()));
        case OT_BITWISE_AND:
            return _builder.CreateGEP(codeGenLval(node->getExpression()), _builder.getInt32(0));
        default:
            return NULL;
    }
    
}

Value *CodeGenerator::codeGenLval(DST::UnaryOperation* node)
{
    Value *val = codeGenLval(node->getExpression());
    switch (node->getOperator()._type)
    {
        case OT_AT:
            return _builder.CreateLoad(val);
        case OT_BITWISE_AND:
            return _builder.CreateGEP(val, _builder.getInt32(0));
        default:
            throw DinoException("Unimplemented binary operation", EXT_GENERAL, node->getLine());
    }
    
}

Value *CodeGenerator::codeGenLval(DST::Expression *node)
{
    if (node == nullptr)
        return NULL;
    switch (node->getExpressionType()) 
    {
        case ET_IDENTIFIER: return codeGenLval((DST::Variable*)node);
        case ET_VARIABLE_DECLARATION: return codeGenLval((DST::VariableDeclaration*)node);
        case ET_MEMBER_ACCESS: return codeGenLval((DST::MemberAccess*)node);
        case ET_UNARY_OPERATION: return codeGenLval((DST::UnaryOperation*)node);
        default: throw DinoException("unimplemented lval expression type.", EXT_GENERAL, node->getLine());
    }
}

Value *CodeGenerator::codeGen(DST::Conversion* node)
{
    return _builder.CreateBitCast(codeGen(node->getExpression()), evalType(node->getType()), "cnvrttmp");
}

Value *CodeGenerator::codeGen(DST::Assignment* node)
{
    Value *left = NULL;
    Value *right = NULL;

    if (node->getLeft()->getType()->getExactType() == EXACT_PROPERTY) // TODO
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
                        throw DinoException("expression is not a setter property", EXT_GENERAL, node->getLine());
                    _builder.CreateCall((llvm::Function*)left, { codeGen(node->getRight()) });
                }
                case EXACT_BASIC:       // Member setter property of basic type
                {
                    auto typeDef = _types[((DST::BasicType*)ac->getLeft()->getType())->getTypeSpecifier()->getTypeDecl()];
                    auto func = typeDef->functions[ac->getRight()];
                    if (!isFunc(func) || ((llvm::Function*)func)->arg_size() != 2)
                        throw DinoException("expression is not a setter property", EXT_GENERAL, node->getLine());
                    return _builder.CreateCall((llvm::Function*)func, { codeGenLval(ac->getLeft()), codeGen(node->getRight()) });
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
                        throw DinoException("expression is not a setter property", EXT_GENERAL, node->getLine());
                    return _builder.CreateCall((llvm::Function*)func, { thisPtr, codeGen(node->getRight()) });
                    //throw DinoException("Unimplemented Error no.1", EXT_GENERAL, node->getLine());   // TODO
                }
                case EXACT_ARRAY:       // Member property of array
                    throw DinoException("Unimplemented Error no.2", EXT_GENERAL, node->getLine());   // TODO
                default:
                    throw DinoException("Unimplemented Error no.3 | " + ac->getLeft()->getType()->toShortString(), EXT_GENERAL, node->getLine());   // TODO
            }
        }

        left = codeGenLval(node->getLeft());
        if (!isFunc(left))
            throw DinoException("expression is not a setter property", EXT_GENERAL, node->getLine());

        _builder.CreateCall((llvm::Function*)left, { codeGen(node->getRight()) });
        return right;
    }

    left = codeGenLval(node->getLeft());
    right = codeGen(node->getRight());
    switch (node->getOperator()._type) 
    {
        case OT_ASSIGN_EQUAL:
            return _builder.CreateStore(right, left);
        case OT_ASSIGN_ADD:
            return _builder.CreateStore(_builder.CreateAdd(_builder.CreateLoad(left), right, "addtmp"), left);
        case OT_ASSIGN_SUBTRACT:
            return _builder.CreateStore(_builder.CreateSub(_builder.CreateLoad(left), right, "subtmp"), left);
        case OT_ASSIGN_MULTIPLY:
            return _builder.CreateStore(_builder.CreateMul(_builder.CreateLoad(left), right, "multmp"), left);
        case OT_ASSIGN_DIVIDE:
            return _builder.CreateStore(_builder.CreateSDiv(_builder.CreateLoad(left), right, "multmp"), left);
        case OT_ASSIGN_MODULUS:
            return _builder.CreateStore(_builder.CreateSRem(_builder.CreateLoad(left), right, "multmp"), left);
        default: throw DinoException("Unimplemented assignment operator", EXT_GENERAL, node->getLine());
    
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

Value *CodeGenerator::codeGen(DST::FunctionCall *node)
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
            llvm::Function *func = NULL;    
            if (ty->getExactType() == EXACT_BASIC)
                typeDef = _types[((DST::BasicType*)ty)->getTypeSpecifier()->getTypeDecl()];
            else 
            {
                if (((DST::PointerType*)ty)->getPtrType()->getExactType() != EXACT_BASIC)
                    throw DinoException("Internal decorator error?", EXT_GENERAL, node->getLine());
                typeDef = _types[((DST::BasicType*)((DST::PointerType*)ty)->getPtrType())->getTypeSpecifier()->getTypeDecl()];
            }
            
            func = typeDef->functions[funcId];

            if (func->arg_size() != node->getArguments()->getExpressions().size() + 1) // + 1 since we are also passing a "this" ptr
                throw DinoException(string("Incorrect # arguments passed (needed ") + 
                    std::to_string(func->arg_size()) + ", got " + std::to_string(node->getArguments()->getExpressions().size()) + ")"
                    , EXT_GENERAL, node->getLine());
            
            std::vector<Value*> args;
            if (ty->getExactType() == EXACT_POINTER)
                args.push_back(codeGen(((DST::MemberAccess*)node->getFunctionId())->getLeft()));    // the "this" pointer
            else args.push_back(codeGenLval(((DST::MemberAccess*)node->getFunctionId())->getLeft()));

        
            int i = -1;
            for (auto &arg : func->args())
            {
                if (i == -1) { i++; continue; }
                auto gen = codeGen(node->getArguments()->getExpressions()[i++]);        
                if (gen->getType() != arg.getType())
                    args.push_back(_builder.CreateBitCast(gen, arg.getType(), "castTmp"));
                else args.push_back(gen);
            }

            return _builder.CreateCall(func, args);

        }
    }


    llvm::Value *funcPtr = codeGenLval(node->getFunctionId());
    llvm::Function *func = NULL;
    if (isFunc(funcPtr))
        func = (llvm::Function*)funcPtr;
    else throw DinoException("expression is not a function!", EXT_GENERAL, node->getLine());

    if (func->arg_size() != node->getArguments()->getExpressions().size())
        throw DinoException(string("Incorrect # arguments passed (needed ") + 
            std::to_string(func->arg_size()) + ", got " + std::to_string(node->getArguments()->getExpressions().size()) + ")"
            , EXT_GENERAL, node->getLine());
        

    std::vector<Value *> args;

    int i = 0;
    for (auto &arg : func->args())
    {
        auto gen = codeGen(node->getArguments()->getExpressions()[i++]);        
        if (gen->getType() != arg.getType())
            args.push_back(_builder.CreateBitCast(gen, arg.getType(), "castTmp"));
        else args.push_back(gen);
    }
            
    return _builder.CreateCall(func, args);
}

Value *CodeGenerator::codeGen(DST::UnaryOperationStatement *node)
{
    static llvm::Function *free = NULL;
    
    switch (node->getOperator()._type)
    {
        case OT_RETURN:
            return _builder.CreateRet(codeGen(node->getExpression()));

        case OT_DELETE:
        {
            if (free == NULL)
            {
                auto type = llvm::FunctionType::get(llvm::Type::getVoidTy(_context), { llvm::Type::getInt8Ty(_context)->getPointerTo() }, false);
                free = llvm::Function::Create(type, llvm::Function::ExternalLinkage, "free", _module.get());
            }
            auto ptr = codeGenLval(node->getExpression());
            
            auto cast = _builder.CreateBitCast(_builder.CreateLoad(ptr), llvm::Type::getInt8Ty(_context)->getPointerTo(), "castTmp");

            _builder.CreateCall(free, { cast });
            return _builder.CreateStore(llvm::Constant::getNullValue(ptr->getType()->getPointerElementType()), ptr);
        }
        default: throw DinoException("Unimplemented unary operation statement!", EXT_GENERAL, node->getLine());
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
            throw DinoException("expression is not a getter property", EXT_GENERAL, node->getLine());
        auto func = (llvm::Function*)lval;
        if (func->arg_size() != 0)
            throw DinoException("expression is not a getter property", EXT_GENERAL, node->getLine());
        return _builder.CreateCall(func, {}, "calltmp");
    }
    auto t = codeGenLval(node);
    auto str = node->getVarId().to_string().c_str();
    return _builder.CreateLoad(t, str);
    //return _builder.CreateLoad(codeGenLval(node), node->getVarId().to_string().c_str());
}

AllocaInst *CodeGenerator::codeGenLval(DST::VariableDeclaration *node) { return codeGen(node); }

AllocaInst *CodeGenerator::codeGen(DST::VariableDeclaration *node) 
{
    auto type = evalType(node->getType());
    auto name = node->getVarId().to_string();
    if (!_builder.GetInsertBlock())
        throw "will this ever happen? idk...";
    auto func = _builder.GetInsertBlock()->getParent();

    // Create an alloca for the variable in the entry block.
    AllocaInst *alloca = CreateEntryBlockAlloca(func, type, name);
    _namedValues[name] = alloca;
    return alloca;
}

llvm::Type *CodeGenerator::evalType(DST::Type *node) 
{
    if (node->getExactType() == EXACT_BASIC)
    {
        if (((DST::BasicType*)node)->getTypeId() == CONDITION_TYPE)
            return llvm::Type::getInt1Ty(_context);
        else if (((DST::BasicType*)node)->getTypeId() == unicode_string("char"))
            return llvm::Type::getInt8Ty(_context);
        else if (((DST::BasicType*)node)->getTypeId() == unicode_string("int"))
            return llvm::Type::getInt64Ty(_context);
        else if (((DST::BasicType*)node)->getTypeId() == unicode_string("void"))
            return llvm::Type::getVoidTy(_context);
        else 
        {
            auto bt = (DST::BasicType*)node;
            if (bt->getTypeSpecifier() && bt->getTypeSpecifier()->getTypeDecl())
                return _types[bt->getTypeSpecifier()->getTypeDecl()]->structType;
            throw DinoException("Type " + node->toShortString() + "does not exist", EXT_GENERAL, node->getLine());
        }
    }
    else if (node->getExactType() == EXACT_ARRAY)
        return llvm::ArrayType::get(evalType(((DST::ArrayType*)node)->getElementType()), ((DST::ArrayType*)node)->getLength());
    else if (node->getExactType() == EXACT_PROPERTY)
        return evalType(((DST::PropertyType*)node)->getReturn());
    if (node->getExactType() == EXACT_POINTER)
    {
        if (((DST::PointerType*)node)->getPtrType()->getExactType() == EXACT_BASIC  // void* is invalid in llvm IR
            && ((DST::BasicType*)(((DST::PointerType*)node)->getPtrType()))->getTypeId() == unicode_string("void"))
            return llvm::Type::getInt8Ty(_context)->getPointerTo();
        auto ty = evalType(((DST::PointerType*)node)->getPtrType());
        return ty->getPointerTo();
    }
    throw DinoException("Only basic types are currently supported in code generation!", EXT_GENERAL, node->getLine());
}

void CodeGenerator::declareType(DST::TypeDeclaration *node)
{
    auto def = new TypeDefinition();
    def->structType = llvm::StructType::create(_context, node->getName().to_string());
    _types[node] = _currentNamespace.back()->types[node->getName()] = def;
}

void CodeGenerator::declareTypeContent(DST::TypeDeclaration *node)
{
    auto def = _types[node];

    std::vector<llvm::Type*> members;
    int count = 0;
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
                declareFunction((DST::FunctionDeclaration*)i.second.first, def);
                break;
            case ST_PROPERTY_DECLARATION:
                declareProperty((DST::PropertyDeclaration*)i.second.first, def);
                break;
            default: break;
        }
    }
    def->structType->setBody(members);
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

void CodeGenerator::declareProperty(DST::PropertyDeclaration *node, CodeGenerator::TypeDefinition *typeDef)
{
    auto propType = evalType(node->getReturnType());

    if (node->getSet())
    {
        vector<llvm::Type*> setParams;
        if (typeDef)
            setParams.push_back(typeDef->structType->getPointerTo());
        setParams.push_back(propType);  // Add "value" parameter to set function
        auto setFuncType = llvm::FunctionType::get(llvm::Type::getVoidTy(_context), setParams, false);
        unicode_string setFuncName = node->getName();
        setFuncName += ".set";
        llvm::Function *setFunc = llvm::Function::Create(setFuncType, llvm::Function::ExternalLinkage, setFuncName.to_string(), _module.get());
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
    }

    if (node->getGet())
    {
        vector<llvm::Type*> getParams;
        if (typeDef)
            getParams.push_back(typeDef->structType->getPointerTo());
        auto getFuncType = llvm::FunctionType::get(propType, getParams, false);
        unicode_string getFuncName = node->getName();
        getFuncName += ".get";
        llvm::Function *getFunc = llvm::Function::Create(getFuncType, llvm::Function::ExternalLinkage, getFuncName.to_string(), _module.get());
        _currentNamespace.back()->values[getFuncName] = getFunc;
        for (auto &arg : getFunc->args())
            arg.setName("this");
        if (typeDef)
            typeDef->functions[getFuncName] = getFunc;
    }
}

void CodeGenerator::codegenProperty(DST::PropertyDeclaration *node, TypeDefinition *typeDef)
{
    if (node->getGet())
    {
        unicode_string getFuncName = node->getName();
        getFuncName += ".get";
        llvm::Value *getFuncPtr = NULL;
        if (typeDef)
            getFuncPtr = typeDef->functions[getFuncName];
        else getFuncPtr = _currentNamespace.back()->values[getFuncName];
        llvm::Function *getFunc = NULL;
        if (isFunc(getFuncPtr))
            getFunc = (llvm::Function*)getFuncPtr;
        else throw DinoException("\"" + getFuncName.to_string() + "\" is not a function", EXT_GENERAL, node->getLine());

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
                _currThisPtr = alloca;
            }
        }

        for (auto i : node->getGet()->getStatements()) 
        {
            auto val = codeGen(i);
            if (val == nullptr)
                throw DinoException("Error while generating IR for statement", EXT_GENERAL, i->getLine());
        }
        if (!_builder.GetInsertBlock()->getTerminator())
            _builder.CreateRetVoid();
        llvm::verifyFunction(*getFunc, &llvm::errs());

        if (!_builder.GetInsertBlock()->getTerminator())
            _builder.CreateRetVoid();
    }

    if (node->getSet())
    {
        unicode_string setFuncName = node->getName();
        setFuncName += ".set";
        llvm::Value *setFuncPtr = NULL;
        if (typeDef)
            setFuncPtr = typeDef->functions[setFuncName];
        else setFuncPtr = _currentNamespace.back()->values[setFuncName];
        llvm::Function *setFunc = NULL;
        if (isFunc(setFuncPtr))
            setFunc = (llvm::Function*)setFuncPtr;
        else throw DinoException("\"" + setFuncName.to_string() + "\" is not a function", EXT_GENERAL, node->getLine());

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

            if (typeDef && isFirst)
                _currThisPtr = alloca;
            isFirst = false;
        }

        for (auto i : node->getSet()->getStatements()) 
        {
            auto val = codeGen(i);
            if (val == nullptr)
                throw DinoException("Error while generating IR for statement", EXT_GENERAL, i->getLine());
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
    if (typeDef)
        types.push_back(typeDef->structType->getPointerTo());
    for (auto i : params) 
        types.push_back(evalType(i->getType()));
    auto returnType = evalType(node->getReturnType());
    auto funcType = llvm::FunctionType::get(returnType, types, false);
    auto funcId = node->getVarDecl()->getVarId().to_string();
    if (funcId == "Main")
        funcId = "main";
    llvm::Function *func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, funcId, _module.get());

    // Set names for all arguments.
    unsigned Idx = 0;
    bool b = true;
    for (auto &arg : func->args())
    {
        if (Idx == 0 && typeDef != nullptr && b)
        {
            arg.setName("this");
            b = false;
        }
        else arg.setName(params[Idx++]->getVarId().to_string());
    }

    _currentNamespace.back()->values[node->getVarDecl()->getVarId()] = func;
    if (typeDef)
        typeDef->functions[node->getVarDecl()->getVarId()] = func;
    return func;
}

void CodeGenerator::codegenFunction(DST::FunctionDeclaration *node, CodeGenerator::TypeDefinition *typeDef)
{
    //std::cout << "codegenning " << "." << node->getVarDecl()->getVarId().to_string() << std::endl;

    llvm::Value *funcPtr = NULL;
    if (typeDef)
        funcPtr = typeDef->functions[node->getVarDecl()->getVarId()];
    else funcPtr = _currentNamespace.back()->values[node->getVarDecl()->getVarId()];
    llvm::Function *func = NULL;
    if (isFunc(funcPtr))
        func = (llvm::Function*)funcPtr;
    else throw DinoException("\"" + node->getVarDecl()->getVarId().to_string() + "\" is not a function", EXT_GENERAL, node->getLine());

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
    for (llvm::Argument &arg : func->args())
    {
        AllocaInst *alloca = CreateEntryBlockAlloca(func, arg.getType(), arg.getName());    // Create an alloca for this variable.
        _builder.CreateStore(&arg, alloca);     // Store the initial value into the alloca.
        _namedValues[arg.getName()] = alloca;   // Add arguments to variable symbol table.

        if (isFirst && typeDef)
            _currThisPtr = alloca;
        isFirst = false;
    }

    for (auto i : node->getContent()->getStatements()) 
    {
        auto val = codeGen(i);
        if (val == nullptr)
            throw DinoException("Error while generating IR for statement", EXT_GENERAL, i->getLine());
    }

    if (!_builder.GetInsertBlock()->getTerminator())
        _builder.CreateRetVoid();
    
    llvm::verifyFunction(*func, &llvm::errs());
}

llvm::Function *CodeGenerator::codeGen(DST::FunctionDeclaration *node)
{
    vector<llvm::Type*> types;
    auto params = node->getParameters();
    for (auto i : params) 
        types.push_back(evalType(i->getType()));
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
            throw DinoException("Error while generating IR for statement", EXT_GENERAL, i->getLine());
    }

    if (!_builder.GetInsertBlock()->getTerminator())
        _builder.CreateBr(_builder.GetInsertBlock());
    
    if (!llvm::verifyFunction(*func, &llvm::errs()))
        llvm::errs() << "Function Vertified!\n---------------------------\n";

    return func;
}

// Get parent function
llvm::Function *CodeGenerator::getParentFunction() 
{
    return _builder.GetInsertBlock() ? _builder.GetInsertBlock()->getParent() : nullptr;
}

llvm::Value *CodeGenerator::codeGen(DST::DoWhileLoop *node)
{
    auto parent = getParentFunction();
    llvm::BasicBlock *condBB = llvm::BasicBlock::Create(_context, "cond");
    llvm::BasicBlock *loopBB = llvm::BasicBlock::Create(_context, "loop");
    llvm::BasicBlock *exitBB = llvm::BasicBlock::Create(_context, "exitLoop");

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
                throw DinoException("Error while generating IR for statement", EXT_GENERAL, i->getLine());
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
                throw DinoException("Error while generating IR for statement", EXT_GENERAL, i->getLine());
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
                throw DinoException("Error while generating IR for statement", EXT_GENERAL, i->getLine());
        }
    }
    _builder.CreateBr(condBB);
    parent->getBasicBlockList().push_back(exitBB);
    _builder.SetInsertPoint(exitBB);
    return br;
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
                throw DinoException("Error while generating IR for statement", EXT_GENERAL, i->getLine());
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
                throw DinoException("Error while generating IR for statement", EXT_GENERAL, i->getLine());
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