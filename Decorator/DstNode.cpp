#include "DstNode.h"

DST::Type * DST::Type::getType()
{
	std::cout << "you probably didn't wanna call this func" << std::endl;
	return typeidTypePtr;
}

vector<DST::Node*> DST::Type::getChildren()
{
	return vector<Node*>();
}

vector<DST::Node*> DST::BasicType::getChildren()
{
	return vector<Node*>();
}

vector<DST::Node*> DST::Variable::getChildren()
{
	vector<Node*> v;
	return v;
}

DST::BinaryOperation::BinaryOperation(AST::BinaryOperation * base, Expression * left, Expression * right) : _base(base), _left(left), _right(right) 
{
	if (_left && _left->getType() && !_left->getType()->readable())
		throw ErrorReporter::report("left value is write-only", ERR_DECORATOR, _left->getPosition());
	if (_right && _right->getType() && !_right->getType()->readable())
		throw ErrorReporter::report("right value is write-only", ERR_DECORATOR, _left->getPosition());
};

vector<DST::Node*> DST::BinaryOperation::getChildren()
{
	vector<Node*> v;
	v.push_back(_left);
	v.push_back(_right);
	return v;
}

vector<DST::Node*> DST::UnaryOperation::getChildren()
{
	vector<Node*> v;
	v.push_back(_expression);
	return v;
}

vector<DST::Node*> DST::ConditionalExpression::getChildren()
{
	vector<Node*> v;
	v.push_back(_condition);
	v.push_back(_thenBranch);
	v.push_back(_elseBranch);
	return v;
}

void DST::UnaryOperation::setType()
{
	switch (_base->getOperator()._type)
	{
	case OT_AT:	// dereference
		if (_expression->getType()->getExactType() != EXACT_POINTER)
			throw ErrorReporter::report("Cannot dereference a non-pointer type", ERR_DECORATOR, getPosition());
		_type = ((DST::PointerType*)_expression->getType())->getPtrType();
		break;
	case OT_NEW:	// dynamic memory allocation
		if (_expression->getExpressionType() != ET_TYPE)
			throw ErrorReporter::report("Expected a type", ERR_DECORATOR, getPosition());
		if (((DST::Type*)_expression)->getExactType() == EXACT_ARRAY)	// new int[10] == int[] != (int[10])@
		{
			if (((DST::ArrayType*)_expression)->getLength() == DST::UNKNOWN_ARRAY_LENGTH)
				throw ErrorReporter::report("Expected an array size specifier", ERR_DECORATOR, getPosition());
			_type = new DST::ArrayType(((DST::ArrayType*)_expression)->getElementType(), DST::UNKNOWN_ARRAY_LENGTH);
		}
		else _type = new DST::PointerType((DST::Type*)_expression);
		// if (_expression->getType()->getExactType() != EXACT_SPECIFIER)
		// 	throw ErrorReporter::report("Expected a type specifier", ERR_DECORATOR, getPosition());
		// _type = new DST::PointerType(new BasicType((TypeSpecifierType*)_expression->getType()));
		break;
	case OT_BITWISE_AND:	// address-of
		_type = new DST::PointerType(_expression->getType());
		break;
	case OT_ADD: case OT_SUBTRACT: case OT_LOGICAL_NOT: case OT_BITWISE_NOT:
		_type = _expression->getType();
		break;
	default: throw ErrorReporter::report("Unimplemented unary operator", ERR_DECORATOR, getPosition());
	}
}

vector<DST::Node*> DST::MemberAccess::getChildren()
{
	vector<Node*> v;
	v.push_back(_left);
	return v;
}

vector<DST::Node*> DST::Literal::getChildren()
{
	vector<Node*> v;
	return v;
}

vector<DST::Node*> DST::ExpressionList::getChildren()
{
	vector<Node*> v;
	for (auto i : _expressions)
		v.push_back(i);
	return v;
}

