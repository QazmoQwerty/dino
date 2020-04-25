#pragma once

#include "../Parser/AstNode.h"
#include <dirent.h>
#include <stack>

using std::stack;

/*
	Decorated Syntax Tree: Abstract Syntax Tree with type info.
*/
namespace DST
{
	static const size_t UNKNOWN_ARRAY_LENGTH = 0;
	static int idCount = 0;

	class BasicType;
	static BasicType *typeidTypePtr;

	class InterfaceDeclaration;
	extern InterfaceDeclaration *_anyInterface;

	/*
		Setup some global variables in the namespace.
		NOTE: function must be called before using any members of this namespace.
	*/
	void setup();

	class Node
	{
		unsigned int _nodeId;	// defined for purpose of the graphic view of the AST.
		int _line;
	public:
		/* Default constructor, does NOT set line.*/
		Node() { _nodeId = idCount++; };

		virtual ~Node() {};

		/* Set the line the node is on */
		void setLine(int line) { _line = line; }

		/* Line the node is on */
		virtual PositionInfo getPosition() const { return PositionInfo{-1, -1, -1, ""}; }

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
		unsigned int getNodeId() const { return _nodeId; };

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

	class NamespaceDeclaration;

	class Program : public Node 
	{
	private:
		unordered_map<unicode_string, NamespaceDeclaration*, UnicodeHasherFunction> _namespaces;
		vector<string> _bcFileImports;
	public:
		virtual bool isStatement() { return false; }
		virtual bool isExpression() { return false; }
		virtual string toString() { return "<Program>"; };
		void addImport(string bcFileName);
		vector<string> getBcFileImports() { return _bcFileImports; }
		virtual vector<Node*> getChildren();

		void addNamespace(NamespaceDeclaration *decl);
		NamespaceDeclaration *getNamespace(unicode_string name) { return _namespaces[name]; }
		unordered_map<unicode_string, NamespaceDeclaration*, UnicodeHasherFunction> getNamespaces() { return _namespaces; }
	};

	/********************** Expressions **********************/

	class Type : public Expression
	{
	protected:
		AST::Expression *_base;
		bool _isReadable;
		bool _isWritable;
		bool _isConst;
	public:
		/* types are readable, writable, and non-constant by default */
		Type(AST::Expression *base) : _base(base) { _isReadable = _isWritable = true; _isConst = false; }
		Type(AST::Expression *base, bool isReadable, bool isWritable) : _base(base) { _isReadable = isReadable;  _isWritable = isWritable; }
		virtual ~Type() { if (_base) delete _base; }
		/*
			The type of the actual type itself is meaningless:
			For example, the expression 'int' has no meaningful type in dino.
			This means this function should NEVER be called.
		*/
		virtual Type *getType();

		/* get the type of type (pointer, basic, array, etc.) */
		virtual ExactType getExactType() = 0;
		virtual ExpressionType getExpressionType() { return ET_TYPE; }

		/* 
			In theory, types are equal if they are EXACTLY the same.
			FIXME - remove this, change the type structure so that each type has a single, unique pointer.
		*/
		virtual bool equals(Type *other) = 0;
		void setNotReadable() { _isReadable = false; }
		void setNotWritable() { _isWritable = false; }
		void setConst() { _isConst = true; }

		/* types are readable by default */
		virtual bool readable() { return _isReadable; }

		/* types are writable by default */
		virtual bool writeable() { return _isWritable; }

		/* types are non-constant by default */
		bool isConst() { return _isConst; }

		/* 
			Taken from the language specification:
			A value x is assignable to a variable of type T ("x is assignable to T") if one of the following conditions applies:
			  * x’s type is identical to T.
			  * x’s type is an interface which implements T.
			  * T is an interface and x is a pointer to a type which implements T.
			  * x is the constant ‘null’  and T is a pointer, function, interface, or array.
			  * T is a setter, which gets a type which x is assignable to.
		*/
		virtual bool assignableTo(Type *type) = 0;

		/* currently, only types with a base AST node have a position */
		virtual PositionInfo getPosition() const { return _base ? _base->getPosition() : PositionInfo{ 0, 0, 0, ""}; }

		/* 
			Short string representation of the type, ready to be pretty-printed. 
			Examples:
				int
				(int, bool)
				void(int, int)
				int { get | set }
				int[20]
		*/ 
		virtual string toShortString() = 0;

		/*
			This is a string to be outputted by AstToFile.h.
			Use toShortString instead for pretty-printing of types.
		*/
		virtual string toString() { return "<Type>"; };
		virtual vector<Node*> getChildren();
	};

	/*
		A placeholder type for variables initialized as 'var'.
		This type should never make it past decoration into the codeGen phase.
	*/
	class UnknownType : public Type
	{
	public:
		UnknownType(AST::Expression *base) : Type(base) {}
		UnknownType() : Type(NULL) {}
		/* 
			Short string representation of the type, ready to be pretty-printed. 
			In other words, always returns "var".
		*/ 
		virtual string toShortString() { return "var"; };
		virtual bool assignableTo(Type *type) { return false; /* 'var' type is not assignable to anything */ }
		virtual ExactType getExactType() { return EXACT_UNKNOWN; };
		virtual bool equals(Type *other) { return other->getExactType() == getExactType(); }
		virtual vector<Node*> getChildren() { return {}; }
	};

	class TypeDeclaration;
	class InterfaceDeclaration;

