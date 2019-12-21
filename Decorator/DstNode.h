#pragma once

#include "../Parser/AstNode.h"

namespace DST
{
	static const int UNKNOWN_ARRAY_LENGTH = 0;
	static int idCount = 0;

	class BasicType;
	static BasicType *typeidTypePtr;

	void setup();

	class Node
	{
		unsigned int _nodeId;	// defined for purpose of the graphic view of the AST.
		int _line;
	public:
		/* Default constructor, does NOT set line.*/
		Node() { _nodeId = idCount++; };

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
		//const unsigned int getNodeId() const { return (this == nullptr) ? -1 : _nodeId; };
		const unsigned int getNodeId() const { return _nodeId; };

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
		virtual Type *getType();
		virtual ExactType getExactType() = 0;
		virtual ExpressionType getExpressionType() { return ET_TYPE; }
		virtual bool equals(Type *other) = 0;
		virtual bool readable() { return true; }
		virtual bool writeable() { return true; }

		virtual string toShortString() = 0;
		virtual string toString() { return "<Type>"; };
		virtual vector<Node*> getChildren();
	};
	
	class PropertyType : public Type
	{
		Type *_return;
		bool _hasGet;
		bool _hasSet;

	public:
		PropertyType(Type *ret, bool hasGet, bool hasSet) : _return(ret), _hasGet(hasGet), _hasSet(hasSet), Type(NULL) {}
		bool hasGet() { return _hasGet; }
		bool hasSet() { return _hasSet; }
		Type *getReturn() { return _return; }

		ExactType getExactType() { return EXACT_PROPERTY; }
		virtual bool equals(Type *other);

		virtual bool readable() { return hasGet(); }
		virtual bool writeable() { return hasSet(); }

		virtual string toShortString();
		virtual string toString() { return "<FunctionType>" + toShortString(); };
		virtual vector<Node*> getChildren() { return vector<Node*>(); };
	};

	class TypeList : public Type
	{
		vector<Type*> _types;
	public:
		TypeList(AST::Expression *base) : Type(base) { }
		void addType(Type *type) { _types.push_back(type); }
		vector<Type*> getTypes() { return _types; }
		size_t size() { return _types.size(); }
		ExactType getExactType() { return EXACT_TYPELIST; }
		virtual bool equals(Type *other);

		virtual string toShortString();
		virtual string toString() { return "<FunctionType>" + toShortString(); };
		virtual vector<Node*> getChildren();
	};
	
	class BasicType : public Type
	{
		unicode_string _typeId;
		unordered_map<unicode_string, Type*, UnicodeHasherFunction> _members;
	public:
		BasicType(AST::Expression *base) : Type(base) { _typeId = dynamic_cast<AST::Identifier*>(_base)->getVarId(); }
		BasicType(unicode_string typeId) : Type(NULL), _typeId(typeId) { }
		unicode_string getTypeId() { return _typeId; }
		ExactType getExactType() { return EXACT_BASIC; }
		virtual bool equals(Type *other)
		{
			return 
				(other->getExactType() == EXACT_PROPERTY && equals(((PropertyType*)other)->getReturn())) ||
				(other->getExactType() == EXACT_BASIC && ((BasicType*)other)->_typeId == _typeId) ||
				(other->getExactType() == EXACT_TYPELIST && ((TypeList*)other)->size() == 1 && equals(((TypeList*)other)->getTypes()[0]));
		}
		void addMember(unicode_string id, Type* type)  { _members[id] = type; }
		Type *getMember(unicode_string id) { return _members[id]; }

		virtual string toShortString() { return _typeId.to_string(); };
		virtual string toString() { return "<BasicType>\\n" + _typeId.to_string(); };
		virtual vector<Node*> getChildren();
	};

	class ArrayType : public Type
	{
		Type *_valueType;
		size_t _length;	// size = 0 means size is unknown.

	public:
		ArrayType(AST::Expression *base) : Type(base) {  }
		ArrayType(Type *valueType, size_t length) : Type(NULL), _valueType(valueType), _length(length) { }
		ArrayType(Type *valueType) : ArrayType(valueType, 0) {}
		ExactType getExactType() { return EXACT_ARRAY; }

		virtual bool equals(Type *other)
		{
			return other->getExactType() == EXACT_ARRAY &&
				((ArrayType*)other)->_length == _length &&
				((ArrayType*)other)->_valueType->equals(_valueType);
		}
		size_t getLength() { return _length; }
		Type *getType() { return _valueType; }