vector<DST::Node*> DST::ArrayLiteral::getChildren()
{
	vector<Node*> v;
	for (auto i : _array)
		v.push_back(i);
	return v;
}

bool DST::StatementBlock::hasReturnType(Type * returnType)
{
	/*if (this == nullptr)
		return false;*/
	bool isVoid = false;
	if (returnType->getExactType() == EXACT_TYPELIST && ((TypeList*)returnType)->getTypes().size() == 1)
		returnType = ((TypeList*)returnType)->getTypes()[0];
	if (returnType->getExactType() == EXACT_BASIC && ((BasicType*)returnType)->getTypeId() == unicode_string("void"))
		isVoid = true;
	for (auto i : _statements) 
	{
		switch (i->getStatementType()) {
			case ST_UNARY_OPERATION:
				if (((DST::UnaryOperationStatement*)i)->getOperator()._type == OT_EXTERN)
					return true;
				if (((DST::UnaryOperationStatement*)i)->getOperator()._type == OT_RETURN)
				{
					if (isVoid) 
					{
						if (((DST::UnaryOperationStatement*)i)->getExpression() == NULL)
							return true;
					}
					else
					{
						if (((DST::UnaryOperationStatement*)i)->getExpression()->getType()->getExactType() == EXACT_NULL)
						{
							auto a = new DST::Conversion(NULL, returnType, ((DST::UnaryOperationStatement*)i)->getExpression());
							((DST::UnaryOperationStatement*)i)->setExpression(a);
							return true;
						}
						if (((DST::UnaryOperationStatement*)i)->getExpression()->getType()->equals(returnType))
							return true;
					}
					std::cout << isVoid << std::endl;
					std::cout << returnType->toShortString() << std::endl;
					std::cout << ((DST::UnaryOperationStatement*)i)->getExpression()->getType()->toShortString() << std::endl;
					throw ErrorReporter::report("Return value type does not match function type.", ERR_DECORATOR, i->getPosition());
				}
				break;
			case ST_IF_THEN_ELSE:
				if (((DST::IfThenElse*)i)->getThenBranch() && ((DST::IfThenElse*)i)->getElseBranch() &&
				 	((DST::IfThenElse*)i)->getThenBranch()->hasReturnType(returnType) &&
					((DST::IfThenElse*)i)->getElseBranch()->hasReturnType(returnType))
					return true;
				break;
			case ST_SWITCH: 
			{
				if (((DST::SwitchCase*)i)->getDefault() == nullptr || !((DST::SwitchCase*)i)->getDefault()->hasReturnType(returnType))
					break;
				bool b = true;
				for (auto i : ((DST::SwitchCase*)i)->getCases()) 
					if (i._statement == nullptr || !i._statement->hasReturnType(returnType)) 
					{ 
						b = false; 
						break; 
					}
				if (b) return true;
				break;
			}
			default: break;
		}
	}
	return isVoid;
}

void DST::StatementBlock::addStatement(DST::Statement *statement)
{ 
	_statements.push_back(statement); 
	if (statement->getStatementType() == ST_UNARY_OPERATION && ((UnaryOperationStatement*)statement)->getOperator()._type == OT_RETURN)
		_hasReturn = true;
}

vector<DST::Node*> DST::StatementBlock::getChildren()
{
	vector<Node*> v;
	for (auto i : _statements)
		v.push_back(i);
	return v;
}

void DST::TypeDeclaration::addDeclaration(Statement * decl, Type * type)
{
	unicode_string varId;
	switch (decl->getStatementType())
	{
	case ST_FUNCTION_DECLARATION: varId = ((FunctionDeclaration*)decl)->getVarDecl()->getVarId(); break;
	case ST_VARIABLE_DECLARATION: varId = ((VariableDeclaration*)decl)->getVarId(); break;
	case ST_PROPERTY_DECLARATION: varId = ((PropertyDeclaration*)decl)->getPropId(); break;
	default: throw ErrorReporter::report("Type declarations may only specify variable, property, and function declarations.", ERR_DECORATOR, decl->getPosition());
	}
	if (_decls.count(varId))
		throw ErrorReporter::report("Multiple members of same name are not allowed", ERR_DECORATOR, decl->getPosition());
	_decls[varId] = std::make_pair(decl, type);
}

