#include "DstNode.h"

vector<DST::Node*> DST::Type::getChildren()
{
	return vector<Node*>();
}

vector<DST::Node*> DST::BasicType::getChildren()
{
	return vector<Node*>();
}

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

vector<DST::Node*> DST::StatementBlock::getChildren()
{
	vector<Node*> v;
	for (auto i : _statements)
		v.push_back(i);
	return v;
}

vector<DST::Node*> DST::TypeDeclaration::getChildren()
{
	return vector<Node*>();
}

vector<DST::Node*> DST::IfThenElse::getChildren()
{
	vector<Node*> v;
	v.push_back(_condition);
	v.push_back(_thenBranch);
	v.push_back(_elseBranch);
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

vector<DST::Node*> DST::FunctionCall::getChildren()
{
	vector<Node*> v;
	v.push_back(_funcPtr);
	v.push_back(_arguments);
	return v;
}

bool DST::FunctionType::equals(Type * other)
{
	if (other->getExactType() != EXACT_FUNCTION)
		return false;
	auto othr = (FunctionType*)other;
	if (_returns.size() != othr->_returns.size() || _parameters.size() != othr->_parameters.size())
		return false;
	for (int i = 0; i < _returns.size(); i++)
		if (_returns[i]->equals(othr->_returns[i]))
			return false;
	for (int i = 0; i < _parameters.size(); i++)
		if (_parameters[i]->equals(othr->_parameters[i]))
			return false;
	return true;
}

string DST::FunctionType::toShortString()
{
	string str = "";
	for (int i = 0; i < _returns.size(); i++)
	{
		if (i > 0)
			str += ", ";
		str += _returns[i]->toShortString();
	}
	str += "(";
	for (int i = 0; i < _parameters.size(); i++)
	{
		if (i > 0)
			str += ", ";
		str += _parameters[i]->toShortString();
	}
	str += ")";
	return str;
}

vector<DST::Node*> DST::FunctionType::getChildren()
{
	return vector<Node*>();
}
