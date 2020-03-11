#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include <stack>
#include "../Lexer/Token.h"
#include "../Other/OperatorsMap.h"
#include "../Other/TypeEnums.h"
#include "AstNode.h"

#include <string>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <sstream> 
#include <set>

#include "../Lexer/Lexer.h"
#include "../Lexer/Preprocessor.h"

using std::set;
using std::unordered_map;

class Parser
{
public:
	static AST::StatementBlock * parseFile(string fileName, bool showLexerOutput = false);
	AST::StatementBlock * includeFile();
	AST::StatementBlock * importFile(PositionInfo currPos);

	Parser(vector<Token*>& tokens) : _tokens(tokens) {_index = 0; }
	Token* getToken(unsigned int index);
	Token* peekToken() { return getToken(_index); }
	Token* nextToken() { return getToken(_index++); }
	
	AST::Node* parse(int lastPrecedence = 0);
	AST::StatementBlock* parseBlock(OperatorType expected = OperatorType::OT_EOF);

	void expectLineBreak();
	void expectOperator(OperatorType ot);
	unicode_string expectIdentifier();
	AST::ExpressionList * expectIdentifierList();

	AST::Expression* convertToExpression(AST::Node* node);
	AST::Statement* convertToStatement(AST::Node* node);

	AST::Statement* parseStatement();
	AST::Expression* parseExpression(int precedence = 0);
	AST::Expression* parseOptionalExpression(int precedence = 0);
	AST::StatementBlock* parseInnerBlock();
	int precedence(Token* token, int category);
	int leftPrecedence(OperatorToken* token, int category);
private:
	static set<string> _parsedFiles;

	bool isOperator(Token * token, OperatorType type) { return token->_type == TT_OPERATOR && ((OperatorToken*)token)->_operator._type == type; };
	bool eatOperator(OperatorType type) { if (isOperator(peekToken(), type)) { nextToken(); return true; } return false; }
	
	bool eatLineBreak() { if (peekToken()->_type == TT_LINE_BREAK) { nextToken(); return true; } return false; }

	void skipLineBreaks() { while (eatLineBreak()); }

	AST::Node* std(Token* token);
	AST::Node* nud(Token* token);
	AST::Node* led(AST::Node * left, Token * token);

	vector<Token*>& _tokens;
	unordered_map<unicode_string, AST::NamespaceDeclaration*, UnicodeHasherFunction> _namespaces;
	unsigned int _index;
};