bool DST::TypeDeclaration::validateImplements(InterfaceDeclaration * inter)
{
	for (auto decl : inter->getDeclarations())
	{
		if (!(_decls.count(decl.first) != 0 && _decls[decl.first].second->equals(_decls[decl.first].second)))
			throw ErrorReporter::report("Type " + _name.to_string() + " does not implement " + decl.first.to_string() + " of " + inter->getName().to_string(), ERR_DECORATOR, _base->getPosition());
	}
	for (auto i : inter->getImplements())
	{
		validateImplements(i);
	}
	return true;
}

bool DST::TypeDeclaration::implements(InterfaceDeclaration * inter)
{
	if (inter == _anyInterface)
		return true;
	for (auto i : _interfaces)
		if (i->implements(inter))
			return true;
	return false;
}

vector<DST::Node*> DST::TypeDeclaration::getChildren()
{
	vector<Node*> v;
	for (auto i : _decls)
		v.push_back(i.second.first);
	return v;
}

vector<DST::Node*> DST::NamespaceDeclaration::getChildren()
{
	vector<Node*> v;
	for (auto i : _decls)
		v.push_back(i.second.first);
	return v;
}

void DST::NamespaceDeclaration::addMember(unicode_string name, Statement * decl, Type * type)
{
	_decls[name] = std::make_pair(decl, type);
}

void DST::InterfaceDeclaration::addDeclaration(Statement * decl, Type * type)
{
	unicode_string varId;
	switch (decl->getStatementType())
	{
	case ST_FUNCTION_DECLARATION: varId = ((FunctionDeclaration*)decl)->getVarDecl()->getVarId(); break;
	case ST_PROPERTY_DECLARATION: varId = ((PropertyDeclaration*)decl)->getName(); break;
	default: throw ErrorReporter::report("Interface declarations may only specify property, and function declarations.", ERR_DECORATOR, decl->getPosition());
	}
	if (_decls.count(varId))
		throw ErrorReporter::report("Multiple members of same name are not allowed", ERR_DECORATOR, decl->getPosition());
	_decls[varId] = std::make_pair(decl, type);
}

void DST::InterfaceDeclaration::notImplements(InterfaceDeclaration * inter)
{
	for (auto decl : inter->getDeclarations())
	{
		if (_decls.count(decl.first) != 0)
			throw ErrorReporter::report("Interface '" + getName().to_string() + "' already has '" + decl.first.to_string() + "' of '" + inter->getName().to_string() + "'", ERR_DECORATOR, _base->getPosition());
	}
	for (auto i : inter->getImplements())
	{
		i->notImplements(inter);
	}
}

bool DST::InterfaceDeclaration::implements(InterfaceDeclaration * inter)
{
	if (inter == this || inter == _anyInterface)
		return true;
	for (auto i : _implements)
		if (i->implements(inter))
			return true;
	return false;
}

 DST::Statement* DST::InterfaceDeclaration::getDeclaration(unicode_string id)
{
	if (_decls.count(id))
		return _decls[id].first;

	for (auto i : _implements)
		return i->getDeclaration(id);

	return NULL;
}

DST::Type *DST::InterfaceDeclaration::getMemberType(unicode_string id)
{
	if(_decls.count(id))
		return _decls[id].second;
	
	for (auto i : _implements)
		return i->getMemberType(id);

	return NULL;
}

vector<DST::Node*> DST::InterfaceDeclaration::getChildren()
{
	vector<Node*> v;
	for (auto i : _decls)
		v.push_back(i.second.first);
	return v;
}

vector<DST::Node*> DST::IfThenElse::getChildren()
{
	vector<Node*> v;
	v.push_back(_condition);
	v.push_back(_thenBranch);
	v.push_back(_elseBranch);
	return v;
}

