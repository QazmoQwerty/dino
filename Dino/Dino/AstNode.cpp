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

vector<AST::Node*> AST::DoWhileLoop::getChildren()
{
	vector<Node*> v;
	v.push_back(_condition);
	v.push_back(_statement);
	return v;
}

vector<AST::Node*> AST::Assignment::getChildren()
{
	vector<Node*> v;
	v.push_back(_right);
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