		virtual string toShortString() { return _valueType->toShortString() + "[" + ((_length) ? std::to_string(_length) : "") + "]"; };
		virtual string toString() { return "<ArrayType>\\n" + toShortString(); };
		virtual vector<Node*> getChildren();
	};

	class FunctionType : public Type
	{
		TypeList *_returns;
		TypeList *_parameters;

	public:
		FunctionType() : Type(NULL) { _returns = new TypeList(NULL); _parameters = new TypeList(NULL); }
		FunctionType(AST::FunctionCall *base) : Type(base) { _returns = new TypeList(base->getFunctionId()); _parameters = new TypeList(base->getArguments()); }
		void addReturn(Type *returnType) { _returns->addType(returnType); }
		void addParameter(Type *parameterType) { _parameters->addType(parameterType); }
		TypeList *getReturns() { return _returns; }
		TypeList *getParameters() { return _parameters; }

		ExactType getExactType() { return EXACT_FUNCTION; }
		virtual bool equals(Type *other);

		virtual string toShortString();
		virtual string toString() { return "<FunctionType>" + toShortString(); };
		virtual vector<Node*> getChildren();
	};

	class Variable : public Expression
	{
		AST::Identifier *_base;
		Type *_type;

	public:
		Variable(AST::Identifier *base, Type *type) : _base(base), _type(type) {};
		void setType(Type *type) { _type = type; };
		virtual Type *getType() { return _type; }
		virtual ExpressionType getExpressionType() { return ET_IDENTIFIER; }

		virtual string toString() { return _base->toString() + "\nType: " + _type->toShortString(); };
		virtual vector<Node*> getChildren();
		unicode_string getVarId() { return _base->getVarId(); }
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
		Operator getOperator() { return _base->getOperator(); }
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
		AST::Literal *getBase() { return _base; }
		void setType(Type *type) { _type = type; }
		Type *getType() { return _type; }
		void *getValue();
		LiteralType getLiteralType() { return _base->getLiteralType(); }

		virtual ExpressionType getExpressionType() { return ET_LITERAL; }

		virtual string toString() { return _base->toString() + "\nType: " + _type->toShortString(); };
		virtual vector<Node*> getChildren();
	};

	class Conversion : public Expression
	{
		AST::FunctionCall *_base;
		Type *_type;
		Expression *_expression;
	public: 
		Conversion(AST::FunctionCall *base, Type *type, Expression *expression) : _base(base), _type(type), _expression(expression) {}
		void setType(Type *type) { _type = type; }
		Type *getType() { return _type; }
		virtual ExpressionType getExpressionType() { return ET_CONVERSION; }

		virtual string toString() { return "<Conversion>\nType: " + _type->toShortString(); };
		virtual vector<Node*> getChildren();
	};

	class StatementBlock;
	class VariableDeclaration;

	class FunctionLiteral : public Expression
	{
		AST::Function *_base;
		FunctionType *_type;
		vector<VariableDeclaration*> _parameters;
		StatementBlock *_content;

	public:
		FunctionLiteral(AST::Function* base) : _base(base) {}
		void setType(FunctionType *type) { _type = type; }
		Type *getType() { return _type; }
		virtual ExpressionType getExpressionType() { return ET_FUNCTION_LITERAL; }

		virtual string toString() { return _base->toString() + "\nType: " + _type->toShortString(); };
		virtual vector<Node*> getChildren();

		void addParameter(VariableDeclaration* parameter) { _parameters.push_back(parameter); }
		void addParameterToStart(VariableDeclaration* parameter) { _parameters.insert(_parameters.begin(), parameter); }
		void setContent(StatementBlock* content) { _content = content; }

		vector<VariableDeclaration*> getParameters() { return _parameters; }
		StatementBlock* getContent() { return _content; }
	};

	class ExpressionList : public Expression
	{
		TypeList *_type;
		AST::Expression *_base;
		vector<Expression*> _expressions;
	public:
		ExpressionList(AST::Expression *base) : _base(base) { _type = new TypeList(base); };
		TypeList *getType() { return _type; }
		virtual ExpressionType getExpressionType() { return ET_LIST; };
		virtual string toString() { return "<ExpressionList>\\nType: " + _type->toShortString(); };
		virtual vector<Node*> getChildren();

		void addExpression(Expression* expression) { _expressions.push_back(expression); _type->addType(expression->getType()); };
		vector<Expression*> getExpressions() { return _expressions; }
	};

