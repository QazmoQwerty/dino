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
		case ET_IDENTIFIER: 			return decorate((AST::Identifier*)node);
		case ET_BINARY_OPERATION: 		return decorate((AST::BinaryOperation*)node);
		case ET_LITERAL: 				return decorate((AST::Literal*)node);
		case ET_UNARY_OPERATION:  		return decorate((AST::UnaryOperation*)node);
		case ET_VARIABLE_DECLARATION: 	return decorate((AST::VariableDeclaration*)node);
		case ET_ASSIGNMENT: 			return decorate((AST::Assignment*)node);
		case ET_LIST: 					return decorate((AST::ExpressionList*)node);
		case ET_FUNCTION_CALL: 			return decorate((AST::FunctionCall*)node);
		case ET_CONDITIONAL_EXPRESSION: return decorate((AST::ConditionalExpression*)node);
		case ET_INCREMENT: 				return decorate((AST::Increment*)node);
		default: throw ErrorReporter::report("Unimplemented expression type in the decorator", ERR_DECORATOR, node->getPosition());
	}
}

DST::Expression *Decorator::decorate(AST::Identifier * node)
{
	unicode_string &name = node->getVarId();

	if (name == unicode_string("var"))
		return DST::UnknownType::get();

	for (int scope = currentScope(); scope >= 0; scope--)
		if (auto ty = _variables[scope][name]) {
			if (ty->isSpecifierTy())
				return ((DST::TypeSpecifierType*)ty)->getBasicTy();
			return new DST::Variable(node, ty);
		}

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
	{
		if (auto ty = _currentNamespace[i]->getMemberType(name)) {
			if (ty->isSpecifierTy())
				return ((DST::TypeSpecifierType*)ty)->getBasicTy();
			return new DST::Variable(node, ty);
		}
	}

	if (_currentProgram) 
		if (auto var = _currentProgram->getNamespace(name))
			return new DST::Variable(node, DST::getNamespaceTy(var));

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
	vector<DST::Expression*> arguments;
	auto args = decorate(node->getArguments());
	if (args && args->getExpressionType() == ET_TYPE)
	{
		vector<DST::Type*> vec;
		if (((DST::Type*)args)->isListTy())
			for (auto i : ((DST::TypeList*)args)->getTypes())
				vec.push_back(i);
		else vec.push_back(((DST::Type*)args));

		if (funcId->getExpressionType() != ET_TYPE)
			throw ErrorReporter::report("expected a type", ERR_DECORATOR, funcId->getPosition());
			
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
		throw ErrorReporter::report("invalid function arguments", ERR_DECORATOR, node->getPosition());
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
			throw "unreachable, leaving for now just to make sure.";

		if (type->isPropertyTy() && ((DST::PropertyType*)type)->hasGet())
			type = ((DST::PropertyType*)type)->getReturn();

		if (type->isPtrTy())
			type = ((DST::PointerType*)type)->getPtrType();
			
		DST::Type *memberType = NULL;
		unicode_string varId = ((AST::Identifier*)node->getRight())->getVarId();
		switch (type->getExactType())
		{
			case EXACT_BASIC:
				if (_currentTypeDecl != ((DST::BasicType*)type)->getTypeSpecifier()->getTypeDecl() && !varId[0].isUpper())
					throw ErrorReporter::report("Cannot access private member \"" + varId.to_string() + "\"", ERR_DECORATOR, node->getPosition());
				memberType = ((DST::BasicType*)type)->getTypeSpecifier()->getMemberType(varId);	
				break;
			case EXACT_NAMESPACE:
				memberType = ((DST::NamespaceType*)type)->getNamespaceDecl()->getMemberType(varId);
				break;
			case EXACT_ARRAY:
				if (varId == unicode_string("Size"))
				{
					memberType = DST::getIntTy()->getPropertyOf(true, false);
					break;
				}
			default: throw ErrorReporter::report("Expression must have class or namespace type", ERR_DECORATOR, left->getPosition());
		}

		if (memberType == nullptr)
			throw ErrorReporter::report("Unkown identifier \"" + varId.to_string() + "\"", ERR_DECORATOR, node->getRight()->getPosition());
		
		if (memberType->isSpecifierTy())	// TODO - why is this line here??
			return ((DST::TypeSpecifierType*)memberType)->getBasicTy();

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
		case RT_BOOLEAN: 
			bo->setType(DST::getBoolTy());
			break;
		case RT_ARRAY:
			if (bo->getLeft()->getType()->isArrayTy())
			{
				// array access
				if (bo->getRight()->getType() != DST::getIntTy() || (bo->getRight()->getType()->isConstTy() && bo->getRight()->getType() == DST::getIntTy()->getConstOf()))
					throw ErrorReporter::report("array index must be an integer value", ERR_DECORATOR, bo->getRight()->getPosition());
				bo->setType(((DST::ArrayType*)bo->getLeft()->getType())->getElementType());
			}
			else if (bo->getLeft()->getType()->isListTy()) 
			{
				// aggregate type access. 
				if (bo->getRight()->getExpressionType() != ET_LITERAL || ((DST::Literal*)bo->getRight())->getLiteralType() != LT_INTEGER)
					throw ErrorReporter::report("aggregate index must be a constant integer value", ERR_DECORATOR, bo->getRight()->getPosition());
				int idx = ((DST::Literal*)bo->getRight())->getIntValue();
				auto list = ((DST::TypeList*)bo->getLeft()->getType());
				if (idx < 0 || size_t(idx) >= list->size())
					throw ErrorReporter::report("aggregate index [" + std::to_string(idx) + "] out of range [" + 
							std::to_string(list->size()) + "]", ERR_DECORATOR, bo->getRight()->getPosition());
				bo->setType(list->getTypes()[idx]);
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
		case (LT_BOOLEAN):		lit->setType(DST::getBoolTy());		break;
		case (LT_CHARACTER):	lit->setType(DST::getCharTy());		break;
		case (LT_FRACTION):		lit->setType(DST::getFloatTy());	break;
		case (LT_INTEGER):		lit->setType(DST::getIntTy());		break;
		case (LT_STRING):		lit->setType(DST::getStringTy()); 	break;
		case (LT_NULL):			lit->setType(DST::getNullTy());	break;
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