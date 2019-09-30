#pragma once

#include "InterpreterValues.h"

class Interpreter
{
private:
	vector<unordered_map<string, Value*>> _variables;	// index represents scope, string represents variable name
	unordered_map<string, TypeDefinition> _types;
	stack<string> _currentNamespace;
	stack<int> _currentMinScope;

	Value* interpretBinaryOp(AST::BinaryOperation* node);
	Value* interpretUnaryOp(AST::UnaryOperation* node);
	Value* interpretIncrement(AST::Increment* node);
	Value* interpretFuncCall(AST::FunctionCall* node);
	Value* interpretLiteral(AST::Literal* node);
	Value* interpretVariable(AST::Variable* node);
	Value* interpretAssignment(AST::Assignment* node);

	Value* interpretStatementBlock(AST::StatementBlock* node);
	Value* interpretTypeDeclaration(AST::TypeDeclaration* node);
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
	Interpreter() { _currentNamespace.push(""); _currentMinScope.push(0); }
};