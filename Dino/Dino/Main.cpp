#include <fstream>
#include <iostream>
#include <sstream> 

#include "Lexer.h"
#include "Preparser.h"
#include "AstNode.h"
#include "AstToFile.h"

int main() 
{
	//std::ifstream t(/*"Text.txt"*/ "DinoSyntax.txt");
	//std::stringstream buffer;
	//buffer << t.rdbuf();
	//std::string str = buffer.str();

	//OperatorsMap::setup();
	//Lexer::setup();
	//
	//try 
	//{
	//	for (vector<Token *>* v : Preparser::Preparse(Lexer::lex(str)))
	//	{
	//		for(Token * t : *v)
	//			printToken(t);
	//		std::cout << std::endl << " ----------------- " << std::endl << std::endl;
	//	}
	//} 
	//catch (DinoException d) 
	//{
	//	std::cout << "ERR_" << d.getType() << ": Error in line " << d.getLine() << ": \"" << d.what() << "\"" << std::endl;
	//	if (d.getInfo() != "")
	//		std::cout << "Additional info: " << d.getInfo() << std::endl;
	//}

	auto ite = new AST::IfThenElse(1);

	
	astToFile("", NULL);

	system("pause");
}