#include "CodeGenerator.h"
#include "DebugInfo.h"

vector<llvm::DIScope*> CodeGenerator::_currDIScope;
llvm::DISubprogram *CodeGenerator::_currDISubProgram;

llvm::DIType* CodeGenerator::evalDIType(DST::Type *node)
{
    if (node->_backendDIType)
        return node->_backendDIType;
    switch (node->getExactType())
    {
        case EXACT_BASIC:
        {
            if (node->isInterfaceTy())      return NULL;
            if (node == DST::getBoolTy())   return node->_backendDIType = _dbuilder->createBasicType("bool", 1, llvm::dwarf::DW_ATE_boolean);
            if (node == DST::getc8Ty())     return node->_backendDIType = _dbuilder->createBasicType("char", 8, llvm::dwarf::DW_ATE_unsigned_char);
            if (node == DST::getc32Ty())    return node->_backendDIType = _dbuilder->createBasicType("uchar", 32, llvm::dwarf::DW_ATE_unsigned_char);

            if (node == DST::geti8Ty())     return node->_backendDIType = _dbuilder->createBasicType("i8", 8, llvm::dwarf::DW_ATE_signed);
            if (node == DST::geti16Ty())    return node->_backendDIType = _dbuilder->createBasicType("i16", 16, llvm::dwarf::DW_ATE_signed);
            if (node == DST::geti32Ty())    return node->_backendDIType = _dbuilder->createBasicType("int", 32, llvm::dwarf::DW_ATE_signed);
            if (node == DST::geti64Ty())    return node->_backendDIType = _dbuilder->createBasicType("i64", 64, llvm::dwarf::DW_ATE_signed);
            if (node == DST::geti128Ty())   return node->_backendDIType = _dbuilder->createBasicType("i128", 128, llvm::dwarf::DW_ATE_signed);

            if (node == DST::getu8Ty())     return node->_backendDIType = _dbuilder->createBasicType("u8", 8, llvm::dwarf::DW_ATE_unsigned);
            if (node == DST::getu16Ty())    return node->_backendDIType = _dbuilder->createBasicType("u16", 16, llvm::dwarf::DW_ATE_unsigned);
            if (node == DST::getu32Ty())    return node->_backendDIType = _dbuilder->createBasicType("uint", 32, llvm::dwarf::DW_ATE_unsigned);
            if (node == DST::getu64Ty())    return node->_backendDIType = _dbuilder->createBasicType("u64", 64, llvm::dwarf::DW_ATE_unsigned);
            if (node == DST::getu128Ty())   return node->_backendDIType = _dbuilder->createBasicType("u128", 128, llvm::dwarf::DW_ATE_unsigned);

            if (node == DST::getf16Ty())    return node->_backendDIType = _dbuilder->createBasicType("f16", 16, llvm::dwarf::DW_ATE_float);
            if (node == DST::getf32Ty())    return node->_backendDIType = _dbuilder->createBasicType("float", 32, llvm::dwarf::DW_ATE_float);
            if (node == DST::getf64Ty())    return node->_backendDIType = _dbuilder->createBasicType("f64", 64, llvm::dwarf::DW_ATE_float);
            if (node == DST::getf128Ty())   return node->_backendDIType = _dbuilder->createBasicType("f128", 128, llvm::dwarf::DW_ATE_float);

            if (node == DST::getVoidTy())   return NULL;
            if (node == DST::getStringTy()) TODO

            if (node->isValueTy())
                TODO
            UNREACHABLE
        }
        case EXACT_FUNCTION: 
        {
            vector<llvm::Metadata*> types;
            auto funcTy = node->as<DST::FunctionType>();
            types.push_back(evalDIType(funcTy->getReturn()));
            for (auto ty : funcTy->getParameters())
                types.push_back(evalDIType(ty));
            return node->_backendDIType = _dbuilder->createSubroutineType(_dbuilder->getOrCreateTypeArray(types));
        }
        case EXACT_POINTER: 
            return node->_backendDIType = _dbuilder->createPointerType(
                evalDIType(node->as<DST::PointerType>()->getPtrType()),
                _builder.getInt8PtrTy()->getPrimitiveSizeInBits()
            );
        case EXACT_ARRAY:  TODO
        case EXACT_TYPELIST: TODO
        case EXACT_CONST: return evalDIType(((DST::ConstType*)node)->getNonConstOf());
        case EXACT_PROPERTY: return evalDIType(((DST::PropertyType*)node)->getReturn());
        default: UNREACHABLE
    }
}

