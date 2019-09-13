#pragma once

#include <vector>
#include <string>
#include <iostream>
#include "TypeEnums.h"
#include "OperatorsMap.h"
using std::string;
using std::vector;

namespace AST
{
	static int _idCount = 0;

	typedef struct Identificator	// Temporary
	{
		string name;
	} Identificator;

	class Node
	{
		unsigned int _nodeId;	// defined for purpose of the graphic view of the AST.
	public:
		Node(unsigned int nodeId) { _nodeId = nodeId; };
		Node() { _nodeId = _idCount++; };
		const unsigned int getNodeId() const { return (this == nullptr) ? -1 : _nodeId; };
		virtual bool isStatement() = 0;
		virtual bool isExpression() = 0;
		virtual string toString() = 0;
		virtual vector<Node*> getChildren() = 0;
	};

	class Statement : public Node
	{
	public:
		Statement(unsigned int nodeId) : Node(nodeId) {};
		Statement() : Node() {};
		virtual bool isStatement() { return true; };
		virtual bool isExpression() { return false; };

		virtual StatementType getType() = 0;
	};

	class Expression : public Node
	{
	public:
		Expression(unsigned int nodeId) : Node(nodeId) {};
		Expression() : Node() {};
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
		StatementBlock() : Statement() {};
		virtual StatementType getType() { return ST_STATEMENT_BLOCK; };
		virtual string toString() { return "<StatementBlock>"; };
		virtual vector<Node*> getChildren();

		void addStatement(Statement* statement) { _statements.push_back(statement); }
	};

	class Block : public Statement
	{
		vector<Node*> _statements;
	public:
		Block(unsigned int nodeId) : Statement(nodeId) {};
		Block() : Statement() {};
		virtual StatementType getType() { return ST_STATEMENT_BLOCK; };
		virtual string toString() { return "<Block>"; };
		virtual vector<Node*> getChildren() { return _statements; }
		void addStatement(Node* statement) { _statements.push_back(statement); }
	};

	class IfThenElse : public Statement
	{
		Expression* _condition;
		Statement* _thenBranch;
		Statement* _elseBranch;

	public:
		IfThenElse(unsigned int nodeId) : Statement(nodeId) {};
		IfThenElse() : Statement() {};
		virtual StatementType getType() { return ST_IF_THEN_ELSE; };
		virtual string toString() { return "<IfThenElse>"; };
		virtual vector<Node*> getChildren();

		void setCondition(Expression* condition) { _condition = condition; }
		void setThenBranch(Statement* thenBranch) { _thenBranch = thenBranch; }
		void setElseBranch(Statement* elseBranch) { _elseBranch = elseBranch; }

		Expression* getCondition() { return _condition; }
		Statement* getThenBranch() { return _thenBranch; }
		Statement* getElseBranch() { return _elseBranch; }
	};

	class WhileLoop : public Statement	// Add 'for' as a seperate class?
	{
		Expression* _condition;
		Statement* _statement;

	public:
		WhileLoop(unsigned int nodeId) : Statement(nodeId) {};
		WhileLoop() : Statement() {};
		virtual StatementType getType() { return ST_WHILE_LOOP; };
		virtual string toString() { return "<While>"; };
		virtual vector<Node*> getChildren();

		void setCondition(Expression* condition) { _condition = condition; }
		void setStatement(Statement* statement) { _statement = statement; }

		Expression* getCondition() { return _condition; }
		Statement* getStatement() { return _statement; }
	};

	class VariableDeclaration : public Statement
	{
		Identificator _varId;	// Temporary
		Identificator _type;
		vector<Identificator> _modifiers; // public, static, reactive, etc.

	public:
		VariableDeclaration(unsigned int nodeId) : Statement(nodeId) {};
		VariableDeclaration() : Statement() {};
		virtual StatementType getType() { return ST_VARIABLE_DECLARATION; };
		virtual string toString() { 
			string modifiers = "";
			for (auto s : _modifiers) {
				modifiers += s.name + ' ';
				std::cout << s.name << std::endl;
			}
			return "<VariableDeclaration>\\n" + modifiers + _type.name + ' ' + _varId.name; 
		};
		virtual vector<Node*> getChildren() { return vector<Node*>(); };

		void setVarId(Identificator varId) { _varId = varId; }
		void setType(Identificator type) { _type = type; }
		void addModifier(Identificator modifier) { _modifiers.push_back(modifier); }
		Identificator getVarId() { return _varId; }
		Identificator getVarType() { return _type; }
	};

	class Assignment : public Statement
	{
		Operator _operator;
		Identificator _left;	// Temporary
		Expression* _right;

	public:
		Assignment(unsigned int nodeId) : Statement(nodeId) {};
		Assignment() : Statement() {};
		virtual StatementType getType() { return ST_ASSIGNMENT; };
		virtual string toString() { return "<Assignment>\\n" + _operator._str; };
		virtual vector<Node*> getChildren();

		void setOperator(Operator op) { _operator = op; }
		void setLeft(Identificator left) { _left = left; }
		void setRight(Expression* right) { _right = right; }
	};

	/********************** Expressions **********************/

	class BinaryOperation : public Expression
	{
		Operator _operator;
		Expression* _left;
		Expression* _right;

	public:
		BinaryOperation(unsigned int nodeId) : Expression(nodeId) {};
		BinaryOperation() : Expression() {};
		virtual ExpressionType getType() { return ET_BINARY_OPERATION; };
		virtual string toString() { return string() + "<BinaryOperator>\\n" + _operator._str; };
		virtual vector<Node*> getChildren();

