﻿#include <fstream>
#include <iostream>
#include <sstream> 

#include "Lexer/Lexer.h"
#include "Parser/AstNode.h"
#include "Utils/AstToFile.h"
#include "CodeGenerator/CodeGenerator.h"
#include "Parser/Parser.h"
#include "Utils/Prettifier.h"
#include "Utils/Unicode/Utf8Handler.h"
#include "Decorator/Decorator.h"
#include "Utils/LibFileWriter/LibFileWriter.h"
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
	bool prettify = false;
} cmdOptions;

void showHelp() 
{
	llvm::errs() << "dino [filepath] [args]\n" 
		<< "    -help (show this help message)\n"
		<< "    -fmt (format: swaps all operators with their unicode counterparts)\n"
		<< "    -v (verbose: show ongoing compilation status)\n"
		<< "    -showlex (prints the output of the lexer)\n"
		<< "    -outAst (output a .gv file of the AST and DST)\n" 
		<< "    -lineAst (show line numbers in the AST and DST files)\n" 
		<< "    -bc [filepath] (output a .bc file to \"filepath\")\n" 
		<< "    -ll [filepath] (output a .ll file to \"filepath\")\n"
		<< "    -o [filepath] (output an executable file to \"filepath\")\n"
		<< "    -lib [dirpath] (build as a library to the directory \"dirpath\")\n"
		<< "    -O[0/1/2/3/s/z/d] (optimization levels, see 'opt -help', '-Od' means no optimizations)\n\n";
	exit(0);
}

