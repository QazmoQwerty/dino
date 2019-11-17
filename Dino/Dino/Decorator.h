#pragma once

#include "DstNode.h"

using std::unordered_map;


class Decorator
{
public:
	 static DST::Node *decorate(AST::Node *node);
	 static DST::Expression *decorate(AST::Variable *node);
	 static DST::VariableDeclaration *decorate(AST::VariableDeclaration *node);
	 
	 static DST::Type *evalType(AST::Expression *node);

	 static unordered_map<unicode_string, DST::Type*, UnicodeHasherFunction> _variables;
	 static unordered_map<unicode_string, /* TypeDefinition* */ void*, UnicodeHasherFunction> _types;	// Should point to a class which saves all the functions, properties and variables of a class
};