		void setOperator(Operator op) { _operator = op; }
		void setLeft(Expression* left) { _left = left; }
		void setRight(Expression* right) { _right = right; }

		Expression* getLeft() { return _left; }
		Expression* getRight() { return _right; }
		Operator getOperator() { return _operator; }
	};

	class UnaryOperation : public Expression
	{
		Operator _operator;
		Expression* _expression;

	public:
		UnaryOperation(unsigned int nodeId) : Expression(nodeId) {};
		UnaryOperation() : Expression() {};
		virtual ExpressionType getType() { return ET_UNARY_OPERATION; };
		virtual string toString() { return string() + "<UnaryOperator>\\n" + _operator._str; };
		virtual vector<Node*> getChildren();

		void setOperator(Operator op) { _operator = op; }
		void setExpression(Expression* expression) { _expression = expression; }
	};

	class FunctionCall : public Expression
	{
		Identificator _functionId;	// Temporary
		vector<Expression*> _parameters;

	public:
		FunctionCall(unsigned int nodeId) : Expression(nodeId) {};
		FunctionCall() : Expression() {};
		virtual ExpressionType getType() { return ET_FUNCTION_CALL; };
		virtual string toString() { return string() + "<FunctionCall>\\n" + _functionId.name; };
		virtual vector<Node*> getChildren();

		void setFunctionId(Identificator funcId) { _functionId = funcId; }
		void addParameter(Expression* parameter) { _parameters.push_back(parameter); }

		Identificator getFunctionId() { return _functionId; }
		vector<Expression*> getParameters() { return _parameters; }
	};

	class Variable : public Expression
	{
		Identificator _varId;	// Temporary

	public:
		void setVarId(Identificator varId) { _varId = varId; }
		virtual Identificator getVarId() { return _varId; }
		Variable(unsigned int nodeId) : Expression(nodeId) {};
		Variable() : Expression() {};
		Variable(unsigned int nodeId, Identificator varId) : Expression(nodeId) { _varId = varId; };
		Variable(Identificator varId) : Expression() { _varId = varId; };
		virtual ExpressionType getType() { return ET_VARIABLE; };
		virtual string toString() { return "<Variable>\\n" + _varId.name; };
		virtual vector<Node*> getChildren() { return vector<Node*>(); };
	};

	class ConditionalExpression : public Expression
	{
		Expression* _condition;
		Expression* _thenBranch;
		Expression* _elseBranch;

	public:
		ConditionalExpression(unsigned int nodeId) : Expression(nodeId) {};
		ConditionalExpression() : Expression() {};
		virtual ExpressionType getType() { return ET_CONDITIONAL_EXPRESSION; };
		virtual string toString() { return string() + "<ConditionaExpression>"; };
		virtual vector<Node*> getChildren();

		void setCondition(Expression* condition) { _condition = condition; }
		void setThenBranch(Expression* thenBranch) { _thenBranch = thenBranch; }
		void setElseBranch(Expression* elseBranch) { _elseBranch = elseBranch; }
	};

	/********************** Literals **********************/

	// Note: literals are expressions.

	class Literal : public Expression
	{
		LiteralType _type;

	public:
		Literal(unsigned int nodeId, LiteralType type) : Expression(nodeId) { _type = type; };
		Literal(LiteralType type) : Expression() { _type = type; };
		virtual ExpressionType getType() { return ET_LITERAL; };
		virtual vector<Node*> getChildren() { return vector<Node*>(); };
		LiteralType getLiteralType() { return _type; }
	};

	class Boolean : public Literal
	{
		bool _value;
	public:
		Boolean(unsigned int nodeId, bool value) : Literal(nodeId, LT_BOOLEAN) { _value = value; }
		Boolean(bool value) : Literal(LT_BOOLEAN) { _value = value; }
		virtual string toString() { return string() + "<IntegerLiteral>\\n" + std::to_string(_value); };
		bool getValue() { return _value; }
	};

	class Integer : public Literal
	{
		int _value;
	public:
		Integer(unsigned int nodeId, int value) : Literal(nodeId, LT_INTEGER) { _value = value; }
		Integer(int value) : Literal(LT_INTEGER) { _value = value; }
		virtual string toString() { return string() + "<IntegerLiteral>\\n" + std::to_string(_value); };
		int getValue() { return _value; }
	};

	class Fraction : public Literal
	{
		float _value;
	public:
		Fraction(unsigned int nodeId, float value) : Literal(nodeId, LT_FRACTION) { _value = value; }
		Fraction(float value) : Literal(LT_FRACTION) { _value = value; }
		virtual string toString() { return string() + "<IntegerLiteral>\\n" + std::to_string(_value); };
		float getValue() { return _value; }
	};

	class Character : public Literal
	{
		char _value;
	public:
		Character(unsigned int nodeId, char value) : Literal(nodeId, LT_STRING) { _value = value; }
		Character(char value) : Literal(LT_STRING) { _value = value; }
		virtual string toString() { return string() + "<IntegerLiteral>\\n'" + _value + '\''; };
		char getValue() { return _value; }
	};

	class String : public Literal
	{
		string _value;
	public:
		String(unsigned int nodeId, string value) : Literal(nodeId, LT_STRING) { _value = value; }
		String(string value) : Literal(LT_STRING) { _value = value; }
		virtual string toString() { return string() + "<IntegerLiteral>\\n\"" + _value + '"'; };
		string getValue() { return _value; }
	};

	class Null : public Literal
	{
	public:
		Null(unsigned int nodeId) : Literal(nodeId, LT_NULL) {}
		Null() : Literal(LT_NULL) {}
		virtual string toString() { return string() + "<IntegerLiteral>\\nnull"; };
	};
}