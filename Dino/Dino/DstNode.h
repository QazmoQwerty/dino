#pragma once

#include "AstNode.h";
#include "Decorator.h";

namespace DST
{
	class Node : public AST::Node
	{
		
	};

	/*
		A statement is code that expresses some action to be carried out.
	*/
	class Statement : virtual public Node
	{
	public:
		Statement() : Node() {};

		virtual StatementType getStatementType() = 0;

		/* This node IS a Statement */
		virtual bool isStatement() { return true; };

		/* This node is NOT an Expression */
		virtual bool isExpression() { return false; };

		virtual bool isDeclaration() { return false; }
	};

	/*
		An expression is code that evaluates to a value.
	*/
	class Expression : virtual public Node
	{
	public:
		Expression() : Node() {};

		virtual ExpressionType getExpressionType() = 0;

		/* This node is NOT a Statement */
		virtual bool isStatement() { return false; };

		/* This node IS an Expression */
		virtual bool isExpression() { return true; };
	};

	/*
		An ExpressionStatement is both an Expression and a Statement;
		it expresses an action to be carried out AND evaluates to a value.
	*/
	class ExpressionStatement : public Expression, public Statement
	{
	public:
		ExpressionStatement() : Expression() {};

		/* This node IS a Statement */
		virtual bool isStatement() { return true; };

		/* This node IS an Expression */
		virtual bool isExpression() { return true; };
	};

	class BinaryOperation : public Expression
	{
		AST::BinaryOperation *_base;
		Expression *_left;
		Expression *_right;

	public:
		BinaryOperation(AST::BinaryOperation *base);
	};

	class ConditionalExpression : public Expression
	{
		AST::ConditionalExpression *_base;
		Expression* _condition;
		Expression* _thenBranch;
		Expression* _elseBranch;
	public:
		ConditionalExpression(AST::ConditionalExpression *base);
	};
}