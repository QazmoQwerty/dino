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
		case ET_IDENTIFIER: 			return decorate((AST::Identifier*)			 node);
		case ET_BINARY_OPERATION: 		return decorate((AST::BinaryOperation*)		 node);
		case ET_LITERAL: 				return decorate((AST::Literal*)				 node);
		case ET_UNARY_OPERATION:  		return decorate((AST::UnaryOperation*)		 node);
		case ET_VARIABLE_DECLARATION: 	return decorate((AST::VariableDeclaration*)	 node);
		case ET_ASSIGNMENT: 			return decorate((AST::Assignment*)			 node);
		case ET_LIST: 					return decorate((AST::ExpressionList*)		 node);
		case ET_FUNCTION_CALL: 			return decorate((AST::FunctionCall*)		 node);
		case ET_CONDITIONAL_EXPRESSION: return decorate((AST::ConditionalExpression*)node);
		case ET_INCREMENT: 				return decorate((AST::Increment*)			 node);
		case ET_COMPARISON: 			return decorate((AST::Comparison*)			 node);
		default: UNREACHABLE
	}
}

DST::Expression *Decorator::decorate(AST::Identifier * node)
{
	unicode_string &name = node->getVarId();

	if (name == unicode_string("var"))
		return DST::UnknownType::get();

	for (int scope = currentScope(); scope >= 0; scope--)
		if (auto exp = _variables[scope][name])
			return exp;

	if (_currentTypeDecl) if (auto ty = _currentTypeDecl->getMemberType(name))
	{
		auto thisId = new AST::Identifier(unicode_string("this"));
		auto bop = new AST::BinaryOperation();
		bop->setLeft(thisId);
		bop->setRight(node);
		bop->setOperator(OperatorsMap::getOperatorByDefinition(OT_PERIOD).second);
		auto acc = new DST::MemberAccess(bop, decorate(thisId));
		acc->setType(ty);
		return acc;
	}

	for (int i = _currentNamespace.size() - 1; i >= 0; i--)
		if (auto member = _currentNamespace[i]->getMemberExp(name))
			return member;

	if (_currentProgram) 
		if (auto var = _currentProgram->getNamespace(name))
			return new DST::Variable(node, DST::NamespaceType::get(var), var);

	
	throw ErrorReporter::report("Identifier '" + name.to_string() + "' is undefined", ErrorReporter::GENERAL_ERROR, node->getPosition());
}

DST::Comparison *Decorator::decorate(AST::Comparison *node)
{
	auto comp = new DST::Comparison(node);
	for (auto i : node->getExpressions())
		comp->addExpression(decorate(i));
	DST::Type *ty = NULL;
	for (auto i : comp->getExpressions())
		if (!ty)
			ty = i->getType();
		else if (!i->getType()->comparableTo(ty))
			throw ErrorReporter::report("cannot compare types \"" + ty->toShortString() 
				+ "\" and \"" + i->getType()->toShortString() + "\"", ErrorReporter::GENERAL_ERROR, i->getPosition());
	return comp;
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
			throw ErrorReporter::report("array literal can't be empty", ErrorReporter::GENERAL_ERROR, node->getPosition());
		else if (val->getExpressionType() == ET_LIST)
		{
			DST::Type *prevType = NULL;
			for (auto val : ((DST::ExpressionList*)val)->getExpressions())
			{
				if (prevType && !val->getType()->equals(prevType))
					throw ErrorReporter::report("Array Literal must have only one type", ErrorReporter::GENERAL_ERROR, node->getPosition());
					prevType = val->getType();
			}
			if (!prevType)
				throw ErrorReporter::report("array literal can't be empty", ErrorReporter::GENERAL_ERROR, node->getPosition());
			return new DST::ArrayLiteral(prevType->getConstOf(), ((DST::ExpressionList*)val)->getExpressions());
		}
		else
		{
			vector<DST::Expression*> v;
			v.push_back(val);
			return new DST::ArrayLiteral(val->getType(), v);
		}
	}
	
	if (node->getOperator()._type == OT_AT && val->getExpressionType() == ET_TYPE)	// Pointer Type
		return ((DST::Type*)val)->getPtrTo();

	return new DST::UnaryOperation(node, val);
}