	class ArrayLiteral : public Expression
	{
		ArrayType *_type;
		vector<Expression*> _array;

	public:
		ArrayLiteral(Type *type, vector<Expression*> arr) : _array(arr) { _type = new ArrayType(type, arr.size()); }
		ArrayType *getType() { return _type; }
		virtual ExpressionType getExpressionType() { return ET_ARRAY; };
		virtual string toString() { return "<ArrayLiteral>\\nType: " + _type->toShortString(); };
		virtual vector<Node*> getChildren();


		void addValue(Expression* value) { _array.push_back(value); };
		vector<Expression*> getArray() { return _array; }
	};

	/********************** Statements **********************/

	class StatementBlock : public Statement
	{
		vector<Statement*> _statements;
	public:
		virtual StatementType getStatementType() { return ST_STATEMENT_BLOCK; };

		bool hasReturnType(Type *returnType);

		vector<Statement*> getStatements() { return _statements; }
		void addStatement(Statement* statement) { _statements.push_back(statement); }

		virtual string toString() { return "<StatementBlock>"; };
		virtual vector<Node*> getChildren();
	};

	class IfThenElse : public Statement
	{
		AST::IfThenElse *_base;
		Expression* _condition;
		StatementBlock *_thenBranch;
		StatementBlock* _elseBranch;

	public:
		IfThenElse(AST::IfThenElse *base) : _base(base) {};
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

	typedef struct CaseClause {
		Expression* _expression;
		StatementBlock* _statement;
	} CaseClause;

	class SwitchCase : public Statement {
		AST::SwitchCase *_base;
		Expression* _expression;
		vector<CaseClause> _cases;
		StatementBlock* _default;

	public:
		SwitchCase(AST::SwitchCase *base) : _base(base), _default(NULL) {};
		virtual StatementType getStatementType() { return ST_SWITCH; };
		virtual string toString() { return "<Switch>"; };
		virtual vector<Node*> getChildren();

		void setExpression(Expression* expression) { _expression = expression; }
		void addCase(Expression* expression, StatementBlock* statement) { _cases.push_back({ expression, statement }); }
		void setDefault(StatementBlock* statement) {
			if (_default) throw DinoException("'default' clause may only be set once", EXT_GENERAL, statement->getLine());
			_default = statement;
		}
		Expression* getExpression() { return _expression; }
		vector<CaseClause> getCases() { return _cases; }
		StatementBlock* getDefault() { return _default; }

	};

	class WhileLoop : public Statement
	{
		AST::WhileLoop *_base;
		Expression* _condition;
		StatementBlock* _statement;

	public:
		WhileLoop(AST::WhileLoop *base) : _base(base) {};
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
		AST::ForLoop *_base;
		Statement* _begin;
		Expression* _condition;
		Statement* _increment;
		StatementBlock* _statement;

	public:
		ForLoop(AST::ForLoop *base) : _base(base) {};
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
		DoWhileLoop(AST::DoWhileLoop *base) : WhileLoop(base) {};
		virtual StatementType getStatementType() { return ST_DO_WHILE_LOOP; };
		virtual string toString() { return "<Do>"; };
	};

	class UnaryOperationStatement : public Statement
	{
		AST::UnaryOperationStatement *_base;
		Expression* _expression;

	public:
		UnaryOperationStatement(AST::UnaryOperationStatement *base, Expression *expression) : _base(base), _expression(expression) {};
		virtual StatementType getStatementType() { return ST_UNARY_OPERATION; };
		virtual string toString() { return _base->toString(); };
		virtual vector<Node*> getChildren();

		void setExpression(Expression* expression) { _expression = expression; }

		Operator getOperator() { return _base->getOperator(); }
		Expression* getExpression() { return _expression; }
	};

	class TypeDeclaration : public Statement 
	{
		unicode_string _name;
		AST::TypeDeclaration *_base;
		//vector<InterfaceDeclaration*> _interfaces;
		unordered_map<unicode_string, pair<Statement*, Type*>, UnicodeHasherFunction> _decls;
	public:
		TypeDeclaration(unicode_string name) : _name(name), _base(NULL) {}
		TypeDeclaration(AST::TypeDeclaration *base) : _name(base->getName()), _base(base) {}
		
		void addDeclaration(Statement *decl, Type *type);
		Statement* getDeclaration(unicode_string id)  { return _decls[id].first; }
		Type* getMemberType(unicode_string id) { return _decls[id].second; }

