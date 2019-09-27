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
	for (auto i : _parameters)
		v.push_back(i);
	v.push_back(_content);
	return v;
}

vector<AST::Node*> AST::UnaryOperationStatement::getChildren()
{
	vector<Node*> v;
	v.push_back(_expression);
	return v;
}

vector<AST::Node*> AST::TypeDeclaration::getChildren()
{
	vector<Node*> v;
	for (auto i : _variableDeclarations)
		v.push_back(i);
	for (auto i : _functionDeclarations)
		v.push_back(i);
	return v;
}

AST::TypeDeclaration::TypeDeclaration() : Statement()
{
	_name = { "" };
	_modifiers = vector<Identificator>();
	_interfaces = vector<Identificator>();
	_variableDeclarations = vector<VariableDeclaration*>();
	_functionDeclarations = vector<Assignment*>();
}