	/*
		Type for identifiers bound to regular types and interfaces.
	*/
	class TypeSpecifierType : public Type
	{
		private:
			TypeDeclaration *_typeDecl;
			InterfaceDeclaration *_interfaceDecl;
		public:
			TypeSpecifierType(DST::TypeDeclaration *decl) : Type(NULL), _typeDecl(decl), _interfaceDecl(NULL) {}
			TypeSpecifierType(DST::InterfaceDeclaration *decl) : Type(NULL), _typeDecl(NULL), _interfaceDecl(decl) {}
			virtual ~TypeSpecifierType();
			virtual ExactType getExactType() { return EXACT_SPECIFIER; }
			virtual bool equals(Type *other) { return other->getExactType() == getExactType(); };
			virtual bool writeable() { return false; }
			TypeDeclaration *getTypeDecl() { return _typeDecl; }
			InterfaceDeclaration *getInterfaceDecl() { return _interfaceDecl; }
			virtual bool assignableTo(Type *type) { return false; /* type specifiers are not assignable */ }

			unicode_string getTypeName();
			Type *getMemberType(unicode_string name);

			/* 
				Short string representation of the type, ready to be pretty-printed. 
				In other words, always returns "typeid".
			*/ 
			virtual string toShortString() { return "typeid"; };
	};

	class NamespaceDeclaration;

	/*
		Type for identifiers bound to namespaces.
	*/
	class NamespaceType : public Type
	{
		private:
			NamespaceDeclaration *_decl;
		public:
			NamespaceType(DST::NamespaceDeclaration *decl) : Type(NULL), _decl(decl) {}
			virtual ~NamespaceType();
			virtual ExactType getExactType() { return EXACT_NAMESPACE; };
			virtual bool equals(Type *other) { return other->getExactType() == getExactType(); };
			virtual bool writeable() { return false; }
			NamespaceDeclaration *getNamespaceDecl() { return _decl; }

			/* 
				Short string representation of the type, ready to be pretty-printed. 
				Example: "Std"
			*/ 
			virtual string toShortString() { return "namespaceType"; };
			virtual bool assignableTo(Type *type) {  return false; /* namespaces are not assignable */ }
	};
	
	/*
		Type for identifiers bound to properties.
	*/
	class PropertyType : public Type
	{
	protected:
		Type *_return;
		bool _hasGet;
		bool _hasSet;

	public:
		PropertyType(Type *ret, bool hasGet, bool hasSet) : Type(NULL), _return(ret), _hasGet(hasGet), _hasSet(hasSet) {}
		virtual ~PropertyType() { if (_return) delete _return; }
		bool hasGet() { return _hasGet; }
		bool hasSet() { return _hasSet; }

		/* The actual type the property gets/sets */
		Type *getReturn() { return _return; }

		ExactType getExactType() { return EXACT_PROPERTY; }
		virtual bool equals(Type *other);

		/* property is readable if it has a getter */
		virtual bool readable() { return hasGet(); }

		/* property is writeable if it has a setter */
		virtual bool writeable() { return hasSet(); }

		/* 
			Short string representation of the type, ready to be pretty-printed. 
			Example: "int { get | set }"
		*/ 
		virtual string toShortString();
		virtual string toString() { return "<FunctionType>" + toShortString(); };
		virtual vector<Node*> getChildren() { return vector<Node*>(); };

		virtual bool assignableTo(DST::Type *type) { return readable() && _return->assignableTo(type); }
	};

	class PointerType : public Type
	{
		Type* _type;
	public:
		PointerType(AST::Expression *base, Type *ptrTo) : Type(base), _type(ptrTo) { }
		PointerType(Type *ptrTo) : Type(NULL), _type(ptrTo) { }
		virtual ~PointerType() { if (_type) delete _type; }
		Type *getPtrType() { return _type; } // returns what type the pointer points to
		ExactType getExactType() { return EXACT_POINTER; }
		virtual bool equals(Type *other);

		/* 
			Short string representation of the type, ready to be pretty-printed. 
			Example: "int@"
		*/ 
		virtual string toShortString();
		virtual vector<Node*> getChildren();
		virtual string toString() { return "<PointerType>" + toShortString(); };

		virtual bool assignableTo(DST::Type *type);
	};

	class NullType : public Type 
	{
	public:
		NullType() : Type(NULL) {}
		ExactType getExactType() { return EXACT_NULL; }
		virtual bool equals(Type *other) { return other->equals(this); };
		/* 
			Short string representation of the type, ready to be pretty-printed. 
			Always returns "<NullType>"
		*/ 
		virtual string toShortString() { return "<NullType>"; };
		virtual vector<Node*> getChildren() { return {}; };
		virtual string toString() { return toShortString(); };

		virtual bool assignableTo(DST::Type *type);
	};

	class TypeList : public Type
	{
		vector<Type*> _types;
	public:
		TypeList(AST::Expression *base) : Type(base) { }
		TypeList(AST::Expression *base, vector<Type*> types) : Type(base), _types(types) { }
		virtual ~TypeList() { _types.clear(); }
		void addType(Type *type);
		vector<Type*> &getTypes() { return _types; }
		size_t size() { return _types.size(); }
		ExactType getExactType() { return EXACT_TYPELIST; }
		virtual bool equals(Type *other);

		/* 
			Short string representation of the type, ready to be pretty-printed. 
			Example: "(int, bool)"
		*/ 
		virtual string toShortString();
		virtual string toString() { return "<FunctionType>" + toShortString(); };
		virtual vector<Node*> getChildren();

		virtual bool assignableTo(DST::Type *type);
	};

	class BasicType : public Type
	{
		TypeSpecifierType *_typeSpec;
	public:
		BasicType(AST::Expression *base, TypeSpecifierType *typeSpec) : Type(base), _typeSpec(typeSpec) {}
		BasicType(TypeSpecifierType *typeSpec) : Type(NULL), _typeSpec(typeSpec) {}
		virtual ~BasicType() { if (_typeSpec) delete _typeSpec; }

