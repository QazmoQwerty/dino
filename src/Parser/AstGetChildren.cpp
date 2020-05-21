/*
    Implementations for all the required getChildren() functions of all AST classes.
    These functions are used for the outputting of graphviz representations of the AST.
    This file is a bunch of boring boilerplate code which you have no reason to read :).
*/
#include "AstNode.h"

vector<AST::Node*> AST::Identifier              ::getChildren() { return { };                                            }
vector<AST::Node*> AST::ForLoop                 ::getChildren() { return { _begin, _condition, _increment, _statement }; }
vector<AST::Node*> AST::IfThenElse              ::getChildren() { return {_condition, _thenBranch, _elseBranch };        }
vector<AST::Node*> AST::WhileLoop               ::getChildren() { return { _condition, _statement };                     }
vector<AST::Node*> AST::Assignment              ::getChildren() { return { _left, _right };                              }
vector<AST::Node*> AST::Increment               ::getChildren() { return { _expression };                                }
vector<AST::Node*> AST::BinaryOperation         ::getChildren() { return { _left, _right };                              }
vector<AST::Node*> AST::UnaryOperation          ::getChildren() { return {_expression};                                  }
vector<AST::Node*> AST::FunctionCall            ::getChildren() { return { _functionId, _arguments };                    }
vector<AST::Node*> AST::ConditionalExpression   ::getChildren() { return { _condition, _thenBranch, _elseBranch};        }
vector<AST::Node*> AST::UnaryOperationStatement ::getChildren() { return { _expression };                                }
vector<AST::Node*> AST::ConstDeclaration        ::getChildren() { return { _expression };                                }
vector<AST::Node*> AST::NamespaceDeclaration    ::getChildren() { return { _statement };                                 }
vector<AST::Node*> AST::PropertyDeclaration     ::getChildren() { return { _decl, _get, _set};                           }
vector<AST::Node*> AST::VariableDeclaration     ::getChildren() { return { _type };                                      }
vector<AST::Node*> AST::EnumDeclaration         ::getChildren() { return {_type};                                        }

vector<AST::Node*> AST::Comparison::getChildren() 
{
	vector<Node*> ret;
	for (auto i : _expressions) ret.push_back(i);
	return ret;
}

vector<AST::Node*> AST::ExpressionList::getChildren()
{
	vector<Node*> v;
	for (auto i : _expressions) v.push_back(i);
	return v;
}

vector<AST::Node*> AST::SwitchCase::getChildren()
{
	vector<Node*> v { _expression, _default };
	for (CaseClause i : _cases)
	{
		v.push_back(i._expression);
		v.push_back(i._statement);
	}
	return v;
}



vector<AST::Node*> AST::TypeDeclaration::getChildren()
{
	vector<Node*> v { _interfaces };
	for (auto i : _variableDeclarations) v.push_back(i);
	for (auto i : _functionDeclarations) v.push_back(i);
	for (auto i : _propertyDeclarations) v.push_back(i);
	return v;
}

vector<AST::Node*> AST::FunctionDeclaration::getChildren()
{
	vector<Node*> v {_decl };
	for (auto i : _parameters) v.push_back(i);
	v.push_back(_content);
	return v;
}

vector<AST::Node*> AST::StatementBlock::getChildren()
{
	vector<Node*> v;
	for (auto i : _statements) v.push_back(i);
	return v;
}

vector<AST::Node*> AST::Function::getChildren()
{
	vector<Node*> v { _returnType };
	for (auto i : _parameters) v.push_back(i);
	v.push_back(_content);
	return v;
}

vector<AST::Node*> AST::InterfaceDeclaration::getChildren()
{
	vector<Node*> v { _implements };
	for (auto i : _properties) v.push_back(i);
	for (auto i : _functions) v.push_back(i);
	return v;
}

