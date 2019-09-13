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
public:
	Value* interpret(AST::Node* node);
};