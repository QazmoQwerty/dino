#include "DstNode.h"

DST::Type * DST::Type::getType()
{
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

vector<DST::Node*> DST::BinaryOperation::getChildren()
{
	vector<Node*> v;
	v.push_back(_left);
	v.push_back(_right);
	return v;
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
				if (((DST::UnaryOperationStatement*)i)->getOperator()._type == OT_RETURN)
				{
					if ((isVoid && ((DST::UnaryOperationStatement*)i)->getExpression() == NULL) ||
						((DST::UnaryOperationStatement*)i)->getExpression()->getType()->equals(returnType))
						return true;
					throw DinoException("Return value type does not match function type.", EXT_GENERAL, i->getLine());
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
	default: throw DinoException("Type declarations may only specify variable, property, and function declarations.", EXT_GENERAL, decl->getLine());
	}
	if (_decls.count(varId))
		throw DinoException("Multiple members of same name are not allowed", EXT_GENERAL, decl->getLine());
	_decls[varId] = std::make_pair(decl, type);
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
	default: throw DinoException("Interface declarations may only specify property, and function declarations.", EXT_GENERAL, decl->getLine());
	}
	if (_decls.count(varId))
		throw DinoException("Multiple members of same name are not allowed", EXT_GENERAL, decl->getLine());
	_decls[varId] = std::make_pair(decl, type);
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

vector<DST::Node*> DST::FunctionCall::getChildren()
{
	vector<Node*> v;
	v.push_back(_funcPtr);
	v.push_back(_arguments);
	return v;
}

bool DST::FunctionType::equals(Type * other)
{
	if (other->getExactType() != EXACT_FUNCTION)
		return false;
	auto othr = (FunctionType*)other;
	return _returns->equals(othr->_returns) && _parameters->equals(othr->_parameters);
}

string DST::FunctionType::toShortString()
{
	if (_returns->size() == 1)
		return _returns->toShortString() + "(" + _parameters->toShortString() + ")";
	return "(" + _returns->toShortString() + ")(" + _parameters->toShortString() + ")";
}

vector<DST::Node*> DST::FunctionType::getChildren()
{
	return vector<Node*>();
}

bool DST::TypeList::equals(Type * other)
{
	if (other->getExactType() == EXACT_BASIC)
		return _types.size() == 1 && _types[0]->equals(other);
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
	string str = "";
	for (unsigned int i = 0; i < _types.size(); i++)
	{
		if (i > 0)
			str += ", ";
		str += _types[i]->toShortString();
	}
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
	return _return->toShortString();
}

void DST::setup()
{
	typeidTypePtr = NULL;	// TODO
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
	return _typeSpec->getTypeDecl()->getMemberType(id); 
}

unicode_string DST::BasicType::getTypeId() { return _typeSpec->getTypeDecl()->getName(); }

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

DST::TypeSpecifierType::~TypeSpecifierType() { if (_typeDecl) delete _typeDecl; }

DST::NamespaceType::~NamespaceType() { if (_decl) delete _decl; }

DST::InterfaceSpecifierType::~InterfaceSpecifierType()
{
	if (_interfaceDecl)
		delete _interfaceDecl;
}
