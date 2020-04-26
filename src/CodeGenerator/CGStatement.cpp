/*
    Code generation for statements.
*/
#include "CodeGenerator.h"

Value *CodeGenerator::codeGen(DST::Statement *node) 
{
    if (node == nullptr) 
        return nullptr;
    switch (node->getStatementType()) 
    {
        case ST_ASSIGNMENT: return codeGen((DST::Assignment*)node);
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

Value *CodeGenerator::codeGen(DST::ConstDeclaration *node) 
{
    return _namedValues.top()[node->getName().to_string()] = codeGen(node->getExpression());
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
                if (_noGC)
                    free = llvm::Function::Create(type, llvm::Function::ExternalLinkage, "free", _module.get());
                else free = llvm::Function::Create(type, llvm::Function::ExternalLinkage, "GC_free", _module.get());
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

llvm::Value *CodeGenerator::codeGen(DST::TryCatch *node)
{

    auto parent = getParentFunction();
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
    auto pad = _builder.CreateLandingPad(_builder.getInt8PtrTy(), 1);
    pad->addClause(llvm::ConstantPointerNull::get(_builder.getInt8PtrTy()));

    auto caught = _builder.CreateCall(beginCatchFunc, pad);
    _namedValues.top()["caught"] = _builder.CreateBitCast(caught, _interfaceType->getPointerTo());

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