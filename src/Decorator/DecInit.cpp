/*
	This file implements the functions which are called at the start of the decoration proccess.
	These functions declare all namespace/types/funcions/etc and call the decoration functions on their contents.
	FIXME - rename parts A-E to something more meaningful.
*/
#include "Decorator.h"

vector<unordered_map<unicode_string, DST::Type*, UnicodeHasherFunction>> Decorator::_variables;
DST::FunctionDeclaration* Decorator::_main;
vector<DST::NamespaceDeclaration*> Decorator::_currentNamespace;
DST::TypeDeclaration *Decorator::_currentTypeDecl;
DST::Program *Decorator::_currentProgram;
DST::NamespaceDeclaration *Decorator::_universalNs;
bool Decorator::_isLibrary;
unsigned Decorator::_loopCount;
vector<DST::Node*> Decorator::_toDelete;

void Decorator::setup(bool isLibrary)
{
	_isLibrary = isLibrary;
	enterBlock();

	_variables[0][unicode_string("bool")] 	= DST::getBoolTy()	     ->getTypeSpecifier();
	_variables[0][unicode_string("int")] 	= DST::getIntTy()	     ->getTypeSpecifier();
	_variables[0][unicode_string("uint")] 	= DST::getUnsignedIntTy()->getTypeSpecifier();
	_variables[0][unicode_string("string")] = DST::getStringTy()     ->getTypeSpecifier();
	_variables[0][unicode_string("char")] 	= DST::getCharTy()	     ->getTypeSpecifier();
	_variables[0][unicode_string("float")] 	= DST::getFloatTy()	     ->getTypeSpecifier();
	_variables[0][unicode_string("void")] 	= DST::getVoidTy()	     ->getTypeSpecifier();
	_variables[0][unicode_string("type")] 	= DST::getTypeidTy()     ->getTypeSpecifier();
	_variables[0][unicode_string("any")] 	= DST::getAnyTy()	     ->getTypeSpecifier();

	_variables[0][unicode_string("i8")] 	= DST::geti8Ty()	->getTypeSpecifier();
	_variables[0][unicode_string("i16")] 	= DST::geti16Ty()	->getTypeSpecifier();
	_variables[0][unicode_string("i32")] 	= DST::geti32Ty()	->getTypeSpecifier();
	_variables[0][unicode_string("i64")] 	= DST::geti64Ty()	->getTypeSpecifier();
	_variables[0][unicode_string("i128")] 	= DST::geti128Ty()	->getTypeSpecifier();

	_variables[0][unicode_string("u8")] 	= DST::getu8Ty()	->getTypeSpecifier();
	_variables[0][unicode_string("u16")] 	= DST::getu16Ty()	->getTypeSpecifier();
	_variables[0][unicode_string("u32")] 	= DST::getu32Ty()	->getTypeSpecifier();
	_variables[0][unicode_string("u64")] 	= DST::getu64Ty()	->getTypeSpecifier();
	_variables[0][unicode_string("u128")] 	= DST::getu128Ty()	->getTypeSpecifier();

	_variables[0][unicode_string("c8")] 	= DST::getc8Ty()	->getTypeSpecifier();
	_variables[0][unicode_string("c32")] 	= DST::getc32Ty()	->getTypeSpecifier();
	_variables[0][unicode_string("uchar")] 	= DST::getc32Ty()	->getTypeSpecifier();

	_variables[0][unicode_string("f32")] 	= DST::getf32Ty()	->getTypeSpecifier();
	_variables[0][unicode_string("f64")] 	= DST::getf64Ty()	->getTypeSpecifier();
	_variables[0][unicode_string("f128")] 	= DST::getf128Ty()	->getTypeSpecifier();

	_currentTypeDecl = NULL;
	_loopCount = 0;
}

