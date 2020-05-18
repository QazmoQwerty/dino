/*
    Functions that handle file parsing + 'include'/'import' statements.
*/
#include "Parser.h"

set<string> Parser::_parsedFiles;

AST::StatementBlock * Parser::parseFile(string fileName, bool showLexerOutput)
{
	if (_parsedFiles.count(fileName))
		return new AST::StatementBlock();
	_parsedFiles.insert(fileName);
	std::ifstream t;
	try { t = std::ifstream(fileName); }
	catch (exception e) { std::cout << e.what() << std::endl; exit(0); }
	std::stringstream buffer;
	buffer << t.rdbuf();
	unicode_string str = buffer.str();
	auto vec = Lexer::lex(str, new SourceFile(fileName));
	if (showLexerOutput)
		for (auto i : vec) 
			printToken(i);
	Parser p = Parser(vec);
	auto block = p.parseBlock();
	return block;
}

AST::StatementBlock * Parser::importFile(ErrorReporter::Position currPos)
{
	auto t = nextToken();
	if (t->_type == TT_LITERAL && ((LiteralToken<bool>*)t)->_literalType != LT_STRING)
		throw ErrorReporter::report("Expected a file path", ERR_GENERAL, currPos);

	string importPath = ((LiteralToken<string>*)t)->_value;
	auto dir = opendir(importPath.c_str());
	if (!dir)
		throw ErrorReporter::report("Could not open directory \"" + importPath + '\"', ERR_GENERAL, currPos);
	AST::StatementBlock *block = NULL;
	while (auto ent = readdir(dir))
	{
		string fileName(ent->d_name);
		if (fileName.substr(fileName.find_last_of(".")) == ".dinh")
		{
			if (block)
				block->addStatement(parseFile(importPath + '/' + fileName));
			else block = parseFile(importPath + '/' + fileName);
		}
	}
	closedir(dir);
	if (!block)
		throw ErrorReporter::report("No .dinh files in directory \"" + importPath + '\"', ERR_GENERAL, currPos);
	auto import = new AST::Import(importPath);
	import->setPosition(currPos);
	block->addStatement(import);
	return block;
}

AST::StatementBlock * Parser::includeFile()
{
	auto t = nextToken();
	if (t->_type == TT_LITERAL && ((LiteralToken<bool>*)t)->_literalType == LT_STRING)
	{
		string fileName = ((LiteralToken<string>*)t)->_value;
		return parseFile(fileName);
	}
	else throw ErrorReporter::report("Expected a file path", ERR_GENERAL, peekToken()->_pos);
}