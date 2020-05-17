/*
	An Abstract Syntax Tree (AST) is a tree representation of the abstract 
	structure of source code written in a programming language. 
	
	Each node of the tree denotes a construct occurring in the source code.
*/

#pragma once

#include <vector>
#include <string>
#include <iostream>
#include "../Utils/TypeEnums.h"
#include "../Utils/ErrorReporter/ErrorReporter.h"
#include "../Utils/OperatorsMap/OperatorsMap.h"
using std::string;
using std::vector;

namespace AST
{
	extern int _idCount;

	class Node
	{
		uint _nodeId;	// defined for purpose of the graphic view of the AST.
		// int _line;
		ErrorReporter::Position _pos;
	public:
		/* Default constructor, does NOT set line.*/
		Node() { _nodeId = _idCount++; };

		virtual ~Node() {}

		/* Set the line the node is on */
		// void setLine(int line) { _line = line; }

		/* Line the node is on */
		// int getLine() const { return _line; }

		ErrorReporter::Position &getPosition() { return _pos; }
		void setPosition(ErrorReporter::Position pos) { _pos = pos; }
		
		/* Returns whether the node represents a Statement */
		virtual bool isStatement() = 0;

		/* Returns whether the node represents an Expression */
		virtual bool isExpression() = 0;

		/* ----------  For AstToFile.h ---------- */

		/* 
			Returns node's unique id number 
			Function defined for AST visual representation - see AstToFile.h 
		*/
		//const uint getNodeId() const { return (this == nullptr) ? -1 : _nodeId; };
		uint getNodeId() const { return _nodeId; };

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

	/***************** ExpressionStatements *****************/

	class Assignment : public ExpressionStatement
	{
		Operator _operator;
		Expression* _left;
		Expression* _right;

	public:
		Assignment() : ExpressionStatement() {};
		virtual ~Assignment() { if (_left) delete _left; if (_right) delete _right; }
		virtual vector<Node*> getChildren();
		virtual string toString() { return "<Assignment>\\n" + _operator._str.to_string(); };
		virtual StatementType getStatementType() { return ST_ASSIGNMENT; };
		virtual ExpressionType getExpressionType() { return ET_ASSIGNMENT; };

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
		bool _isInc;

	public:
		Increment(bool isIncrement) : ExpressionStatement(), _isInc(isIncrement) {};
		virtual ~Increment() { if (_expression) delete _expression; }
		virtual StatementType getStatementType() { return ST_INCREMENT; };
		virtual ExpressionType getExpressionType() { return ET_INCREMENT; };
		virtual string toString() { return "<Increment>\\n" + _operator._str.to_string(); };
		virtual vector<Node*> getChildren();

		bool isIncrement() { return _isInc; }
		void setOperator(Operator op) { _operator = op; }
		void setExpression(Expression* expression) { _expression = expression; }
		Operator getOperator() { return _operator; }
		Expression* getExpression() { return _expression; }
	};

	class FunctionCall : public ExpressionStatement
	{
		Expression* _functionId;
		Expression* _arguments;

	public:
		FunctionCall() : ExpressionStatement() {};
		virtual ~FunctionCall() { if (_functionId) delete _functionId; if (_arguments) delete _arguments; }
		virtual StatementType getStatementType() { return ST_FUNCTION_CALL; };
		virtual ExpressionType getExpressionType() { return ET_FUNCTION_CALL; };
		virtual string toString() { return string() + "<FunctionCall>\\n"; };
		virtual vector<Node*> getChildren();

		void setFunctionId(Expression* funcId) { _functionId = funcId; }
		void setArguments(Expression* arguments) { _arguments = arguments; }

		Expression* getFunctionId() { return _functionId; }
		Expression* getArguments() { return _arguments; }
	};

	class VariableDeclaration : public ExpressionStatement
	{
		Expression* _type;
		unicode_string _varId;

	public:
		VariableDeclaration() : ExpressionStatement() {};
		virtual ~VariableDeclaration() { }
		virtual bool isDeclaration() { return true; }
		virtual StatementType getStatementType() { return ST_VARIABLE_DECLARATION; };
		virtual ExpressionType getExpressionType() { return ET_VARIABLE_DECLARATION; };
		virtual string toString() {
			return "<VariableDeclaration>\\n" + _varId.to_string();
		};
		virtual vector<Node*> getChildren();

		void setVarId(unicode_string varId) { _varId = varId; }
		void setType(Expression* type) { _type = type; }
		unicode_string &getVarId() { return _varId; }
		Expression* getVarType() { return _type; }
	};