DST::Program * Decorator::decorateProgram(AST::StatementBlock * node)
{
	_currentProgram = new DST::Program();

	for (auto i : node->getStatements())
	{
		if (i->getStatementType() == ST_IMPORT)
			_currentProgram->addImport(((AST::Import*)i)->getImportPath());
		else
		{ 
			if (i->getStatementType() != ST_NAMESPACE_DECLARATION)
				throw ErrorReporter::report("everything must be in a namespace!", ERR_DECORATOR, i->getPosition());
			
			_currentProgram->addNamespace(partA((AST::NamespaceDeclaration*)i, ((AST::NamespaceDeclaration*)i)->getName() == "Std"));
		}
	}

	for (auto i : _currentProgram->getNamespaces())
		partB(i.second);

	for (auto i : _currentProgram->getNamespaces())
		partC(i.second);
	
	if (!_main && !_isLibrary)
		throw ErrorReporter::report("No entry point (Main function)", ERR_DECORATOR, POSITION_INFO_NONE);
	
	for (auto i : _currentProgram->getNamespaces())
		partD(i.second);

	for (auto i : _currentProgram->getNamespaces())
		partE(i.second);

	return _currentProgram;
}

DST::NamespaceDeclaration *Decorator::partA(AST::NamespaceDeclaration *node, bool isStd)
{
	auto shallowDecl = new DST::NamespaceDeclaration(node);
	for (auto i : node->getStatement()->getStatements())
	{
		if (i->getStatementType() == ST_NAMESPACE_DECLARATION)
		{
			auto decl = (AST::NamespaceDeclaration*)i;
			auto tempDecl = partA(decl);
			shallowDecl->addMember(decl->getName(), tempDecl, DST::getNamespaceTy(tempDecl));
		}
		else if (i->getStatementType() == ST_INTERFACE_DECLARATION)
		{
			auto decl = (AST::InterfaceDeclaration*)i;
			auto tempDecl = new DST::InterfaceDeclaration(decl);
			auto specifier = DST::TypeSpecifierType::get(tempDecl);
			shallowDecl->addMember(decl->getName(), tempDecl, specifier);
			if (isStd && decl->getName() == "Error")
			{
				// delete DST::_builtinTypes._string;
				_variables[0][unicode_string("error")] = DST::_builtinTypes._error = specifier;
			}
		}
		else if (i->getStatementType() == ST_TYPE_DECLARATION)
		{
			auto decl = (AST::TypeDeclaration*)i;
			auto tempDecl = new DST::TypeDeclaration(decl);
			auto specifier = DST::TypeSpecifierType::get(tempDecl);
			shallowDecl->addMember(decl->getName(), tempDecl, specifier);
			if (isStd)
			{
				if (decl->getName() == "String")
					_variables[0][unicode_string("string")]	= DST::_builtinTypes._string = specifier;
				else if (decl->getName() == "NullPointerError")
					DST::_builtinTypes._nullPtrError = specifier;
			}
		}
	}
	return shallowDecl;
}

void Decorator::partB(DST::NamespaceDeclaration *node)
{
	if (node == _universalNs)
		return;
	_currentNamespace.push_back(node);
	for (auto i : node->getMembers())
	{
		switch (i.second.first->getStatementType())
		{
		case ST_NAMESPACE_DECLARATION:
			partB((DST::NamespaceDeclaration*)i.second.first);
			break;
		case ST_INTERFACE_DECLARATION:
		{
			auto decl = (DST::InterfaceDeclaration*)i.second.first;
			if (decl->getBase()->getImplements())
				for (auto i : decl->getBase()->getImplements()->getExpressions())
				{
					auto dec = decorate(i);
					if (!dec->getType()->isSpecifierTy() || !dec->getType()->as<DST::TypeSpecifierType>()->getInterfaceDecl())
						throw ErrorReporter::report("Expected an interface specifier", ERR_DECORATOR, dec->getPosition());
					decl->addImplements(dec->getType()->as<DST::TypeSpecifierType>()->getInterfaceDecl());
				}
			break;
		}
		case ST_TYPE_DECLARATION:
		{
			auto decl = (DST::TypeDeclaration*)i.second.first;
			if (decl->getBase()->getInterfaces())
				for (auto i : decl->getBase()->getInterfaces()->getExpressions())
				{
					auto dec = decorate(i);
					if (!dec->getType()->isSpecifierTy() || !dec->getType()->as<DST::TypeSpecifierType>()->getInterfaceDecl())
						throw ErrorReporter::report("Expected an interface specifier", ERR_DECORATOR, dec->getPosition());
					decl->addInterface(dec->getType()->as<DST::TypeSpecifierType>()->getInterfaceDecl());
				}
			break;
		}
		default:
			break;
		}
	}
	_currentNamespace.pop_back();
}

