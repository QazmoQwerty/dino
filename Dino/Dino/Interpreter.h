#pragma once
#include "AstNode.h"

class Interpreter
{
private:
	unordered_map<string, Value*> _variables;
public:
	static Value* interpret(AST::Node* node);
};

class Value
{
private:
	string _type;
public:
	Value(string type) { _type = type; }
};

class IntValue : public Value
{
private:
	int _value;
public:
	IntValue() : Value("int") { _value = 0; }
	IntValue(int value) : Value("int") { _value = value; }
	int setValue() { return _value; }
	int getValue() { return _value; }
};