	/********************** Statements **********************/

	class StatementBlock : public Statement
	{
		vector<Statement*> _statements;
		// string _fileName; // file the block is in (only used if the block is top-level)

	public:
		// StatementBlock() : Statement() { _fileName = ""; };
		StatementBlock() : Statement() { };
		virtual ~StatementBlock() { _statements.clear(); }
		virtual StatementType getStatementType() { return ST_STATEMENT_BLOCK; };
		virtual string toString() { return "<StatementBlock>"; };
		virtual vector<Node*> getChildren();

		// void setFile(string fileName) { _fileName = fileName; }
		// string getFile() { return _fileName; }
		vector<Statement*> getStatements() { return _statements; }
		void addStatement(Statement* statement) { 
			if (statement && statement->getStatementType() == ST_STATEMENT_BLOCK)
				for (auto i : ((StatementBlock*)statement)->_statements)
					_statements.push_back(i);
			else _statements.push_back(statement); 
		}
	};

	class IfThenElse : public Statement
	{
		Expression* _condition;
		StatementBlock* _thenBranch;
		StatementBlock* _elseBranch;

	public:
		IfThenElse() : Statement() {};
		virtual ~IfThenElse() { if (_condition) delete _condition; if (_thenBranch) delete _thenBranch; if (_elseBranch) delete _elseBranch; }
		virtual StatementType getStatementType() { return ST_IF_THEN_ELSE; };
		virtual string toString() { return "<IfThenElse>"; };
		virtual vector<Node*> getChildren();

		void setCondition(Expression* condition) { _condition = condition; }
		void setThenBranch(StatementBlock* thenBranch) { _thenBranch = thenBranch; }
		void setElseBranch(StatementBlock* elseBranch) { _elseBranch = elseBranch; }

		Expression* getCondition() { return _condition; }
		StatementBlock* getThenBranch() { return _thenBranch; }
		StatementBlock* getElseBranch() { return _elseBranch; }
	};

	class TryCatch : public Statement
	{
		StatementBlock* _tryBlock;
		StatementBlock* _catchBlock;

	public:
		virtual ~TryCatch() { if (_tryBlock) delete _tryBlock; if (_catchBlock) delete _catchBlock; }
		virtual StatementType getStatementType() { return ST_TRY_CATCH; };
		virtual string toString() { return "<TryCatch>"; };
		virtual vector<Node*> getChildren() { return { _tryBlock, _catchBlock }; };

		void setTryBlock(StatementBlock* tryBlock) { _tryBlock = tryBlock; }
		void setCatchBlock(StatementBlock* catchBlock) { _catchBlock = catchBlock; }
	
		StatementBlock* getTryBlock() { return _tryBlock; }
		StatementBlock* getCatchBlock() { return _catchBlock; }
	};

	class Import : public Statement 
	{
		string _toImport;
	public:
		Import(string toImport) : _toImport(toImport) {}
		string getImportPath() { return _toImport; }
		virtual StatementType getStatementType() { return ST_IMPORT; };
		virtual string toString() { return "<Import>\\n\"" + _toImport + "\""; };
		virtual vector<Node*> getChildren() { return {}; };
	};

	typedef struct CaseClause {
		Expression* _expression;
		StatementBlock* _statement;
	} CaseClause;

	class SwitchCase : public Statement
	{
		Expression* _expression;
		vector<CaseClause> _cases;
		StatementBlock* _default;

	public:
		SwitchCase() : Statement(), _expression(NULL), _default(NULL) {};
		virtual ~SwitchCase() { if (_expression) delete _expression; if (_default) delete _default; _cases.clear(); }

		virtual StatementType getStatementType() { return ST_SWITCH; };
		virtual string toString() { return "<Switch>"; };
		virtual vector<Node*> getChildren();

		void setExpression(Expression* expression) { _expression = expression; }
		void addCase(Expression* expression, StatementBlock* statement) { _cases.push_back({ expression, statement }); }
		void setDefault(StatementBlock* statement) {
			if (_default) throw ErrorReporter::report("'default' clause may only be set once", ErrorReporter::GENERAL_ERROR, statement->getPosition());;
			_default = statement; 
		}
		Expression* getExpression() { return _expression; }
		vector<CaseClause> getCases() { return _cases; }
		StatementBlock* getDefault() { return _default; }
	};

	class WhileLoop : public Statement
	{
		Expression* _condition;
		StatementBlock* _statement;