		TypeSpecifierType *getTypeSpecifier() { return _typeSpec; }
		unicode_string getTypeId();
		ExactType getExactType() { return EXACT_BASIC; }
		virtual bool equals(Type *other)
		{
			return 
				(other->getExactType() == EXACT_PROPERTY && equals(((PropertyType*)other)->getReturn())) ||
				(other->getExactType() == EXACT_BASIC && ((BasicType*)other)->_typeSpec == _typeSpec) ||
				(other->getExactType() == EXACT_TYPELIST && ((TypeList*)other)->size() == 1 && equals(((TypeList*)other)->getTypes()[0]));
		}
		Type *getMember(unicode_string id);

		// TODO - someone somewhere is calling this instead of getTypeSpecifier(): find it and remove this
		virtual Type *getType() { return _typeSpec; }

		/* 
			Short string representation of the type, ready to be pretty-printed. 
			Example: "int"
		*/ 
		virtual string toShortString();
		virtual string toString() { return "<BasicType>\\n" + toShortString(); };
		virtual vector<Node*> getChildren();
		virtual bool assignableTo(Type *type);
	};

	#define UNKNOWN_ARR_SIZE 0

	class ArrayType : public Type
	{
		Type *_valueType;
		size_t _length;			// size = 0 means size is unknown.
		Expression *_lenExp;	// for stuff like "new int[a]"

	public:
		ArrayType(AST::Expression *base) : Type(base), _lenExp(NULL) {  }
		ArrayType(Type *valueType, size_t length) : Type(NULL), _valueType(valueType), _length(length), _lenExp(NULL) { }
		ArrayType(Type *valueType, Expression *lenExp) : Type(NULL), _valueType(valueType), _length(DST::UNKNOWN_ARRAY_LENGTH), _lenExp(lenExp) { }
		ArrayType(Type *valueType, size_t length, Expression *lenExp) : Type(NULL), _valueType(valueType), _length(length), _lenExp(lenExp) { }
		ArrayType(Type *valueType) : Type(NULL), _valueType(valueType), _length(DST::UNKNOWN_ARRAY_LENGTH), _lenExp(NULL) { }
		virtual ~ArrayType() { if (_valueType) delete _valueType; }
		ExactType getExactType() { return EXACT_ARRAY; }
		virtual Type *getType() { return _valueType; }
		Expression *getLenExp() { return _lenExp; }

		virtual bool equals(Type *other)
		{
			if (other->getExactType() == EXACT_TYPELIST && ((DST::TypeList*)other)->size() == 1)
				return equals(((DST::TypeList*)other)->getTypes()[0]);
			return other->getExactType() == EXACT_ARRAY &&
				((ArrayType*)other)->_length == _length &&
				((ArrayType*)other)->_valueType->equals(_valueType);
		}
		size_t getLength() { return _length; }
		Type *getElementType() { return _valueType; }

		/* 
			Short string representation of the type, ready to be pretty-printed. 
			Example: "int[10]"
		*/ 
		virtual string toShortString() { return _valueType->toShortString() + "[" + ((_length != DST::UNKNOWN_ARRAY_LENGTH) ? std::to_string(_length) : "") + "]"; };
		virtual string toString() { return "<ArrayType>\\n" + toShortString(); };
		virtual vector<Node*> getChildren();

		virtual bool assignableTo(DST::Type *type) 
		{
			if (!type)
				return false;
			if (type->getExactType() == EXACT_PROPERTY)
				return ((DST::PropertyType*)type)->writeable() && assignableTo(((DST::PropertyType*)type)->getReturn());
			return type->getExactType() == EXACT_ARRAY && 
				((DST::ArrayType*)type)->_length == _length && 
				_valueType->assignableTo(((DST::ArrayType*)type)->_valueType);
		}
	};

	class FunctionType : public Type
	{
		TypeList *_returns;
		TypeList *_parameters;

	public:
		FunctionType() : Type(NULL) { _returns = new TypeList(NULL); _parameters = new TypeList(NULL); }
		FunctionType(AST::FunctionCall *base) : Type(base) { _returns = new TypeList(base->getFunctionId()); _parameters = new TypeList(base->getArguments()); }
		virtual ~FunctionType() { if (_returns) delete _returns; if (_parameters) delete _parameters; }
		void addReturn(Type *returnType) { _returns->addType(returnType); }
		void addParameter(Type *parameterType) { _parameters->addType(parameterType); }
		TypeList *getReturns() { return _returns; }
		TypeList *getParameters() { return _parameters; }

		ExactType getExactType() { return EXACT_FUNCTION; }
		virtual bool equals(Type *other);

		virtual bool assignableTo(DST::Type *type)
		{
			if (!type)
				return false;
			if (type->getExactType() == EXACT_PROPERTY)
				return ((DST::PropertyType*)type)->writeable() && assignableTo(((DST::PropertyType*)type)->getReturn());
			return type->getExactType() == EXACT_FUNCTION && 
				_returns->assignableTo(((DST::FunctionType*)type)->_returns) && 
				_parameters->assignableTo(((DST::FunctionType*)type)->_parameters);
		}

		/* 
			Short string representation of the type, ready to be pretty-printed. 
			Example: "void(int, bool)"
		*/ 
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
		virtual ~Variable() { if (_base) delete _base; if (_type) delete _type; }
		void setType(Type *type) { _type = type; };
		virtual Type *getType() { return _type; };
		virtual ExpressionType getExpressionType() { return ET_IDENTIFIER; }
		virtual PositionInfo getPosition() const { return _base ? _base->getPosition() : PositionInfo{ 0, 0, 0, ""}; }

