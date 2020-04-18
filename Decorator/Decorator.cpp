#include "Decorator.h"

#define MAIN_FUNC "Main"

vector<unordered_map<unicode_string, DST::Type*, UnicodeHasherFunction>> Decorator::_variables;
DST::FunctionDeclaration* Decorator::_main;
vector<DST::NamespaceDeclaration*> Decorator::_currentNamespace;
DST::TypeDeclaration *Decorator::_currentTypeDecl;
DST::Program *Decorator::_currentProgram;
DST::NullType *Decorator::_nullType;
DST::UnknownType *Decorator::_unknownType;
DST::NamespaceDeclaration *_universalNs;
bool Decorator::_isLibrary;
unsigned Decorator::_loopCount;

//unordered_map<unicode_string, DST::TypeDeclaration*, UnicodeHasherFunction> Decorator::_types;
vector<DST::Node*> Decorator::_toDelete;

#define createBasicType(name) _variables[0][unicode_string(name)] = new DST::TypeSpecifierType(new DST::TypeDeclaration(unicode_string(name)));

#define ERROR_TYPE_NAME "error"

void Decorator::setup(bool isLibrary)
{
	_isLibrary = isLibrary;
	enterBlock();
	createBasicType("type");
	createBasicType("int");
	createBasicType("bool");
	createBasicType("string");
	createBasicType("char");
	createBasicType("float");
	createBasicType("void");

	_unknownType = new DST::UnknownType();
	_nullType = new DST::NullType();
	_currentTypeDecl = NULL;
	_loopCount = 0;
	// DST::_anyInterface = new DST::InterfaceDeclaration(new AST::InterfaceDeclaration(unicode_string("any")));
	// std::cout << "here1\n" << DST::_anyInterface << "\n";
	// _variables[0][DST::_anyInterface->getName()] = new DST::TypeSpecifierType(DST::_anyInterface);
	// std::cout << "here2\n";
}

void Decorator::createErrorInterfaceType()
{

	auto astNamespace = new AST::NamespaceDeclaration();
	astNamespace->setName(unicode_string("."));
	_universalNs = new DST::NamespaceDeclaration(astNamespace);

	auto interfaceDecl1 = new AST::InterfaceDeclaration();
	interfaceDecl1->setName(unicode_string(ERROR_TYPE_NAME));
	auto interfaceDecl2 = new DST::InterfaceDeclaration(interfaceDecl1);

	auto varDecl = new AST::VariableDeclaration();
	varDecl->setVarId(unicode_string("Msg"));
	auto propDecl = new AST::PropertyDeclaration(varDecl);
	auto stringTy = new DST::BasicType(getPrimitiveType("int"));
	auto propTy = new DST::PropertyType(stringTy, true, false);
	auto decPropDecl = new DST::PropertyDeclaration(propDecl, NULL, NULL, propTy);

	interfaceDecl2->addDeclaration(decPropDecl, new DST::PropertyType(stringTy, true, false));

	auto ty = _variables[0][unicode_string(ERROR_TYPE_NAME)] = new DST::TypeSpecifierType(interfaceDecl2);

	_universalNs->addMember(interfaceDecl2->getName(), interfaceDecl2, ty);
	_currentProgram->addNamespace(_universalNs);
}	


DST::TypeSpecifierType *Decorator::getPrimitiveType(std::string name)
{
	auto ret = (DST::TypeSpecifierType*)_variables[0][(unicode_string(name))];
	if (ret == nullptr)
		throw "primitive type " + name + " does not exist!";
	return ret;
}

