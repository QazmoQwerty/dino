// #pragma once

#include "Utils/CLI/CLI.h"

int main(int argc, char *argv[])
{
	auto cmd = CLI::getCmdOptions(argc, argv);

	CodeGenerator::setup(cmd.outputLib, cmd.emitDebugInfo, cmd.noGC);
	OperatorsMap::setup();
	Lexer::setup();
	DST::setup();
	Decorator::setup(cmd.outputLib);
	try
	{
		if (cmd.verbose) llvm::errs() << "Starting build...\n";
		AST::Node *ast = Parser::parseFile(cmd.fileName, cmd.showLexerOutput);

		if (cmd.verbose) llvm::errs() << "Finished parsing...\n";
		if (cmd.outputAstFiles) 
		{
			astToFile("AstDisplay.gv", ast, cmd.showLineAST);
			if (cmd.verbose) llvm::errs() << "Wrote \"AstDisplay.gv\"...\n";	
		}

		DST::Program* dst = Decorator::decorateProgram(dynamic_cast<AST::StatementBlock*>(ast));
		if (cmd.verbose) llvm::errs() << "Finished decorating...\n";

		if (cmd.outputAstFiles)
		{
			dstToFile("DstDisplay.gv", dst, false);
			if (cmd.verbose) llvm::errs() << "Wrote \"DstDisplay.gv\"...\n";	
		}
		
		CodeGenerator::startCodeGen(dst);
		if (cmd.verbose) llvm::errs() << "Finished generating IR...\n";

		string bcFileName = "";
		if (cmd.outputBc) 
			bcFileName = string(cmd.bcFileName);
		else if (cmd.outputExe || cmd.run || cmd.outputLl)
		{
			bcFileName = CLI::runCmd("mktemp dinoBcTmp-XXXXX.bc", false);
			if (bcFileName[bcFileName.size() - 1] == '\n')
				bcFileName.pop_back();
		}

		string exeFileName = "";
		if (cmd.outputExe) 
			exeFileName = string(cmd.exeFileName);
		else if (cmd.run)
		{
			exeFileName = CLI::runCmd("mktemp dinoExeTmp-XXXXX", false);
			if (exeFileName[exeFileName.size() - 1] == '\n')
				exeFileName.pop_back();
		}

		if (cmd.outputBc || cmd.outputExe || cmd.outputLl || cmd.run)
		{
			CodeGenerator::writeBitcodeToFile(dst, bcFileName);
			if (cmd.verbose) llvm::errs() << "outputted .bc file\n";
		}

		if (cmd.outputLib)
		{
			mkdir(cmd.libFileName, 0777);
			auto libBcFileName = string(cmd.libFileName) + '/' + string(cmd.libFileName) + ".bc";
			LibFileWriter::Write(string(cmd.libFileName) + '/' + string(cmd.libFileName) + ".dinh", libBcFileName, dst);
			CodeGenerator::writeBitcodeToFile(dst, libBcFileName);
			if (cmd.verbose) llvm::errs() << "outputted lib files\n";
		}

		if (cmd.optLevel && strcmp(cmd.optLevel, "-Od"))
		{
			CLI::runCmd(string("opt ") + bcFileName + " " + cmd.optLevel + " -o " + bcFileName);
			if (cmd.verbose) llvm::errs() << "Optimized bitcode...\n";
		}

		if (cmd.outputLl)
		{
			CLI::runCmd(string("llvm-dis ") + bcFileName + " -o " + cmd.llFileName);
			if (cmd.verbose) llvm::errs() << "outputted .ll file\n";
		}

		if (cmd.outputExe || cmd.run)
		{
			CLI::runCmd(string("clang++ -Wno-override-module -lgc ") + bcFileName + " -o " + exeFileName);
			if (cmd.run) system((string("./") + exeFileName).c_str());
			else if (cmd.verbose) llvm::errs() << "outputted ELF file\n";
		}

		if (bcFileName != "" && !cmd.outputBc)
			CLI::runCmd("rm " + bcFileName);

		if (exeFileName != "" && !cmd.outputExe)
			CLI::runCmd("rm " + exeFileName);
		
	} 
	catch (exception e) { llvm::errs() << e.what() << "\n"; exit(1); }
	catch (const char *err) { llvm::errs() << err << "\n"; exit(1); }
	catch (string err) { llvm::errs() << err << "\n"; exit(1); }
	catch (ErrorReporter::Error err) { 
		ErrorReporter::showAll(); 
		llvm::errs() << "\n";
		exit(1); 
	}

	llvm::llvm_shutdown();
	return 0;
}