#include <fstream>
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
#include "LibFileWriter/LibFileWriter.h"
#include <stdio.h>

typedef struct cmdOptions 
{
	const char *fileName;
	bool showHelp = false;
	bool showLexerOutput = false; 
	bool outputAstFiles = false; 
	bool showLineAST = false;
	bool executeInterpret = false;
	bool showIR = false;
	bool outputBc = false;
	const char *bcFileName;
	bool outputExe = false;
	const char *exeFileName;
	bool outputLib = false;
	const char *libFileName;
	const char *libBcFileName;
} cmdOptions;

void showHelp() 
{
	std::cout << "Dino.exe [filepath] [args]" << std::endl
		<< "-help (show this help message)" << std::endl
		<< "-showlex (prints the output of the lexer)" << std::endl
		<< "-outAst (output a .gv file of the AST and DST)" << std::endl
		<< "-lineAst (show line numbers in the AST and DST files)" << std::endl
		<< "-i (run the program in an LLVM interpreter for testing purposes)" << std::endl
		<< "-bc [filepath] (output a .bc file to \"filepath\")" << std::endl
		<< "-o [filepath] (output a .exe file to \"filepath\")" << std::endl << std::endl;
}

cmdOptions *getCmdOptions(int argc, char *argv[]) 
{
	auto options = new cmdOptions();

	try 
	{
		if (argc > 1 && strcmp(argv[1], "-help") == 0) {
			showHelp();
			exit(0);
		}
		else 
		{
			if (argc == 1)
				throw "incorrect usage! (use -help for more info)";
			options->fileName = argv[1];
			for(int i = 2; i < argc; i++) 
			{
				if 		(strcmp(argv[i], "-help") == 0) 		options->showHelp = true;
				else if (strcmp(argv[i], "-showLex") == 0) 	options->showLexerOutput = true;
				else if (strcmp(argv[i], "-outAst") == 0) 	options->outputAstFiles = true;
				else if (strcmp(argv[i], "-lineAst") == 0) 	options->showLineAST = true;
				else if (strcmp(argv[i], "-showIR") == 0) 	options->showIR = true;
				else if (strcmp(argv[i], "-i") == 0)	options->executeInterpret = true;
				else if (strcmp(argv[i], "-bc") == 0)
				{
					options->outputBc = true;
					if (++i >= argc)
						throw "Error: missing file name after '-bc'";
					else options->bcFileName = argv[i];
				}	
				else if (strcmp(argv[i], "-o") == 0)
				{
					options->outputExe = true;
					if (++i >= argc)
						throw "Error: missing file name after '-o'";
					else options->exeFileName = argv[i];
				}
				else if (strcmp(argv[i], "-lib") == 0)
				{
					options->outputLib = true;
					if (++i >= argc)
						throw "Error: missing file name after '-lib'";
					else options->libFileName = argv[i];
					if (++i >= argc)
						throw "Error: missing second file name after '-lib'";
					else options->libBcFileName = argv[i];
				}
			}
		}
		if (options->showHelp)
			showHelp();
	} 
	catch (const char * c) 
	{
		std::cout << c << std::endl;
		delete options;
		exit(0);
	}
	return options;
}

int main(int argc, char *argv[])
{
	auto cmd = getCmdOptions(argc, argv);

	CodeGenerator::setup();
	OperatorsMap::setup();
	Lexer::setup();
	Decorator::setup(cmd->outputLib);
	DST::setup();
	try
	{
		AST::Node *ast = Parser::parseFile(cmd->fileName, cmd->showLexerOutput);

		if (cmd->outputAstFiles) 
		{
			astToFile("AstDisplay.gv", ast, cmd->showLineAST);
			std::cout << "Wrote \"AstDisplay.gv\"..." << std::endl;	
		}
		std::cout << "Finished parsing..." << std::endl;

		DST::Program* dst = Decorator::decorateProgram(dynamic_cast<AST::StatementBlock*>(ast));

		std::cout << "Finished decorating..." << std::endl;
		if (cmd->outputAstFiles)
			dstToFile("DstDisplay.gv", dst, false);
		
		auto mainFunc = CodeGenerator::startCodeGen(dst);
		std::cout << "Finished generating IR..." << std::endl;

		if (cmd->executeInterpret)
			CodeGenerator::execute(mainFunc);

		if (cmd->outputBc) 
		{
			CodeGenerator::writeBitcodeToFile(dst, cmd->bcFileName);
			llvm::errs() << "outputted .bc file\n";
		}

		if (cmd->outputExe)
		{
			if (!cmd->outputBc)
			{
				cmd->bcFileName = "temp.bc";
				CodeGenerator::writeBitcodeToFile(dst, cmd->bcFileName);
			}
			system((string("llc ") + cmd->bcFileName + " -o temp.s").c_str());
			system((string("gcc temp.s -no-pie -o ") + cmd->exeFileName).c_str());
			llvm::errs() << "outputted ELF file\n";
		}

		if (cmd->outputLib)
		{
			LibFileWriter::Write(cmd->libFileName, cmd->libBcFileName, dst);
			CodeGenerator::writeBitcodeToFile(dst, cmd->libBcFileName);
			std::cout << "outputted lib files" << std::endl;
		}

		Decorator::clear();
	} 
	catch (DinoException e) { std::cout << e.errorMsg() << std::endl; }
	catch (exception e) { std::cout << e.what() << std::endl; }
	catch (const char *err) { std::cout << err << std::endl; }

	//if (argc <= 1)
	//	system("pause");
	return 0;
}