DST::Node *Decorator::decorate(AST::Node * node)
{
	if (node == nullptr)
		return NULL;
	if (node->isExpression())
		return decorate(dynamic_cast<AST::Expression*>(node));
	else return decorate(dynamic_cast<AST::Statement*>(node));
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
			shallowDecl->addMember(decl->getName(), tempDecl, new DST::NamespaceType(tempDecl));
		}
		else if (i->getStatementType() == ST_INTERFACE_DECLARATION)
		{
			auto decl = (AST::InterfaceDeclaration*)i;
			auto tempDecl = new DST::InterfaceDeclaration(decl);
			//shallowDecl->addMember(decl->getName(), tempDecl, new DST::InterfaceSpecifierType(tempDecl));
			auto specifier = new DST::TypeSpecifierType(tempDecl);
			shallowDecl->addMember(decl->getName(), tempDecl, specifier);
			if (isStd)
			{
				if (decl->getName() == "Error")
				{
					// specifier->getInterfaceDecl()->getBase()->setName(unicode_string("error"));
					_variables[0][unicode_string("error")] = specifier;
				}
			}
		}
		else if (i->getStatementType() == ST_TYPE_DECLARATION)
		{
			auto decl = (AST::TypeDeclaration*)i;
			auto tempDecl = new DST::TypeDeclaration(decl);
			auto specifier = new DST::TypeSpecifierType(tempDecl);
			shallowDecl->addMember(decl->getName(), tempDecl, specifier);
			if (isStd)
			{
				if (decl->getName() == "String") {
					// specifier->getTypeDecl()->setName(unicode_string("string"));
					delete _variables[0][unicode_string("string")];
					_variables[0][unicode_string("string")]	= specifier;
				}
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
					if (dec->getType()->getExactType() != EXACT_SPECIFIER && ((DST::TypeSpecifierType*)dec->getType())->getInterfaceDecl())
						throw ErrorReporter::report("Expected an interface specifier", ERR_DECORATOR, dec->getPosition());
					//decl->addImplements(((DST::InterfaceSpecifierType*)dec->getType())->getInterfaceDecl());
					decl->addImplements(((DST::TypeSpecifierType*)dec->getType())->getInterfaceDecl());
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
					if (dec->getType()->getExactType() != EXACT_SPECIFIER && ((DST::TypeSpecifierType*)dec->getType())->getInterfaceDecl())
						throw ErrorReporter::report("Expected an interface specifier", ERR_DECORATOR, dec->getPosition());
					//decl->addInterface(((DST::InterfaceSpecifierType*)dec->getType())->getInterfaceDecl());
					decl->addInterface(((DST::TypeSpecifierType*)dec->getType())->getInterfaceDecl());
				}
			break;
		}
		default:
			break;
		}
	}
	_currentNamespace.pop_back();
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
				auto type = new DST::FunctionType();
				type->addReturn(funcDecl->getReturnType());
				for (auto param : funcDecl->getParameters())
					type->addParameter(param->getType());
				decl->addDeclaration(funcDecl, type);
				leaveBlock();
			}
			for (auto prop : decl->getBase()->getProperties())
			{
				auto retType = evalType(prop->getVarDecl()->getVarType());
				auto type = new DST::PropertyType(retType, prop->hasGet(), prop->hasSet());
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
				auto type = new DST::FunctionType();
				type->addReturn(funcDecl->getReturnType());	
				for (auto param : funcDecl->getParameters())
					type->addParameter(param->getType());
				decl->addDeclaration(funcDecl, type);
				leaveBlock();
			}
			for (auto prop : decl->getBase()->getPropertyDeclarations())
			{
				auto retType = evalType(prop->getVarDecl()->getVarType());
				auto type = new DST::PropertyType(retType, prop->getGet(), prop->getSet());
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
			auto funcDecl = new DST::FunctionDeclaration(func, decorate(func->getVarDecl()));
			for (auto param : func->getParameters())
				funcDecl->addParameter(decorate(param));
			leaveBlock();
			auto type = new DST::FunctionType();
			type->addReturn(funcDecl->getReturnType());
			for (auto param : funcDecl->getParameters())
				type->addParameter(param->getType());
			node->addMember(func->getVarDecl()->getVarId(), funcDecl, type);
			
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
			auto type = new DST::PropertyType(retType, prop->getGet(), prop->getSet());
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
			exp->getType()->setNotWritable();
			exp->getType()->setConst();
			constDecl->setExpression(exp);
			node->addMember(decl->getName(), constDecl, constDecl->getExpression()->getType());
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
						if(i != otherI) 
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
					_variables[currentScope()][unicode_string("this")] = new DST::PointerType(new DST::BasicType((DST::TypeSpecifierType*)i.second.second));
					
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
					auto retType = ((DST::PropertyType*)member.second.second)->getReturn();
					enterBlock();
					_variables[currentScope()][unicode_string("this")] = new DST::PointerType(new DST::BasicType((DST::TypeSpecifierType*)i.second.second));
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
			auto retType = ((DST::PropertyType*)i.second.second)->getReturn();
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

DST::Program * Decorator::decorateProgram(AST::StatementBlock * node)
{
	_currentProgram = new DST::Program();

	// createErrorInterfaceType();

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

DST::Expression * Decorator::decorate(AST::Expression * node)
{
	if (node == nullptr)
		return NULL;
	switch (dynamic_cast<AST::Expression*>(node)->getExpressionType())
	{
	case ET_IDENTIFIER:
		return decorate(dynamic_cast<AST::Identifier*>(node));
	case ET_BINARY_OPERATION:
		return decorate(dynamic_cast<AST::BinaryOperation*>(node));
	case ET_LITERAL:
		return decorate(dynamic_cast<AST::Literal*>(node));
	case ET_UNARY_OPERATION: 
		return decorate(dynamic_cast<AST::UnaryOperation*>(node));
	case ET_VARIABLE_DECLARATION:
		return decorate(dynamic_cast<AST::VariableDeclaration*>(node));
	case ET_ASSIGNMENT:
		return decorate(dynamic_cast<AST::Assignment*>(node));
	case ET_LIST:
		return decorate(dynamic_cast<AST::ExpressionList*>(node));
	case ET_FUNCTION_CALL:
		return decorate(dynamic_cast<AST::FunctionCall*>(node));
	case ET_CONDITIONAL_EXPRESSION:
		return decorate(dynamic_cast<AST::ConditionalExpression*>(node));
	case ET_INCREMENT:
		return decorate(dynamic_cast<AST::Increment*>(node));
	default: 
		throw ErrorReporter::report("Unimplemented expression type in the decorator", ERR_DECORATOR, node->getPosition());
	}
}

DST::Statement * Decorator::decorate(AST::Statement * node)
{
	if (node == nullptr)
		return NULL;
	switch (dynamic_cast<AST::Statement*>(node)->getStatementType())
	{
	case ST_FUNCTION_CALL:
	{
		auto n = decorate(dynamic_cast<AST::FunctionCall*>(node));
		if (!n->isStatement())
			throw ErrorReporter::report("expected a statement", ERR_DECORATOR, node->getPosition());
		return dynamic_cast<DST::ExpressionStatement*>(n);
	}
	case ST_NAMESPACE_DECLARATION:
		return decorate(dynamic_cast<AST::NamespaceDeclaration*>(node));
	case ST_FUNCTION_DECLARATION:
		return decorate(dynamic_cast<AST::FunctionDeclaration*>(node));
	case ST_VARIABLE_DECLARATION:
		return decorate(dynamic_cast<AST::VariableDeclaration*>(node));
	case ST_CONST_DECLARATION:
		return decorate(dynamic_cast<AST::ConstDeclaration*>(node));
	case ST_ASSIGNMENT:
		return decorate(dynamic_cast<AST::Assignment*>(node));
	case ST_STATEMENT_BLOCK:
		return decorate(dynamic_cast<AST::StatementBlock*>(node));
	case ST_PROPERTY_DECLARATION:
		return decorate(dynamic_cast<AST::PropertyDeclaration*>(node));
	case ST_IF_THEN_ELSE:
		return decorate(dynamic_cast<AST::IfThenElse*>(node));
	case ST_SWITCH:
		return decorate(dynamic_cast<AST::SwitchCase*>(node));
	case ST_FOR_LOOP:
		return decorate(dynamic_cast<AST::ForLoop*>(node));
	case ST_WHILE_LOOP: 
		return decorate(dynamic_cast<AST::WhileLoop*>(node));
	case ST_UNARY_OPERATION:
		return decorate(dynamic_cast<AST::UnaryOperationStatement*>(node));
	case ST_TYPE_DECLARATION:
		return decorate(dynamic_cast<AST::TypeDeclaration*>(node));
	case ST_DO_WHILE_LOOP: 
		return decorate(dynamic_cast<AST::DoWhileLoop*>(node));
	case ST_INCREMENT:
		return decorate(dynamic_cast<AST::Increment*>(node));
	case ST_TRY_CATCH:
		return decorate(dynamic_cast<AST::TryCatch*>(node));
	default: 
		throw ErrorReporter::report("Unimplemented statement type in the decorator", ERR_DECORATOR, node->getPosition());
	}
}

DST::Expression *Decorator::decorate(AST::Identifier * node)
{
	unicode_string &name = node->getVarId();

	if (name == unicode_string("var"))
		return _unknownType;


	for (int scope = currentScope(); scope >= 0; scope--)
		if (auto var = _variables[scope][name]) {
			if (var->getExactType() == EXACT_SPECIFIER)
				return new DST::BasicType(node, (DST::TypeSpecifierType*)var);
			return new DST::Variable(node, var);
		}

	if (_currentTypeDecl) if (auto var = _currentTypeDecl->getMemberType(name))
	{
		auto thisId = new AST::Identifier(unicode_string("this"));
		auto bop = new AST::BinaryOperation();
		bop->setLeft(thisId);
		bop->setRight(node);
		bop->setOperator(OperatorsMap::getOperatorByDefinition(OT_PERIOD).second);
		auto acc = new DST::MemberAccess(bop, decorate(thisId));
		acc->setType(var);
		return acc;

		//if (var->getExactType() == EXACT_SPECIFIER)
		//	return new DST::BasicType(node, (DST::TypeSpecifierType*)var);
		//return new DST::Variable(node, var);
	}

	for (int i = _currentNamespace.size() - 1; i >= 0; i--)
	{
		if (auto var = _currentNamespace[i]->getMemberType(name)) {
			if (var->getExactType() == EXACT_SPECIFIER)
				return new DST::BasicType(node, (DST::TypeSpecifierType*)var);
			return new DST::Variable(node, var);
		}
	}

	if (_currentProgram) 
		if (auto var = _currentProgram->getNamespace(name))
			return new DST::Variable(node, new DST::NamespaceType(var));

	throw ErrorReporter::report("Identifier '" + name.to_string() + "' is undefined", ERR_DECORATOR, node->getPosition());
}

DST::Increment *Decorator::decorate(AST::Increment *node)
{
	return new DST::Increment(node, decorate(node->getExpression()), node->isIncrement());
}

DST::FunctionDeclaration * Decorator::decorate(AST::FunctionDeclaration * node)
{
	enterBlock();
	auto decl = new DST::FunctionDeclaration(node, decorate(node->getVarDecl()));
	for (auto i : node->getParameters())
		decl->addParameter(decorate(i));

	auto type = new DST::FunctionType();
	type->addReturn(decl->getReturnType());
	for (auto i : decl->getParameters())
		type->addParameter(i->getType());
	_variables[currentScope() - 1][decl->getVarDecl()->getVarId()] = type;

	decl->setContent(decorate(node->getContent()));
	if (decl->getContent() && !decl->getContent()->hasReturnType(decl->getReturnType()))
		throw ErrorReporter::report("Not all control paths lead to a return value.", ERR_DECORATOR, node->getPosition());
	leaveBlock();
	return decl;
}

DST::PropertyDeclaration * Decorator::decorate(AST::PropertyDeclaration * node)
{
	unicode_string name = node->getVarDecl()->getVarId();
	DST::Type* type = evalType(node->getVarDecl()->getVarType());
	for (int scope = currentScope(); scope >= 0; scope--)
		if (_variables[scope].count(name))
			throw ErrorReporter::report("Identifier '" + name.to_string() + "' is already in use", ERR_DECORATOR, node->getPosition());

	DST::StatementBlock *get = decorate(node->getGet());

	if (get && !get->hasReturnType(type))
		throw ErrorReporter::report("Not all control paths lead to a return value.", ERR_DECORATOR, node->getPosition());

	enterBlock();
	_variables[currentScope()][unicode_string("value")] = type;
	DST::StatementBlock *set = decorate(node->getSet());
	leaveBlock();

	_variables[currentScope()][name] = new DST::PropertyType(type, get, set);
	return new DST::PropertyDeclaration(node, get, set, type);
}

DST::UnaryOperationStatement * Decorator::decorate(AST::UnaryOperationStatement * node)
{
	if (node->getOperator()._type == OT_BREAK && _loopCount == 0)
		throw ErrorReporter::report("\"break\" used outside of loop", ERR_DECORATOR, node->getPosition());
	if (node->getOperator()._type == OT_CONTINUE && _loopCount == 0)
		throw ErrorReporter::report("\"continue\" used outside of loop", ERR_DECORATOR, node->getPosition());
	return new DST::UnaryOperationStatement(node, decorate(node->getExpression()));
}

DST::TypeDeclaration * Decorator::decorate(AST::TypeDeclaration * node)
{
	enterBlock();
	auto decl = new DST::TypeDeclaration(node);
	vector<DST::Statement*> decls;

	for (auto i : node->getVariableDeclarations()) 
	{
		auto dec = decorate(i);
		decl->addDeclaration(dec, _variables[currentScope()][dec->getVarId()]);
	}
	for (auto i : node->getFunctionDeclarations()) 
	{
		auto dec = decorate(i);
		decl->addDeclaration(dec, _variables[currentScope()][dec->getVarDecl()->getVarId()]);
	}
	for (auto i : node->getPropertyDeclarations())
	{
		auto dec = decorate(i);
		decl->addDeclaration(dec, _variables[currentScope()][dec->getPropId()]);
	}

	leaveBlock();
	_variables[currentScope()][decl->getName()] = new DST::TypeSpecifierType(decl);
	return decl;
}

DST::Expression * Decorator::decorate(AST::UnaryOperation * node)
{
	auto val = decorate(node->getExpression());
	if (node->getOperator()._type == OT_SQUARE_BRACKETS_OPEN)	// Array Literal
	{
		if (!val)
			throw ErrorReporter::report("array literal can't be empty", ERR_DECORATOR, node->getPosition());
		else if (val->getExpressionType() == ET_LIST)
		{
			DST::Type *prevType = NULL;
				for (auto val : ((DST::ExpressionList*)val)->getExpressions())
				{
					if (prevType && !val->getType()->equals(prevType))
						throw ErrorReporter::report("Array Literal must have only one type", ERR_DECORATOR, node->getPosition());
						prevType = val->getType();
				}
			if (!prevType)
				throw ErrorReporter::report("array literal can't be empty", ERR_DECORATOR, node->getPosition());
			return new DST::ArrayLiteral(prevType, ((DST::ExpressionList*)val)->getExpressions());
		}
		else
		{
			vector<DST::Expression*> v;
			v.push_back(val);
			return new DST::ArrayLiteral(val->getType(), v);
		}
	}
	
	if (node->getOperator()._type == OT_AT && val->getExpressionType() == ET_TYPE)	// Pointer Type
		return new DST::PointerType(node, (DST::Type*)val);

	return new DST::UnaryOperation(node, val);
}

//bool Decorator::isBool(DST::Type *type) { return DST::BasicType(getPrimitiveType("bool")).equals(type); }

DST::Expression * Decorator::decorate(AST::ConditionalExpression * node)
{
	auto expr = new DST::ConditionalExpression(node);
	expr->setCondition(decorate(node->getCondition()));
	expr->setThenBranch(decorate(node->getThenBranch()));
	expr->setElseBranch(decorate(node->getElseBranch()));

	if (!expr->getCondition())
		throw ErrorReporter::report("Expected a condition", ERR_DECORATOR, node->getPosition());
	if (!isCondition(expr->getCondition()))
		throw ErrorReporter::report("Condition must be of bool type", ERR_DECORATOR, node->getPosition());
	if (!expr->getThenBranch())
		throw ErrorReporter::report("Expected then branch of conditional expression", ERR_DECORATOR, node->getPosition());
	if (!expr->getElseBranch())
		throw ErrorReporter::report("Expected else branch of conditional expression", ERR_DECORATOR, node->getPosition());
	if (!expr->getThenBranch()->getType()->equals(expr->getElseBranch()->getType()))
		throw ErrorReporter::report("Operand types are incompatible (\"" + expr->getThenBranch()->getType()->toShortString() + 
							"\" and \"" + expr->getElseBranch()->getType()->toShortString() + "\")", ERR_DECORATOR, node->getPosition());
	return expr;
}

DST::ConstDeclaration *Decorator::decorate(AST::ConstDeclaration * node)
{
	auto decl = new DST::ConstDeclaration(node);
	decl->setExpression(decorate(node->getExpression()));
	unicode_string name = node->getName();
	if (_variables[currentScope()].count(name))
		throw ErrorReporter::report("Identifier '" + name.to_string() + "' is already in use", ERR_DECORATOR, node->getPosition());
	decl->getExpression()->getType()->setNotWritable();
	decl->getExpression()->getType()->setConst();
	_variables[currentScope()][name] = decl->getExpression()->getType();
	return decl;
}

DST::VariableDeclaration *Decorator::decorate(AST::VariableDeclaration * node)
{
	auto decl = new DST::VariableDeclaration(node);
	unicode_string name = node->getVarId();
	decl->setType(evalType(node->getVarType()));

	if (_variables[currentScope()].count(name))
		throw ErrorReporter::report("Identifier '" + name.to_string() + "' is already in use", ERR_DECORATOR, node->getPosition());
	_variables[currentScope()][name] = decl->getType();
	return decl;
}

DST::Assignment * Decorator::decorate(AST::Assignment * node)
{
	auto assignment = new DST::Assignment(node, decorate(node->getLeft()), decorate(node->getRight()));

	if (!assignment->getLeft()->getType()->writeable())
		throw ErrorReporter::report("lvalue is read-only", ERR_DECORATOR, node->getPosition());
	if (!assignment->getRight()->getType()->readable())
		throw ErrorReporter::report("rvalue is write-only", ERR_DECORATOR, node->getPosition());

	if (assignment->getLeft()->getExpressionType() == ET_LIST)
	{
		auto list = (DST::ExpressionList*)assignment->getLeft();
		auto leftTypes = (DST::TypeList*)assignment->getLeft()->getType();
		auto rightTypes = (DST::TypeList*)assignment->getRight()->getType();
		for (unsigned int i = 0; i < list->size(); i++)
		{
			if (list->getExpressions()[i]->getType()->getExactType() == EXACT_UNKNOWN)
			{
				if (list->getExpressions()[i]->getExpressionType() != ET_VARIABLE_DECLARATION)
					throw ErrorReporter::report("Unknown type?.", ERR_DECORATOR, node->getPosition());
				((DST::VariableDeclaration*)list->getExpressions()[i])->setType(rightTypes->getTypes()[i]);
				leftTypes->getTypes()[i] = rightTypes->getTypes()[i];
				_variables[currentScope()][((DST::VariableDeclaration*)list->getExpressions()[i])->getVarId()] = rightTypes->getTypes()[i];
			}
		}
	}
	
	if (assignment->getLeft()->getType()->getExactType() == EXACT_UNKNOWN)
	{
		if (assignment->getLeft()->getExpressionType() != ET_VARIABLE_DECLARATION)
			throw ErrorReporter::report("Unknown type?.", ERR_DECORATOR, node->getPosition());
		((DST::VariableDeclaration*)assignment->getLeft())->setType(assignment->getRight()->getType());
		_variables[currentScope()][((DST::VariableDeclaration*)assignment->getLeft())->getVarId()] = assignment->getRight()->getType();
	}

	if (!assignment->getRight()->getType()->assignableTo(assignment->getLeft()->getType()))
		throw ErrorReporter::report("Type \"" + assignment->getRight()->getType()->toShortString() + "\" is not assignable to type \""
			  						+ assignment->getLeft()->getType()->toShortString(), ERR_DECORATOR, node->getPosition());

	if (assignment->getRight()->getType()->getExactType() == EXACT_NULL)
		assignment->setRight(new DST::Conversion(NULL, assignment->getLeft()->getType(), assignment->getRight()));
	
	// else if (!assignment->getLeft()->getType()->equals(assignment->getRight()->getType()))	SWITCHEROO
	// 	throw ErrorReporter::report("Assignment of different types invalid.\n\tleft type is: " + assignment->getLeft()->getType()->toShortString() + 
	// 						"\n\tright type is: " + assignment->getRight()->getType()->toShortString(), ERR_DECORATOR, node->getPosition());

	assignment->setType(assignment->getLeft()->getType());
	return assignment;
}

DST::Expression * Decorator::decorate(AST::FunctionCall * node)
{
	auto funcId = decorate(node->getFunctionId());
	DST::ExpressionList *arguments;
	if (node->getArguments() == nullptr || node->getArguments()->getExpressionType() != ET_LIST)
	{
		auto list = new DST::ExpressionList(node->getArguments());
		if (node->getArguments()) 
		{
			auto dec = decorate(node->getArguments());
			if (!dec->getType()->readable())
				throw ErrorReporter::report("argument is write-only", ERR_DECORATOR, dec->getPosition());
			list->addExpression(dec);
		}
		arguments = list;
	}
	else arguments = (DST::ExpressionList*)decorate((AST::ExpressionList*)node->getArguments());	// TODO - fix unsafe conversion

	if (funcId->getExpressionType() == ET_TYPE)	// function type OR conversion
	{
		bool areAllTypes = true;
		for (auto i : arguments->getExpressions())
			if (i->getExpressionType() != ET_TYPE)
				areAllTypes = false;

		if (areAllTypes)
		{
			// function type
			auto type = new DST::FunctionType(node);
			type->addReturn((DST::Type*)funcId);
			for (auto i : arguments->getExpressions())
				type->addParameter((DST::Type*)i);
			return type;
		}
		if (arguments->getExpressions().size() == 1) // conversion
			return new DST::Conversion(node, (DST::Type*)funcId, arguments->getExpressions()[0]);
		throw ErrorReporter::report("invalid function arguments", ERR_DECORATOR, node->getPosition());
	}

	auto call = new DST::FunctionCall(node, funcId, arguments);
	return call;
}

DST::FunctionLiteral * Decorator::decorate(AST::Function * node)
{
	enterBlock();
	auto lit = new DST::FunctionLiteral(node);
	auto type = new DST::FunctionType();
	if (node->getReturnType() == NULL)
		type->addReturn(new DST::BasicType(getPrimitiveType("void")));
	else type->addReturn(evalType(node->getReturnType()));
	for (auto i : node->getParameters())
	{
		auto param = decorate(i);
		type->addParameter(param->getType());
		lit->addParameter(param);
	}
	lit->setContent(decorate(node->getContent()));
	lit->setType(type);
	if (lit->getContent() && !lit->getContent()->hasReturnType(type->getReturns()))
		throw ErrorReporter::report("Not all control paths lead to a return value.", ERR_DECORATOR, node->getPosition());
	leaveBlock();
	return lit;
}

DST::Expression * Decorator::decorate(AST::BinaryOperation * node)
{
	if (node->getOperator()._type == OT_PERIOD) 
	{
		auto left = decorate(node->getLeft());

		auto type = left->getType();

		if (node->getRight()->getExpressionType() != ET_IDENTIFIER)
			throw ErrorReporter::report("Expected an identifier", ERR_DECORATOR, node->getPosition());

		if (type->getExactType() == EXACT_TYPELIST && ((DST::TypeList*)type)->getTypes().size() == 1)
			type = ((DST::TypeList*)type)->getTypes()[0];

		if (type->getExactType() == EXACT_PROPERTY && ((DST::PropertyType*)type)->hasGet())
			type = ((DST::PropertyType*)type)->getReturn();

		if (type->getExactType() == EXACT_POINTER)
			type = ((DST::PointerType*)type)->getPtrType();
			

		DST::Type *memberType = NULL;
		unicode_string varId = ((AST::Identifier*)node->getRight())->getVarId();
		if (type->getExactType() == EXACT_BASIC)
		{
			if (_currentTypeDecl != ((DST::BasicType*)type)->getTypeSpecifier()->getTypeDecl() && !varId[0].isUpper())
				throw ErrorReporter::report("Cannot access private member \"" + varId.to_string() + "\"", ERR_DECORATOR, node->getPosition());
			memberType = ((DST::BasicType*)type)->getTypeSpecifier()->getMemberType(varId);
		}
		else if (type->getExactType() == EXACT_NAMESPACE)
			memberType = ((DST::NamespaceType*)type)->getNamespaceDecl()->getMemberType(varId);
		else if (type->getExactType() == EXACT_ARRAY && varId == unicode_string("Size"))
			memberType = new DST::PropertyType(new DST::BasicType(getPrimitiveType("int")), true, false);
		else throw ErrorReporter::report("Expression must have class or namespace type", ERR_DECORATOR, left->getPosition());

		if (memberType == nullptr)
			throw ErrorReporter::report("Unkown identifier \"" + varId.to_string() + "\"", ERR_DECORATOR, node->getRight()->getPosition());
		
		// TODO - are we leaking memory here?
		if (memberType->getExactType() == EXACT_SPECIFIER)
			return new DST::BasicType((DST::TypeSpecifierType*)memberType);	

		auto access = new DST::MemberAccess(node, left);
		access->setType(memberType);
		return access;
	}

	if (node->getOperator()._type == OT_AS)
		return new DST::Conversion(NULL, evalType(node->getRight()), decorate(node->getLeft()));

	auto left = decorate(node->getLeft());
	auto right = decorate(node->getRight());

	if (left->getExpressionType() == ET_TYPE)
	{
		if (right)
		{
			if (!(right->getExpressionType() == ET_LITERAL && ((DST::Literal*)right)->getLiteralType() == LT_INTEGER))
				throw ErrorReporter::report("array size must be a literal integer", ERR_DECORATOR, right->getPosition());
			return new DST::ArrayType((DST::Type*)left, *((int*)((DST::Literal*)(right))->getValue()));
		}
		else return new DST::ArrayType((DST::Type*)left, DST::UNKNOWN_ARRAY_LENGTH);
	}

	auto bo = new DST::BinaryOperation(node, left, right);

	switch (OperatorsMap::getReturnType(node->getOperator()._type))
	{
		case RT_BOOLEAN: 
			bo->setType(new DST::BasicType(getPrimitiveType("bool")));
			break;
		case RT_ARRAY:
			// array access
			if (bo->getLeft()->getType()->getExactType() == EXACT_ARRAY)
			{
				DST::BasicType *intType = new DST::BasicType(getPrimitiveType("int"));
				if(!bo->getRight()->getType()->equals(intType))
					throw ErrorReporter::report("array index must be an integer value", ERR_DECORATOR, bo->getRight()->getPosition());
				bo->setType(((DST::ArrayType*)bo->getLeft()->getType())->getElementType());
				_toDelete.push_back(intType);
			}
			// // array declaration
			// else
			// {
			// 	if (bo->getLeft()->getExpressionType() != ET_TYPE)
			// 		throw ErrorReporter::report("expected a type", ERR_DECORATOR, node->getPosition());
			// 	if (bo->getRight())
			// 	{
			// 		if (!(bo->getRight()->getExpressionType() == ET_LITERAL && ((DST::Literal*)bo->getRight())->getLiteralType() == LT_INTEGER))
			// 			throw ErrorReporter::report("array size must be a literal integer", ERR_DECORATOR, node->getPosition());
			// 		//bo->setType(new DST::ArrayType((DST::Type*)bo->getLeft(), *((int*)((DST::Literal*)(bo->getRight()))->getValue())));
			// 		auto ret = new DST::ArrayType((DST::Type*)bo->getLeft(), *((int*)((DST::Literal*)(bo->getRight()))->getValue()));
			// 		_toDelete.push_back(bo);
			// 		return ret;
			// 	}
			// 	else 
			// 	{
			// 		//bo->setType(new DST::ArrayType((DST::Type*)bo->getLeft(), DST::UNKNOWN_ARRAY_LENGTH));
			// 		auto ret = new DST::ArrayType((DST::Type*)bo->getLeft(), DST::UNKNOWN_ARRAY_LENGTH);
			// 		_toDelete.push_back(bo);
			// 		return ret;
			// 	}
			// }
			break;
		case RT_VOID: 
			throw ErrorReporter::report("Could not decorate, unimplemented operator.", ERR_DECORATOR, node->getPosition());

		default: 
			if (!bo->getLeft()->getType()->equals(bo->getRight()->getType()))
				throw ErrorReporter::report("left-type != right-type", ERR_DECORATOR, node->getPosition());
			bo->setType(bo->getLeft()->getType());
			break;
	}

	return bo;
}

DST::Expression * Decorator::decorate(AST::Literal * node)
{
	if (node->getLiteralType() == LT_FUNCTION)
		return decorate((AST::Function*)node);
	auto lit = new DST::Literal(node);
	switch (node->getLiteralType()) 
	{
	case (LT_BOOLEAN):		lit->setType(new DST::BasicType(getPrimitiveType("bool")));	break;
	case (LT_CHARACTER):	lit->setType(new DST::BasicType(getPrimitiveType("char")));	break;
	case (LT_FRACTION):		lit->setType(new DST::BasicType(getPrimitiveType("float")));	break;
	case (LT_INTEGER):		lit->setType(new DST::BasicType(getPrimitiveType("int")));	break;
	case (LT_STRING):		lit->setType(new DST::BasicType(getPrimitiveType("string"))); break;
	case (LT_NULL):			lit->setType(_nullType);	break;
	default: break;
	}
	return lit;
}

DST::Expression * Decorator::decorate(AST::ExpressionList * node)
{
	vector<DST::Expression*> vec;

	bool isTypeList = true;
	for (auto i : node->getExpressions())
	{
		auto dec = decorate(i);
		if (dec->getExpressionType() != ET_TYPE)
			isTypeList = false;
		vec.push_back(dec);
	}

	if (isTypeList)
	{
		auto list = new DST::TypeList(node);
		for (auto i : vec)
			list->addType((DST::Type*)i);
		return list;
	}

	return new DST::ExpressionList(node, vec);
}

DST::NamespaceDeclaration * Decorator::decorate(AST::NamespaceDeclaration * node)
{
	auto decl = new DST::NamespaceDeclaration(node);
	_variables[currentScope()][decl->getName()] = new DST::NamespaceType(decl);
	enterBlock();
	for (auto i : node->getStatement()->getStatements())
	{
		auto d = decorate(i);
		unicode_string name;
		
		switch (d->getStatementType())
		{
		case ST_NAMESPACE_DECLARATION: 	name = ((DST::NamespaceDeclaration*)d)->getName(); break;
		case ST_PROPERTY_DECLARATION:  	name = ((DST::PropertyDeclaration*)d)->getName(); break;
		case ST_FUNCTION_DECLARATION:  	name = ((DST::FunctionDeclaration*)d)->getVarDecl()->getVarId(); break;
		case ST_VARIABLE_DECLARATION:  	name = ((DST::VariableDeclaration*)d)->getVarId(); break;
		case ST_TYPE_DECLARATION: 		name = ((DST::TypeDeclaration*)d)->getName(); break;
		default: throw ErrorReporter::report("Expected a declaration", ERR_DECORATOR, d->getPosition());
		}

		decl->addMember(name, d, _variables[currentScope()][name]);
	}
		
	leaveBlock();
	return decl;
}

DST::StatementBlock * Decorator::decorate(AST::StatementBlock * node)
{
	if (!node)
		return NULL;

	enterBlock();
	auto bl = new DST::StatementBlock();
	for (auto i : dynamic_cast<AST::StatementBlock*>(node)->getStatements())
		bl->addStatement(decorate(i));
	leaveBlock();
	return bl;
}

DST::IfThenElse * Decorator::decorate(AST::IfThenElse * node)
{
	auto ite = new DST::IfThenElse(node);
	ite->setCondition(decorate(node->getCondition()));
	if (!isCondition(ite->getCondition()))
		throw ErrorReporter::report("Expected a condition", ERR_DECORATOR, node->getCondition()->getPosition());
	ite->setThenBranch(decorate(node->getThenBranch()));
	ite->setElseBranch(decorate(node->getElseBranch()));
	return ite;
}

DST::SwitchCase * Decorator::decorate(AST::SwitchCase * node)
{
	auto sc = new DST::SwitchCase(node);
	sc->setExpression(decorate(node->getExpression()));
	sc->setDefault(decorate(node->getDefault()));
	for (auto c : node->getCases()) {
		DST::CaseClause clause;
		auto expr = decorate(c._expression);
		if (expr->getExpressionType() == ET_LIST)
			clause = { ((DST::ExpressionList*)expr)->getExpressions(), decorate(c._statement) };
		else clause = { { expr }, decorate(c._statement) };
		for (auto i : clause._expressions) 
			if (!i->getType()->equals(sc->getExpression()->getType()))
				throw ErrorReporter::report("case clause has type \"" + i->getType()->toShortString() + "\" instead of the required \"" 
					+ sc->getExpression()->getType()->toShortString() + "\" type", ERR_DECORATOR, i->getPosition());
		sc->addCase(clause);
	}
	return sc;
}

DST::TryCatch * Decorator::decorate(AST::TryCatch *node)
{
	auto tryCatch = new DST::TryCatch(node);
	tryCatch->setTryBlock(decorate(node->getTryBlock()));
	enterBlock();
	_variables[currentScope()][unicode_string("caught")] = new DST::BasicType(getPrimitiveType(ERROR_TYPE_NAME));
	tryCatch->setCatchBlock(decorate(node->getCatchBlock()));
	leaveBlock();
	return tryCatch;
}

DST::ForLoop * Decorator::decorate(AST::ForLoop * node)
{
	enterBlock();
	_loopCount++;
	auto loop = new DST::ForLoop(node);
	loop->setBegin(decorate(node->getBegin()));
	loop->setCondition(decorate(node->getCondition()));
	if (!isCondition(loop->getCondition()))
		throw ErrorReporter::report("Expected a condition", ERR_DECORATOR, loop->getCondition()->getPosition());
	loop->setIncrement(decorate(node->getIncrement()));
	loop->setStatement(decorate(node->getStatement()));
	_loopCount--;
	leaveBlock();
	return loop;
}

DST::WhileLoop * Decorator::decorate(AST::WhileLoop * node)
{
	auto loop = new DST::WhileLoop(node);
	_loopCount++;
	loop->setCondition(decorate(node->getCondition()));
	if (!isCondition(loop->getCondition()))
		throw ErrorReporter::report("Expected a condition", ERR_DECORATOR, loop->getCondition()->getPosition());
	loop->setStatement(decorate(node->getStatement()));
	_loopCount--;
	return loop;
}

DST::DoWhileLoop * Decorator::decorate(AST::DoWhileLoop * node)
{
	auto loop = decorate(dynamic_cast<AST::WhileLoop*>(node));
	return new DST::DoWhileLoop(loop);
}

DST::Type * Decorator::evalType(AST::Expression * node)
{
	auto ret = decorate(node);
	if (ret->getExpressionType() != ET_TYPE)
		throw ErrorReporter::report("expected a type", ERR_DECORATOR, ret->getPosition());
	return (DST::Type*)ret;
}

bool Decorator::isCondition(DST::Expression * node)
{
	return node && node->getType()->getExactType() == EXACT_BASIC
		&& dynamic_cast<DST::BasicType*>(node->getType())->getTypeId() == CONDITION_TYPE;
}

void Decorator::leaveBlock()
{
	for (auto i : _variables[currentScope()])
		_toDelete.push_back(i.second);
	_variables.pop_back();
}

void Decorator::clear()
{
	leaveBlock();
	for (auto i : _toDelete)
		if (i != nullptr)
		{
			try
			{
				delete i;
			}
			catch (...)
			{
				// continue;
			}
		}
}
