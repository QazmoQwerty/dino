/*
    Decoration of Expressions.
*/
#include "Decorator.h"

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
	{
		auto exp = decorate(node->getLeft());
		auto ty = evalType(node->getRight());
		if (exp->getType()->getExactType() == EXACT_BASIC && ((DST::BasicType*)exp->getType())->getTypeSpecifier()->getInterfaceDecl() &&
			ty->getExactType() == EXACT_BASIC && ((DST::BasicType*)ty)->getTypeSpecifier()->getTypeDecl())
		{
			auto conv = new DST::Conversion(NULL, new DST::PointerType(ty), exp);
			auto op = new AST::UnaryOperation();
			op->setPosition(node->getPosition());
			op->setOperator(OperatorsMap::getOperatorByDefinition(OT_AT).second);
			return new DST::UnaryOperation(op, conv);
		}
		return new DST::Conversion(NULL, evalType(node->getRight()), decorate(node->getLeft()));
	}

	auto left = decorate(node->getLeft());
	auto right = decorate(node->getRight());

	if (left->getExpressionType() == ET_TYPE)
	{
		if (right)
		{
			if (!(right->getExpressionType() == ET_LITERAL && ((DST::Literal*)right)->getLiteralType() == LT_INTEGER))
				return new DST::ArrayType((DST::Type*)left, right);	
				// throw ErrorReporter::report("array size must be a literal integer", ERR_DECORATOR, right->getPosition());
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
				if (!bo->getRight()->getType()->equals(intType))
					throw ErrorReporter::report("array index must be an integer value", ERR_DECORATOR, bo->getRight()->getPosition());
				bo->setType(((DST::ArrayType*)bo->getLeft()->getType())->getElementType());
				_toDelete.push_back(intType);
			}
			else throw ErrorReporter::report("type \"" + bo->getLeft()->getType()->toShortString() 
								+ "\" is not an array", ERR_DECORATOR, bo->getLeft()->getPosition());
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