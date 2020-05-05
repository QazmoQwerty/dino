/*
	The Decorator is our semantic analyzer.
	The Decorator gets an AST from the Parser and transforms it into a DST (Decorated Syntax Tree).
	This process includes the checking of (most) compile-time errors:
		* type checking
		* making sure there are no duplicate variable names
		* making sure there is a 'Main' function
		* etc.
*/
#pragma once

#include "DstNode.h"

#define CONDITION_TYPE unicode_string("bool")
#include <stack>

using std::stack;
using std::unordered_map;

#define ERROR_TYPE_NAME "error"
#define MAIN_FUNC "Main"

class Decorator
{
public:
	static void setup(bool isLibrary = false);
	static DST::Node *decorate(AST::Node *node);
	static void clear();
	static DST::Program *decorateProgram(AST::StatementBlock* node);

private:
	static DST::NamespaceDeclaration *partA(AST::NamespaceDeclaration *node, bool isStd = false);
	static void partB(DST::NamespaceDeclaration *node);	
	static void partC(DST::NamespaceDeclaration *node);
	static void partD(DST::NamespaceDeclaration *node);
	static void partE(DST::NamespaceDeclaration *node);

	static DST::UnsetGenericType *createGenericTy(AST::Expression *exp);

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
	static DST::Comparison *decorate(AST::Comparison * node);
	static DST::Increment *decorate(AST::Increment *node);

	// Statements
	static DST::StatementBlock *decorate(AST::StatementBlock *node);
	static DST::IfThenElse *decorate(AST::IfThenElse *node);
	static DST::SwitchCase *decorate(AST::SwitchCase *node);
	static DST::TryCatch *decorate(AST::TryCatch *node);
	static DST::ForLoop *decorate(AST::ForLoop *node);
	static DST::WhileLoop *decorate(AST::WhileLoop *node);
	static DST::DoWhileLoop *decorate(AST::DoWhileLoop *node);
	static DST::UnaryOperationStatement * decorate(AST::UnaryOperationStatement * node);
	static DST::ConstDeclaration * decorate(AST::ConstDeclaration *node);
	
	// ExpressionStatements
	static DST::VariableDeclaration *decorate(AST::VariableDeclaration *node);
	static DST::Assignment *decorate(AST::Assignment *node);
	static DST::Type *evalType(AST::Expression *node);

	static bool isCondition(DST::Expression *node);

	static vector<unordered_map<unicode_string, DST::Expression*, UnicodeHasherFunction>> _variables;
	static vector<DST::NamespaceDeclaration*> _currentNamespace;
	static DST::TypeDeclaration *_currentTypeDecl;
	static DST::Program *_currentProgram;
	static DST::FunctionDeclaration* _main;
	static DST::NamespaceDeclaration *_universalNs;
	static vector<DST::Node*> _toDelete;
	static bool _isLibrary;
	static unsigned _loopCount;

	// Scope
	static uint currentScope() { return (int)_variables.size() - 1; }
	static void enterBlock() { _variables.push_back(unordered_map<unicode_string, DST::Expression*, UnicodeHasherFunction>()); }
	static void leaveBlock();
};

