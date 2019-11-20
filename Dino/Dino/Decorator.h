#pragma once

#include "DstNode.h"

#define CONDITION_TYPE unicode_string("bool")

using std::unordered_map;

class Decorator
{
public:
	static void setup();

	 static DST::Node *decorate(AST::Node *node);
	 static DST::Expression *decorate(AST::Expression *node);
	 static DST::Statement *decorate(AST::Statement *node);

	 // Expressions
	 static DST::Expression *decorate(AST::Variable *node);
	 static DST::BinaryOperation *decorate(AST::BinaryOperation *node);
	 static DST::Literal *decorate(AST::Literal *node);
	 
	 //Statements
	 static DST::StatementBlock *decorate(AST::StatementBlock *node);
	 static DST::IfThenElse *decorate(AST::IfThenElse *node);
	 static DST::ForLoop *decorate(AST::ForLoop *node);
	 static DST::WhileLoop *decorate(AST::WhileLoop *node);

	 // ExpressionStatements
	 static DST::VariableDeclaration *decorate(AST::VariableDeclaration *node);
	 static DST::Assignment *decorate(AST::Assignment *node);

	 
	 static DST::Type *evalType(AST::Expression *node);

	 static bool isCondition(DST::Expression *node);

	 static unordered_map<unicode_string, DST::Type*, UnicodeHasherFunction> _variables;
	 static unordered_map<unicode_string, DST::TypeDeclaration*, UnicodeHasherFunction> _types;	// Should point to a class which saves all the functions, properties and variables of a class
};

