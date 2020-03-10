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
#include <sys/stat.h>

typedef struct cmdOptions 
{
	const char *fileName;
	bool showHelp = false;
	bool verbose = false;
	bool showLexerOutput = false; 
	bool outputAstFiles = false; 
	bool showLineAST = false;
	bool executeInterpret = false;
	bool showIR = false;
	bool outputBc = false;
	const char *bcFileName;
	bool outputLl = false;
	const char *llFileName;
	bool outputExe = false;
	const char *exeFileName;
	bool outputLib = false;
	const char *libFileName;
	char *optLevel = NULL;
	
	// const char *libBcFileName;
} cmdOptions;

void showHelp() 
{
	llvm::errs() << "Dino.exe [filepath] [args]\n" 
		<< "-help (show this help message)\n"
		<< "-v (verbose: show ongoing compilation status"
		<< "-showlex (prints the output of the lexer)\n"
		<< "-outAst (output a .gv file of the AST and DST)\n" 
		<< "-lineAst (show line numbers in the AST and DST files)\n" 
		<< "-i (run the program in an LLVM interpreter for testing purposes - for debug purposes only!)\n" 
		<< "-bc [filepath] (output a .bc file to \"filepath\")\n" 
		<< "-ll [filepath] (output a .ll file to \"filepath\")\n"
		<< "-o [filepath] (output a .exe file to \"filepath\")\n"
		<< "-lib [dirpath] (build as a library to the directory \"dirpath\")\n"
		<< "-O[0/1/2/3/s/z/d] (optimization levels, see 'opt -help', '-Od' means no optimizations)\n\n";
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
				else if (strcmp(argv[i], "-v") == 0)			options->verbose = true;
				else if (strcmp(argv[i], "-showLex") == 0) 	options->showLexerOutput = true;
				else if (strcmp(argv[i], "-outAst") == 0) 	options->outputAstFiles = true;
				else if (strcmp(argv[i], "-lineAst") == 0) 	options->showLineAST = true;
				else if (strcmp(argv[i], "-showIR") == 0) 	options->showIR = true;
				else if (strcmp(argv[i], "-i") == 0)	options->executeInterpret = true;
				else if (!strcmp(argv[i], "-O0") || !strcmp(argv[i], "-O1") || !strcmp(argv[i], "-O2") 
					|| !strcmp(argv[i], "-O3") || !strcmp(argv[i], "-Os") || !strcmp(argv[i], "-Oz") || !strcmp(argv[i], "-Od"))	
					options->optLevel = argv[i];
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
				}
				else if (strcmp(argv[i], "-ll") == 0)
				{
					options->outputLl = true;
					if (++i >= argc)
						throw "Error: missing file name after '-ll'";
					else options->llFileName = argv[i];
				}
			}
		}
		if (options->showHelp)
			showHelp();
	} 
	catch (const char * c) 
	{
		llvm::errs() << c << "\n";
		delete options;
		exit(0);
	}
	if (!options->outputBc)
		options->bcFileName = "temp.bc";
	return options;
}

int main(int argc, char *argv[])
{
	auto cmd = getCmdOptions(argc, argv);

	CodeGenerator::setup(cmd->outputLib);
	OperatorsMap::setup();
	Lexer::setup();
	Decorator::setup(cmd->outputLib);
	DST::setup();
	try
	{
		AST::Node *ast = Parser::parseFile(cmd->fileName, cmd->showLexerOutput);

		if (cmd->verbose)
			llvm::errs() << "Finished parsing...\n";
		if (cmd->outputAstFiles) 
		{
			astToFile("AstDisplay.gv", ast, cmd->showLineAST);
			if (cmd->verbose)
				llvm::errs() << "Wrote \"AstDisplay.gv\"...\n";	
		}

		DST::Program* dst = Decorator::decorateProgram(dynamic_cast<AST::StatementBlock*>(ast));

		if (cmd->verbose)
			llvm::errs() << "Finished decorating...\n";
		if (cmd->outputAstFiles)
		{
			dstToFile("DstDisplay.gv", dst, false);
			if (cmd->verbose)
				llvm::errs() << "Wrote \"DstDisplay.gv\"...\n";	
		}
		
		auto mainFunc = CodeGenerator::startCodeGen(dst);

		if (cmd->verbose)
			llvm::errs() << "Finished generating IR...\n";

		if (cmd->executeInterpret)
			CodeGenerator::execute(mainFunc);

		if (cmd->outputBc || cmd->outputExe || cmd->outputLl)
		{
			CodeGenerator::writeBitcodeToFile(dst, cmd->bcFileName);
			if (cmd->verbose)
				llvm::errs() << "outputted .bc file\n";
		}

		if (cmd->outputLib)
		{			
			mkdir(cmd->libFileName, 0777);
			auto bcFileName = string(cmd->libFileName) + '/' + string(cmd->libFileName) + ".bc";
			LibFileWriter::Write(string(cmd->libFileName) + '/' + string(cmd->libFileName) + ".dinoh", bcFileName, dst);
			CodeGenerator::writeBitcodeToFile(dst, bcFileName);
			if (cmd->verbose)
				llvm::errs() << "outputted lib files\n";
		}

		if (cmd->optLevel && strcmp(cmd->optLevel, "-Od"))
		{
			system((string("opt ") + cmd->bcFileName + " " + cmd->optLevel).c_str());
			if (cmd->verbose)
				llvm::errs() << "Optimized bitcode...\n";
		}

		if (cmd->outputLl)
		{
			system((string("llvm-dis ") + cmd->bcFileName + " -o " + cmd->llFileName).c_str());
			if (cmd->verbose)
				llvm::errs() << "outputted .ll file\n";
		}

		if (cmd->outputExe)
		{
			system((string("clang++ -Wno-override-module ") + cmd->bcFileName + " -o " + cmd->exeFileName).c_str());
			if (cmd->verbose)
				llvm::errs() << "outputted ELF file\n";
		}

		// TODO - clear memory!
		// Decorator::clear();	
	} 
	catch (DinoException e) { llvm::errs() << e.errorMsg() << "\n"; }
	catch (exception e) { llvm::errs() << e.what() << "\n"; }
	catch (const char *err) { llvm::errs() << err << "\n"; }

	llvm::llvm_shutdown();
	return 0;
}