llvm::DIFile *CodeGenerator::getDIFile(SourceFile *file)
{
    if (!file->_backendDIFile)
        file->_backendDIFile = _dbuilder->createFile(file->getName(), file->getPath());
    return file->_backendDIFile;
}

void CodeGenerator::diGenSetup()
{
    if (!_emitDebugInfo) return;

    _dbuilder = new llvm::DIBuilder(*_module);
}

void CodeGenerator::diGenFinalize()
{
    if (!_emitDebugInfo) return;
    _module.get()->addModuleFlag(llvm::Module::ModFlagBehavior::Warning, "Debug Info Version", 3);
    _dbuilder->finalize();
}

void CodeGenerator::diEnterNamespace(NamespaceMembers *ns)
{
    if (!_emitDebugInfo) return;
    _currDIScope.push_back(ns->diScope);
}

void CodeGenerator::diLeaveNamespace()
{
    if (!_emitDebugInfo) return;
    _currDIScope.pop_back();
}

void CodeGenerator::diGenNamespace(NamespaceMembers *node)
{
    if (!_emitDebugInfo) return;

    node->diScope = _dbuilder->createNameSpace(getCurrDIScope(node->decl), node->decl->getName().to_string(), false);
}

void CodeGenerator::diGenVarDecl(DST::VariableDeclaration *decl, llvm::Value *val)
{
    if (!_emitDebugInfo) return;

    auto pos = decl->getPosition();

    auto info = _dbuilder->createAutoVariable(
        getCurrDIScope(decl),
        decl->getVarId().to_string(),
        getDIFile(pos.file),
        pos.line,
        evalDIType(decl->getType())
    );

    _dbuilder->insertDeclare(
        val, info,
        llvm::DIExpression::get(_context, {}),
        llvm::DILocation::get(_context, pos.line, pos.startPos, getCurrDIScope(decl)),
        _builder.GetInsertBlock()
    );
}

void CodeGenerator::diGenFuncStart(DST::FunctionDeclaration* decl, llvm::Function *func)
{
    if (!_emitDebugInfo) return;

    auto unit = getDIFile(decl->getPosition().file);
    _currDISubProgram = _dbuilder->createFunction(
        getCurrDIScope(decl), decl->getVarDecl()->getVarId().to_string(), llvm::StringRef(), unit, decl->getPosition().line,
        (llvm::DISubroutineType*)evalDIType(decl->getFuncType()),
        false /* internal linkage */, true /* definition */, decl->getPosition().startPos//, llvm::DINode::FlagPrototyped
    );
    func->setSubprogram(_currDISubProgram);
    _currDIScope.push_back(_currDISubProgram);
    diEmitLocation(NULL);
}

void CodeGenerator::diGenFuncParam(DST::FunctionDeclaration* decl, llvm::Function *func, uint idx, llvm::AllocaInst *alloca)
{
    if (!_emitDebugInfo) return;

    auto varDecl = decl->getParameters()[idx];
    auto pos = varDecl->getPosition();
    auto info = _dbuilder->createParameterVariable(
        getCurrDIScope(decl),
        varDecl->getVarId().to_string(),
        idx+1,
        getDIFile(pos.file),
        pos.line,
        evalDIType(varDecl->getType())
    );

    _dbuilder->insertDeclare(
        alloca, info,
        llvm::DIExpression::get(_context, {}),
        llvm::DILocation::get(_context, pos.line, pos.startPos, getCurrDIScope(decl)),
        _builder.GetInsertBlock()
    );
}

void CodeGenerator::diGenFuncEnd(DST::FunctionDeclaration* decl, llvm::Function *func)
{
    if (!_emitDebugInfo) return;
    _dbuilder->finalizeSubprogram(_currDISubProgram);
    _currDISubProgram = NULL;
    _currDIScope.pop_back();
}

llvm::DIScope *CodeGenerator::getCurrDIScope(DST::Node *node)
{
    if (_currDIScope.size() == 0)
    {
        if (!_compileUnit)
            _compileUnit = _dbuilder->createCompileUnit(
                llvm::dwarf::DW_LANG_C_plus_plus, 
                getDIFile(node->getPosition().file), 
                "dino", false, "", 0
            );
        return _compileUnit;
    }
    return _currDIScope.back();
}

void CodeGenerator::diEmitLocation(DST::Node *node)
{
    if (!_emitDebugInfo) return;

    if (!node)
        _builder.SetCurrentDebugLocation(llvm::DebugLoc());
    else _builder.SetCurrentDebugLocation(llvm::DebugLoc::get(
        node->getPosition().line, 
        node->getPosition().startPos, 
        getCurrDIScope(node)
    ));
}