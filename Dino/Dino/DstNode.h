#pragma once

#include "AstNode.h"

namespace DST
{
	class Node
	{
		
	};

	/*
		A statement is code that expresses some action to be carried out.
	*/
	class Statement : virtual public Node
	{
	public:
		
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
		/* This node IS a Statement */
		virtual bool isStatement() { return true; };

		/* This node IS an Expression */
		virtual bool isExpression() { return true; };
	};

	class Type : public Expression
	{
	protected:
		AST::Expression *_base;
	public:
		Type(AST::Expression *base) : _base(base) { }
		virtual ExactType getExactType() = 0;
		virtual ExpressionType getExpressionType() { return ET_TYPE; }
	};

	class BasicType : public Type 
	{
	public:
		BasicType(AST::Expression *base) : Type(base) { }
		unicode_string getTypeId() { return dynamic_cast<AST::Variable*>(_base)->getVarId(); }
		ExactType getExactType() { return EXACT_BASIC; }
	};

	class VariableDeclaration : public ExpressionStatement
	{
		AST::VariableDeclaration *_base;
		Type *_type;

	public:
		VariableDeclaration(AST::VariableDeclaration *base) : _base(base) { };

		Type *getType() { return _type; }
		void setType(Type *type) { _type = type; }

		virtual ExpressionType getExpressionType() { return ET_VARIABLE_DECLARATION; }
		virtual StatementType getStatementType() { return ST_VARIABLE_DECLARATION; }
	};

	class Variable : public Expression
	{
		AST::Variable *_base;
		Type *_type;

	public:
		Variable(AST::Variable *base, Type *type) : _base(base), _type(type) {};
		void setType(Type *type) { _type = type; };
		virtual ExpressionType getExpressionType() { return ET_VARIABLE; }
	};
}