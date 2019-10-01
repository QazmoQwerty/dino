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

string AST::InterfaceDeclaration::toString()
{
	string str = "<InterfaceDeclaration>\\n" + _name.name;
	for (unsigned int i = 0; i < _implements.size(); i++)
	{
		if (i == 0) str += " is ";
		str += _implements[i].name;
		if (i < _implements.size() - 1)
			str += ", ";
	}
	return str;
}

vector<AST::Node*> AST::InterfaceDeclaration::getChildren()
{
	vector<Node*> v;
	for (auto i : _declarations)
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
	string str = "<TypeDeclaration>\\n" + _name.name;
	for (unsigned int i = 0; i < _interfaces.size(); i++)
	{
		if (i == 0) str += " is ";
		str += _interfaces[i].name;
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

AST::InterfaceDeclaration::InterfaceDeclaration()
{
	_name = { "" };
	_modifiers = vector<Identificator>();
	_implements = vector<Identificator>();
	_declarations = vector<VariableDeclaration*>();
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
