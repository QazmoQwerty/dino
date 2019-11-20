#pragma once

#include "DstNode.h"

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

	 // ExpressionStatements
	 static DST::VariableDeclaration *decorate(AST::VariableDeclaration *node);
	 static DST::Assignment *decorate(AST::Assignment *node);

	 
	 static DST::Type *evalType(AST::Expression *node);

	 static unordered_map<unicode_string, DST::Type*, UnicodeHasherFunction> _variables;
	 static unordered_map<unicode_string, DST::TypeDeclaration*, UnicodeHasherFunction> _types;	// Should point to a class which saves all the functions, properties and variables of a class
};