string runCmd(string cmd, bool printOutput = true) // if print output is false, nothing will be printed untill the entire command is done
{
    std::string result = "";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) throw std::runtime_error("popen() failed in getOutputFromCmd");
    try {
        while (!feof(pipe)) {
			char c;
            if ((c=getc(pipe)) != EOF)
			{
                result += c;
                
                if (printOutput)
				{
					std::cout << c;
					std::cout.flush();
				}
			}
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}

cmdOptions *getCmdOptions(int argc, char *argv[]) 
{
	auto options = new cmdOptions();

	try 
	{
		for(int i = 1; i < argc; i++) 
		{
			if (strlen(argv[i]) && argv[i][0] == '-') 
			{
				if 		(!strcmp(argv[i], "-help") || 
							!strcmp(argv[i], "-h")) 		options->showHelp 			= true;
				else if (!strcmp(argv[i], "-verbose") || 
							!strcmp(argv[i], "-v"))		options->verbose 			= true;
				else if (!strcmp(argv[i], "-lex")) 		options->showLexerOutput 	= true;
				else if (!strcmp(argv[i], "-ast")) 		options->outputAstFiles 	= true;
				else if (!strcmp(argv[i], "-fmt")) 		options->prettify 			= true;
				else if (!strcmp(argv[i], "-lineAst")) 	options->showLineAST 		= true;
				else if (!strcmp(argv[i], "-showIR")) 	options->showIR 			= true;
				else if (!strcmp(argv[i], "-O0") || !strcmp(argv[i], "-O1") || !strcmp(argv[i], "-O2") 
						|| !strcmp(argv[i], "-O3") || !strcmp(argv[i], "-Os") || !strcmp(argv[i], "-Oz") || !strcmp(argv[i], "-Od"))	
					options->optLevel = argv[i];
				else if (!strcmp(argv[i], "-bc"))
				{
					options->outputBc = true;
					if (++i >= argc)
						throw "missing file name after '-bc'";
					else options->bcFileName = argv[i];
				}	
				else if (!strcmp(argv[i], "-o"))
				{
					options->outputExe = true;
					if (++i >= argc)
						throw "missing file name after '-o'";
					else options->exeFileName = argv[i];
				}
				else if (!strcmp(argv[i], "-lib"))
				{
					options->outputLib = true;
					if (++i >= argc)
						throw "missing file name after '-lib'";
					else options->libFileName = argv[i];
				}
				else if (!strcmp(argv[i], "-ll"))
				{
					options->outputLl = true;
					if (++i >= argc)
						throw "missing file name after '-ll'";
					else options->llFileName = argv[i];
				}
				else throw "unknown flag \"" + string(argv[i]) + "\"";
			}
			else 
			{
				if (options->fileName)
					throw "more than one input file specified";
				options->fileName = argv[i];
			}
		}
		if (options->showHelp)
			showHelp();
		if (!options->fileName)
			throw "incorrect usage, missing input file\nTry \"dino -h\" for more info.";
	} 
	catch (const char * c) 
	{
		llvm::errs() << FRED(BOLD("Error: ")) << c << "\n";
		delete options;
		exit(0);
	}
	catch (string s) 
	{
		llvm::errs() << FRED(BOLD("Error: ")) << s << "\n";
		delete options;
		exit(0);
	}
	return options;
}

int main(int argc, char *argv[])
{
	auto cmd = getCmdOptions(argc, argv);

	if (cmd->prettify) 
	{
		Prettifier::Prettify(cmd->fileName);
		if (!cmd->outputBc && !cmd->outputExe && !cmd->outputLib && !cmd->outputLl && !cmd->outputAstFiles)
			exit(0);
	}


	CodeGenerator::setup(cmd->outputLib);
	OperatorsMap::setup();
	Lexer::setup();
	DST::setup();
	Decorator::setup(cmd->outputLib);
	try
	{
		if (cmd->verbose)
			llvm::errs() << "Starting build...\n";
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
		
		CodeGenerator::startCodeGen(dst);

		if (cmd->verbose)
			llvm::errs() << "Finished generating IR...\n";

		string bcFileName = "";
		if (cmd->outputBc) 
			bcFileName = string(cmd->bcFileName);
		else if(cmd->outputExe || cmd->outputLl)
		{
			bcFileName = runCmd("mktemp dinoBcTmp-XXXXX.bc", false);
			if (bcFileName[bcFileName.size() - 1] == '\n')
				bcFileName.pop_back();
		}

		if (cmd->outputBc || cmd->outputExe || cmd->outputLl)
		{
			CodeGenerator::writeBitcodeToFile(dst, bcFileName);
			if (cmd->verbose)
				llvm::errs() << "outputted .bc file\n";
		}

		if (cmd->outputLib)
		{
			mkdir(cmd->libFileName, 0777);
			auto libBcFileName = string(cmd->libFileName) + '/' + string(cmd->libFileName) + ".bc";
			LibFileWriter::Write(string(cmd->libFileName) + '/' + string(cmd->libFileName) + ".dinh", libBcFileName, dst);
			CodeGenerator::writeBitcodeToFile(dst, libBcFileName);
			if (cmd->verbose)
				llvm::errs() << "outputted lib files\n";
		}

		if (cmd->optLevel && strcmp(cmd->optLevel, "-Od"))
		{
			runCmd(string("opt ") + bcFileName + " " + cmd->optLevel + " -o " + bcFileName);
			if (cmd->verbose)
				llvm::errs() << "Optimized bitcode...\n";
		}

		if (cmd->outputLl)
		{
			runCmd(string("llvm-dis ") + bcFileName + " -o " + cmd->llFileName);
			if (cmd->verbose)
				llvm::errs() << "outputted .ll file\n";
		}

		if (cmd->outputExe)
		{
			runCmd(string("clang++ -Wno-override-module -lgc ") + bcFileName + " -o " + cmd->exeFileName);
			if (cmd->verbose)
				llvm::errs() << "outputted ELF file\n";
		}

		if (bcFileName != "" && !cmd->outputBc)
			runCmd("rm " + bcFileName);

		// TODO - clear memory!
		// Decorator::clear();	
	} 
	catch (exception e) { llvm::errs() << e.what() << "\n"; }
	catch (const char *err) { llvm::errs() << err << "\n"; }
	catch (string err) { llvm::errs() << err << "\n"; }
	catch (Error err) { ErrorReporter::showAll(); }

	llvm::llvm_shutdown();
	return 0;
}