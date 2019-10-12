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

	typedef struct Type
	{
		vector<string> _prefixModifiers;
		string _typeName;
		vector<string> _postfixModifiers;
	} Type;

	class Node
	{
		unsigned int _nodeId;	// defined for purpose of the graphic view of the AST.
	public:
		Node() { _nodeId = _idCount++; };
		const unsigned int getNodeId() const { return (this == nullptr) ? -1 : _nodeId; };
		virtual bool isStatement() = 0;
		virtual bool isExpression() = 0;
		virtual string toString() = 0;
		virtual vector<Node*> getChildren() = 0;
	};

	class Statement : virtual public Node
	{
	public:
		Statement() : Node() {};
		virtual bool isStatement() { return true; };
		virtual bool isExpression() { return false; };

		virtual StatementType getStatementType() = 0;
	};

	class Expression : virtual public Node
	{
	public:
		Expression() : Node() {};
		virtual bool isStatement() { return false; };
		virtual bool isExpression() { return true; };

		virtual ExpressionType getExpressionType() = 0;
	};

	class ExpressionStatement : public Expression, public Statement
	{
	public:
		ExpressionStatement() : Expression() {};
		virtual bool isStatement() { return true; };
		virtual bool isExpression() { return true; };
	};

	/***************** ExpressionStatements *****************/

	class Assignment : public ExpressionStatement
	{
		Operator _operator;
		Expression* _left;
		Expression* _right;

	public:
		Assignment() : ExpressionStatement() {};
		virtual StatementType getStatementType() { return ST_ASSIGNMENT; };
		virtual ExpressionType getExpressionType() { return ET_ASSIGNMENT; };
		virtual string toString() { return "<Assignment>\\n" + _operator._str; };
		virtual vector<Node*> getChildren();

		void setOperator(Operator op) { _operator = op; }
		void setLeft(Expression* left) { _left = left; }
		void setRight(Expression* right) { _right = right; }

		Operator getOperator() { return _operator; }
		Expression* getLeft() { return _left; }
		Expression* getRight() { return _right; }
	};

	class Increment : public ExpressionStatement
	{
		Operator _operator;
		Expression* _expression;

	public:
		Increment() : ExpressionStatement() {};
		virtual StatementType getStatementType() { return ST_INCREMENT; };
		virtual ExpressionType getExpressionType() { return ET_INCREMENT; };
		virtual string toString() { return "<Increment>\\n" + _operator._str; };
		virtual vector<Node*> getChildren();

		void setOperator(Operator op) { _operator = op; }
		void setExpression(Expression* expression) { _expression = expression; }
		Operator getOperator() { return _operator; }
		Expression* getExpression() { return _expression; }
	};

	class FunctionCall : public ExpressionStatement
	{
		Expression* _functionId;
		vector<Expression*> _parameters;

	public:
		FunctionCall() : ExpressionStatement() {};
		virtual StatementType getStatementType() { return ST_FUNCTION_CALL; };
		virtual ExpressionType getExpressionType() { return ET_FUNCTION_CALL; };
		virtual string toString() { return string() + "<FunctionCall>\\n"/* + _functionId.name*/; };
		virtual vector<Node*> getChildren();

		void setFunctionId(Expression* funcId) { _functionId = funcId; }
		void addParameter(Expression* parameter) { _parameters.push_back(parameter); }

		Expression* getFunctionId() { return _functionId; }
		vector<Expression*> getParameters() { return _parameters; }
	};

	class VariableDeclaration : public ExpressionStatement
	{
		vector<VariableModifier> _modifiers;
		Expression* _type;
		string _varId;

	public:
		VariableDeclaration() : ExpressionStatement() {};
		virtual StatementType getStatementType() { return ST_VARIABLE_DECLARATION; };
		virtual ExpressionType getExpressionType() { return ET_VARIABLE_DECLARATION; };
		virtual string toString() {
			return "<VariableDeclaration>\\n" + _varId;
		};
		virtual vector<Node*> getChildren();

		void setVarId(string varId) { _varId = varId; }
		void setType(Expression* type) { _type = type; }
		void addModifier(VariableModifier modifier) { _modifiers.push_back(modifier); }
		string getVarId() { return _varId; }
		Expression* getVarType() { return _type; }
		vector<VariableModifier> getModifiers() { return _modifiers; }
	};

	/*class ExpressionStatementList : public ExpressionStatement
	{
		vector<ExpressionStatement*> _expStatements;
	public:
		ExpressionStatementList() : ExpressionStatement() {};
		virtual StatementType getStatementType() { return ST_EXP_STATEMENT_LIST; };
		virtual ExpressionType getExpressionType() { return ET_EXP_STATEMENT_LIST; };
		virtual string toString() { return "<ExpressionStatementList>"; };
		virtual vector<Node*> getChildren();

		void addStatement(ExpressionStatement* expStatement) { _expStatements.push_back(expStatement); }
		vector<ExpressionStatement*> getExpressionStatements() { return _expStatements; }
	};*/

	/*class VariableDeclaration : public ExpressionStatement
	{
		string _varId;
		Type _type;

	public:
		VariableDeclaration() : ExpressionStatement() {};
		virtual StatementType getStatementType() { return ST_VARIABLE_DECLARATION; };
		virtual ExpressionType getExpressionType() { return ET_VARIABLE_DECLARATION; };
		virtual string toString() {
			string str = "<VariableDeclaration>\\n";
			for (auto s : _type._prefixModifiers)
				str += s + ' ';
			str += _type._typeName;
			for (auto s : _type._postfixModifiers)
				str += s;
			return str + ' ' + _varId;
		};
		virtual vector<Node*> getChildren() { return vector<Node*>(); };

		void setVarId(string varId) { _varId = varId; }
		void setType(Type type) { _type = type; }
		string getVarId() { return _varId; }
		Type getVarType() { return _type; }
	};*/

	/********************** Statements **********************/

	class StatementBlock : public Statement
	{
		vector<Statement*> _statements;
	public:
		StatementBlock() : Statement() {};
		virtual StatementType getStatementType() { return ST_STATEMENT_BLOCK; };
		virtual string toString() { return "<StatementBlock>"; };
		virtual vector<Node*> getChildren();

		void addStatement(Statement* statement) { _statements.push_back(statement); }
	};

	class IfThenElse : public Statement
	{
		Expression* _condition;
		Statement* _thenBranch;
		Statement* _elseBranch;

	public:
		IfThenElse() : Statement() {};
		virtual StatementType getStatementType() { return ST_IF_THEN_ELSE; };
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
		WhileLoop() : Statement() {};
		virtual StatementType getStatementType() { return ST_WHILE_LOOP; };
		virtual string toString() { return "<While>"; };
		virtual vector<Node*> getChildren();

		void setCondition(Expression* condition) { _condition = condition; }
		void setStatement(Statement* statement) { _statement = statement; }

		Expression* getCondition() { return _condition; }
		Statement* getStatement() { return _statement; }
	};

	class DoWhileLoop : public WhileLoop
	{
	public:
		DoWhileLoop() : WhileLoop() {};
		virtual StatementType getStatementType() { return ST_DO_WHILE_LOOP; };
		virtual string toString() { return "<Do>"; };
		
	};

	class UnaryOperationStatement : public Statement
	{
		Operator _operator;
		Expression* _expression;

	public:
		UnaryOperationStatement() : Statement() {};
		virtual StatementType getStatementType() { return ST_UNARY_OPERATION; };
		virtual string toString() { return string() + "<UnaryOperatorStatement>\\n" + _operator._str; };
		virtual vector<Node*> getChildren();

		void setOperator(Operator op) { _operator = op; }
		void setExpression(Expression* expression) { _expression = expression; }

		Operator getOperator() { return _operator; }
		Expression* getExpression() { return _expression; }
	};

	class FunctionDeclaration : public Statement
	{
	private:
		VariableDeclaration* _decl;
		vector<VariableDeclaration*> _parameters;
		StatementBlock* _content;

	public:
		FunctionDeclaration(VariableDeclaration* decl) { _decl = decl; };
		virtual StatementType getStatementType() { return ST_FUNCTION_DECLARATION; };
		virtual string toString() { return "<FunctionDeclaration>\\n"; };
		virtual vector<Node*> getChildren();

		void setVarDecl(VariableDeclaration* decl) { _decl = decl; }
		void addParameter(Node* parameter);
		void addParameterToStart(VariableDeclaration* parameter) { _parameters.insert(_parameters.begin(), parameter); }
		void setContent(StatementBlock* content) { _content = content; }

		VariableDeclaration* getVarDecl() { return _decl; }
		vector<VariableDeclaration*> getParameters() { return _parameters; }
		StatementBlock* getContent() { return _content; }
	};

	class InterfaceDeclaration : public Statement
	{
		string _name;
		//vector<string> _modifiers;
		vector<string> _implements;
		vector<VariableDeclaration*> _properties;
		vector<FunctionDeclaration*> _functions;

	public:
		InterfaceDeclaration();
		virtual StatementType getStatementType() { return ST_INTERFACE_DECLARATION; };
		virtual string toString();
		virtual vector<Node*> getChildren();

		string getName() { return _name; }
		//vector<string> getModifiers() { return _modifiers; };
		vector<string> getImplements() { return _implements; }
		vector<VariableDeclaration*> getProperties() { return _properties; }
		vector<FunctionDeclaration*> getFunctions() { return _functions; }

		void setName(string id) { _name = id; }
		//void addModifier(string modifier) { _modifiers.push_back(modifier); }
		void addImplements(string interface) { _implements.push_back(interface); }
		void addProperty(VariableDeclaration* property) { _properties.push_back(property); }
		void addFunction(FunctionDeclaration* function) { 
			if (function && function->getContent())
				throw "functions inside interfaces must not have a body!";
			_functions.push_back(function); 
		}
	};

	class PropertyDeclaration : public Statement {
	private:
		VariableDeclaration* _decl;
		Statement* _get;
		Statement* _set;
	public:
		PropertyDeclaration(VariableDeclaration* decl) { _decl = decl; };
		virtual StatementType getStatementType() { return ST_PROPERTY_DECLARATION; };
		virtual string toString() { return "<PropertyDeclaration>"; };
		virtual vector<Node*> getChildren();

		void setGet(Statement* get) { _get = get; }
		void setSet(Statement* set) { _set = set; }
		Statement* getGet() { return _get; }
		Statement* getSet() { return _set; }
		VariableDeclaration* getVarDecl() { return _decl; }
	};

	class TypeDeclaration : public Statement
	{
		string _name;
		//vector<string> _modifiers;
		vector<string> _interfaces;
		vector<VariableDeclaration*> _variableDeclarations;
		vector<FunctionDeclaration*> _functionDeclarations;
		vector<PropertyDeclaration*> _propertyDeclarations;

	public:
		TypeDeclaration();
		virtual StatementType getStatementType() { return ST_TYPE_DECLARATION; };
		virtual string toString();
		virtual vector<Node*> getChildren();

		string getName() { return _name; }
		//vector<string> getModifiers() { return _modifiers; };
		vector<string> getInterfaces() { return _interfaces; }
		vector<VariableDeclaration*> getVariableDeclarations() { return _variableDeclarations; }
		vector<FunctionDeclaration*> getFunctionDeclarations() { return _functionDeclarations; }
		vector<PropertyDeclaration*> getPropertyDeclarations() { return _propertyDeclarations; }

		void setName(string id) { _name = id; }
		//void addModifier(string modifier) { _modifiers.push_back(modifier); }
		void addInterface(string interface) { _interfaces.push_back(interface); }
		void addVariableDeclaration(VariableDeclaration* variableDeclaration) { _variableDeclarations.push_back(variableDeclaration); }
		void addFunctionDeclaration(FunctionDeclaration* functionDeclaration) { _functionDeclarations.push_back(functionDeclaration); }
		void addPropertyDeclaration(PropertyDeclaration* propertyDeclaration) { _propertyDeclarations.push_back(propertyDeclaration); }
	};

	class NamespaceDeclaration : public Statement
	{
		string _name;
		Statement* _statement;

	public:
		NamespaceDeclaration() { _name = ""; };
		virtual StatementType getStatementType() { return ST_NAMESPACE_DECLARATION; };
		virtual string toString() { return "<NamespaceDeclaration>\\n" + _name; };
		virtual vector<Node*> getChildren();

		string getName() { return _name; }
		Statement* getStatement() { return _statement; }

		void setName(string id) { _name = id; }
		void setStatement(Statement* statement) { _statement = statement; }
	};

	class StatementList : public Statement
	{
		vector<Statement*> _statements;
	public:
		StatementList() : Statement() {};
		virtual StatementType getStatementType() { return ST_LIST; };
		virtual string toString() { return "<StatementList>"; };
		virtual vector<Node*> getChildren();

		void addStatement(Statement* statement);
		vector<Statement*> getStatements() { return _statements; }
	};

	/********************** Expressions **********************/

	class ExpressionList : public Expression
	{
		vector<Expression*> _expressions;
	public:
		ExpressionList() : Expression() {};
		virtual ExpressionType getExpressionType() { return ET_LIST; };
		virtual string toString() { return "<ExpressionList>"; };
		virtual vector<Node*> getChildren();

		void addExpression(Expression* expression);
		vector<Expression*> getExpressions() { return _expressions; }
	};

	class BinaryOperation : public Expression
	{
		Operator _operator;
		Expression* _left;
		Expression* _right;

	public:
		BinaryOperation() : Expression() {};
		virtual ExpressionType getExpressionType() { return ET_BINARY_OPERATION; };
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
		bool _isPostfix;

	public:
		UnaryOperation() : Expression() { _isPostfix = false; };
		virtual ExpressionType getExpressionType() { return ET_UNARY_OPERATION; };
		virtual string toString() { return string() + "<" + (_isPostfix ? "Postfix" : "")+ "UnaryOperator>\\n" + _operator._str; };
		virtual vector<Node*> getChildren();

		void setOperator(Operator op) { _operator = op; }
		void setExpression(Expression* expression) { _expression = expression; }
		void setIsPostfix(bool isPostfix) { _isPostfix = isPostfix; }

		Operator getOperator() { return _operator; }
		Expression* getExpression() { return _expression; }
		bool isPostfix() { return _isPostfix; }
	};

	class Variable : public Expression
	{
		string _varId;
	public:
		Variable(string varId) : Expression() { _varId = varId; };
		Variable() : Expression() {};
		void setVarId(string varId) { _varId = varId; }
		string getVarId() { return _varId; }
		virtual ExpressionType getExpressionType() { return ET_VARIABLE; };
		virtual string toString() { return "<Variable>\\n" + _varId; };
		virtual vector<Node*> getChildren() { return vector<Node*>(); };
	};

	class ConditionalExpression : public Expression
	{
		Expression* _condition;
		Expression* _thenBranch;
		Expression* _elseBranch;

	public:
		ConditionalExpression() : Expression() {};
		virtual ExpressionType getExpressionType() { return ET_CONDITIONAL_EXPRESSION; };
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
		Literal(LiteralType type) : Expression() { _type = type; };
		virtual ExpressionType getExpressionType() { return ET_LITERAL; };
		virtual vector<Node*> getChildren() { return vector<Node*>(); };
		LiteralType getLiteralType() { return _type; }
	};

	class Boolean : public Literal
	{
		bool _value;
	public:
		Boolean(bool value) : Literal(LT_BOOLEAN) { _value = value; }
		virtual string toString() { return string() + "<BoolLiteral>\\n" + std::to_string(_value); };
		bool getValue() { return _value; }
	};

	class Integer : public Literal
	{
		int _value;
	public:
		Integer(int value) : Literal(LT_INTEGER) { _value = value; }
		virtual string toString() { return string() + "<IntegerLiteral>\\n" + std::to_string(_value); };
		int getValue() { return _value; }
	};

	class Fraction : public Literal
	{
		float _value;
	public:
		Fraction(float value) : Literal(LT_FRACTION) { _value = value; }
		virtual string toString() { return string() + "<FracLiteral>\\n" + std::to_string(_value); };
		float getValue() { return _value; }
	};
	
	class Character : public Literal
	{
		char _value;
	public:
		Character(char value) : Literal(LT_CHARACTER) { _value = value; }
		virtual string toString() { return string() + "<CharLiteral>\\n'" + _value + '\''; };
		char getValue() { return _value; }
	};

	class String : public Literal
	{
		string _value;
	public:
		String(string value) : Literal(LT_STRING) { _value = value; }
		virtual string toString() { return string() + "<StringLiteral>\\n" + _value; };
		string getValue() { return _value; }
	};

	class Function : public Literal
	{
		vector<VariableDeclaration*> _parameters;
		StatementBlock* _content;
		Expression* _returnType;

	public:
		Function() : Literal(LT_FUNCTION) { }
		virtual string toString() { return string() + "<FunctionLiteral>"; };
		virtual vector<Node*> getChildren();

		void addParameter(Node* parameter);
		void addParameterToStart(VariableDeclaration* parameter) { _parameters.insert(_parameters.begin(), parameter); }
		void setContent(StatementBlock* content) { _content = content; }
		void setReturnType(Expression* type) { _returnType = type; }

		vector<VariableDeclaration*> getParameters() { return _parameters; }
		StatementBlock* getContent() { return _content; }
		Expression* getReturnType() { return _returnType; }
	};

	/*class TypeLiteral : public Literal
	{
		Type _type;
	public:
		TypeLiteral() : Literal(LT_TYPE) {}
		virtual string toString() { return string() + "<TypeLiteral>\\n" + _type._typeName; };
		Type getType() { return _type; }
		void setType(Type type) { _type = type; }
	};*/

	class Null : public Literal
	{
	public:
		Null() : Literal(LT_NULL) {}
		virtual string toString() { return string() + "<NullLiteral>"; };
	};

}