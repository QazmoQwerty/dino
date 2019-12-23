#include "CodeGenerator.h"

void CodeGenerator::setup()
{
    // Nothing yet
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
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

    llvm::outs() << "Result: " << GV.IntVal << "\n";
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
        default: return NULL;
    }
}

Value *CodeGenerator::codeGen(DST::Expression *node) 
{
    switch (node->getExpressionType()) 
    {
        case ET_BINARY_OPERATION: return codeGen((DST::BinaryOperation*)node);
        case ET_LITERAL: return codeGen((DST::Literal*)node);
        case ET_ASSIGNMENT: return codeGen((DST::Assignment*)node);
        case ET_IDENTIFIER: return codeGen((DST::Variable*)node);
        case ET_VARIABLE_DECLARATION: return codeGen((DST::VariableDeclaration*)node);
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
    case LT_INTEGER:
        return llvm::ConstantInt::get(_context, llvm::APInt( /* 64 bit width */ 64, ((AST::Integer*)node->getBase())->getValue(), /* signed */ true));
    default:
        return NULL;
    }
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
        case OT_SMALLER:
            return _builder.CreateICmpULT(left, right, "cmptmp");
        case OT_EQUAL:
            return _builder.CreateICmpEQ(left, right, "cmptmp");
        case OT_NOT_EQUAL:
            return _builder.CreateICmpNE(left, right, "cmptmp");
        default:
            return NULL;
    }
    
}

Value *CodeGenerator::codeGen(DST::Assignment* node)
{
    AllocaInst *left;
    Value *right;
    if (node->getLeft()->getExpressionType() == ET_IDENTIFIER)
        left = _namedValues[((DST::Variable*)node->getLeft())->getVarId().to_string()]; // temporary for tests
    else left = codeGen((DST::VariableDeclaration*)node->getLeft());
    right = codeGen(node->getRight());
    return _builder.CreateStore(right, left);
}

/// CreateEntryBlockAlloca - Create an alloca instruction in the entry block of
/// the function.  This is used for mutable variables etc.
AllocaInst *CodeGenerator::CreateEntryBlockAlloca(llvm::Function *func, llvm::Type *type, const string &varName) 
{ 
    llvm::IRBuilder<> TmpB(&func->getEntryBlock(), func->getEntryBlock().begin());
    return TmpB.CreateAlloca(type, nullptr, varName);
}

Value *CodeGenerator::codeGen(DST::UnaryOperationStatement *node)
{
    switch (node->getOperator()._type)
    {
        case OT_RETURN:
            _builder.CreateStore(codeGen(node->getExpression()), _currRetPtr);
            return _builder.CreateBr(_currFuncExit);
            //return _builder.CreateRet();
        default: return NULL;
    }
}

Value *CodeGenerator::codeGen(DST::Variable *node)
{
    auto v = _namedValues[node->getVarId().to_string()];
    
    return _builder.CreateLoad(v, node->getVarId().to_string().c_str());
}

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
        else if (((DST::BasicType*)node)->getTypeId() == unicode_string("int"))
            return llvm::Type::getInt64Ty(_context);
        else if (((DST::BasicType*)node)->getTypeId() == unicode_string("void"))
            return llvm::Type::getVoidTy(_context);
        else throw DinoException("Only int/bool/void is currently supported in code generation!", EXT_GENERAL, node->getLine());
    }
    else throw DinoException("Only basic types are currently supported in code generation!", EXT_GENERAL, node->getLine());
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


    auto retType = evalType(node->getReturnType());
    
    _currRetPtr = CreateEntryBlockAlloca(func, retType, "ret");
    _currFuncExit = llvm::BasicBlock::Create(_context, "exit", func);

    for (auto i : node->getContent()->getStatements()) 
    {
        auto val = codeGen(i);
        if (val == nullptr)
            throw DinoException("Error while generating IR for statement", EXT_GENERAL, i->getLine());
    }

    if (!retType->isVoidTy()) {
        _builder.SetInsertPoint(_currFuncExit);
        _builder.CreateRet(_builder.CreateLoad(_currRetPtr));
    }

    func->getBasicBlockList().push_back(_currFuncExit);

    llvm::verifyFunction(*func);

    return func;
}

// Get parent function
llvm::Function *CodeGenerator::getParentFunction() 
{
    return _builder.GetInsertBlock() ? _builder.GetInsertBlock()->getParent() : nullptr;
}

llvm::Value *CodeGenerator::codeGen(DST::WhileLoop *node)
{
    Value *cond = codeGen(node->getCondition());
    auto parent = getParentFunction();
    llvm::BasicBlock *condBB = llvm::BasicBlock::Create(_context, "cond");
    llvm::BasicBlock *loopBB = llvm::BasicBlock::Create(_context, "loop");
    llvm::BasicBlock *exitBB = llvm::BasicBlock::Create(_context, "exitLoop");

    _builder.SetInsertPoint(condBB);
    auto br = _builder.CreateCondBr(cond, loopBB, exitBB);
    parent->getBasicBlockList().push_back(condBB);

    if (node->getStatement())
    {
        _builder.SetInsertPoint(loopBB);
        for (auto i : node->getStatement()->getStatements()) 
        {
            auto val = codeGen(i);
            if (val == nullptr)
                throw DinoException("Error while generating IR for statement", EXT_GENERAL, i->getLine());
        }
        _builder.CreateBr(exitBB);
        parent->getBasicBlockList().push_back(condBB);
    }
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
    llvm::BasicBlock *mergeBB = llvm::BasicBlock::Create(_context, "ifcont");
    auto br = _builder.CreateCondBr(cond, 
        node->getThenBranch() ? thenBB : mergeBB,
        node->getElseBranch() ? elseBB : mergeBB);
    if (node->getThenBranch())
    {
        _builder.SetInsertPoint(thenBB);
        for (auto i : node->getThenBranch()->getStatements()) 
        {
            auto val = codeGen(i);
            if (val == nullptr)
                throw DinoException("Error while generating IR for statement", EXT_GENERAL, i->getLine());
        }
        _builder.CreateBr(mergeBB);
        parent->getBasicBlockList().push_back(thenBB);
    }

    if (node->getElseBranch())
    {
        _builder.SetInsertPoint(elseBB);
        for (auto i : node->getElseBranch()->getStatements()) 
        {
            auto val = codeGen(i);
            if (val == nullptr)
                throw DinoException("Error while generating IR for statement", EXT_GENERAL, i->getLine());
        }
        _builder.CreateBr(mergeBB);
        parent->getBasicBlockList().push_back(elseBB);
    }

    parent->getBasicBlockList().push_back(mergeBB);
    _builder.SetInsertPoint(mergeBB);
    return br;
}