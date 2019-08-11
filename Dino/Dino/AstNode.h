#pragma once

#include <vector>
#include <string>
#include <iostream>
#include "TypeEnums.h"
using std::string;
using std::vector;

namespace AST
{
	class Node
	{
		unsigned int _nodeId;	// defined for purpose of the graphic view of the AST.
	public:
		Node(unsigned int nodeId) { _nodeId = nodeId; };
		const unsigned int getNodeId() const { return _nodeId; };
		virtual bool isStatement() = 0;
		virtual bool isExpression() = 0;
		virtual string toString() = 0;
	};

	class Statement : public Node
	{
	public:
		virtual bool isStatement() { return true; };
		virtual bool isExpression() { return false; };

		virtual StatementType getType() = 0;
	};

	class Expression : public Node
	{
	public:
		virtual bool isStatement() { return false; };
		virtual bool isExpression() { return true; };

		virtual ExpressionType getType() = 0;
	};

	/********************** Statements **********************/


	class StatementBlock : public Statement
	{
		vector<Statement*> _statements;
	public:
		virtual StatementType getType() { return ST_UNKNOWN; };
		virtual string toString() { return "<StatementBlock>"; };
	};

	class IfThenElse : public Statement
	{
		Expression* _condition;
		Statement* _ifBranch;
		Statement* _elseBranch;

	public:
		virtual StatementType getType() { return ST_UNKNOWN; };
		virtual string toString() { return "<IfThenElse>"; };
	};

	class WhileLoop : public Statement	// Add 'for' as a seperate class?
	{
		Expression* _condition;
		Statement* _statement;

	public:
		virtual StatementType getType() { return ST_UNKNOWN; };
		virtual string toString() { return "<While>"; };
	};

	class VariableDeclaration : public Statement
	{
		Identificator _varId;	// Temporary
		VariableType _type;
		vector<VariableModifier> _modifiers; // public, static, reactive, etc.

	public:
		virtual StatementType getType() { return ST_UNKNOWN; };
		virtual string toString() { return "<VariableDeclaration>"; };
	};

	class Assignment : public Statement
	{
		AssignmentOperator _operator;
		Identificator _left;	// Temporary
		Expression* _right;

	public:
		virtual StatementType getType() { return ST_UNKNOWN; };
		virtual string toString() { return "<Assignment>"; };
	};

	/********************** Expressions **********************/

	class BinaryOperation : public Expression
	{
		BinaryOperator _operator;
		Expression* _left;
		Expression* _right;

	public:
		virtual ExpressionType getType() { return ET_UNKNOWN; };
		virtual string toString() { return string() + "<BinaryOperator>\\n" + "TODO"; };
	};

	class UnaryOperation : public Expression
	{
		UnaryOperator _operator;
		Expression* _left;
		Expression* _right;

	public:
		virtual string toString() { return string() + "<UnaryOperator>\\n" + "TODO"; };
	};

	class FunctionCall : public Expression
	{
		Identificator _functionId;	// Temporary
		vector<Expression*> _parameters;

	public:
		virtual ExpressionType getType() { return ET_UNKNOWN; };
		virtual string toString() { return string() + "<FunctionCall>\\n" + "TODO"; };
	};

	class ConditionalExpression : public Expression
	{
		Expression* _condition;
		Expression* _ifBranch;
		Expression* _elseBranch;

	public:
		virtual ExpressionType getType() { return ET_UNKNOWN; };
		virtual string toString() { return string() + "<ConditionaExpression>"; };
	};

	typedef struct Identificator	// Temporary
	{
		string name;
	} Identificator;
}