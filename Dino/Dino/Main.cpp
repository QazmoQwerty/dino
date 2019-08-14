#include <fstream>
#include <iostream>
#include <sstream> 

#include "Lexer.h"
#include "Preparser.h"
#include "AstNode.h"
#include "AstToFile.h"
#include "Parser.h"

int main() 
{
	std::ifstream t("Text.txt"/* "DinoSyntax.txt"*/);
	std::stringstream buffer;
	buffer << t.rdbuf();
	std::string str = buffer.str();

	OperatorsMap::setup();
	Lexer::setup();
	try
	{
		Parser p = Parser(Preparser::Preparse(Lexer::lex(str)));
		AST::Node* ast = p.parse();
		astToFile("AstDisplay.gv", ast);
	} 
	catch (exception e) { std::cout << e.what(); }
	

	/*int i = 0;

	auto ite = new AST::IfThenElse(++i);
	auto condition = new AST::BinaryOperation(++i);
	condition->setLeft(new AST::Integer(++i, 12));
	condition->setRight(new AST::Integer(++i, 14));
	auto block1 = new AST::StatementBlock(++i);
	auto block2 = new AST::StatementBlock(++i);
	ite->setCondition(condition);
	ite->setIfBranch(block1);
	ite->setElseBranch(block2);
	astToFile("AstDisplay.gv", ite);*/

	system("pause");
}