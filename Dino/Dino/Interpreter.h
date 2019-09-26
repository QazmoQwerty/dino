#pragma once
#include "AstNode.h"
#include <algorithm>

class Value
{
private:
	string _type;
	bool _isReturn;
	bool _isTemp;
public:
	Value(string type) { _type = type; _isReturn = false; _isTemp = true; }
	void setReturn() { _isReturn = true; }
	bool isReturn() { return _isReturn; }
	string getType() { return _type; }
	void setIsTemp() { _isTemp = true; }
	void setNotTemp() { _isTemp = false; }
	bool isTemp() { return _isTemp; }
	virtual string toString() = 0;
};

class IntValue : public Value
{
private:
	int _value;
public:
	IntValue() : Value("int") { _value = 0; }
	IntValue(int value) : Value("int") { _value = value; }
	void setValue(int value) { _value = value; }
	int getValue() { return _value; }
	virtual string toString() { return std::to_string(_value); };
};

class FracValue : public Value
{
private:
	float _value;
public:
	FracValue() : Value("frac") { _value = 0; }
	FracValue(float value) : Value("frac") { _value = value; }
	void setValue(float value) { _value = value; }
	float getValue() { return _value; }
	virtual string toString() { return std::to_string(_value); };
};

class CharValue : public Value
{
private:
	char _value;
public:
	CharValue() : Value("char") { _value = 0; }
	CharValue(char value) : Value("char") { _value = value; }
	void setValue(char value) { _value = value; }
	char getValue() { return _value; }
	virtual string toString() { return std::to_string(_value); };
};

class StringValue : public Value
{
private:
	string _value;
public:
	StringValue() : Value("string") { _value = ""; }
	StringValue(string value) : Value("string") { _value = value; }
	void setValue(string value) { _value = value; }
	string getValue() { return _value; }
	virtual string toString() { return _value; };
};

class BoolValue : public Value
{
private:
	bool _value;
public:
	BoolValue() : Value("bool") { _value = false; }
	BoolValue(bool value) : Value("bool") { _value = value; }
	void setValue(bool value) { _value = value; }
	bool getValue() { return _value; }
	virtual string toString() { return std::to_string(_value); };
};

class FuncValue : public Value
{
private:
	string _returnType;
	AST::Function* _value;
public:
	FuncValue() : Value("func") { _value = NULL; _returnType = ""; }
	FuncValue(string returnType) : Value("func") { _returnType = returnType; _value = NULL; }
	FuncValue(AST::Function* value) : Value("func") { _value = value; _returnType = value->getReturnType().name; }
	void setValue(AST::Function* value) 
	{ 
		if (_returnType != "" && value->getReturnType().name != _returnType)
			throw "Error: Function pointer to different value";	// TODO - fix error msg
		_value = value; _returnType = value->getReturnType().name;
	}
	AST::Function* getValue() { return _value; }
	virtual string toString() { return _value->toString(); };
};

struct VariableTypeDefinition
{
	string type;
	vector<string> modifiers;
} typedef VariableTypeDefinition;

struct TypeDefinition
{
	string _name;
	unordered_map<string, VariableTypeDefinition> _variables;
	//unordered_map<string, FuncValue*> _functions;
} typedef TypeDefinition;

class TypeValue : public Value
{
private:
	unordered_map<string, TypeDefinition> &_types;
	TypeDefinition _typeDefinition;
	unordered_map<string, Value*> _variables;
public:
	TypeValue(string typeName, unordered_map<string, TypeDefinition> &types);
	virtual string toString() { return std::to_string(NULL); };

	//void setVariable(string name, Value* val);
	Value* getVariable(string name);
};



class Interpreter
{
private:
	vector<unordered_map<string, Value*>> _variables;	// index represents scope, string represents variable name
	unordered_map<string, TypeDefinition> _types;
	int _scope;

	Value* interpretBinaryOp(AST::BinaryOperation* node);
	Value* interpretUnaryOp(AST::UnaryOperation* node);
	Value* interpretIncrement(AST::Increment* node);
	Value* interpretFuncCall(AST::FunctionCall* node);
	Value* interpretLiteral(AST::Literal* node);
	Value* interpretVariable(AST::Variable* node);
	Value* interpretAssignment(AST::Assignment* node);

	Value* interpretUnaryOpStatement(AST::UnaryOperationStatement* node);
	Value* interpretVariableDeclaration(AST::VariableDeclaration* node);
	Value* interpretIfThenElse(AST::IfThenElse* node);
	Value* interpretWhileLoop(AST::WhileLoop* node);
	Value* interpretDoWhileLoop(AST::DoWhileLoop* node);

	Value* copyValue(Value* val);
	int currentScope() { return _variables.size() - 1; }
	void enterBlock() { _variables.push_back(unordered_map<string, Value*>()); }
	void leaveBlock();
public:
	Value* interpret(AST::Node* node);
	Interpreter() { _scope = 0; _variables.push_back(unordered_map<string, Value*>()); _types = unordered_map<string, TypeDefinition>(); }
};