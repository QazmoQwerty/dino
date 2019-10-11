//#pragma once
//
//#include "AstNode.h"
//#include <algorithm>
//#include <stack>
//using std::stack;
//
//class Value
//{
//private:
//	string _type;
//	bool _isReturn;
//	bool _isTemp;
//public:
//	Value(string type) { _type = type; _isReturn = false; _isTemp = true; }
//	void setReturn() { _isReturn = true; }
//	bool isReturn() { return _isReturn; }
//	string getType() { return _type; }
//	void setIsTemp() { _isTemp = true; }
//	void setNotTemp() { _isTemp = false; }
//	bool isTemp() { return _isTemp; }
//	virtual string toString() = 0;
//};
//
//class IntValue : public Value
//{
//private:
//	int _value;
//public:
//	IntValue() : Value("int") { _value = 0; }
//	IntValue(int value) : Value("int") { _value = value; }
//	void setValue(int value) { _value = value; }
//	int getValue() { return _value; }
//	virtual string toString() { return std::to_string(_value); };
//};
//
//class FracValue : public Value
//{
//private:
//	float _value;
//public:
//	FracValue() : Value("frac") { _value = 0; }
//	FracValue(float value) : Value("frac") { _value = value; }
//	void setValue(float value) { _value = value; }
//	float getValue() { return _value; }
//	virtual string toString() { return std::to_string(_value); };
//};
//
//class CharValue : public Value
//{
//private:
//	char _value;
//public:
//	CharValue() : Value("char") { _value = 0; }
//	CharValue(char value) : Value("char") { _value = value; }
//	void setValue(char value) { _value = value; }
//	char getValue() { return _value; }
//	virtual string toString() { return string() + _value; };
//};
//
//class StringValue : public Value
//{
//private:
//	string _value;
//public:
//	StringValue() : Value("string") { _value = ""; }
//	StringValue(string value) : Value("string") { _value = value; }
//	void setValue(string value) { _value = value; }
//	string getValue() { return _value; }
//	virtual string toString() { return _value; };
//};
//
//class BoolValue : public Value
//{
//private:
//	bool _value;
//public:
//	BoolValue() : Value("bool") { _value = false; }
//	BoolValue(bool value) : Value("bool") { _value = value; }
//	void setValue(bool value) { _value = value; }
//	bool getValue() { return _value; }
//	virtual string toString() { return _value ? "true" : "false"; };
//};
//
//class FuncValue : public Value
//{
//private:
//	Type _returnType;
//	AST::Function* _value;
//public:
//	FuncValue() : Value("func") { _value = NULL; _returnType = ""; }
//	FuncValue(Type returnType) : Value("func") { _returnType = returnType; _value = NULL; }
//	FuncValue(AST::Function* value) : Value("func") { _value = value; _returnType = value->getReturnType(); }
//	void setValue(AST::Function* value);
//	Type getReturnType() { return _returnType; }
//	AST::Function* getValue() { return _value; }
//	virtual string toString() { return _value->toString(); };
//};
//
//class PropertyValue : public Value
//{
//private:
//	string _returnType;
//	Value* _thisPtr;
//	AST::Statement* _get;
//	AST::Statement* _set;
//public:
//	PropertyValue(AST::Statement* set, AST::Statement* get, string returnType) : Value("property") { _set = set; _get = get; _returnType = returnType; }
//	AST::Statement* getGet() { return _get; }
//	AST::Statement* getSet() { return _set; }
//	string getReturnType() { return _returnType; }
//	Value* getThisPtr() { return _thisPtr; }
//	virtual string toString() { return "<propertyTODO>"; };
//	void setThisPtr(Value* thisPtr) { _thisPtr = thisPtr; }
//};
//
//struct VariableTypeDefinition
//{
//	string type;
//	vector<string> modifiers;
//} typedef VariableTypeDefinition;
//
//struct FunctionDefinition
//{
//	FuncValue* value;
//	vector<string> modifiers;
//} typedef FunctionDefinition;
//
//struct PropertyDefinition 
//{
//	PropertyValue* value;
//	vector<string> modifiers;
//} typedef PropertyDefinition;
//
//struct TypeDefinition
//{
//	string _name;
//	unordered_map<string, VariableTypeDefinition> _variables;
//	unordered_map<string, PropertyDefinition> _properties;
//	unordered_map<string, FunctionDefinition> _functions;
//} typedef TypeDefinition;
//
//class TypeValue : public Value
//{
//private:
//	unordered_map<string, TypeDefinition> &_types;
//	TypeDefinition _typeDefinition;
//	unordered_map<string, Value*> _variables;
//public:
//	TypeValue(string typeName, unordered_map<string, TypeDefinition> &types);
//	virtual string toString() { return std::to_string(NULL); };
//	Value* getVariable(string name, string scope);
//	bool hasVariable(string name);
//};
//
//class PtrValue : public Value
//{
//private:
//	string _ptrType;
//	Value* _value;
//public:
//	PtrValue(string ptrType, Value* value) : Value("ptr") { _ptrType = ptrType; _value = value; }
//	virtual string toString() { return (_value) ? _value->toString() : "nullptr"; }
//	void setPtrType(string ptrType) { _ptrType = ptrType; }
//	void setValue(Value* value) { _value = value; }
//	string getPtrType() { return _ptrType; }
//	Value* getValue() { return _value; }
//};
//
//bool hasModifier(vector<string> &modifiers, string modifier);