DST::Expression * Decorator::decorate(AST::ConditionalExpression * node)
{
	auto expr = new DST::ConditionalExpression(node);
	expr->setCondition(decorate(node->getCondition()));
	expr->setThenBranch(decorate(node->getThenBranch()));
	expr->setElseBranch(decorate(node->getElseBranch()));

	if (!expr->getCondition())
		throw ErrorReporter::report("Expected a condition", ErrorReporter::GENERAL_ERROR, node->getPosition());
	if (!isCondition(expr->getCondition()))
		throw ErrorReporter::report("Condition must be of bool type", ErrorReporter::GENERAL_ERROR, node->getPosition());
	if (!expr->getThenBranch())
		throw ErrorReporter::report("Expected then branch of conditional expression", ErrorReporter::GENERAL_ERROR, node->getPosition());
	if (!expr->getElseBranch())
		throw ErrorReporter::report("Expected else branch of conditional expression", ErrorReporter::GENERAL_ERROR, node->getPosition());
	if (!expr->getThenBranch()->getType()->equals(expr->getElseBranch()->getType()))
		throw ErrorReporter::report("Operand types are incompatible (\"" + expr->getThenBranch()->getType()->toShortString() + 
							"\" and \"" + expr->getElseBranch()->getType()->toShortString() + "\")", ErrorReporter::GENERAL_ERROR, node->getPosition());
	return expr;
}

DST::Expression * Decorator::decorate(AST::FunctionCall * node)
{
	auto funcId = decorate(node->getFunctionId());
	vector<DST::Expression*> arguments;
	auto args = decorate(node->getArguments());
	if (args && args->getExpressionType() == ET_TYPE)
	{
		vector<DST::Type*> vec;
		if (((DST::Type*)args)->isListTy())
			for (auto i : ((DST::Type*)args)->as<DST::TypeList>()->getTypes())
				vec.push_back(i);
		else vec.push_back(((DST::Type*)args));

		if (funcId->getExpressionType() != ET_TYPE)
			throw ErrorReporter::report("expected a type", ErrorReporter::GENERAL_ERROR, funcId->getPosition());
			
		return DST::FunctionType::get((DST::Type*)funcId, vec);
	}
	else if (args && args->getExpressionType() == ET_LIST)
		for (auto i : ((DST::ExpressionList*)args)->getExpressions())
			arguments.push_back(i);
	else if (args) arguments.push_back(args);

	if (funcId->getExpressionType() == ET_TYPE)	// function type OR conversion
	{
		bool areAllTypes = true;
		for (auto i : arguments)
			if (i->getExpressionType() != ET_TYPE)
				areAllTypes = false;

		if (areAllTypes)
		{
			// function type
			vector<DST::Type*> vec;
			for (auto i : arguments)
				vec.push_back((DST::Type*)i);
			return DST::FunctionType::get((DST::Type*)funcId, vec);
		}
		if (arguments.size() == 1) // conversion
			return new DST::Conversion(node, (DST::Type*)funcId, arguments[0]);
		throw ErrorReporter::report("invalid function arguments", ErrorReporter::GENERAL_ERROR, node->getPosition());
	}

	auto call = new DST::FunctionCall(node, funcId, arguments);
	return call;
}

DST::FunctionLiteral * Decorator::decorate(AST::Function * node)
{
	enterBlock();
	auto lit = new DST::FunctionLiteral(node);
	DST::Type* ret = NULL;
	vector<DST::Type*> params;
	if (node->getReturnType() == NULL)
		ret = DST::getVoidTy();
	else ret = evalType(node->getReturnType());
	for (auto i : node->getParameters())
	{
		auto param = decorate(i);
		params.push_back(param->getType());
		lit->addParameter(param);
	}
	lit->setContent(decorate(node->getContent()));
	lit->setType(DST::FunctionType::get(ret, params));
	if (lit->getContent() && !lit->getContent()->hasReturnType(lit->getType()->getReturn()))
		throw ErrorReporter::report("Not all control paths lead to a return value.", ErrorReporter::GENERAL_ERROR, node->getPosition());
	leaveBlock();
	return lit;
}

