#include <fstream>
#include <iostream>
#include <sstream> 

#include "Lexer.h"

int main()
{
	std::ifstream t("Text.txt");
	std::stringstream buffer;
	buffer << t.rdbuf();
	std::string str = buffer.str();

	OperatorsMap::setup();
	Lexer::setup();
	
	for (Token * t : Lexer::lex(str))
	{
		printToken(t);
	}

	system("pause");
}