	public:
		WhileLoop() : Statement() {};
		virtual ~WhileLoop() { if (_condition) delete _condition; if (_statement) delete _statement; }
		virtual StatementType getStatementType() { return ST_WHILE_LOOP; };
		virtual string toString() { return "<While>"; };
		virtual vector<Node*> getChildren();

		void setCondition(Expression* condition) { _condition = condition; }
		void setStatement(StatementBlock* statement) { _statement = statement; }

		Expression* getCondition() { return _condition; }
		StatementBlock* getStatement() { return _statement; }
	};

	class ForLoop : public Statement
	{
		Statement* _begin;
		Expression* _condition;
		Statement* _increment;
		StatementBlock* _statement;

	public:
		ForLoop() : Statement() {};
		virtual ~ForLoop() { 	if (_condition) delete _condition; if (_begin) delete _begin; 
								if (_increment) delete _increment; if (_statement) delete _statement;}
		virtual StatementType getStatementType() { return ST_FOR_LOOP; };
		virtual string toString() { return "<For>"; };
		virtual vector<Node*> getChildren();

		void setCondition(Expression* condition) { _condition = condition; }
		void setStatement(StatementBlock* statement) { _statement = statement; }
		void setBegin(Statement* begin) { _begin = begin; }
		void setIncrement(Statement* increment) { _increment = increment; }

		Expression* getCondition() { return _condition; }
		StatementBlock* getStatement() { return _statement; }
		Statement* getBegin() { return _begin; }
		Statement* getIncrement() { return _increment; }
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
		UnaryOperationStatement() : _expression(NULL) {};
		virtual ~UnaryOperationStatement() { if (_expression) delete _expression; }
		virtual StatementType getStatementType() { return ST_UNARY_OPERATION; };
		virtual string toString() { return string() + "<UnaryOperationStatement>\\n" + _operator._str.to_string(); };
		virtual vector<Node*> getChildren();

		void setOperator(Operator op) { _operator = op; }
		void setExpression(Expression* expression) { _expression = expression; }

		Operator getOperator() { return _operator; }
		Expression* getExpression() { return _expression; }
	};

	class ConstDeclaration : public Statement
	{
		unicode_string _name;
		Expression* _expression;

	public:
		virtual ~ConstDeclaration() { if (_expression) delete _expression; }
		virtual StatementType getStatementType() { return ST_CONST_DECLARATION; };
		virtual string toString() { return string() + "<ConstDeclaration>\\n" + _name.to_string(); };
		virtual vector<Node*> getChildren();
		virtual bool isDeclaration() { return true; }


		void setName(unicode_string name) { _name = name; }
		void setExpression(Expression* expression) { _expression = expression; }

		unicode_string getName() { return _name; }
		Expression* getExpression() { return _expression; }
	};

	class FunctionDeclaration : public Statement
	{
	private:
		VariableDeclaration* _decl;
		vector<VariableDeclaration*> _parameters;
		StatementBlock* _content;
		vector<Expression*> _generics;
		

	public:
		FunctionDeclaration(VariableDeclaration* decl) : _content(NULL) { _decl = decl; };
		virtual ~FunctionDeclaration() { if (_decl) delete _decl; if (_content) delete _content; _parameters.clear(); }
		void setGenerics(Expression* generics);
		vector<Expression*> getGenerics() { return _generics; }
		virtual bool isDeclaration() { return true; }
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

	class PropertyDeclaration;
	class ExpressionList;

	class InterfaceDeclaration : public Statement
	{
		unicode_string _name;
		ExpressionList* _implements;
		vector<PropertyDeclaration*> _properties;
		vector<FunctionDeclaration*> _functions;

	public:
		InterfaceDeclaration() : _implements(NULL) {};
		InterfaceDeclaration(unicode_string name) : _name(name), _implements(NULL) {};
		virtual ~InterfaceDeclaration();
		virtual bool isDeclaration() { return true; }
		virtual StatementType getStatementType() { return ST_INTERFACE_DECLARATION; };
		virtual string toString();
		virtual vector<Node*> getChildren();

		unicode_string getName() { return _name; }
		ExpressionList* getImplements() { return _implements; }
		vector<PropertyDeclaration*> getProperties() { return _properties; }
		vector<FunctionDeclaration*> getFunctions() { return _functions; }

		void setImplements(ExpressionList *implements) { _implements = implements; }
		void setName(unicode_string id) { _name = id; }
		//void addImplements(Expression *interface) { _implements->addExpression(interface); }
		void addProperty(PropertyDeclaration* property) { _properties.push_back(property); }
		void addFunction(FunctionDeclaration* function);
	};

