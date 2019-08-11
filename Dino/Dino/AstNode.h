#pragma once

#include <vector>
#include <string>
#include <iostream>
#include "TypeEnums.h"
using std::string;
using std::vector;

namespace AST
{
	typedef struct Identificator	// Temporary
	{
		string name;
	} Identificator;

	class Node
	{
		unsigned int _nodeId;	// defined for purpose of the graphic view of the AST.
	public:
		Node(unsigned int nodeId) { _nodeId = nodeId; };
		const unsigned int getNodeId() const { return _nodeId; };
		virtual bool isStatement() = 0;
		virtual bool isExpression() = 0;
		virtual string toString() = 0;
		virtual vector<Node*> getChildren() = 0;
	};

	class Statement : public Node
	{
	public:
		Statement(unsigned int nodeId) : Node(nodeId) {};
		virtual bool isStatement() { return true; };
		virtual bool isExpression() { return false; };

		virtual StatementType getType() = 0;
	};

	class Expression : public Node
	{
	public:
		Expression(unsigned int nodeId) : Node(nodeId) {};
		virtual bool isStatement() { return false; };
		virtual bool isExpression() { return true; };

		virtual ExpressionType getType() = 0;
	};

	/********************** Statements **********************/

	class StatementBlock : public Statement
	{
		vector<Statement*> _statements;
	public:
		StatementBlock(unsigned int nodeId) : Statement(nodeId) {};
		virtual StatementType getType() { return ST_UNKNOWN; };
		virtual string toString() { return "<StatementBlock>"; };
		virtual vector<Node*> getChildren();

		void addStatement(Statement* statement) { _statements.push_back(statement); }
	};

	class IfThenElse : public Statement
	{
		Expression* _condition;
		Statement* _ifBranch;
		Statement* _elseBranch;

	public:
		IfThenElse(unsigned int nodeId) : Statement(nodeId) {};
		virtual StatementType getType() { return ST_UNKNOWN; };
		virtual string toString() { return "<IfThenElse>"; };
		virtual vector<Node*> getChildren();

		void setCondition(Expression* condition) { _condition = condition; }
		void setIfBranch(Statement* ifBranch) { _ifBranch = ifBranch; }
		void setElseBranch(Statement* elseBranch) { _elseBranch = elseBranch; }
	};

	class WhileLoop : public Statement	// Add 'for' as a seperate class?
	{
		Expression* _condition;
		Statement* _statement;

	public:
		WhileLoop(unsigned int nodeId) : Statement(nodeId) {};
		virtual StatementType getType() { return ST_UNKNOWN; };
		virtual string toString() { return "<While>"; };
		virtual vector<Node*> getChildren();

		void setCondition(Expression* condition) { _condition = condition; }
		void setStatement(Statement* statement) { _statement = statement; }
	};

	class VariableDeclaration : public Statement
	{
		Identificator _varId;	// Temporary
		VariableType _type;
		vector<VariableModifier> _modifiers; // public, static, reactive, etc.

	public:
		VariableDeclaration(unsigned int nodeId) : Statement(nodeId) {};
		virtual StatementType getType() { return ST_UNKNOWN; };
		virtual string toString() { return "<VariableDeclaration>"; };
		virtual vector<Node*> getChildren() { return vector<Node*>(); };

		void setVarId(Identificator varId) { _varId = varId; }
		void setType(VariableType type) { _type = type; }
		void addModifier(VariableModifier modifier) { _modifiers.push_back(modifier); }
	};

	class Assignment : public Statement
	{
		AssignmentOperator _operator;
		Identificator _left;	// Temporary
		Expression* _right;

	public:
		Assignment(unsigned int nodeId) : Statement(nodeId) {};
		virtual StatementType getType() { return ST_UNKNOWN; };
		virtual string toString() { return "<Assignment>"; };
		virtual vector<Node*> getChildren();

		void setOperator(AssignmentOperator op) { _operator = op; }
		void setLeft(Identificator left) { _left = left; }
		void setRight(Expression* right) { _right = right; }
	};

	/********************** Expressions **********************/

	class BinaryOperation : public Expression
	{
		BinaryOperator _operator;
		Expression* _left;
		Expression* _right;

	public:
		BinaryOperation(unsigned int nodeId) : Expression(nodeId) {};
		virtual ExpressionType getType() { return ET_UNKNOWN; };
		virtual string toString() { return string() + "<BinaryOperator>\\n" + "TODO"; };
		virtual vector<Node*> getChildren();

		void setOperator(BinaryOperator op) { _operator = op; }
		void setLeft(Expression* left) { _left = left; }
		void setRight(Expression* right) { _right = right; }
	};

	class UnaryOperation : public Expression
	{
		UnaryOperator _operator;
		Expression* _left;
		Expression* _right;

	public:
		UnaryOperation(unsigned int nodeId) : Expression(nodeId) {};
		virtual string toString() { return string() + "<UnaryOperator>\\n" + "TODO"; };
		virtual vector<Node*> getChildren();

		void setOperator(UnaryOperator op) { _operator = op; }
		void setLeft(Expression* left) { _left = left; }
		void setRight(Expression* right) { _right = right; }
	};

	class FunctionCall : public Expression
	{
		Identificator _functionId;	// Temporary
		vector<Expression*> _parameters;

	public:
		FunctionCall(unsigned int nodeId) : Expression(nodeId) {};
		virtual ExpressionType getType() { return ET_UNKNOWN; };
		virtual string toString() { return string() + "<FunctionCall>\\n" + "TODO"; };
		virtual vector<Node*> getChildren();

		void setFunctionId(Identificator funcId) { _functionId = funcId; }
		void addParameter(Expression* parameter) { _parameters.push_back(parameter); }
	};

	class ConditionalExpression : public Expression
	{
		Expression* _condition;
		Expression* _ifBranch;
		Expression* _elseBranch;

	public:
		ConditionalExpression(unsigned int nodeId) : Expression(nodeId) {};
		virtual ExpressionType getType() { return ET_UNKNOWN; };
		virtual string toString() { return string() + "<ConditionaExpression>"; };
		virtual vector<Node*> getChildren();

		void setCondition(Expression* condition) { _condition = condition; }
		void setIfBranch(Expression* ifBranch) { _ifBranch = ifBranch; }
		void setElseBranch(Expression* elseBranch) { _elseBranch = elseBranch; }
	};

	class Literal : public Expression
	{
		LiteralType _type;

	public:
		Literal(unsigned int nodeId, LiteralType type) : Expression(nodeId) { _type = type; };
		virtual ExpressionType getType() { return ET_UNKNOWN; };
		virtual string toString() { return string() + "<Literal>\\n" + "TODO"; };
		virtual vector<Node*> getChildren() { return vector<Node*>(); };
	};

	class Integer : public Literal
	{
		int _value;
	public:
		Integer(unsigned int nodeId, int value) : Literal(nodeId, LT_INTEGER) { _value = value; }
		virtual string toString() { return string() + "<IntegerLiteral>\\n" + std::to_string(_value); };
	};
}