vector<DST::Node*> DST::SwitchCase::getChildren()
{
	vector<Node*> v;
	v.push_back(_expression);
	v.push_back(_default);
	for (CaseClause i : _cases)
	{
		v.push_back(i._expression);
		v.push_back(i._statement);
	}
	return v;
}

vector<DST::Node*> DST::WhileLoop::getChildren()
{
	vector<Node*> v;
	v.push_back(_condition);
	v.push_back(_statement);
	return v;
}

vector<DST::Node*> DST::ForLoop::getChildren()
{
	vector<Node*> v;
	v.push_back(_begin);
	v.push_back(_condition);
	v.push_back(_increment);
	v.push_back(_statement);
	return v;
}

vector<DST::Node*> DST::UnaryOperationStatement::getChildren()
{
	vector<Node*> v;
	v.push_back(_expression);
	return v;
}

vector<DST::Node*> DST::ConstDeclaration::getChildren()
{
	return { _expression };
}

vector<DST::Node*> DST::VariableDeclaration::getChildren()
{
	vector<Node*> v;
	return v;
}

vector<DST::Node*> DST::Assignment::getChildren()
{
	vector<Node*> v;
	v.push_back(_left);
	v.push_back(_right);
	return v;
}

vector<DST::Node*> DST::FunctionDeclaration::getChildren()
{
	vector<Node*> v;
	v.push_back(_decl);
	for (auto i : _parameters)
		v.push_back(i);
	v.push_back(_content);
	return v;
}

DST::Type * DST::FunctionDeclaration::getReturnType()
{
	return _decl->getType();
}

vector<DST::Node*> DST::PropertyDeclaration::getChildren()
{
	vector<Node*> v;
	v.push_back(_get);
	v.push_back(_set);
	return v;
}

DST::FunctionCall::FunctionCall(AST::FunctionCall * base, Expression * funcPtr, ExpressionList * arguments) : _base(base)
{
	setFunctionId(funcPtr);
	setArguments(arguments);
}

bool DST::BasicType::assignableTo(DST::Type *type)
{
	if (!type)
		return false;
	if (type->getExactType() == EXACT_PROPERTY)
		return ((DST::PropertyType*)type)->writeable() && assignableTo(((DST::PropertyType*)type)->getReturn());
	if (type->getExactType() != EXACT_BASIC) 
		return false;
	auto other = (DST::BasicType*)type;
	if (!other->_typeSpec) return false;
	if (_typeSpec->getTypeDecl())
		return other->_typeSpec->getTypeDecl() == _typeSpec->getTypeDecl();
	// if (_typeSpec->getInterfaceDecl()) 
	// 	return _typeSpec->getInterfaceDecl()->implements(other->_typeSpec->getInterfaceDecl());
	return false;
}

bool DST::PointerType::assignableTo(DST::Type *type)
{
	if (!type)
		return false;
	if (type->getExactType() == EXACT_PROPERTY)
		return ((DST::PropertyType*)type)->writeable() && assignableTo(((DST::PropertyType*)type)->getReturn());
	if (type->getExactType() == EXACT_BASIC && _type->getExactType() == EXACT_BASIC && ((DST::BasicType*)_type)->getTypeSpecifier()->getTypeDecl())
		return ((DST::BasicType*)type)->getTypeSpecifier()->getInterfaceDecl() &&
				((DST::BasicType*)_type)->getTypeSpecifier()->getTypeDecl()->implements(((DST::BasicType*)type)->getTypeSpecifier()->getInterfaceDecl());
	return type->getExactType() == EXACT_POINTER && _type->assignableTo(((DST::PointerType*)type)->_type);
}

vector<DST::Node*> DST::FunctionCall::getChildren()
{
	vector<Node*> v;
	v.push_back(_funcPtr);
	v.push_back(_arguments);
	return v;
}

