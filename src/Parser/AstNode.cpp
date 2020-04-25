#include "AstNode.h"
#include "../Decorator/DstNode.h"

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
	//v.push_back(nullptr);
	if (_elseBranch == nullptr)
		v.push_back(NULL);
	else v.push_back(_elseBranch);
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
	v.push_back(_arguments);
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

void AST::Function::addParameters(Expression * parameters)
{
	if (parameters == nullptr)
		return;
	if (!parameters->isExpression())
		throw ErrorReporter::report("expected a variable declaration", ERR_PARSER, parameters->getPosition());
	auto exp = dynamic_cast<Expression*>(parameters);
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
			else throw ErrorReporter::report("expected a variable declaration", ERR_PARSER, i->getPosition());
		}
		break;
	default:
		throw ErrorReporter::report("expected a variable declaration", ERR_PARSER, exp->getPosition());
		break;
	}
}

AST::InterfaceDeclaration::~InterfaceDeclaration() 
{
	delete _implements; _properties.clear(); _functions.clear(); 
}

string AST::InterfaceDeclaration::toString()
{
	string str = "<InterfaceDeclaration>\\n" + _name.to_string();
	/*for (unsigned int i = 0; i < _implements.size(); i++)
	{
		if (i == 0) str += " is ";
		str += _implements[i].to_string();
		if (i < _implements.size() - 1)
			str += ", ";
	}*/
	return str;
}

vector<AST::Node*> AST::InterfaceDeclaration::getChildren()
{
	vector<Node*> v;
	v.push_back(_implements);
	for (auto i : _properties)
		v.push_back(i);
	for (auto i : _functions)
		v.push_back(i);
	return v;
}

void AST::InterfaceDeclaration::addFunction(FunctionDeclaration * function)
{
	if (function && function->getContent())
		throw ErrorReporter::report("functions inside interfaces may not have a body", ERR_PARSER, function->getPosition());
	_functions.push_back(function);
}

vector<AST::Node*> AST::UnaryOperationStatement::getChildren()
{
	vector<Node*> v;
	v.push_back(_expression);
	return v;
}

AST::TypeDeclaration::~TypeDeclaration() 
{ 
	delete _interfaces; _variableDeclarations.clear(); _functionDeclarations.clear(); _propertyDeclarations.clear();
}

string AST::TypeDeclaration::toString()
{
	string str = "<TypeDeclaration>\\n" + _name.to_string();
	/*for (unsigned int i = 0; i < _interfaces.size(); i++)
	{
		if (i == 0) str += " is ";
		str += _interfaces[i].to_string();
		if (i < _interfaces.size() - 1)
			str += ", ";
	}*/
	return str;
}

vector<AST::Node*> AST::TypeDeclaration::getChildren()
{
	vector<Node*> v;
	v.push_back(_interfaces);
	for (auto i : _variableDeclarations)
		v.push_back(i);
	for (auto i : _functionDeclarations)
		v.push_back(i);
	for (auto i : _propertyDeclarations)
		v.push_back(i);
	return v;
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
	v.push_back(_get != nullptr ? _get : NULL);
	//v.push_back(_set != nullptr ? _set : NULL);
	if (_set == nullptr)
		v.push_back(NULL);
	else v.push_back(_set);
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
	if (parameter == nullptr)
		return;
	if (!parameter->isExpression())
		throw ErrorReporter::report("expected a variable declaration", ERR_PARSER, parameter->getPosition());
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
			else throw ErrorReporter::report("expected a variable declaration", ERR_PARSER, parameter->getPosition());
		}
		break;
	default:
		throw ErrorReporter::report("expected a variable declaration", ERR_PARSER, exp->getPosition());
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
	if (expression && expression->getExpressionType() == ET_LIST)
		for (auto i : dynamic_cast<AST::ExpressionList*>(expression)->getExpressions())
			_expressions.push_back(i);
	else _expressions.push_back(expression);
}

vector<AST::Node*> AST::ForLoop::getChildren()
{
	vector<Node*> v;
	v.push_back(_begin);
	v.push_back(_condition);
	v.push_back(_increment);
	v.push_back(_statement);
	return v;
}

vector<AST::Node*> AST::SwitchCase::getChildren()
{
	vector<Node*> v;
	v.push_back(_expression);
	v.push_back(_default);
	for (CaseClause i : _cases)
	{
		v.push_back(i._expression);
		v.push_back(i._statement);
	}
	return v;
}

vector<AST::Node*> AST::Identifier::getChildren()
{
	return vector<Node*>();
}

vector<AST::Node*> AST::ConstDeclaration::getChildren()
{
	return { _expression };
	/*vector<Node*> v;
	v.push_back(_expression);
	return v;*/
}