		virtual string toString() { return _base->toString() + "\nType: " + _type->toShortString(); };
		virtual vector<Node*> getChildren();
		unicode_string &getVarId() { return _base->getVarId(); }
	};

	class BinaryOperation : public Expression
	{
		AST::BinaryOperation *_base;
		Type *_type;
		Expression *_left;
		Expression *_right;

	public:
		BinaryOperation(AST::BinaryOperation *base) : _base(base) {};
		BinaryOperation(AST::BinaryOperation *base, Expression *left, Expression *right);
		virtual ~BinaryOperation() { if (_base) delete _base; if (_type) delete _type; 
							if (_left) delete _left; if (_right) delete _right; }
		void setType(Type *type) { _type = type; };
		Operator getOperator() { return _base->getOperator(); }
		virtual Type *getType() { return _type; }
		virtual ExpressionType getExpressionType() { return ET_BINARY_OPERATION; }
		Expression *getLeft() { return _left; }
		Expression *getRight() { return _right; }
		virtual PositionInfo getPosition() const { return _base ? _base->getPosition() : PositionInfo{ 0, 0, 0, ""}; }

		virtual string toString() { return _base->toString() + "\nType: " + _type->toShortString(); };
		virtual vector<Node*> getChildren();
	};

	class UnaryOperation : public Expression
	{
		AST::UnaryOperation *_base;
		Type *_type;
		Expression* _expression;
		void setType();

	public:
		UnaryOperation(AST::UnaryOperation *base) : _base(base) {  };
		UnaryOperation(AST::UnaryOperation *base, Expression *expression) : _base(base), _expression(expression) { setType(); };
		virtual ~UnaryOperation() { if (_expression) delete _expression; }
		virtual ExpressionType getExpressionType() { return ET_UNARY_OPERATION; };
		virtual string toString() { return _base->toString() + "\nType: " + _type->toShortString(); };
		virtual vector<Node*> getChildren();

		virtual PositionInfo getPosition() const { return _base ? _base->getPosition() : PositionInfo{ 0, 0, 0, ""}; }
		virtual Type *getType() { return _type; }
		void setType(Type *type) { _type = type; }
		void setExpression(Expression* expression) { _expression = expression; }
		Operator getOperator() { return _base->getOperator(); }
		Expression* getExpression() { return _expression; }
		bool isPostfix() { return _base->isPostfix(); }
	};

	class ConditionalExpression : public Expression
	{
		AST::ConditionalExpression *_base;
		Expression* _condition;
		Expression* _thenBranch;
		Expression* _elseBranch;

	public:
		ConditionalExpression(AST::ConditionalExpression *base) : _base(base) {};
		virtual ~ConditionalExpression() { if (_condition) delete _condition; if (_thenBranch) delete _thenBranch; if (_elseBranch) delete _elseBranch; }
		virtual ExpressionType getExpressionType() { return ET_CONDITIONAL_EXPRESSION; };
		virtual string toString() { return _base->toString() + "\\nType: " + getType()->toShortString(); };
		virtual vector<Node*> getChildren();
		virtual Type *getType() { return _thenBranch->getType(); }
		virtual PositionInfo getPosition() const { return _base ? _base->getPosition() : PositionInfo{ 0, 0, 0, ""}; }

		void setCondition(Expression* condition) { _condition = condition; }
		void setThenBranch(Expression* thenBranch) { _thenBranch = thenBranch; }
		void setElseBranch(Expression* elseBranch) { _elseBranch = elseBranch; }

		Expression* getCondition() { return _condition; }
		Expression* getThenBranch() { return _thenBranch; }
		Expression* getElseBranch() { return _elseBranch; }

	};

	class Increment : public ExpressionStatement
	{
		AST::Increment *_base;
		Expression *_expr;
		Type *_type;
		bool _increment;
		
	public:
		Increment(AST::Increment *base, Expression *expr, bool isIncrement) : _base(base), _expr(expr), _type(_expr->getType()), _increment(isIncrement) {};
		virtual ~Increment() { if (_expr) delete _expr; }
		virtual ExpressionType getExpressionType() { return ET_INCREMENT; };
		virtual StatementType getStatementType() { return ST_INCREMENT; };
		virtual string toString() { return _base->toString() + "\\nType: " + getType()->toShortString(); };
		virtual vector<Node*> getChildren() { return {}; };
		virtual Type *getType() { return _expr->getType(); }
		virtual PositionInfo getPosition() const { return _base ? _base->getPosition() : PositionInfo{ 0, 0, 0, ""}; }
		Expression *getExpression() { return _expr; };
		bool isIncrement() const { return _increment; }
	};

	class MemberAccess : public Expression
	{
		AST::BinaryOperation *_base;
		Type *_type;
		Expression *_left;

	public:
		MemberAccess(AST::BinaryOperation *base) : _base(base) {};
		MemberAccess(AST::BinaryOperation *base, Expression *left) : _base(base), _left(left) 
		{
			if (base->getRight()->getExpressionType() != ET_IDENTIFIER)
				throw ErrorReporter::report("right of member access must be an identifier", ERR_DECORATOR, _base->getPosition());
		};
		virtual ~MemberAccess() { if (_base) delete _base; if (_type) delete _type; if (_left) delete _left; }
		void setType(Type *type) { _type = type; };
		Operator getOperator() { return _base->getOperator(); }
		virtual Type *getType() { return _type; }
		virtual ExpressionType getExpressionType() { return ET_MEMBER_ACCESS; }
		Expression *getLeft() { return _left; }
		unicode_string &getRight() { return ((AST::Identifier*)_base->getRight())->getVarId(); }
		virtual PositionInfo getPosition() const { return _base ? _base->getPosition() : PositionInfo{ 0, 0, 0, ""}; }