	class PropertyDeclaration : public Statement {
	private:
		VariableDeclaration* _decl;
		StatementBlock* _get;
		StatementBlock* _set;
		bool _hasGet;
		bool _hasSet;
	public:
		PropertyDeclaration(VariableDeclaration* decl) : _get(NULL), _set(NULL), _hasGet(false), _hasSet(false) { _decl = decl; };
		virtual ~PropertyDeclaration() { if (_decl) delete _decl; if (_get) delete _get; if (_set) delete _set;}
		virtual bool isDeclaration() { return true; }
		virtual StatementType getStatementType() { return ST_PROPERTY_DECLARATION; };
		virtual string toString() { return string("<PropertyDeclaration>\\n") + (_hasGet && _hasSet ? "get | set" : _hasSet ? "set" : _hasGet ? "get" : ""); };
		virtual vector<Node*> getChildren();

		void setGet(StatementBlock* get) { _get = get; _hasGet = true; }
		void setSet(StatementBlock* set) { _set = set; _hasSet = true; }
		bool hasGet() { return _hasGet; }
		bool hasSet() { return _hasSet; }
		StatementBlock* getGet() { return _get; }
		StatementBlock* getSet() { return _set; }
		VariableDeclaration* getVarDecl() { return _decl; }
	};

	class TypeDeclaration : public Statement
	{
		unicode_string _name;
		ExpressionList* _interfaces;
		vector<VariableDeclaration*> _variableDeclarations;
		vector<FunctionDeclaration*> _functionDeclarations;
		vector<PropertyDeclaration*> _propertyDeclarations;

	public:
		TypeDeclaration() : _interfaces(NULL) {};
		virtual ~TypeDeclaration();
		virtual bool isDeclaration() { return true; }
		virtual StatementType getStatementType() { return ST_TYPE_DECLARATION; };
		virtual string toString();
		virtual vector<Node*> getChildren();

		unicode_string &getName() { return _name; }
		ExpressionList* getInterfaces() { return _interfaces; }
		vector<VariableDeclaration*> getVariableDeclarations() { return _variableDeclarations; }
		vector<FunctionDeclaration*> getFunctionDeclarations() { return _functionDeclarations; }
		vector<PropertyDeclaration*> getPropertyDeclarations() { return _propertyDeclarations; }

		void setName(unicode_string id) { _name = id; }
		void setInterfaces(ExpressionList* interfaces) { _interfaces = interfaces; }
		void addVariableDeclaration(VariableDeclaration* variableDeclaration) { _variableDeclarations.push_back(variableDeclaration); }
		void addFunctionDeclaration(FunctionDeclaration* functionDeclaration) { _functionDeclarations.push_back(functionDeclaration); }
		void addPropertyDeclaration(PropertyDeclaration* propertyDeclaration) { _propertyDeclarations.push_back(propertyDeclaration); }
	};

	class TypeExtension : public Statement
	{
		unicode_string _name;
		ExpressionList* _interfaces;
		vector<VariableDeclaration*> _variableDeclarations;
		vector<FunctionDeclaration*> _functionDeclarations;
		vector<PropertyDeclaration*> _propertyDeclarations;

	public:
		TypeExtension() : _interfaces(NULL) {};
		virtual ~TypeExtension();
		virtual bool isDeclaration() { return true; }
		virtual StatementType getStatementType() { return ST_TYPE_DECLARATION; };
		virtual string toString();
		virtual vector<Node*> getChildren();

		unicode_string &getName() { return _name; }
		ExpressionList* getInterfaces() { return _interfaces; }
		vector<VariableDeclaration*> getVariableDeclarations() { return _variableDeclarations; }
		vector<FunctionDeclaration*> getFunctionDeclarations() { return _functionDeclarations; }
		vector<PropertyDeclaration*> getPropertyDeclarations() { return _propertyDeclarations; }

		void setName(unicode_string id) { _name = id; }
		void setInterfaces(ExpressionList* interfaces) { _interfaces = interfaces; }
		void addVariableDeclaration(VariableDeclaration* variableDeclaration) { _variableDeclarations.push_back(variableDeclaration); }
		void addFunctionDeclaration(FunctionDeclaration* functionDeclaration) { _functionDeclarations.push_back(functionDeclaration); }
		void addPropertyDeclaration(PropertyDeclaration* propertyDeclaration) { _propertyDeclarations.push_back(propertyDeclaration); }
	};
	
