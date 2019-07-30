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
	
	try 
	{
		for (Token * t : Lexer::lex(str))
		{
			printToken(t);
		}
	} 
	catch (DinoException d) 
	{
		std::cout << "ERR_" << d.getType() << ": Error in line " << d.getLine() << ": \"" << d.what() << "\"" << std::endl;
		if (d.getInfo() != "")
			std::cout << "Additional info: " << d.getInfo() << std::endl;
	}

	system("pause");
}