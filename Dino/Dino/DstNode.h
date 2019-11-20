#pragma once

#include "AstNode.h"

namespace DST
{
	static int _idCount = 0;

	class Node
	{
		unsigned int _nodeId;	// defined for purpose of the graphic view of the AST.
		int _line;
	public:
		/* Default constructor, does NOT set line.*/
		Node() { _nodeId = _idCount++; };

		/* Set the line the node is on */
		void setLine(int line) { _line = line; }

		/* Line the node is on */
		const int getLine() const { return _line; }

		/* Returns whether the node represents a Statement */
		virtual bool isStatement() = 0;

		/* Returns whether the node represents an Expression */
		virtual bool isExpression() = 0;

		/* ----------  For AstToFile.h ---------- */

		/*
			Returns node's unique id number
			Function defined for AST visual representation - see AstToFile.h
		*/
		const unsigned int getNodeId() const { return (this == nullptr) ? -1 : _nodeId; };

		/*
			Returns a string representation of the node (excluding children info)
			Function defined for AST visual representation - see AstToFile.h
		*/
		virtual string toString() = 0;

		/*
			Returns a vector of all children owned by the node.
			Function defined for AST visual representation - see AstToFile.h
		*/
		virtual vector<Node*> getChildren() = 0;
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

	class Type;

	/*
		An expression is code that evaluates to a value.
	*/
	class Expression : virtual public Node
	{
	public:
		virtual ExpressionType getExpressionType() = 0;
		virtual Type *getType() = 0;

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

	/********************** Expressions **********************/

	class Type : public Expression
	{
	protected:
		AST::Expression *_base;
	public:
		Type(AST::Expression *base) : _base(base) { }
		virtual Type *getType() { return NULL; } // TODO - pointer to a global "type" type specifier.
		virtual ExactType getExactType() = 0;
		virtual ExpressionType getExpressionType() { return ET_TYPE; }

		virtual string toShortString() = 0;
		virtual string toString() { return "<Type>"; };
		virtual vector<Node*> getChildren();
	};

	class BasicType : public Type 
	{
		unicode_string _typeId;
	public:
		BasicType(AST::Expression *base) : Type(base) { _typeId = dynamic_cast<AST::Variable*>(_base)->getVarId(); }
		BasicType(unicode_string typeId) : Type(NULL), _typeId(typeId) { }
		unicode_string getTypeId() { return _typeId; }
		ExactType getExactType() { return EXACT_BASIC; }

		virtual string toShortString() { return _typeId.to_string(); };
		virtual string toString() { return "<BasicType>\n" + _typeId.to_string(); };
		virtual vector<Node*> getChildren();
	};

	class Variable : public Expression
	{
		AST::Variable *_base;
		Type *_type;

	public:
		Variable(AST::Variable *base, Type *type) : _base(base), _type(type) {};
		void setType(Type *type) { _type = type; };
		virtual Type *getType() { return _type; }
		virtual ExpressionType getExpressionType() { return ET_VARIABLE; }

		virtual string toString() { return _base->toString() + "\nType: " + _type->toShortString(); };
		virtual vector<Node*> getChildren();
	};

	class BinaryOperation : public Expression
	{
		AST::BinaryOperation *_base;
		Type *_type;
		Expression *_left;
		Expression *_right;

	public:
		BinaryOperation(AST::BinaryOperation *base) : _base(base) {};
		BinaryOperation(AST::BinaryOperation *base, Expression *left, Expression *right) : _base(base), _left(left), _right(right) {};
		void setType(Type *type) { _type = type; };
		virtual Type *getType() { return _type; }
		virtual ExpressionType getExpressionType() { return ET_BINARY_OPERATION; }
		Expression *getLeft() { return _left; }
		Expression *getRight() { return _right; }

		virtual string toString() { return _base->toString() + "\nType: " + _type->toShortString(); };
		virtual vector<Node*> getChildren();
	};

	class Literal : public Expression
	{
		AST::Literal *_base;
		Type *_type;
	public:
		Literal(AST::Literal* base) : _base(base) {}
		void setType(Type *type) { _type = type; }
		Type *getType() { return _type; }
		virtual ExpressionType getExpressionType() { return ET_LITERAL; }

		virtual string toString() { return _base->toString() + "\nType: " + _type->toShortString(); };
		virtual vector<Node*> getChildren();
	};

	/********************** Statements **********************/

	class StatementBlock : public Statement
	{
		vector<Statement*> _statements;
	public:
		virtual StatementType getStatementType() { return ST_STATEMENT_BLOCK; };

		vector<Statement*> getStatements() { return _statements; }
		void addStatement(Statement* statement) { _statements.push_back(statement); }

		virtual string toString() { return "<StatementBlock>"; };
		virtual vector<Node*> getChildren();
	};

	class TypeDeclaration : public Statement 
	{
		unicode_string _name;
		/*vector<unicode_string> _interfaces;
		vector<VariableDeclaration*> _variableDeclarations;
		vector<FunctionDeclaration*> _functionDeclarations;
		vector<PropertyDeclaration*> _propertyDeclarations;*/
	public:
		TypeDeclaration(unicode_string name) : _name(name) {}
		virtual StatementType getStatementType() { return ST_TYPE_DECLARATION; };

		virtual string toString() { return "<TypeDeclaration>"; };
		virtual vector<Node*> getChildren();
	};

	/***************** ExpressionStatements *****************/

	class VariableDeclaration : public ExpressionStatement
	{
		AST::VariableDeclaration *_base;
		Type *_type;

	public:
		VariableDeclaration(AST::VariableDeclaration *base) : _base(base) { };

		void setType(Type *type) { _type = type; }
		virtual Type *getType() { return _type; }

		virtual ExpressionType getExpressionType() { return ET_VARIABLE_DECLARATION; }
		virtual StatementType getStatementType() { return ST_VARIABLE_DECLARATION; }

		virtual string toString() { return _base->toString() + "\nType: " + _type->toShortString(); };
		virtual vector<Node*> getChildren();
	};

	class Assignment : public ExpressionStatement
	{
		AST::Assignment *_base;
		Type *_type;
		Expression *_left;
		Expression *_right;

	public:
		Assignment(AST::Assignment *base) : _base(base) {};
		Assignment(AST::Assignment *base, Expression *left, Expression *right) : _base(base), _left(left), _right(right) {};
		void setType(Type *type) { _type = type; };
		virtual Type *getType() { return _type; }
		virtual ExpressionType getExpressionType() { return ET_ASSIGNMENT; }
		virtual StatementType getStatementType() { return ST_ASSIGNMENT; }
		Expression *getLeft() { return _left; }
		Expression *getRight() { return _right; }

		virtual string toString() { return _base->toString() + "\nType: " + _type->toShortString(); };
		virtual vector<Node*> getChildren();
	};
}