		virtual StatementType getStatementType() { return ST_TYPE_DECLARATION; };
		virtual bool isDeclaration() { return true; }

		unicode_string getName() { return _name; }
		virtual string toString() { return "<TypeDeclaration>\\n" + _name.to_string(); };
		virtual vector<Node*> getChildren();
	};

	class VariableDeclaration;

	class FunctionDeclaration : public Statement
	{
		AST::FunctionDeclaration *_base;
		VariableDeclaration *_decl;
		vector<VariableDeclaration*> _parameters;
		StatementBlock *_content;

	public:
		FunctionDeclaration(AST::FunctionDeclaration *base, VariableDeclaration *decl) : _base(base), _decl(decl){};
		virtual bool isDeclaration() { return true; }
		virtual StatementType getStatementType() { return ST_FUNCTION_DECLARATION; };
		virtual string toString() { return "<FunctionDeclaration>"; };
		virtual vector<Node*> getChildren();

		void setVarDecl(VariableDeclaration* decl) { _decl = decl; }
		void addParameter(VariableDeclaration* parameter) { _parameters.push_back(parameter); }
		void addParameterToStart(VariableDeclaration* parameter) { _parameters.insert(_parameters.begin(), parameter); }
		void setContent(StatementBlock* content) { _content = content; }

		VariableDeclaration* getVarDecl() { return _decl; }
		Type *getReturnType();
		vector<VariableDeclaration*> getParameters() { return _parameters; }
		StatementBlock* getContent() { return _content; }
	};

	class PropertyDeclaration : public Statement {
	private:
		AST::PropertyDeclaration *_base;
		Type *_type;
		StatementBlock* _get;
		StatementBlock* _set;
	public:
		PropertyDeclaration(AST::PropertyDeclaration *base, StatementBlock *get, StatementBlock *set, Type *type) : _base(base), _get(get), _set(set), _type(type) {};
		virtual bool isDeclaration() { return true; }
		virtual StatementType getStatementType() { return ST_PROPERTY_DECLARATION; };
		virtual string toString() { return "<PropertyDeclaration>\\n" + _type->toShortString() + " " + _base->getVarDecl()->getVarId().to_string(); };
		virtual vector<Node*> getChildren();
		unicode_string getPropId() { return _base->getVarDecl()->getVarId(); }
		/*void setGet(Statement* get) { _get = get; }
		void setSet(Statement* set) { _set = set; }
		Statement* getGet() { return _get; }
		Statement* getSet() { return _set; }*/
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
		unicode_string getVarId() { return _base->getVarId(); }

		virtual string toString() { return _base->toString() + "\nType: " + _type->toShortString(); };
		virtual vector<Node*> getChildren();
		virtual bool isDeclaration() { return true; }
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

	/*typedef struct FunctionParameter 
	{
		unicode_string name;
		Type *type;
	} FunctionParameter;*/

	class FunctionCall : public ExpressionStatement
	{
		Type *_type;
		AST::FunctionCall *_base;
		Expression *_funcPtr;
		ExpressionList *_arguments;

	public:
		FunctionCall(AST::FunctionCall *base) : _base(base) {};
		FunctionCall(AST::FunctionCall *base, Expression *funcPtr, ExpressionList *arguments);
		virtual StatementType getStatementType() { return ST_FUNCTION_CALL; };
		virtual ExpressionType getExpressionType() { return ET_FUNCTION_CALL; };
		virtual string toString() { return string() + "<FunctionCall>\\nType: " + _type->toShortString(); };
		virtual vector<Node*> getChildren();

		void setFunctionId(Expression* funcId)
		{
			if (funcId->getType()->getExactType() != EXACT_FUNCTION)
				throw DinoException("Cannot invoke expression as function", EXT_GENERAL, funcId->getLine());
			_funcPtr = funcId;
			_type = ((FunctionType*)funcId->getType())->getReturns();
		}
		void setArguments(ExpressionList* arguments) 
		{
			if (!((FunctionType*)_funcPtr->getType())->getParameters()->equals(arguments->getType()))
				throw DinoException("Argument types do not match function parameters", EXT_GENERAL, arguments->getLine());
			_arguments = arguments; 
		}

		Type *getType() { return _type; }
		Expression* getFunctionId() { return _funcPtr; }
		Expression* getParameters() { return _arguments; }
	};
}