bool DST::FunctionType::equals(Type * other)
{
	if (other->getExactType() == EXACT_TYPELIST && ((DST::TypeList*)other)->size() == 1)
		return equals(((DST::TypeList*)other)->getTypes()[0]);
	if (other->getExactType() != EXACT_FUNCTION)
		return false;
	auto othr = (FunctionType*)other;
	return _returns->equals(othr->_returns) && _parameters->equals(othr->_parameters);
}

string DST::FunctionType::toShortString()
{
	return _returns->toShortString() + "(" + _parameters->toShortString() + ")";
}

vector<DST::Node*> DST::FunctionType::getChildren()
{
	return vector<Node*>();
}

void DST::TypeList::addType(Type * type)
{
	if (type->getExactType() == EXACT_TYPELIST)
		for (auto i : ((TypeList*)type)->getTypes())
			_types.push_back(i);
	else _types.push_back(type);
}

bool DST::TypeList::equals(Type * other)
{
	// if (other->getExactType() == EXACT_BASIC)
	// 	return _types.size() == 1 && _types[0]->equals(other);
	if (_types.size() == 1)
		return _types[0]->equals(other);
	if (other->getExactType() != EXACT_TYPELIST)
		return false;
	auto othr = (TypeList*)other;
	if (_types.size() != othr->_types.size())
		return false;
	for (unsigned int i = 0; i < _types.size(); i++)
		if (!_types[i]->equals(othr->_types[i]))
			return false;
	return true;
}

DST::FunctionDeclaration::~FunctionDeclaration() { if (_base) delete _base; if (_decl) delete _decl; if (_content) delete _content; _parameters.clear(); }

string DST::TypeList::toShortString()
{
	string str = "(";
	for (unsigned int i = 0; i < _types.size(); i++)
	{
		if (i > 0)
			str += ", ";
		str += _types[i]->toShortString();
	}
	str += ")";
	return str;
}

vector<DST::Node*> DST::TypeList::getChildren()
{
	return vector<Node*>();
}

bool DST::PropertyType::equals(Type * other)
{
	return _return->equals(other);
}

string DST::PropertyType::toShortString()
{
	return _return->toShortString() + ((_hasGet && _hasSet) ? "{get|set}" : _hasGet ? "{get}" : "{set}");
}

void DST::setup()
{
	typeidTypePtr = NULL;	// TODO
	// DST::_anyInterface = new DST::InterfaceDeclaration(new AST::InterfaceDeclaration(unicode_string("any")));
	// std::cout << "setup dst\n" << _anyInterface << std::endl;
	// std::cout << DST::_anyInterface << "\n";
	//typeidTypePtr = new BasicType(unicode_string("typeid"));
}

vector<DST::Node*> DST::ArrayType::getChildren()
{
	return vector<Node*>();
}

DST::FunctionLiteral::~FunctionLiteral() { if (_base) delete _base; if (_type) delete _type; if (_content) delete _content; _parameters.clear(); }

vector<DST::Node*> DST::FunctionLiteral::getChildren()
{
	vector<Node*> v;
	for (auto i : _parameters)
		v.push_back(i);
	v.push_back(_content);
	return v;
}

DST::Type *DST::BasicType::getMember(unicode_string id) 
{ 
	return _typeSpec->getMemberType(id); 
}

unicode_string DST::BasicType::getTypeId() { return _typeSpec->getTypeName(); }

vector<DST::Node*> DST::Conversion::getChildren()
{
	vector<Node*> v;
	v.push_back(_expression);
	return v;
}

void * DST::Literal::getValue() 
{
	switch (_base->getLiteralType())
	{
		case LT_INTEGER:
			return new int((dynamic_cast<AST::Integer*>(_base)->getValue()));
		default: return NULL;
	}
}

DST::TypeSpecifierType::~TypeSpecifierType() { if (_typeDecl) delete _typeDecl; if (_interfaceDecl) delete _interfaceDecl; }

