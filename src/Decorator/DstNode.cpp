/*
	This file implements the bulk of the DST classes' member functions.
	For implementations of Type see DstTypes.cpp.
*/
#include "DstNode.h"

namespace DST
{
	BinaryOperation::BinaryOperation(AST::BinaryOperation * base, Expression * left, Expression * right) : _base(base), _left(left), _right(right) 
	{
		if (_left && _left->getType() && !_left->getType()->readable())
			throw ErrorReporter::report("left value is write-only", ERR_DECORATOR, _left->getPosition());
		if (_right && _right->getExpressionType() != ET_TYPE && _right->getType() && !_right->getType()->readable())
			throw ErrorReporter::report("right value is write-only", ERR_DECORATOR, _left->getPosition());
	};

	void UnaryOperation::setType()
	{
		switch (_base->getOperator()._type)
		{
		case OT_AT:	// dereference
			if (!_expression->getType()->isPtrTy())
				throw ErrorReporter::report("Cannot dereference a non-pointer type", ERR_DECORATOR, getPosition());
			_type = ((PointerType*)_expression->getType())->getPtrType();
			break;
		case OT_NEW:	// dynamic memory allocation
			if (_expression->getExpressionType() != ET_TYPE)
				throw ErrorReporter::report("Expected a type", ERR_DECORATOR, getPosition());
			if (((Type*)_expression)->isArrayTy())	// new int[10] == int[] != (int[10])@
			{
				if (((ArrayType*)_expression)->getLength() == UNKNOWN_ARRAY_LENGTH && ((ArrayType*)_expression)->getLenExp() == NULL)
					throw ErrorReporter::report("Expected an array size specifier", ERR_DECORATOR, getPosition());
				_type = DST::ArrayType::get(((ArrayType*)_expression)->getElementType(), ((ArrayType*)_expression)->getLenExp());
			}
			else _type = ((Type*)_expression)->getPtrTo();
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

	bool StatementBlock::hasReturnType(Type * returnType)
	{
		/*if (this == nullptr)
			return false;*/
		bool isVoid = false;
		if (returnType->isListTy() && ((TypeList*)returnType)->getTypes().size() == 1)
			throw "unreachable, leaving this to make sure";
		if (returnType == getVoidTy())
			isVoid = true;
		for (auto i : _statements) 
		{
			switch (i->getStatementType()) {
				case ST_UNARY_OPERATION:
				{
					auto unaryOp = (UnaryOperationStatement*)i;
					if (unaryOp->getOperator()._type == OT_EXTERN)
						return true;
					if (unaryOp->getOperator()._type == OT_RETURN)
					{
						if (isVoid && unaryOp->getExpression() == NULL) 
							return true;
						else
						{
							if (unaryOp->getExpression() == NULL)
								throw ErrorReporter::report("cannot return void in non void function", ERR_DECORATOR, i->getPosition());
							if (unaryOp->getExpression()->getType()->isNullTy())
							{
								auto a = new Conversion(NULL, returnType, unaryOp->getExpression());
								unaryOp->setExpression(a);
								return true;
							}
							if (unaryOp->getExpression()->getType()->assignableTo(returnType))
								return true;
						}
						std::cout << isVoid << std::endl;
						std::cout << returnType->toShortString() << std::endl;
						std::cout << unaryOp->getExpression()->getType()->toShortString() << std::endl;
						throw ErrorReporter::report("Return value type does not match function type.", ERR_DECORATOR, i->getPosition());
					}
					break;
				}
				case ST_IF_THEN_ELSE:
					if (((IfThenElse*)i)->getThenBranch() != NULL && 
						((IfThenElse*)i)->getElseBranch() != NULL &&
						((IfThenElse*)i)->getThenBranch()->hasReturnType(returnType) &&
						((IfThenElse*)i)->getElseBranch()->hasReturnType(returnType))
						return true;
					break;
				case ST_SWITCH: 
				{
					if (((SwitchCase*)i)->getDefault() == nullptr || !((SwitchCase*)i)->getDefault()->hasReturnType(returnType))
						break;
					bool b = true;
					for (auto i : ((SwitchCase*)i)->getCases()) 
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

	void StatementBlock::addStatement(Statement *statement)
	{ 
		_statements.push_back(statement); 
		if (statement->getStatementType() == ST_UNARY_OPERATION && ((UnaryOperationStatement*)statement)->getOperator()._type == OT_RETURN)
			_hasReturn = true;
	}

	void TypeDeclaration::addDeclaration(Statement * decl, Type * type)
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

	bool TypeDeclaration::validateImplements(InterfaceDeclaration * inter)
	{
		for (auto decl : inter->getDeclarations())
			if (!(_decls.count(decl.first) != 0 && _decls[decl.first].second->equals(_decls[decl.first].second)))
				throw ErrorReporter::report("Type " + _name.to_string() + " does not implement " + decl.first.to_string() 
											+ " of " + inter->getName().to_string(), ERR_DECORATOR, _base->getPosition());
		for (auto i : inter->getImplements())
			validateImplements(i);
		return true;
	}

	bool TypeDeclaration::implements(InterfaceDeclaration * inter)
	{
		if (inter == getAnyInterface())
			return true;
		for (auto i : _interfaces)
			if (i->implements(inter))
				return true;
		return false;
	}

	void NamespaceDeclaration::addMember(unicode_string name, Statement * decl, Expression * exp)
	{
		_decls[name] = std::make_pair(decl, exp);
	}

	void InterfaceDeclaration::addDeclaration(Statement * decl, Type * type)
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

	void InterfaceDeclaration::notImplements(InterfaceDeclaration * inter)
	{
		for (auto decl : inter->getDeclarations())
			if (_decls.count(decl.first) != 0)
				throw ErrorReporter::report("Interface '" + getName().to_string() + "' already has '" + decl.first.to_string() 
											+ "' of '" + inter->getName().to_string() + "'", ERR_DECORATOR, _base->getPosition());
		for (auto i : inter->getImplements())
			i->notImplements(inter);
	}

	bool InterfaceDeclaration::implements(InterfaceDeclaration * inter)
	{
		if (inter == this || inter == getAnyInterface())
			return true;
		for (auto i : _implements)
			if (i->implements(inter))
				return true;
		return false;
	}

	Statement* InterfaceDeclaration::getDeclaration(unicode_string id)
	{
		if (_decls.count(id))
			return _decls[id].first;
		for (auto i : _implements)
			return i->getDeclaration(id);
		return NULL;
	}

	Type *InterfaceDeclaration::getMemberType(unicode_string id)
	{
		if (_decls.count(id))
			return _decls[id].second;
		for (auto i : _implements)
			return i->getMemberType(id);
		return NULL;
	}

	Type * FunctionDeclaration::getReturnType()
	{
		return _decl->getType();
	}

	FunctionCall::FunctionCall(AST::FunctionCall *base, Expression *funcPtr, vector<Expression*> arguments) : _base(base)
	{
		setFunctionId(funcPtr);
		setArguments(arguments);
	}

	FunctionDeclaration::~FunctionDeclaration() { if (_base) delete _base; if (_decl) delete _decl; if (_content) delete _content; _parameters.clear(); }

	FunctionLiteral::~FunctionLiteral() { if (_base) delete _base; if (_content) delete _content; _parameters.clear(); }

	int Literal::getIntValue() 
	{
		if (getLiteralType() != LT_INTEGER)
			throw "literal is not an integer";
		return ((AST::Integer*)_base)->getValue();
	}

	void FunctionDeclaration::setVarDecl(VariableDeclaration *decl)
	{ 
		if (decl->getType()->isUnknownTy())
			throw ErrorReporter::report("function return types may not be inferred", ERR_DECORATOR, decl->getPosition());
		_decl = decl; 
	}

	void FunctionDeclaration::addParameter(VariableDeclaration *parameter)
	{ 
		if (parameter->getType()->isUnknownTy())
			throw ErrorReporter::report("function parameter types may not be inferred", ERR_DECORATOR, parameter->getPosition());
		_parameters.push_back(parameter); 
	}

	void FunctionDeclaration::addParameterToStart(VariableDeclaration *parameter)
	{
		if (parameter->getType()->isUnknownTy())
			throw ErrorReporter::report("function parameter types may not be inferred", ERR_DECORATOR, parameter->getPosition());
		_parameters.insert(_parameters.begin(), parameter); 
	}

	FunctionType *FunctionDeclaration::getFuncType()
	{
		vector<Type*> params;
		for (auto i : _parameters)
			params.push_back(i->getType());
		return FunctionType::get(getReturnType(), params);
	}

	void Program::addImport(string bcFileName) {

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

	void Program::addNamespace(NamespaceDeclaration * decl ) { _namespaces[decl->getName()] = decl; }
}