DST::UnsetGenericType *Decorator::createGenericTy(AST::Expression *exp)
{
	switch (exp->getExpressionType())
	{
		case ET_BINARY_OPERATION:
		{
			auto bo = (AST::BinaryOperation*)exp;
			if (bo->getOperator()._type != OT_IS || bo->getLeft()->getExpressionType() != ET_IDENTIFIER)
				throw ErrorReporter::report("invalid generic type declaration (2)", ERR_DECORATOR, exp->getPosition());
			auto ret = new DST::UnsetGenericType(((AST::Identifier*)bo->getLeft())->getVarId());
			auto right = decorate(bo->getRight());
			if (right->getExpressionType() != ET_TYPE || !((DST::Type*)right)->isInterfaceTy())
				throw ErrorReporter::report("invalid generic type declaration (3)", ERR_DECORATOR, exp->getPosition());
			ret->addImplements(((DST::Type*)right)->as<DST::TypeSpecifierType>()->getInterfaceDecl());
			return ret;
		}
		case ET_IDENTIFIER:
			return new DST::UnsetGenericType(((AST::Identifier*)exp)->getVarId());
		default:
			throw ErrorReporter::report("invalid generic type declaration (1)", ERR_DECORATOR, exp->getPosition());
	}
}

void Decorator::partC(DST::NamespaceDeclaration *node)
{
	if (node == _universalNs)
		return;
	_currentNamespace.push_back(node);
	for (auto i : node->getMembers())
	{
		switch (i.second.first->getStatementType())
		{
		case ST_NAMESPACE_DECLARATION:
			partC((DST::NamespaceDeclaration*)i.second.first);
			break;
		case ST_INTERFACE_DECLARATION:
		{
			auto decl = (DST::InterfaceDeclaration*)i.second.first;
			for (auto func : decl->getBase()->getFunctions())
			{
				enterBlock();
				auto funcDecl = new DST::FunctionDeclaration(func, decorate(func->getVarDecl()));
				for (auto param : func->getParameters())
					funcDecl->addParameter(decorate(param));
				decl->addDeclaration(funcDecl, funcDecl->getFuncType());
				leaveBlock();
			}
			for (auto prop : decl->getBase()->getProperties())
			{
				auto retType = evalType(prop->getVarDecl()->getVarType());
				auto type = retType->getPropertyOf(prop->hasGet(), prop->hasSet());
				decl->addDeclaration(new DST::PropertyDeclaration(prop, NULL, NULL, type), type);
			}
			break;
		}
		case ST_TYPE_DECLARATION:
		{
			auto decl = (DST::TypeDeclaration*)i.second.first;
			for (auto var : decl->getBase()->getVariableDeclarations())
			{
				auto varDecl = new DST::VariableDeclaration(var);
				varDecl->setType(evalType(var->getVarType()));
				decl->addDeclaration(varDecl, varDecl->getType());
			}
			for (auto func : decl->getBase()->getFunctionDeclarations())
			{
				enterBlock();
				auto funcDecl = new DST::FunctionDeclaration(func, decorate(func->getVarDecl()));
				for (auto param : func->getParameters())
					funcDecl->addParameter(decorate(param));
				decl->addDeclaration(funcDecl, funcDecl->getFuncType());
				leaveBlock();
			}
			for (auto prop : decl->getBase()->getPropertyDeclarations())
			{
				auto retType = evalType(prop->getVarDecl()->getVarType());
				auto type = retType->getPropertyOf(prop->getGet(), prop->getSet());
				decl->addDeclaration(new DST::PropertyDeclaration(prop, NULL, NULL, type), type);
			}
			break;
		}
		default: break;
		}
	}
	for (auto i : node->getBase()->getStatement()->getStatements())
	{
		if (i) switch (i->getStatementType())
		{
		case ST_FUNCTION_DECLARATION:
		{
			auto func = (AST::FunctionDeclaration*)i;
			enterBlock();
			auto funcDecl = new DST::FunctionDeclaration(func);
			for (auto generic : func->getGenerics())
			{
				auto gen = createGenericTy(generic);
				funcDecl->addGeneric(gen);
				_variables[currentScope()][gen->getName()] = gen;
			}
			funcDecl->setVarDecl(decorate(func->getVarDecl()));
			for (auto param : func->getParameters())
				funcDecl->addParameter(decorate(param));
			leaveBlock();
			node->addMember(func->getVarDecl()->getVarId(), funcDecl, funcDecl->getFuncType());
			
			if (func->getVarDecl()->getVarId() == MAIN_FUNC)
			{
				if (_isLibrary)
					throw ErrorReporter::report("Main function can't be declared in a library", ERR_DECORATOR, func->getPosition());
				if (_main)
					throw ErrorReporter::report("Main function can't be declared more than once", ERR_DECORATOR, func->getPosition());
				_main = funcDecl;
			}
			break;
		}
		case ST_PROPERTY_DECLARATION:
		{
			auto prop = (AST::PropertyDeclaration*)i;
			auto retType = evalType(prop->getVarDecl()->getVarType());
			auto type = DST::PropertyType::get(retType, prop->getGet(), prop->getSet());
			node->addMember(prop->getVarDecl()->getVarId(), new DST::PropertyDeclaration(prop, NULL, NULL, type), type);
			break;
		}
		case ST_VARIABLE_DECLARATION:
		{
			auto decl = (AST::VariableDeclaration*)i;
			auto varDecl = new DST::VariableDeclaration(decl);
			varDecl->setType(evalType(decl->getVarType()));
			node->addMember(decl->getVarId(), varDecl, varDecl->getType());
			break;
		}
		case ST_CONST_DECLARATION:
		{
			auto decl = (AST::ConstDeclaration*)i;
			auto constDecl = new DST::ConstDeclaration(decl);
			auto exp = decorate(decl->getExpression());
			constDecl->setExpression(exp);
			node->addMember(decl->getName(), constDecl, constDecl->getType());
			break;
		}
		default:
			break;
		}
	}
	_currentNamespace.pop_back();
}

