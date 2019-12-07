#pragma once

#include "DstNode.h"

#define CONDITION_TYPE unicode_string("bool")

using std::unordered_map;

class Decorator
{
public:
	static void setup();
	static DST::Node *decorate(AST::Node *node);
	static void clear();

private:
	static DST::Expression *decorate(AST::Expression *node);
	static DST::Statement *decorate(AST::Statement *node);

	// Expressions
	static DST::Expression *decorate(AST::Identifier *node);
	static DST::BinaryOperation *decorate(AST::BinaryOperation *node);
	static DST::Expression *decorate(AST::Literal *node);
	static DST::ExpressionList *decorate(AST::ExpressionList *node);
	static DST::Expression *decorate(AST::FunctionCall *node);
	static DST::FunctionLiteral *decorate(AST::Function *node);

	// Statements
	static DST::StatementBlock *decorate(AST::StatementBlock *node);
	static DST::IfThenElse *decorate(AST::IfThenElse *node);
	static DST::SwitchCase *decorate(AST::SwitchCase *node);
	static DST::ForLoop *decorate(AST::ForLoop *node);
	static DST::WhileLoop *decorate(AST::WhileLoop *node);
	static DST::FunctionDeclaration *decorate(AST::FunctionDeclaration *node);
	static DST::PropertyDeclaration * decorate(AST::PropertyDeclaration * node);
	static DST::UnaryOperationStatement * decorate(AST::UnaryOperationStatement * node);
	
	// ExpressionStatements
	static DST::VariableDeclaration *decorate(AST::VariableDeclaration *node);
	static DST::Assignment *decorate(AST::Assignment *node);
	

	static DST::Type *evalType(AST::Expression *node);

	static bool isCondition(DST::Expression *node);

	static vector<unordered_map<unicode_string, DST::Type*, UnicodeHasherFunction>> _variables;
	static unordered_map<unicode_string, DST::TypeDeclaration*, UnicodeHasherFunction> _types;
	static vector<void*> _toDelete;

	// Scope
	static unsigned int currentScope() { return (int)_variables.size() - 1; }
	static void enterBlock() { _variables.push_back(unordered_map<unicode_string, DST::Type*, UnicodeHasherFunction>()); }
	static void leaveBlock();
};

