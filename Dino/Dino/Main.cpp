#include <fstream>
#include <iostream>
#include <sstream> 

#include "Lexer.h"
#include "Preprocessor.h"
#include "AstNode.h"
#include "AstToFile.h"
#include "Parser.h"
#include "Interpreter.h"

int main() 
{
	std::ifstream t(/*"TestCode.dino"*/"DinoCodeExamples/LinkedList.dino" /*"DinoSyntax.txt"*/);
	std::stringstream buffer;
	buffer << t.rdbuf();
	std::string str = buffer.str();

	OperatorsMap::setup();
	Lexer::setup();
	try
	{
		auto lexed = Lexer::lex(str);
		auto vec = Preprocessor::preprocess(lexed);
		
		//for (auto i : vec) printToken(i);

		Parser p = Parser(vec);
		AST::Node* ast = p.parseBlock();
		astToFile("AstDisplay.gv", ast);

		Interpreter i;
		i.interpret(ast);
	} 
	catch (exception e) { std::cout << e.what() << std::endl; }
	catch (const char *err) { std::cout << err << std::endl; }

	system("pause");
}