void Decorator::partD(DST::NamespaceDeclaration *node)
{
	if (node == _universalNs)
		return;
	for (auto i : node->getMembers())
	{
		switch (i.second.first->getStatementType())
		{
			case ST_INTERFACE_DECLARATION:
			{
				auto decl = (DST::InterfaceDeclaration*)i.second.first;
				for (auto i : decl->getImplements())
					decl->notImplements(i); // check if interface does not implement something that it inherits.
				break;
			}
			case ST_TYPE_DECLARATION:
			{
				auto decl = (DST::TypeDeclaration*)i.second.first;
				for (auto i : decl->getInterfaces())
				{
					// check if type implements the interface.
					decl->validateImplements(i);
					
					// check if the interfaces that the type implements are distinct
					for (auto otherI : decl->getInterfaces())
						if (i != otherI) 
							otherI->notImplements(i);
				}
				break;
			}
			default: break;
		}
	}
}

void Decorator::partE(DST::NamespaceDeclaration *node)
{
	if (node == _universalNs)
		return;
	_currentNamespace.push_back(node);
	for (auto i : node->getMembers())
	{
		switch (i.second.first->getStatementType())
		{
		case ST_NAMESPACE_DECLARATION:
			partE((DST::NamespaceDeclaration*)i.second.first);
			break;
		case ST_TYPE_DECLARATION:
		{
			auto decl = (DST::TypeDeclaration*)i.second.first;
			_currentTypeDecl = decl;
			for (auto member : decl->getMembers())
			{
				if (member.second.first->getStatementType() == ST_FUNCTION_DECLARATION)
				{
					auto decl = (DST::FunctionDeclaration*)member.second.first;
					enterBlock();
					_variables[currentScope()][unicode_string("this")] = i.second.second->as<DST::TypeSpecifierType>()->getBasicTy()->getPtrTo();
					
					for (auto param : decl->getParameters())	// Add function parameters to variables map
						_variables[currentScope()][param->getVarId()] = param->getType();
					decl->setContent(decorate(decl->getBase()->getContent()));
					if (!decl->getContent())
						throw ErrorReporter::report("Method must have a body.", ERR_DECORATOR, decl->getPosition());
					if (!decl->getContent()->hasReturnType(decl->getReturnType()))
						throw ErrorReporter::report("Not all control paths lead to a return value.", ERR_DECORATOR, decl->getPosition());
					leaveBlock();
				}
				else if (member.second.first->getStatementType() == ST_PROPERTY_DECLARATION)
				{
					auto decl = (DST::PropertyDeclaration*)member.second.first;
					auto retType = member.second.second->as<DST::PropertyType>()->getReturn();
					enterBlock();
					_variables[currentScope()][unicode_string("this")] = i.second.second->as<DST::TypeSpecifierType>()->getBasicTy()->getPtrTo();
					if (decl->getBase()->getGet())
					{
						decl->setGet(decorate(decl->getBase()->getGet()));
						if (decl->getGet() && !decl->getGet()->hasReturnType(retType))
							throw ErrorReporter::report("Not all control paths lead to a return value.", ERR_DECORATOR, decl->getPosition());
					}
					if (decl->getBase()->getSet())
					{
						_variables[currentScope()][unicode_string("value")] = retType;
						decl->setSet(decorate(decl->getBase()->getSet()));
					}
					leaveBlock();
				}
			}
			_currentTypeDecl = NULL;
			break;
		}
		case ST_FUNCTION_DECLARATION:
		{
			auto decl = (DST::FunctionDeclaration*)i.second.first;
			enterBlock();
			for (auto i : decl->getGenerics())		// Add generics to variabes map
				_variables[currentScope()][i->getName()] = i;
			for (auto i : decl->getParameters())	// Add function parameters to variabes map
				_variables[currentScope()][i->getVarId()] = i->getType();
			decl->setContent(decorate(decl->getBase()->getContent()));
			if (decl->getContent() && !decl->getContent()->hasReturnType(decl->getReturnType()))
				throw ErrorReporter::report("Not all control paths lead to a return value.", ERR_DECORATOR, decl->getPosition());
			leaveBlock();
			break;
		}
		case ST_PROPERTY_DECLARATION:
		{
			auto decl = (DST::PropertyDeclaration*)i.second.first;
			auto retType = i.second.second->as<DST::PropertyType>()->getReturn();
			if (decl->getBase()->getGet())
			{
				decl->setGet(decorate(decl->getBase()->getGet()));
				if (decl->getGet() && !decl->getGet()->hasReturnType(retType))
					throw ErrorReporter::report("Not all control paths lead to a return value.", ERR_DECORATOR, decl->getPosition());
			}
			if (decl->getBase()->getSet())
			{
				enterBlock();
				_variables[currentScope()][unicode_string("value")] = retType;
				decl->setSet(decorate(decl->getBase()->getSet()));
				leaveBlock();
			}
			break;
		}
		default: 
			break;
		}
	}
	_currentNamespace.pop_back();
}