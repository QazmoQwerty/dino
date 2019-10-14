#include "AstNode.h"

vector<AST::Node*> AST::StatementBlock::getChildren()
{
	vector<Node*> v;
	for (auto i : _statements)
		v.push_back(i);
	return v;
}

vector<AST::Node*> AST::IfThenElse::getChildren()
{
	vector<Node*> v;
	v.push_back(_condition);
	v.push_back(_thenBranch);
	v.push_back(_elseBranch);
	return v;
}

vector<AST::Node*> AST::WhileLoop::getChildren()
{
	vector<Node*> v;
	v.push_back(_condition);
	v.push_back(_statement);
	return v;
}

vector<AST::Node*> AST::Assignment::getChildren()
{
	vector<Node*> v;
	v.push_back(_left);
	v.push_back(_right);
	return v;
}

vector<AST::Node*> AST::Increment::getChildren()
{
	vector<Node*> v;
	v.push_back(_expression);
	return v;
}

vector<AST::Node*> AST::BinaryOperation::getChildren()
{
	vector<Node*> v;
	v.push_back(_left);
	v.push_back(_right);
	return v;
}

vector<AST::Node*> AST::UnaryOperation::getChildren()
{
	vector<Node*> v;
	v.push_back(_expression);
	return v;
}

vector<AST::Node*> AST::FunctionCall::getChildren()
{
	vector<Node*> v;
	v.push_back(_functionId);
	for (auto i : _parameters)
		v.push_back(i);
	return v;
}

vector<AST::Node*> AST::ConditionalExpression::getChildren()
{
	vector<Node*> v;
	v.push_back(_condition);
	v.push_back(_thenBranch);
	v.push_back(_elseBranch);
	return v;
}

vector<AST::Node*> AST::Function::getChildren()
{
	vector<Node*> v;
	v.push_back(_returnType);
	for (auto i : _parameters)
		v.push_back(i);
	v.push_back(_content);
	return v;
}

void AST::Function::addParameter(Node * parameter)
{
	if (!parameter->isExpression())
		throw DinoException("expected a variable declaration", EXT_GENERAL, parameter->getLine());
	auto exp = dynamic_cast<Expression*>(parameter);
	switch (exp->getExpressionType())
	{
	case(ET_VARIABLE_DECLARATION):
		_parameters.push_back(dynamic_cast<VariableDeclaration*>(exp));
		break;
	case(ET_LIST):
		for (auto i : dynamic_cast<ExpressionList*>(exp)->getExpressions())
		{
			if (i->getExpressionType() == ET_VARIABLE_DECLARATION)
				_parameters.push_back(dynamic_cast<VariableDeclaration*>(i));
			else throw DinoException("expected a variable declaration", EXT_GENERAL, i->getLine());
		}
		break;
	default:
		throw DinoException("expected a variable declaration", EXT_GENERAL, exp->getLine());
	}
}

string AST::InterfaceDeclaration::toString()
{
	string str = "<InterfaceDeclaration>\\n" + _name;
	for (unsigned int i = 0; i < _implements.size(); i++)
	{
		if (i == 0) str += " is ";
		str += _implements[i];
		if (i < _implements.size() - 1)
			str += ", ";
	}
	return str;
}

vector<AST::Node*> AST::InterfaceDeclaration::getChildren()
{
	vector<Node*> v;
	for (auto i : _properties)
		v.push_back(i);
	for (auto i : _functions)
		v.push_back(i);
	return v;
}

vector<AST::Node*> AST::UnaryOperationStatement::getChildren()
{
	vector<Node*> v;
	v.push_back(_expression);
	return v;
}

string AST::TypeDeclaration::toString()
{
	string str = "<TypeDeclaration>\\n" + _name;
	for (unsigned int i = 0; i < _interfaces.size(); i++)
	{
		if (i == 0) str += " is ";
		str += _interfaces[i];
		if (i < _interfaces.size() - 1)
			str += ", ";
	}
	return str;
}

vector<AST::Node*> AST::TypeDeclaration::getChildren()
{
	vector<Node*> v;
	for (auto i : _variableDeclarations)
		v.push_back(i);
	for (auto i : _functionDeclarations)
		v.push_back(i);
	for (auto i : _propertyDeclarations)
		v.push_back(i);
	return v;
}

AST::TypeDeclaration::TypeDeclaration() : Statement()
{
	_name = { "" };
	//_modifiers = vector<string>();
	_interfaces = vector<string>();
	_variableDeclarations = vector<VariableDeclaration*>();
	_functionDeclarations = vector<FunctionDeclaration*>();
}

AST::InterfaceDeclaration::InterfaceDeclaration()
{
	_name = { "" };
	//_modifiers = vector<string>();
}

vector<AST::Node*> AST::NamespaceDeclaration::getChildren()
{
	vector<Node*> v;
	v.push_back(_statement);
	return v;
}

vector<AST::Node*> AST::PropertyDeclaration::getChildren()
{
	vector<Node*> v;
	v.push_back(_decl);
	v.push_back(_get);
	v.push_back(_set);
	return v;
}

vector<AST::Node*> AST::VariableDeclaration::getChildren()
{
	vector<Node*> vec;
	vec.push_back(_type);
	return vec;
}

vector<AST::Node*> AST::FunctionDeclaration::getChildren()
{
	vector<Node*> v;
	v.push_back(_decl);
	for (auto i : _parameters)
		v.push_back(i);
	v.push_back(_content);
	return v;
}

void AST::FunctionDeclaration::addParameter(Node * parameter)
{
	if (!parameter->isExpression())
		throw DinoException("expected a variable declaration", EXT_GENERAL, parameter->getLine());
	auto exp = dynamic_cast<Expression*>(parameter);
	switch (exp->getExpressionType())
	{
	case(ET_VARIABLE_DECLARATION):
		_parameters.push_back(dynamic_cast<VariableDeclaration*>(exp));
		break;
	case(ET_LIST):
		for (auto i : dynamic_cast<ExpressionList*>(exp)->getExpressions())
		{
			if (i->getExpressionType() == ET_VARIABLE_DECLARATION)
				_parameters.push_back(dynamic_cast<VariableDeclaration*>(i));
			throw DinoException("expected a variable declaration", EXT_GENERAL, i->getLine());
		}
		break;
	default:
		throw DinoException("expected a variable declaration", EXT_GENERAL, exp->getLine());
	}
}

vector<AST::Node*> AST::ExpressionList::getChildren()
{
	vector<Node*> v;
	for (auto i : _expressions)
		v.push_back(i);
	return v;
}

void AST::ExpressionList::addExpression(Expression * expression)
{
	if (expression->getExpressionType() == ET_LIST)
		for (auto i : dynamic_cast<AST::ExpressionList*>(expression)->getExpressions())
			_expressions.push_back(i);
	else _expressions.push_back(expression);
}

vector<AST::Node*> AST::StatementList::getChildren()
{
	vector<Node*> v;
	for (auto i : _statements)
		v.push_back(i);
	return v;
}

void AST::StatementList::addStatement(Statement * statement)
{
	if (statement->getStatementType() == ST_LIST)
		for (auto i : dynamic_cast<AST::StatementList*>(statement)->getStatements())
			_statements.push_back(i);
	else _statements.push_back(statement);
}

//vector<AST::Node*> AST::ExpressionStatementList::getChildren()
//{
//	vector<Node*> v;
//	for (auto i : _expStatements)
//		v.push_back(i);
//	return v;
//}

vector<AST::Node*> AST::UnaryAssignment::getChildren()
{
	vector<Node*> v;
	v.push_back(_expression);
	return v;
}