unicode_string DST::TypeSpecifierType::getTypeName()
{
	if (_typeDecl) return _typeDecl->getName();
	if (_interfaceDecl) return _interfaceDecl->getName();
	return unicode_string();
}

DST::Type * DST::TypeSpecifierType::getMemberType(unicode_string name)
{
	if (_typeDecl) return _typeDecl->getMemberType(name);
	if (_interfaceDecl) return _interfaceDecl->getMemberType(name);
	return NULL;
}

DST::NamespaceType::~NamespaceType() { if (_decl) delete _decl; }

bool DST::PointerType::equals(Type * other)
{
	if (other == nullptr) 
		return false;

	switch (other->getExactType())
	{
		case EXACT_NULL:
			return true;

		case EXACT_TYPELIST:
			return equals(((TypeList*)other)->getTypes()[0]);
		
		case EXACT_PROPERTY:
			return equals(((PropertyType*)other)->getReturn());
		
		case EXACT_POINTER:
			if (_type->getExactType() != EXACT_SPECIFIER)
				return ((PointerType*)other)->_type->equals(_type);
			else if (auto inter = ((TypeSpecifierType*)(_type))->getInterfaceDecl())
			{
				auto otype = ((TypeSpecifierType*)((PointerType*)other)->_type)->getTypeDecl();
				auto ointer = ((TypeSpecifierType*)((PointerType*)other)->_type)->getInterfaceDecl();
				if (otype)
					return otype->implements(inter);
				else if (ointer)
					return inter->implements(ointer) || ointer->implements(inter);
				else return false;
			}
		default:
			return false;
	}
}

string DST::BasicType::toShortString() 
{ 
	if (!_base || _base->getExpressionType() == ET_IDENTIFIER)
		return getTypeId().to_string(); 
	else if (_base->getExpressionType() != ET_BINARY_OPERATION || ((AST::BinaryOperation*)_base)->getOperator()._type != OT_PERIOD)
		throw "internal error in DST::BasicType::toShortString()";
	stack<AST::Identifier*> ids;
	auto bo = ((AST::BinaryOperation*)_base);
	while (bo)
	{
		if (auto id = dynamic_cast<AST::Identifier*>(bo->getRight()))
			ids.push(id);
		else throw "internal error in DST::BasicType::toShortString()";
		if (bo->getLeft()->getExpressionType() == ET_BINARY_OPERATION && ((AST::BinaryOperation*)_base)->getOperator()._type == OT_PERIOD)
			bo = (AST::BinaryOperation*)bo->getLeft();
		else if (auto id = dynamic_cast<AST::Identifier*>(bo->getLeft()))
			ids.push(id);
		else throw "internal error in DST::BasicType::toShortString()";
	}
	string ret = ids.top()->getVarId().to_string();
	ids.pop();
	while (!ids.empty()) 
	{
		ret += "." + ids.top()->getVarId().to_string();
		ids.pop();
	}
	return ret;
}

string DST::PointerType::toShortString()
{
	return _type->toShortString() + "@";
}

vector<DST::Node*> DST::PointerType::getChildren()
{
	vector<Node*> v;
	v.push_back(_type);
	return v;
}

void DST::Program::addImport(string bcFileName) {

	auto dir = opendir(bcFileName.c_str());
	if (!dir)
		throw ErrorReporter::report("Could not open directory \"" + bcFileName + '\"', ERR_DECORATOR, PositionInfo{0, 0, 0, ""});
	while (auto ent = readdir(dir))
	{
		string fileName(ent->d_name);
		if (fileName.substr(fileName.find_last_of(".")) == ".bc")
			_bcFileImports.push_back(bcFileName + '/' + fileName); 
	}
	closedir(dir);
}

vector<DST::Node*> DST::Program::getChildren()
{
	vector <Node*> v;
	for (auto i : _namespaces)
		v.push_back(i.second);
	return v;
}

void DST::Program::addNamespace(NamespaceDeclaration * decl ) { _namespaces[decl->getName()] = decl; }
