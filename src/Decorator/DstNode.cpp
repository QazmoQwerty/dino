/*
	This file implements the bulk of the DST classes' member functions.
	For implementations of DST::Type see DstTypes.cpp.
*/
#include "DstNode.h"

DST::BinaryOperation::BinaryOperation(AST::BinaryOperation * base, Expression * left, Expression * right) : _base(base), _left(left), _right(right) 
{
	if (_left && _left->getType() && !_left->getType()->readable())
		throw ErrorReporter::report("left value is write-only", ERR_DECORATOR, _left->getPosition());
	if (_right && _right->getType() && !_right->getType()->readable())
		throw ErrorReporter::report("right value is write-only", ERR_DECORATOR, _left->getPosition());
};

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
			if (((DST::ArrayType*)_expression)->getLength() == DST::UNKNOWN_ARRAY_LENGTH && ((DST::ArrayType*)_expression)->getLenExp() == NULL)
				throw ErrorReporter::report("Expected an array size specifier", ERR_DECORATOR, getPosition());
			_type = new DST::ArrayType(((DST::ArrayType*)_expression)->getElementType(), DST::UNKNOWN_ARRAY_LENGTH, ((DST::ArrayType*)_expression)->getLenExp());
		}
		else _type = ((DST::Type*)_expression)->getPtrTo();
		break;
	case OT_BITWISE_AND:	// address-of
		_type = _expression->getType()->getPtrTo();
		break;
	case OT_ADD: case OT_SUBTRACT: case OT_LOGICAL_NOT: case OT_BITWISE_NOT:
		_type = _expression->getType();
		break;
	default: throw ErrorReporter::report("Unimplemented unary operator", ERR_DECORATOR, getPosition());
	}
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
					if (isVoid && ((DST::UnaryOperationStatement*)i)->getExpression() == NULL) 
						return true;
					else
					{
						if (((DST::UnaryOperationStatement*)i)->getExpression() == NULL)
							throw ErrorReporter::report("cannot return void in non void function", ERR_DECORATOR, i->getPosition());
						if (((DST::UnaryOperationStatement*)i)->getExpression()->getType()->getExactType() == EXACT_NULL)
						{
							auto a = new DST::Conversion(NULL, returnType, ((DST::UnaryOperationStatement*)i)->getExpression());
							((DST::UnaryOperationStatement*)i)->setExpression(a);
							return true;
						}
						if (((DST::UnaryOperationStatement*)i)->getExpression()->getType()->assignableTo(returnType))
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

void DST::TypeDeclaration::addDeclaration(Statement * decl, Type * type)
{
	unicode_string varId;
	switch (decl->getStatementType())
	{
	case ST_FUNCTION_DECLARATION: varId = ((FunctionDeclaration*)decl)->getVarDecl()->getVarId(); break;
	case ST_VARIABLE_DECLARATION: varId = ((VariableDeclaration*)decl)->getVarId(); break;
	case ST_PROPERTY_DECLARATION: varId = ((PropertyDeclaration*)decl)->getName(); break;
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
	if (_decls.count(id))
		return _decls[id].second;
	
	for (auto i : _implements)
		return i->getMemberType(id);

	return NULL;
}

DST::Type * DST::FunctionDeclaration::getReturnType()
{
	return _decl->getType();
}

DST::FunctionCall::FunctionCall(AST::FunctionCall * base, Expression * funcPtr, ExpressionList * arguments) : _base(base)
{
	setFunctionId(funcPtr);
	setArguments(arguments);
}

DST::FunctionDeclaration::~FunctionDeclaration() { if (_base) delete _base; if (_decl) delete _decl; if (_content) delete _content; _parameters.clear(); }

DST::FunctionLiteral::~FunctionLiteral() { if (_base) delete _base; if (_content) delete _content; _parameters.clear(); }

int DST::Literal::getIntValue() 
{
	if (getLiteralType() != LT_INTEGER)
		throw "literal is not an integer";
	return ((AST::Integer*)_base)->getValue();
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

void DST::FunctionDeclaration::setVarDecl(DST::VariableDeclaration *decl)
{ 
	if (decl->getType()->getExactType() == EXACT_UNKNOWN)
		throw ErrorReporter::report("function return types may not be inferred", ERR_DECORATOR, decl->getPosition());
	_decl = decl; 
}

void DST::FunctionDeclaration::addParameter(DST::VariableDeclaration *parameter)
{ 
	if (parameter->getType()->getExactType() == EXACT_UNKNOWN)
		throw ErrorReporter::report("function parameter types may not be inferred", ERR_DECORATOR, parameter->getPosition());
	_parameters.push_back(parameter); 
}

void DST::FunctionDeclaration::addParameterToStart(DST::VariableDeclaration *parameter)
{
	if (parameter->getType()->getExactType() == EXACT_UNKNOWN)
		throw ErrorReporter::report("function parameter types may not be inferred", ERR_DECORATOR, parameter->getPosition());
	_parameters.insert(_parameters.begin(), parameter); 
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

void DST::Program::addNamespace(NamespaceDeclaration * decl ) { _namespaces[decl->getName()] = decl; }