	class Literal;

	typedef struct EnumMember {
		unicode_string id;
		Literal *val;
	} EnumMember;

	class EnumDeclaration : public Statement 
	{
		unicode_string _name;
		Expression *_type;
		vector<EnumMember> _members;

	public:
		EnumDeclaration(unicode_string name) : _name(name), _type(NULL) {}
		void addMember(unicode_string id, Literal *val = NULL) { _members.push_back({id, val}); }
		void addMember(EnumMember member) { _members.push_back(member); }
		void setType(Expression *type) { _type = type; }
		Expression *getType() { return _type; }
		unicode_string &getName() { return _name; }
		vector<EnumMember> getMembers() { return _members; }
		virtual bool isDeclaration() { return true; }

		virtual StatementType getStatementType() { return ST_ENUM_DECLARATION; };
		virtual string toString() { return "<EnumDeclaration>\\n" + _name.to_string(); };
		virtual vector<Node*> getChildren() { TODO };
	};

	class NamespaceDeclaration : public Statement
	{
		unicode_string _name;
		StatementBlock* _statement;

	public:
		NamespaceDeclaration() { _name = ""; };
		virtual ~NamespaceDeclaration() { if (_statement) delete _statement; }
		virtual bool isDeclaration() { return true; }
		virtual StatementType getStatementType() { return ST_NAMESPACE_DECLARATION; };
		virtual string toString() { return "<NamespaceDeclaration>\\n" + _name.to_string(); };
		virtual vector<Node*> getChildren();


		unicode_string getName() { return _name; }
		StatementBlock* getStatement() { return _statement; }

		void setName(unicode_string id) { _name = id; }
		void setStatement(StatementBlock* statement) { _statement = statement; }
	};

	/********************** Expressions **********************/

	class ExpressionList : public Expression
	{
		vector<Expression*> _expressions;
	public:
		ExpressionList() : Expression() {};
		virtual ~ExpressionList() { _expressions.clear(); }
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
		BinaryOperation() : Expression(), _left(NULL), _right(NULL) {};
		virtual ~BinaryOperation() { if (_right) delete _right; if (_left) delete _left; }
		virtual ExpressionType getExpressionType() { return ET_BINARY_OPERATION; };
		virtual string toString() { return string() + "<BinaryOperator>\\n" + _operator._str.to_string(); };
		virtual vector<Node*> getChildren();

		void setOperator(Operator op) { _operator = op; }
		void setLeft(Expression* left) { _left = left; }
		void setRight(Expression* right) { _right = right; }

		Expression* getLeft() { return _left; }
		Expression* getRight() { return _right; }
		Operator getOperator() { return _operator; }
	};

	class Comparison : public Expression
	{
		vector<Operator> _operators;
		vector<Expression*> _expressions;

	public:
		Comparison() : Expression() {};
		virtual ~Comparison() {  }
		virtual ExpressionType getExpressionType() { return ET_COMPARISON; };
		virtual string toString() { return string() + "<Comparison>\\nTODO"; };
		virtual vector<Node*> getChildren() 
		{
			vector<Node*> ret;
			for (auto i : _expressions)
				ret.push_back(i);
			return ret;
		}

		void addOperator(Operator op) { _operators.push_back(op); }
		void addExpression(Expression *exp) { _expressions.push_back(exp); }

		vector<Operator> getOperators() { return _operators; }
		vector<Expression*> getExpressions() { return _expressions; }
	};

	class UnaryOperation : public Expression
	{
		Operator _operator;
		Expression* _expression;
		bool _isPostfix;

	public:
		UnaryOperation() : Expression() { _isPostfix = false; };
		virtual ~UnaryOperation() { if (_expression) delete _expression; }
		virtual ExpressionType getExpressionType() { return ET_UNARY_OPERATION; };
		virtual string toString() { return string() + "<" + (_isPostfix ? "Postfix" : "")+ "UnaryOperator>\\n" + _operator._str.to_string(); };
		virtual vector<Node*> getChildren();

		void setOperator(Operator op) { _operator = op; }
		void setExpression(Expression* expression) { _expression = expression; }
		void setIsPostfix(bool isPostfix) { _isPostfix = isPostfix; }

		Operator getOperator() { return _operator; }
		Expression* getExpression() { return _expression; }
		bool isPostfix() { return _isPostfix; }
	};

