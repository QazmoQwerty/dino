#pragma once
#include "AstNode.h"

class Value
{
private:
	string _type;
public:
	Value(string type) { _type = type; }
	string getType() { return _type; }
	virtual string toString() = 0;
};

class Interpreter
{
private:
	unordered_map<string, Value*> _variables;
	Value* interpretBinaryOp(AST::BinaryOperation* node);
	Value* interpretUnaryOp(AST::UnaryOperation* node);
	Value* interpretFuncCall(AST::FunctionCall* node);
	Value* interpretLiteral(AST::Literal* node);
	Value* interpretVariable(AST::Variable* node);

	void interpretVariableDeclaration(AST::VariableDeclaration* node);
	void interpretIfThenElse(AST::IfThenElse* node);
	void interpretWhileLoop(AST::WhileLoop* node);
	void interpretDoWhileLoop(AST::DoWhileLoop* node);
public:
	Value* interpret(AST::Node* node);
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
	FuncValue() : Value("func") { _value = NULL; }
	FuncValue(AST::Function* value) : Value("func") { _value = value; }
	void setValue(AST::Function* value) { _value = value; }
	AST::Function* getValue() { return _value; }
	virtual string toString() { return _value->toString(); };
	/*Value* callFunction(vector<Value*> parameters)
	{
		for (auto varDecl : _value->getParameters())
		{
			Value* v = interpret(varDecl);
		}
	}*/
};