		virtual string toString() { return "<MemberAccess>\n." + getRight().to_string() + "\nType: " + _type->toShortString(); };
		virtual vector<Node*> getChildren();
	};

	class Literal : public Expression
	{
		AST::Literal *_base;
		Type *_type;
	public:
		Literal(AST::Literal* base) : _base(base) {}
		virtual ~Literal() { if (_base) delete _base; if (_type) delete _type; }
		AST::Literal *getBase() { return _base; }
		void setType(Type *type) { _type = type; }
		Type *getType() { return _type; }
		void *getValue();
		LiteralType getLiteralType() { return _base->getLiteralType(); }
		virtual PositionInfo getPosition() const { return _base ? _base->getPosition() : PositionInfo{ 0, 0, 0, ""}; }

		virtual ExpressionType getExpressionType() { return ET_LITERAL; }
		string toShortString() { return _base->toShortString(); }
		virtual string toString() { return _base->toString() + "\nType: " + _type->toShortString(); };
		virtual vector<Node*> getChildren();
	};

	class Conversion : public Expression
	{
		AST::FunctionCall *_base;
		Type *_type;
		Expression *_expression;
	public: 
		Conversion(AST::FunctionCall *base, Type *type, Expression *expression) : _base(base), _expression(expression) { setType(type); }
		virtual ~Conversion() { if (_base) delete _base; if (_type) delete _type; if(_expression) delete _expression; }
		void setType(Type *type) { 
			// if (_expression->getType()->getExactType() == EXACT_BASIC && ((DST::BasicType*)_expression->getType())->getTypeSpecifier()->getInterfaceDecl())
			// 	_type = new DST::PointerType(type);
			// else _type = type; 
			_type = type; 
		}
		Type *getType() { return _type; }
		virtual ExpressionType getExpressionType() { return ET_CONVERSION; }
		virtual PositionInfo getPosition() const { return _base ? _base->getPosition() : PositionInfo{ 0, 0, 0, ""}; }
		Expression *getExpression() { return _expression; }
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
		virtual ~FunctionLiteral();
		void setType(FunctionType *type) { _type = type; }
		Type *getType() { return _type; }
		virtual ExpressionType getExpressionType() { return ET_FUNCTION_LITERAL; }
		virtual PositionInfo getPosition() const { return _base ? _base->getPosition() : PositionInfo{ 0, 0, 0, ""}; }

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
		ExpressionList(AST::Expression *base, vector<Expression*> expressions) : _base(base), _expressions(expressions) 
		{ 
			_type = new TypeList(base);
			for (auto i : _expressions) 
				_type->addType(i->getType());
		};
		virtual ~ExpressionList() { if (_base) delete _base; if (_type) delete _type; _expressions.clear(); }
		TypeList *getType() { return _type; }
		virtual ExpressionType getExpressionType() { return ET_LIST; };
		virtual string toString() { return "<ExpressionList>\\nType: " + _type->toShortString(); };
		virtual vector<Node*> getChildren();
		virtual PositionInfo getPosition() const { return _base ? _base->getPosition() : PositionInfo{ 0, 0, 0, ""}; }
		size_t size() { return _expressions.size(); }

		void addExpression(Expression* expression) { _expressions.push_back(expression); _type->addType(expression->getType()); };
		vector<Expression*> getExpressions() { return _expressions; }
	};

	class ArrayLiteral : public Expression
	{
		ArrayType *_type;
		vector<Expression*> _array;
		// TODO - add AST::UnaryOperation *_base
	public:
		ArrayLiteral(Type *type, vector<Expression*> arr) : _array(arr) { _type = new ArrayType(type, arr.size()); }
		virtual ~ArrayLiteral() { /*if (_base) delete _base; */if (_type) delete _type; _array.clear(); }
		ArrayType *getType() { return _type; }
		virtual ExpressionType getExpressionType() { return ET_ARRAY; };
		virtual string toString() { return "<ArrayLiteral>\\nType: " + _type->toShortString(); };
		virtual vector<Node*> getChildren();
		//virtual PositionInfo getPosition() const { return _base ? _base->getPosition() : PositionInfo{ 0, 0, 0, ""}; }

		void addValue(Expression* value) { _array.push_back(value); };
		vector<Expression*> getArray() { return _array; }
	};

	/********************** Statements **********************/

	class StatementBlock : public Statement
	{
		// TODO - AST::StatementBlock *_base
		vector<Statement*> _statements;
		bool _hasReturn;
	public:
		virtual StatementType getStatementType() { return ST_STATEMENT_BLOCK; };
		StatementBlock() : _hasReturn(false) {}
		virtual ~StatementBlock() { _statements.clear(); }
		//virtual PositionInfo getPosition() const { return _base ? _base->getPosition() : PositionInfo{ 0, 0, 0, ""}; }

		bool hasReturnType(Type *returnType);
		bool hasReturn() { return _hasReturn; }

		vector<Statement*> getStatements() { return _statements; }
		void addStatement(Statement* statement);

		virtual string toString() { return "<StatementBlock>"; };
		virtual vector<Node*> getChildren();
	};

	class TryCatch : public Statement
	{
		AST::TryCatch *_base;
		StatementBlock* _tryBlock;
		StatementBlock* _catchBlock;