DST::Expression * Decorator::decorate(AST::BinaryOperation * node)
{
	if (node->getOperator()._type == OT_PERIOD) 
	{
		auto left = decorate(node->getLeft());
		if (node->getRight()->getExpressionType() != ET_IDENTIFIER)
			throw ErrorReporter::report("Expected an identifier", ErrorReporter::GENERAL_ERROR, node->getPosition());

		if (left->getExpressionType() == ET_TYPE)
		{
			if (!((DST::Type*)left)->isEnumTy())
				throw ErrorReporter::report("dino does not have static type members", ErrorReporter::GENERAL_ERROR, node->getLeft()->getPosition());
			auto enumTy = ((DST::Type*)left)->as<DST::EnumType>();
			auto ident = ((AST::Identifier*)node->getRight());
			if (!enumTy->getEnumDecl()->hasMember(ident->getVarId()))
				throw ErrorReporter::report("\"" + enumTy->toShortString() + "\" has no member named \"" 
					+ ident->getVarId().to_string() + "\"", ErrorReporter::GENERAL_ERROR, node->getRight()->getPosition());
			return enumTy->getEnumDecl()->getEnumLiteral(ident->getVarId(), ident->getPosition());
		}

		auto type = left->getType();

		if (type->isPropertyTy() && type->as<DST::PropertyType>()->hasGet())
			type = type->as<DST::PropertyType>()->getReturn();

		if (type->isPtrTy())
			type = type->as<DST::PointerType>()->getPtrType();
			
		DST::Type *memberType = NULL;
		unicode_string varId = ((AST::Identifier*)node->getRight())->getVarId();
		if (type->isBasicTy())
		{
			auto bt = type->as<DST::BasicType>();
			if (!(bt->isValueTy() && _currentTypeDecl == bt->getTypeDecl()) && !varId[0].isUpper())
				throw ErrorReporter::report("Cannot access private member \"" + varId.to_string() + "\"", ErrorReporter::GENERAL_ERROR, node->getPosition());
			memberType = bt->getMember(varId);	
		}
		else if (type->isArrayTy() && varId == unicode_string("Size"))
			memberType = DST::getIntTy()->getPropertyOf(true, false);
		else if (type->isNamespaceTy())
		{
			auto member = type->as<DST::NamespaceType>()->getNamespaceDecl()->getMemberExp(varId);
			if (member->getExpressionType() == ET_TYPE)
				return member;
			memberType = member->getType();
		}
		else 
		{
			std::cout << memberType->toShortString() << "\n";
			throw ErrorReporter::report("Expression must have class or namespace type", ErrorReporter::GENERAL_ERROR, left->getPosition());
		}

		if (memberType == nullptr)
			throw ErrorReporter::report("Unkown identifier \"" + varId.to_string() + "\"", ErrorReporter::GENERAL_ERROR, node->getRight()->getPosition());

		auto access = new DST::MemberAccess(node, left);
		access->setType(memberType);
		return access;
	}

	if (node->getOperator()._type == OT_AS)
	{
		auto exp = decorate(node->getLeft());
		auto ty = evalType(node->getRight());
		if (exp->getType()->isInterfaceTy() && ty->isValueTy())
		{
			auto conv = new DST::Conversion(NULL, ty->getPtrTo(), exp);
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
			if (right->getExpressionType() != ET_LITERAL || ((DST::Literal*)right)->getLiteralType() != LT_INTEGER)
				return ((DST::Type*)left)->getArrayOf(right);
			return ((DST::Type*)left)->getArrayOf(((DST::Literal*)right)->getIntValue());
		}
		else return ((DST::Type*)left)->getArrayOf();
	}

	auto bo = new DST::BinaryOperation(node, left, right);

	switch (OperatorsMap::getReturnType(node->getOperator()._type))
	{
		default: 
			if (!bo->getLeft()->getType()->equals(bo->getRight()->getType()))
				throw ErrorReporter::report(
					"incompatible types in operation", "incompatible types", 
					ErrorReporter::GENERAL_ERROR, node->getPosition()
				).withSecondary(
					"left is `" + bo->getLeft()->getType()->toShortString() + "`",
					node->getLeft()->getPosition()
				).withSecondary(
					"right is `" + bo->getRight()->getType()->toShortString() + "`",
					node->getRight()->getPosition()
				);
			bo->setType(bo->getLeft()->getType());
			break;
		case RT_BOOLEAN: 
			switch (node->getOperator()._type)
			{
				case OT_IS: case OT_IS_NOT:
					if (bo->getRight()->getExpressionType() != ET_TYPE)
						throw ErrorReporter::report("expected a type", ErrorReporter::GENERAL_ERROR, node->getRight()->getPosition());
					break;
				default:
					if (!bo->getLeft()->getType()->equals(bo->getRight()->getType()))
						throw ErrorReporter::report(
							"incompatible types in operation", "incompatible types", 
							ErrorReporter::GENERAL_ERROR, node->getPosition()
						).withSecondary(
							"left is `" + bo->getLeft()->getType()->toShortString() + "`",
							node->getLeft()->getPosition()
						).withSecondary(
							"right is `" + bo->getRight()->getType()->toShortString() + "`",
							node->getRight()->getPosition()
						);
					break;
			}			
			bo->setType(DST::getBoolTy());
			break;
		case RT_ARRAY:
			if (bo->getLeft()->getType()->isArrayTy())
			{
				// array access
				if (bo->getRight()->getType() != DST::getIntTy() || (bo->getRight()->getType()->isConstTy() && bo->getRight()->getType() == DST::getIntTy()->getConstOf()))
					throw ErrorReporter::report("array index must be an integer value", ErrorReporter::GENERAL_ERROR, bo->getRight()->getPosition());
				bo->setType(bo->getLeft()->getType()->as<DST::ArrayType>()->getElementType());
			}
			else if (bo->getLeft()->getType()->isListTy()) 
			{
				// aggregate type access. 
				if (bo->getRight()->getExpressionType() != ET_LITERAL || ((DST::Literal*)bo->getRight())->getLiteralType() != LT_INTEGER)
					throw ErrorReporter::report("aggregate index must be a constant integer value", ErrorReporter::GENERAL_ERROR, bo->getRight()->getPosition());
				int idx = ((DST::Literal*)bo->getRight())->getIntValue();
				auto list = bo->getLeft()->getType()->as<DST::TypeList>();
				if (idx < 0 || size_t(idx) >= list->size())
					throw ErrorReporter::report("aggregate index [" + std::to_string(idx) + "] out of range [" + 
							std::to_string(list->size()) + "]", ErrorReporter::GENERAL_ERROR, bo->getRight()->getPosition());
				bo->setType(list->getTypes()[idx]);
			}
			else throw ErrorReporter::report("type \"" + bo->getLeft()->getType()->toShortString() 
								+ "\" is not an array", ErrorReporter::GENERAL_ERROR, bo->getLeft()->getPosition());
			break;
		case RT_VOID: 
			UNREACHABLE
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
		case (LT_BOOLEAN):		lit->setType(DST::getBoolTy());		break;
		case (LT_CHARACTER):	lit->setType(DST::getCharTy());		break;
		case (LT_FRACTION):		lit->setType(DST::getFloatTy());	break;
		case (LT_INTEGER):		lit->setType(DST::getIntTy());		break;
		case (LT_STRING):		lit->setType(DST::getStringTy()); 	break;
		case (LT_NULL):			lit->setType(DST::getNullTy());		break;
		default: break;
	}
	return lit;
}

DST::Expression * Decorator::decorate(AST::ExpressionList * node)
{
	vector<DST::Expression*> vec;

	if (node->getExpressions().size() == 0)
		return NULL;

	if (node->getExpressions().size() == 1)
		return decorate(node->getExpressions()[0]);

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
		if (vec.size() == 1)
			return vec[0];
		vector<DST::Type*> types;
		for (auto i : vec)
			types.push_back((DST::Type*)i);
		return DST::TypeList::get(types);
	}

	return new DST::ExpressionList(node, vec);
}