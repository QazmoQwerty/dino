#pragma once

#include "DstNode.h"

#define CONDITION_TYPE unicode_string("bool")
#include <stack>

using std::stack;
using std::unordered_map;

class Decorator
{
public:
	static void setup();
	static DST::Node *decorate(AST::Node *node);
	static void clear();
	static DST::Program *decorateProgram(AST::StatementBlock* node);

private:
	static DST::NamespaceDeclaration *partA(AST::NamespaceDeclaration *node);
	static void partB(DST::NamespaceDeclaration *node);
	static void partC(DST::NamespaceDeclaration *node);
	static void partD(DST::NamespaceDeclaration *node);
	static void partE(DST::NamespaceDeclaration *node);

	static DST::Expression *decorate(AST::Expression *node);
	static DST::Statement *decorate(AST::Statement *node);

	// Expressions
	static DST::Expression *decorate(AST::Identifier *node);
	static DST::Expression *decorate(AST::BinaryOperation *node);
	static DST::Expression *decorate(AST::Literal *node);
	static DST::Expression *decorate(AST::ExpressionList *node);
	static DST::Expression *decorate(AST::FunctionCall *node);
	static DST::FunctionLiteral *decorate(AST::Function *node);
	static DST::Expression *decorate(AST::UnaryOperation * node);
	static DST::Expression *decorate(AST::ConditionalExpression * node);
	static DST::Increment *decorate(AST::Increment *node);

	// Statements
	static DST::NamespaceDeclaration *decorate(AST::NamespaceDeclaration *node);
	static DST::StatementBlock *decorate(AST::StatementBlock *node);
	static DST::IfThenElse *decorate(AST::IfThenElse *node);
	static DST::SwitchCase *decorate(AST::SwitchCase *node);
	static DST::ForLoop *decorate(AST::ForLoop *node);
	static DST::WhileLoop *decorate(AST::WhileLoop *node);
	static DST::DoWhileLoop *decorate(AST::DoWhileLoop *node);
	static DST::FunctionDeclaration *decorate(AST::FunctionDeclaration *node);
	static DST::PropertyDeclaration * decorate(AST::PropertyDeclaration * node);
	static DST::UnaryOperationStatement * decorate(AST::UnaryOperationStatement * node);
	static DST::TypeDeclaration * decorate(AST::TypeDeclaration * node);
	static DST::ConstDeclaration * decorate(AST::ConstDeclaration *node);
	
	// ExpressionStatements
	static DST::VariableDeclaration *decorate(AST::VariableDeclaration *node);
	static DST::Assignment *decorate(AST::Assignment *node);
	

	static DST::TypeSpecifierType *getPrimitiveType(std::string name);
	static DST::Type *evalType(AST::Expression *node);

	static bool isCondition(DST::Expression *node);
	//static bool isBool(DST::Type *type);

	static vector<unordered_map<unicode_string, DST::Type*, UnicodeHasherFunction>> _variables;
	static vector<DST::NamespaceDeclaration*> _currentNamespace;
	static DST::TypeDeclaration *_currentTypeDecl;
	static DST::Program *_currentProgram;
	static DST::FunctionDeclaration* _main;
	static DST::NullType *_nullType;
	static DST::UnknownType *_unknownType;
	//static unordered_map<unicode_string, DST::TypeDeclaration*, UnicodeHasherFunction> _types;
	static vector<DST::Node*> _toDelete;

	// Scope
	static unsigned int currentScope() { return (int)_variables.size() - 1; }
	static void enterBlock() { _variables.push_back(unordered_map<unicode_string, DST::Type*, UnicodeHasherFunction>()); }
	static void leaveBlock();
};

