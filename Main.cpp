﻿#include <fstream>
#include <iostream>
#include <sstream> 

#include "Lexer/Lexer.h"
#include "Lexer/Preprocessor.h"
#include "Parser/AstNode.h"
#include "Other/AstToFile.h"
#include "CodeGenerator/CodeGenerator.h"
#include "Parser/Parser.h"
#include "Other/Utf8Handler.h"
#include "Decorator/Decorator.h"
#include <stdio.h>

char* getCmdOption(char ** begin, char ** end, const std::string & option)
{
	/*char ** itr = std::find(begin, end, option);
	if (itr != end && ++itr != end)
		return *itr;*/
	return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option)
{
	//return std::find(begin, end, option) != end;
	return false;
}	

int main(int argc, char *argv[])
{
	CodeGenerator::setup();

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

	//try {
	//	if (argc <= 1)
	//		t = std::ifstream(/*"DinoCodeExamples/Test.dino"*/"Test.dino");
	//	else t = std::ifstream(argv[1]);
	//}
	//catch (exception e) {
	//	std::cout << e.what() << std::endl;
	//	exit(0);
	//}
	//std::stringstream buffer;
	//buffer << t.rdbuf();
	//std::string str = buffer.str();

	OperatorsMap::setup();
	Lexer::setup();
	Decorator::setup();
	DST::setup();
	try
	{
		/*auto lexed = Lexer::lex(str);
		auto vec = Preprocessor::preprocess(lexed);

		std::cout << "Finished lexing..." << std::endl;

		if (showLexerOutput or true)
			for (auto i : vec) printToken(i);

		Parser p = Parser(vec);
		AST::Node* ast = p.parseBlock();*/
		AST::Node *ast = Parser::parseFile(argc <= 1 ? "DinoCodeExamples/Test.dino" : argv[1]);

		if (outputAstFile)
			astToFile("AstDisplay.gv", ast, showLineAST);

		std::cout << "Finished parsing..." << std::endl;

		//DST::Node* dst = Decorator::decorate(ast);
		DST::Program* dst = Decorator::decorateProgram(dynamic_cast<AST::StatementBlock*>(ast));
		dstToFile("DstDisplay.gv", dst, false);

		std::cout << "Finished decorating..." << std::endl;

		auto mainFunc = CodeGenerator::startCodeGen(dst);
		std::cout << "Finished generating IR..." << std::endl;
		CodeGenerator::execute(mainFunc);

		Decorator::clear();
	} 
	catch (DinoException e) { std::cout << e.errorMsg() << std::endl; }
	catch (exception e) { std::cout << e.what() << std::endl; }
	catch (const char *err) { std::cout << err << std::endl; }

	//if (argc <= 1)
	//	system("pause");
	return 0;
}