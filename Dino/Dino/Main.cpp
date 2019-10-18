#include <fstream>
#include <iostream>
#include <sstream> 

#include "Lexer.h"
#include "Preprocessor.h"
#include "AstNode.h"
#include "AstToFile.h"
#include "Parser.h"
//#include "Interpreter.h"
#include <Windows.h>
#include "Utf8Handler.h"

char* getCmdOption(char ** begin, char ** end, const std::string & option)
{
	char ** itr = std::find(begin, end, option);
	if (itr != end && ++itr != end)
		return *itr;
	return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option)
{
	return std::find(begin, end, option) != end;
}	

int main(int argc, char *argv[])
{
	std::ifstream t;
	bool showLexerOutput = false, outputAstFile = true, executeInterpret = true, showLineAST = false;

	if (cmdOptionExists(argv, argv + argc, "-help"))
	{
		std::cout << "Dino.exe [filepath] [args]" << std::endl;
		std::cout << "-showlex (prints out the output of the lexer)" << std::endl;
		std::cout << "-noAst (stops the program from outputting a .gv file of the AST)" << std::endl;
		std::cout << "-noRun (stops the interpreter from executing the program)" << std::endl << std::endl;
		return 0;
	}
	showLexerOutput = cmdOptionExists(argv, argv + argc, "-showlex");
	outputAstFile = !cmdOptionExists(argv, argv + argc, "-noAst");
	executeInterpret = !cmdOptionExists(argv, argv + argc, "-noRun");

	try {
		if (argc <= 1)
			t = std::ifstream("DinoCodeExamples/Test.dino");
		else t = std::ifstream(argv[1]);
	}
	catch (exception e) {
		std::cout << e.what() << std::endl;
		exit(0);
	}
	std::stringstream buffer;
	buffer << t.rdbuf();
	std::string str = buffer.str();
	//SetConsoleOutputCP(65001);
	//std::string test = std::string(u8"שלום");
	//bool b = test == str;
	//std::cout << b << std::endl << str.length() << std::endl;




	OperatorsMap::setup();
	Lexer::setup();
	try
	{
		auto lexed = Lexer::lex(str);
		auto vec = Preprocessor::preprocess(lexed);
		
		if (showLexerOutput)
			for (auto i : vec) printToken(i);

		Parser p = Parser(vec);
		AST::Node* ast = p.parseBlock();

		if (outputAstFile)
			astToFile("AstDisplay.gv", ast, showLineAST);

		
		/*if (executeInterpret) 
		{
			Interpreter i;
			i.interpret(ast);
		}*/
	} 
	catch (DinoException e) { std::cout << e.errorMsg() << std::endl; }
	catch (exception e) { std::cout << e.what() << std::endl; }
	catch (const char *err) { std::cout << err << std::endl; }

	if (argc <= 1)
		system("pause");
}