	public:
		TryCatch(AST::TryCatch *base) : _base(base) {}
		virtual ~TryCatch() { if (_base) delete _base; if (_tryBlock) delete _tryBlock; if (_catchBlock) delete _catchBlock; }
		virtual StatementType getStatementType() { return ST_TRY_CATCH; };
		virtual string toString() { return "<TryCatch>"; };
		virtual vector<Node*> getChildren() { return { _tryBlock, _catchBlock }; };

		void setTryBlock(StatementBlock* tryBlock) { _tryBlock = tryBlock; }
		void setCatchBlock(StatementBlock* catchBlock) { _catchBlock = catchBlock; }
	
		StatementBlock* getTryBlock() { return _tryBlock; }
		StatementBlock* getCatchBlock() { return _catchBlock; }
	};

	class IfThenElse : public Statement
	{
		AST::IfThenElse *_base;
		Expression* _condition;
		StatementBlock *_thenBranch;
		StatementBlock* _elseBranch;

	public:
		IfThenElse(AST::IfThenElse *base) : _base(base) {};
		virtual ~IfThenElse() { if (_base) delete _base; if (_condition) delete _condition; 
								if (_thenBranch) delete _thenBranch; if (_elseBranch) delete _elseBranch; }
		virtual StatementType getStatementType() { return ST_IF_THEN_ELSE; };
		virtual string toString() { return "<IfThenElse>"; };
		virtual vector<Node*> getChildren();
		virtual PositionInfo getPosition() const { return _base ? _base->getPosition() : PositionInfo{ 0, 0, 0, ""}; }

		void setCondition(Expression* condition) { _condition = condition; }
		void setThenBranch(StatementBlock* thenBranch) { _thenBranch = thenBranch; }
		void setElseBranch(StatementBlock* elseBranch) { _elseBranch = elseBranch; }

		Expression* getCondition() { return _condition; }
		StatementBlock* getThenBranch() { return _thenBranch; }
		StatementBlock* getElseBranch() { return _elseBranch; }
	};

	typedef struct CaseClause {
		vector<Expression*> _expressions;
		StatementBlock* _statement;
	} CaseClause;

	class SwitchCase : public Statement {
		AST::SwitchCase *_base;
		Expression* _expression;
		vector<CaseClause> _cases;
		StatementBlock* _default;

	public:
		SwitchCase(AST::SwitchCase *base) : _base(base), _default(NULL) {};
		virtual ~SwitchCase() { if (_base) delete _base; if (_expression) delete _expression; 
								if (_default) delete _default; _cases.clear(); }
		virtual StatementType getStatementType() { return ST_SWITCH; };
		virtual string toString() { return "<Switch>"; };
		virtual vector<Node*> getChildren();
		virtual PositionInfo getPosition() const { return _base ? _base->getPosition() : PositionInfo{ 0, 0, 0, ""}; }

		void setExpression(Expression* expression) { _expression = expression; }
		void addCase(vector<Expression*> expressions, StatementBlock* statement) { CaseClause c = { expressions, statement }; _cases.push_back(c); }
		void addCase(CaseClause clause) { _cases.push_back(clause); }
		void setDefault(StatementBlock* statement) {
			if (_default) throw ErrorReporter::report("'default' clause may only be set once", ERR_DECORATOR, statement->getPosition());
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
		WhileLoop(DST::WhileLoop *loop) : _base(loop->_base), _condition(loop->_condition), _statement(loop->_statement) {};
		virtual ~WhileLoop() { if (_base) delete _base; if (_condition) delete _condition; if (_statement) delete _statement; } 
		
		virtual StatementType getStatementType() { return ST_WHILE_LOOP; };
		virtual string toString() { return "<While>"; };
		virtual vector<Node*> getChildren();
		virtual PositionInfo getPosition() const { return _base ? _base->getPosition() : PositionInfo{ 0, 0, 0, ""}; }

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
		virtual ~ForLoop() { if (_base) delete _base; if (_condition) delete _condition; if (_statement) delete _statement;
								if (_begin) delete _begin; if (_increment) delete _increment; }
		virtual StatementType getStatementType() { return ST_FOR_LOOP; };
		virtual string toString() { return "<For>"; };
		virtual vector<Node*> getChildren();
		virtual PositionInfo getPosition() const { return _base ? _base->getPosition() : PositionInfo{ 0, 0, 0, ""}; }

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
		DoWhileLoop(DST::WhileLoop *base) : WhileLoop(base) {};
		virtual StatementType getStatementType() { return ST_DO_WHILE_LOOP; };
		virtual string toString() { return "<Do>"; };
	};

	class UnaryOperationStatement : public Statement
	{
		AST::UnaryOperationStatement *_base;
		Expression* _expression;

	public:
		UnaryOperationStatement(AST::UnaryOperationStatement *base, Expression *expression) : _base(base), _expression(expression) {};
		virtual ~UnaryOperationStatement() { if (_base) delete _base; if (_expression) delete _expression; } 
		virtual StatementType getStatementType() { return ST_UNARY_OPERATION; };
		virtual string toString() { return _base->toString(); };
		virtual vector<Node*> getChildren();
		virtual PositionInfo getPosition() const { return _base ? _base->getPosition() : PositionInfo{ 0, 0, 0, ""}; }

		void setExpression(Expression* expression) { _expression = expression; }

		Operator getOperator() { return _base->getOperator(); }
		Expression* getExpression() { return _expression; };
	};

	class ConstDeclaration : public Statement
	{
		AST::ConstDeclaration *_base;
		Expression* _expression;

