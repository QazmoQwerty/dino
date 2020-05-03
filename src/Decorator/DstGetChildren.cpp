/*
    Implementations for all the required getChildren() functions of all DST classes.
    These functions are used for the outputting of graphviz representations of the DST.
    This file is a bunch of boring boilerplate code which you have no reason to read :).

    TODO - (not important) clean up this code by making use of initializer lists.
*/
#include "DstNode.h"

vector<DST::Node*> DST::Variable::getChildren()
{
	vector<Node*> v;
	return v;
}

vector<DST::Node*> DST::BinaryOperation::getChildren()
{
	vector<Node*> v;
	v.push_back(_left);
	v.push_back(_right);
	return v;
}

vector<DST::Node*> DST::UnaryOperation::getChildren()
{
	vector<Node*> v;
	v.push_back(_expression);
	return v;
}

vector<DST::Node*> DST::ConditionalExpression::getChildren()
{
	vector<Node*> v;
	v.push_back(_condition);
	v.push_back(_thenBranch);
	v.push_back(_elseBranch);
	return v;
}

vector<DST::Node*> DST::MemberAccess::getChildren()
{
	vector<Node*> v;
	v.push_back(_left);
	return v;
}

vector<DST::Node*> DST::Literal::getChildren()
{
	vector<Node*> v;
	return v;
}

vector<DST::Node*> DST::ExpressionList::getChildren()
{
	vector<Node*> v;
	for (auto i : _expressions)
		v.push_back(i);
	return v;
}

vector<DST::Node*> DST::ArrayLiteral::getChildren()
{
	vector<Node*> v;
	for (auto i : _array)
		v.push_back(i);
	return v;
}

vector<DST::Node*> DST::StatementBlock::getChildren()
{
	vector<Node*> v;
	for (auto i : _statements)
		v.push_back(i);
	return v;
}

vector<DST::Node*> DST::TypeDeclaration::getChildren()
{
	vector<Node*> v;
	for (auto i : _decls)
		v.push_back(i.second.first);
	return v;
}

vector<DST::Node*> DST::NamespaceDeclaration::getChildren()
{
	vector<Node*> v;
	for (auto i : _decls)
		v.push_back(i.second.first);
	return v;
}

vector<DST::Node*> DST::InterfaceDeclaration::getChildren()
{
	vector<Node*> v;
	for (auto i : _decls)
		v.push_back(i.second.first);
	return v;
}

vector<DST::Node*> DST::IfThenElse::getChildren()
{
	vector<Node*> v;
	v.push_back(_condition);
	v.push_back(_thenBranch);
	v.push_back(_elseBranch);
	return v;
}

vector<DST::Node*> DST::SwitchCase::getChildren()
{
	vector<Node*> v;
	v.push_back(_expression);
	v.push_back(_default);
	for (CaseClause i : _cases)
	{
		for (auto e : i._expressions)
		v.push_back(e);
		v.push_back(i._statement);
	}
	return v;
}

vector<DST::Node*> DST::WhileLoop::getChildren()
{
	vector<Node*> v;
	v.push_back(_condition);
	v.push_back(_statement);
	return v;
}

vector<DST::Node*> DST::ForLoop::getChildren()
{
	vector<Node*> v;
	v.push_back(_begin);
	v.push_back(_condition);
	v.push_back(_increment);
	v.push_back(_statement);
	return v;
}

vector<DST::Node*> DST::UnaryOperationStatement::getChildren()
{
	vector<Node*> v;
	v.push_back(_expression);
	return v;
}

vector<DST::Node*> DST::ConstDeclaration::getChildren()
{
	return { _expression };
}

vector<DST::Node*> DST::VariableDeclaration::getChildren()
{
	vector<Node*> v;
	return v;
}

vector<DST::Node*> DST::Assignment::getChildren()
{
	vector<Node*> v;
	v.push_back(_left);
	v.push_back(_right);
	return v;
}

vector<DST::Node*> DST::FunctionDeclaration::getChildren()
{
	vector<Node*> v;
	v.push_back(_decl);
	for (auto i : _parameters)
		v.push_back(i);
	v.push_back(_content);
	return v;
}

vector<DST::Node*> DST::PropertyDeclaration::getChildren()
{
	vector<Node*> v;
	v.push_back(_get);
	v.push_back(_set);
	return v;
}

vector<DST::Node*> DST::FunctionCall::getChildren()
{
	vector<Node*> v;
	v.push_back(_funcPtr);
	for (auto i : _arguments)
		v.push_back(i);
	return v;
}

vector<DST::Node*> DST::FunctionLiteral::getChildren()
{
	vector<Node*> v;
	for (auto i : _parameters)
		v.push_back(i);
	v.push_back(_content);
	return v;
}

vector<DST::Node*> DST::Conversion::getChildren()
{
	vector<Node*> v;
	v.push_back(_expression);
	return v;
}

vector<DST::Node*> DST::Program::getChildren()
{
	vector <Node*> v;
	for (auto i : _namespaces)
		v.push_back(i.second);
	return v;
}

vector<DST::Node*> DST::Type::getChildren()
{
	return vector<Node*>();
}

vector<DST::Node*> DST::PointerType::getChildren()
{
	vector<Node*> v;
	v.push_back(_type);
	return v;
}

vector<DST::Node*> DST::FunctionType::getChildren()
{
	return vector<Node*>();
}

vector<DST::Node*> DST::TypeList::getChildren()
{
	return vector<Node*>();
}

vector<DST::Node*> DST::ArrayType::getChildren()
{
	return vector<Node*>();
}