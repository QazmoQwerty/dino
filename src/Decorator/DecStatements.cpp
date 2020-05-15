/*
    Decoration of statements.
*/
#include "Decorator.h"

DST::Statement * Decorator::decorate(AST::Statement * node)
{
	if (node == nullptr)
		return NULL;
	switch (dynamic_cast<AST::Statement*>(node)->getStatementType())
	{
	case ST_FUNCTION_CALL:
	{
		auto n = decorate((AST::FunctionCall*)node);
		if (!n->isStatement())
			throw ErrorReporter::report("expected a statement", ERR_DECORATOR, node->getPosition());
		return dynamic_cast<DST::ExpressionStatement*>(n);
	}
	case ST_VARIABLE_DECLARATION: return decorate((AST::VariableDeclaration*)	  node);
	case ST_CONST_DECLARATION:    return decorate((AST::ConstDeclaration*)		  node);
	case ST_ASSIGNMENT:           return decorate((AST::Assignment*)			  node);
	case ST_STATEMENT_BLOCK:      return decorate((AST::StatementBlock*)		  node);
	case ST_IF_THEN_ELSE:         return decorate((AST::IfThenElse*)			  node);
	case ST_SWITCH:               return decorate((AST::SwitchCase*)			  node);
	case ST_FOR_LOOP:             return decorate((AST::ForLoop*)				  node);
	case ST_WHILE_LOOP:           return decorate((AST::WhileLoop*)				  node);
	case ST_UNARY_OPERATION:      return decorate((AST::UnaryOperationStatement*) node);
	case ST_DO_WHILE_LOOP:        return decorate((AST::DoWhileLoop*)			  node);
	case ST_INCREMENT:            return decorate((AST::Increment*)				  node);
	case ST_TRY_CATCH:            return decorate((AST::TryCatch*)				  node);
	default: throw ErrorReporter::report("Unimplemented statement type in the decorator", ERR_DECORATOR, node->getPosition());
	}
}

DST::UnaryOperationStatement * Decorator::decorate(AST::UnaryOperationStatement * node)
{
	if (node->getOperator()._type == OT_BREAK && _loopCount == 0)
		throw ErrorReporter::report("\"break\" used outside of loop", ERR_DECORATOR, node->getPosition());
	if (node->getOperator()._type == OT_CONTINUE && _loopCount == 0)
		throw ErrorReporter::report("\"continue\" used outside of loop", ERR_DECORATOR, node->getPosition());
	return new DST::UnaryOperationStatement(node, decorate(node->getExpression()));
}

DST::ConstDeclaration *Decorator::decorate(AST::ConstDeclaration * node)
{
	auto decl = new DST::ConstDeclaration(node);
	decl->setExpression(decorate(node->getExpression()));
	unicode_string name = node->getName();
	if (_variables[currentScope()].count(name))
		throw ErrorReporter::report("Identifier '" + name.to_string() + "' is already in use", ERR_DECORATOR, node->getPosition());
	_variables[currentScope()][name] = decl->getExpression();
	return decl;
}

DST::StatementBlock * Decorator::decorate(AST::StatementBlock * node)
{
	if (!node) return NULL;
	enterBlock();
	auto bl = new DST::StatementBlock();
	for (auto i : node->getStatements())
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
			if (!i->getType()->assignableTo(sc->getExpression()->getType()->getNonConstOf()))
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
	_variables[currentScope()][unicode_string("caught")] = new DST::Variable(unicode_string("caught"), DST::getErrorTy());
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

DST::VariableDeclaration *Decorator::decorate(AST::VariableDeclaration * node)
{
	auto decl = new DST::VariableDeclaration(node);
	unicode_string name = node->getVarId();
	decl->setType(evalType(node->getVarType()));
	if (_variables[currentScope()].count(name))
		throw ErrorReporter::report("Identifier '" + name.to_string() + "' is already in use", ERR_DECORATOR, node->getPosition());
	_variables[currentScope()][name] = new DST::Variable(name, decl->getType());
	return decl;
}

DST::Assignment * Decorator::decorate(AST::Assignment * node)
{
	auto right = decorate(node->getRight());
	auto assignment = new DST::Assignment(node, decorate(node->getLeft()), right);

	if (!assignment->getLeft()->getType()->writeable())
		throw ErrorReporter::report("lvalue is read-only", ERR_DECORATOR, node->getPosition());
	if (!assignment->getRight()->getType()->readable())
		throw ErrorReporter::report("rvalue is write-only", ERR_DECORATOR, node->getPosition());

	if (assignment->getLeft()->getExpressionType() == ET_LIST)
	{
		auto list = (DST::ExpressionList*)assignment->getLeft();
		auto leftTypes = ((DST::ExpressionList*)assignment->getLeft())->getType();
		auto rightTypes = ((DST::ExpressionList*)assignment->getRight())->getType();
		for (uint i = 0; i < list->size(); i++)
		{
			if (list->getExpressions()[i]->getType()->isUnknownTy())
			{
				if (list->getExpressions()[i]->getExpressionType() != ET_VARIABLE_DECLARATION)
					throw ErrorReporter::report("inferred type is invalid in this context", ERR_DECORATOR, node->getPosition());
				((DST::VariableDeclaration*)list->getExpressions()[i])->setType(rightTypes->getTypes()[i]);
				leftTypes->getTypes()[i] = rightTypes->getTypes()[i]->getNonPropertyOf()->getNonConstOf();
				auto name = ((DST::VariableDeclaration*)list->getExpressions()[i])->getVarId();
				_variables[currentScope()][name] = new DST::Variable(name, rightTypes->getTypes()[i]);
			}
		}
	}
	
	if (assignment->getLeft()->getType()->isUnknownTy())
	{
		if (assignment->getLeft()->getExpressionType() != ET_VARIABLE_DECLARATION)
			throw ErrorReporter::report("inferred type is invalid in this context", ERR_DECORATOR, node->getPosition());
		switch (assignment->getRight()->getType()->getNonConstOf()->getNonPropertyOf()->getExactType())
		{
			case EXACT_UNKNOWN: case EXACT_NULL:
				throw ErrorReporter::report("Could not infer type", ERR_DECORATOR, assignment->getLeft()->getPosition());
			default:
				((DST::VariableDeclaration*)assignment->getLeft())->setType(
					assignment->getRight()->getType()->getNonPropertyOf()->getNonConstOf()
				); break;
		}
		auto name = ((DST::VariableDeclaration*)assignment->getLeft())->getVarId();
		_variables[currentScope()][name] = new DST::Variable(name, assignment->getLeft()->getType());
	}

	if (!assignment->getRight()->getType()->assignableTo(assignment->getLeft()->getType()))
		throw ErrorReporter::report("Type \"" + assignment->getRight()->getType()->toShortString() + "\" is not assignable to type \""
			  						+ assignment->getLeft()->getType()->toShortString() + "\"", ERR_DECORATOR, node->getPosition());

	if (assignment->getRight()->getType()->isNullTy())
		assignment->setRight(new DST::Conversion(NULL, assignment->getLeft()->getType(), assignment->getRight()));

	assignment->setType(assignment->getLeft()->getType());
	return assignment;
}