	class Identifier : public Expression
	{
		unicode_string _varId;
	public:
		Identifier(unicode_string varId) : Expression() { _varId = varId; };
		Identifier() : Expression() {};
		void setVarId(unicode_string varId) { _varId = varId; }
		unicode_string &getVarId() { return _varId; }
		virtual ExpressionType getExpressionType() { return ET_IDENTIFIER; };
		virtual string toString() { return "<Identifier>\\n" + _varId.to_string(); };
		virtual vector<Node*> getChildren();
	};

	class ConditionalExpression : public Expression
	{
		Expression* _condition;
		Expression* _thenBranch;
		Expression* _elseBranch;

	public:
		ConditionalExpression() : Expression() {};
		virtual ~ConditionalExpression() { if (_condition) delete _condition; if (_thenBranch) delete _thenBranch; if (_elseBranch) delete _elseBranch; }
		virtual ExpressionType getExpressionType() { return ET_CONDITIONAL_EXPRESSION; };
		virtual string toString() { return string() + "<ConditionaExpression>"; };
		virtual vector<Node*> getChildren();

		void setCondition(Expression* condition) { _condition = condition; }
		void setThenBranch(Expression* thenBranch) { _thenBranch = thenBranch; }
		void setElseBranch(Expression* elseBranch) { _elseBranch = elseBranch; }

		Expression* getCondition() { return _condition; }
		Expression* getThenBranch() { return _thenBranch; }
		Expression* getElseBranch() { return _elseBranch; }

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
		virtual string toShortString() = 0;
	};

	class Boolean : public Literal
	{
		bool _value;
	public:
		Boolean(bool value) : Literal(LT_BOOLEAN) { _value = value; }
		virtual string toString() { return string() + "<BoolLiteral>\\n" + std::to_string(_value); };
		bool getValue() { return _value; }
		virtual string toShortString() { return _value ? "true" : "false"; }
	};

	class Integer : public Literal
	{
		int _value;
	public:
		Integer(int value) : Literal(LT_INTEGER) { _value = value; }
		virtual string toString() { return string() + "<IntegerLiteral>\\n" + std::to_string(_value); };
		int getValue() { return _value; }
		virtual string toShortString() { return std::to_string(_value); }
	};

	class Fraction : public Literal
	{
		float _value;
	public:
		Fraction(float value) : Literal(LT_FRACTION) { _value = value; }
		virtual string toString() { return string() + "<FracLiteral>\\n" + std::to_string(_value); };
		float getValue() { return _value; }
		virtual string toShortString() { return std::to_string(_value); }
	};
	
	class Character : public Literal
	{
		unicode_char _value;
	public:
		Character(unicode_char value) : Literal(LT_CHARACTER) { _value = value; }
		virtual string toString() { return string() + "<CharLiteral>\\n'" + _value.to_string() + '\''; };
		unicode_char getValue() { return _value; }
		virtual string toShortString() { return _value.to_string(); }
	};

	class String : public Literal
	{
		string _value;
	public:
		String(string value) : Literal(LT_STRING) { _value = value; }
		virtual string toString() { return string() + "<StringLiteral>\\n" + _value; };
		string getValue() { return _value; }
		virtual string toShortString() { return _value; }
	};

	class Function : public Literal
	{
		vector<VariableDeclaration*> _parameters;
		StatementBlock* _content;
		Expression* _returnType;
		ExpressionList* _generics;

	public:
		Function() : Literal(LT_FUNCTION), _content(NULL), _returnType(NULL), _generics(NULL) { }
		virtual ~Function() { if (_content) delete _content; _parameters.clear(); }
		virtual string toString() { return string() + "<FunctionLiteral>"; };
		virtual vector<Node*> getChildren();

		void setGenerics(ExpressionList* generics) { _generics = generics; }
		ExpressionList* getGenerics() { return _generics; }
		void addParameters(Expression* parameters);
		void addParameterToStart(VariableDeclaration* parameter) { _parameters.insert(_parameters.begin(), parameter); }
		void setContent(StatementBlock* content) { _content = content; }
		void setReturnType(Expression* type) { _returnType = type; }

		vector<VariableDeclaration*> getParameters() { return _parameters; }
		StatementBlock* getContent() { return _content; }
		Expression* getReturnType() { return _returnType; }
		virtual string toShortString() { return "TODO"; }
	};

	class Null : public Literal
	{
	public:
		Null() : Literal(LT_NULL) {}
		virtual string toString() { return string() + "<NullLiteral>"; };
		virtual string toShortString() { return "null"; }
	};

}