	public:
		ConstDeclaration(AST::ConstDeclaration *base) : _base(base) { }
		virtual ~ConstDeclaration() { if (_expression) delete _expression; }
		virtual StatementType getStatementType() { return ST_CONST_DECLARATION; };
		virtual string toString() { return _base->toString(); };
		virtual vector<Node*> getChildren();
		void setExpression(Expression* expression) { _expression = expression; }
		Expression* getExpression() { return _expression; }
		unicode_string getName() { return _base->getName(); }
		AST::ConstDeclaration *getBase() { return _base; }

		virtual bool isDeclaration() { return true; }
	};

	class InterfaceDeclaration : public Statement
	{
		AST::InterfaceDeclaration *_base;
		vector<InterfaceDeclaration*> _implements;
		unordered_map<unicode_string, pair<Statement*, Type*>, UnicodeHasherFunction> _decls;
	public:
		//InterfaceDeclaration(unicode_string name) : _base(NULL) {}
		InterfaceDeclaration(AST::InterfaceDeclaration *base) : _base(base) {}
		virtual ~InterfaceDeclaration() { if (_base) delete _base; _decls.clear(); }
		
		AST::InterfaceDeclaration *getBase() { return _base; }
		vector<InterfaceDeclaration*> getImplements() { return _implements; }
		void addImplements(InterfaceDeclaration *implement) { _implements.push_back(implement); }
		void notImplements(InterfaceDeclaration* inter);
		bool implements(InterfaceDeclaration* inter);
		void addDeclaration(Statement *decl, Type *type);
		Statement* getDeclaration(unicode_string id);
		unordered_map<unicode_string, pair<Statement*, Type*>, UnicodeHasherFunction> getDeclarations() { return _decls; }
		Type* getMemberType(unicode_string id);
		virtual PositionInfo getPosition() const { return _base ? _base->getPosition() : PositionInfo{ 0, 0, 0, ""}; }

		virtual StatementType getStatementType() { return ST_INTERFACE_DECLARATION; };
		virtual bool isDeclaration() { return true; }

		unicode_string getName() { return _base->getName(); }
		virtual string toString() { return "<InterfaceDeclaration>\\n" + getName().to_string() + "\\nimplements " + std::to_string(_implements.size()); };
		virtual vector<Node*> getChildren();
	};

	class TypeDeclaration : public Statement 
	{
		unicode_string _name;
		AST::TypeDeclaration *_base;
		vector<InterfaceDeclaration*> _interfaces;
		unordered_map<unicode_string, pair<Statement*, Type*>, UnicodeHasherFunction> _decls;
	public:
		TypeDeclaration(unicode_string name) : _name(name), _base(NULL) {}
		TypeDeclaration(AST::TypeDeclaration *base) : _name(base->getName()), _base(base) {}
		virtual ~TypeDeclaration() { if (_base) delete _base; _decls.clear(); } 

		AST::TypeDeclaration *getBase() { return _base; }
		void setName(unicode_string name) { _name = name; }
		void addDeclaration(Statement *decl, Type *type);
		Statement* getDeclaration(unicode_string id)  { return _decls[id].first; }
		Type* getMemberType(unicode_string id) { return _decls[id].second; }
		unordered_map<unicode_string, pair<Statement*, Type*>, UnicodeHasherFunction> getMembers() { return _decls; }
		virtual PositionInfo getPosition() const { return _base ? _base->getPosition() : PositionInfo{ 0, 0, 0, ""}; }

		vector<InterfaceDeclaration*> getInterfaces() { return _interfaces; }
		void addInterface(InterfaceDeclaration *interface) { _interfaces.push_back(interface); }
		bool validateImplements(InterfaceDeclaration *inter);
		bool implements(InterfaceDeclaration *inter);

		virtual StatementType getStatementType() { return ST_TYPE_DECLARATION; };
		virtual bool isDeclaration() { return true; }

		unicode_string getName() { return _name; }
		virtual string toString() { return "<TypeDeclaration>\\n" + _name.to_string() + "\\nimplements " + std::to_string(_interfaces.size()); };
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
		string _llvmFuncId;

		FunctionDeclaration(AST::FunctionDeclaration *base, VariableDeclaration *decl) : _base(base), _decl(decl), _content(NULL) {};
		virtual ~FunctionDeclaration();
		virtual bool isDeclaration() { return true; }
		virtual StatementType getStatementType() { return ST_FUNCTION_DECLARATION; };
		virtual string toString() { return "<FunctionDeclaration>"; };
		virtual vector<Node*> getChildren();
		virtual PositionInfo getPosition() const { return _base ? _base->getPosition() : PositionInfo{ 0, 0, 0, ""}; }

		AST::FunctionDeclaration *getBase() { return _base; }

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
		string _llvmGetFuncId, _llvmSetFuncId;

		PropertyDeclaration(AST::PropertyDeclaration *base, StatementBlock *get, StatementBlock *set, Type *type) : _base(base), _type(type), _get(get), _set(set) {};
		virtual ~PropertyDeclaration() { if (_base) delete _base; if (_type) delete _type; if (_get) delete _get; if (_set) delete _set; } 
		virtual bool isDeclaration() { return true; }
		virtual StatementType getStatementType() { return ST_PROPERTY_DECLARATION; };
		virtual string toString() { return "<PropertyDeclaration>\\n" + _type->toShortString() + " " + _base->getVarDecl()->getVarId().to_string(); };
		virtual vector<Node*> getChildren();
		unicode_string getName() { return _base->getVarDecl()->getVarId(); }
		virtual PositionInfo getPosition() const { return _base ? _base->getPosition() : PositionInfo{ 0, 0, 0, ""}; }
		unicode_string getPropId() { return _base->getVarDecl()->getVarId(); }
		Type *getReturnType() { return _type; }
		void setGet(StatementBlock* get) { _get = get; }
		void setSet(StatementBlock* set) { _set = set; }
		StatementBlock* getGet() { return _get; }
		StatementBlock* getSet() { return _set; }
		AST::PropertyDeclaration *getBase() { return _base; };
	};

