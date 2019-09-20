
#include "Preparser.h"

vector<Token*>& Preparser::Preparse(vector<Token*> tokens)
{
	auto preparsedTokens = new vector<Token*>();

	for (Token * token : tokens)
	{
		if (token->_type == TT_MULTI_LINE_COMMENT || token->_type == TT_WHITESPACE)
		{
			delete token;
			continue;
		}
		if (token->_type == TT_SINGLE_LINE_COMMENT || token->_type == TT_NEWLINE)	// TODO - make it so you can have multiple-line sections of code.
		{
			token->_type = TT_LINE_BREAK;
			token->_data = "|";
		}
		if (token->_type == TT_LINE_BREAK && (preparsedTokens->size() == 0 || preparsedTokens->back()->_type == TT_LINE_BREAK))
		{
			delete token;
			continue;
		}
		if (token->_type == TT_IDENTIFIER && OperatorsMap::getWordOperators().count(token->_data))
		{
			auto ot = new OperatorToken;
			ot->_data = token->_data;
			ot->_line = token->_line;
			ot->_type = TT_OPERATOR;
			ot->_operator = OperatorsMap::getWordOperators().find(ot->_data)->second;
			delete token;
			token = ot;
		}
		preparsedTokens->push_back(token);
	}
	auto eof = new OperatorToken;
	eof->_data = "EOF";
	eof->_type = TT_OPERATOR;
	eof->_line = preparsedTokens->back()->_line; // TODO: fix crush when file is empty.
	eof->_operator = { OT_EOF, "EOF", NULL, NULL };
	preparsedTokens->push_back(eof);
	return *preparsedTokens;
}
