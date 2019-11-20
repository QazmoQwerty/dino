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
	//v.push_back(_type);
	return v;
}

vector<DST::Node*> DST::BinaryOperation::getChildren()
{
	vector<Node*> v;
	//v.push_back(_type);
	v.push_back(_left);
	v.push_back(_right);
	return v;
}

vector<DST::Node*> DST::Literal::getChildren()
{
	vector<Node*> v;
	//v.push_back(_type);
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

vector<DST::Node*> DST::VariableDeclaration::getChildren()
{
	vector<Node*> v;
	//v.push_back(_type);
	return v;
}

vector<DST::Node*> DST::Assignment::getChildren()
{
	vector<Node*> v;
	//v.push_back(_type);
	v.push_back(_left);
	v.push_back(_right);
	return v;
}