	class NamespaceDeclaration : public Statement
	{
	private:
		AST::NamespaceDeclaration *_base;
		unordered_map<unicode_string, std::pair<Statement*, Type*>, UnicodeHasherFunction> _decls;
	public:
		NamespaceDeclaration(AST::NamespaceDeclaration *base) : _base(base) {}
		virtual ~NamespaceDeclaration() { if (_base) delete _base; _decls.clear(); } 
		virtual StatementType getStatementType() { return ST_NAMESPACE_DECLARATION; };
		virtual string toString() { return _base->toString(); };
		virtual vector<Node*> getChildren();
		unicode_string getName() { return _base->getName(); }
		virtual PositionInfo getPosition() const { return _base ? _base->getPosition() : PositionInfo{ 0, 0, 0, ""}; }
		virtual bool isDeclaration() { return true; }

		AST::NamespaceDeclaration *getBase() { return _base; }
		unordered_map<unicode_string, std::pair<Statement*, Type*>, UnicodeHasherFunction> getMembers() { return _decls; }

		Statement *getMember(unicode_string id) { return _decls[id].first; }
		Type *getMemberType(unicode_string id) { if (_decls.count(id) == 0) return NULL; return _decls[id].second; }
		void addMember(unicode_string name, Statement * decl, Type * type);
	};

	/***************** ExpressionStatements *****************/

	class VariableDeclaration : public ExpressionStatement
	{
		AST::VariableDeclaration *_base;
		Type *_type;

	public:
		VariableDeclaration(AST::VariableDeclaration *base) : _base(base) { };
		virtual ~VariableDeclaration() { if (_base) delete _base; if (_type) delete _type; } 

		void setType(Type *type) { _type = type; }
		virtual Type *getType() { return _type; }
		virtual PositionInfo getPosition() const { return _base ? _base->getPosition() : PositionInfo{ 0, 0, 0, ""}; }

		virtual ExpressionType getExpressionType() { return ET_VARIABLE_DECLARATION; }
		virtual StatementType getStatementType() { return ST_VARIABLE_DECLARATION; }
		unicode_string &getVarId() { return _base->getVarId(); }

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
		virtual ~Assignment() { if (_base) delete _base; if (_type) delete _type; if (_left) delete _left; if (_right) delete _right;} 
		void setType(Type *type) { _type = type; };
		virtual Type *getType() { return _type; }
		virtual ExpressionType getExpressionType() { return ET_ASSIGNMENT; }
		virtual PositionInfo getPosition() const { return _base ? _base->getPosition() : PositionInfo{ 0, 0, 0, ""}; }
		virtual StatementType getStatementType() { return ST_ASSIGNMENT; }
		Expression *getLeft() { return _left; }
		Expression *getRight() { return _right; }
		void setRight(Expression *right) { _right = right; }
		Operator getOperator() { return _base->getOperator(); }
		virtual string toString() { return _base->toString() + "\nType: " + _type->toShortString(); };
		virtual vector<Node*> getChildren();
	};

	class FunctionCall : public ExpressionStatement
	{
		Type *_type;
		AST::FunctionCall *_base;
		Expression *_funcPtr;
		ExpressionList *_arguments;

	public:
		FunctionCall(AST::FunctionCall *base) : _base(base) {};
		FunctionCall(AST::FunctionCall *base, Expression *funcPtr, ExpressionList *arguments);
		virtual ~FunctionCall() { if (_base) delete _base; if (_type) delete _type; if (_funcPtr) delete _funcPtr; if (_arguments) delete _arguments; } 
		virtual StatementType getStatementType() { return ST_FUNCTION_CALL; };
		virtual ExpressionType getExpressionType() { return ET_FUNCTION_CALL; };
		virtual string toString() { return string() + "<FunctionCall>\\nType: " + _type->toShortString(); };
		virtual vector<Node*> getChildren();
		virtual PositionInfo getPosition() const { return _base ? _base->getPosition() : PositionInfo{ 0, 0, 0, ""}; }

		void setFunctionId(Expression* funcId)
		{
			if (funcId->getType()->getExactType() != EXACT_FUNCTION)
				throw ErrorReporter::report("Cannot invoke expression as function", ERR_DECORATOR, funcId->getPosition());
			_funcPtr = funcId;
			_type = ((FunctionType*)funcId->getType())->getReturns();
			if (_type->getExactType() == EXACT_TYPELIST && ((DST::TypeList*)_type)->size() == 1)
				_type = ((DST::TypeList*)_type)->getTypes()[0];
		}
		
		void setArguments(ExpressionList* arguments) 
		{
			if (!arguments->getType()->assignableTo(((FunctionType*)_funcPtr->getType())->getParameters()))
				throw ErrorReporter::report(string("Argument types do not match function parameters.\nArguments are: ") + arguments->getType()->toShortString()
				 + "\nShould be: " + ((FunctionType*)_funcPtr->getType())->getParameters()->toShortString(), ERR_DECORATOR, getPosition());
			_arguments = arguments; 
		}

		Type *getType() { return _type; }
		Expression* getFunctionId() { return _funcPtr; }
		ExpressionList* getArguments() { return _arguments; }
	};
}