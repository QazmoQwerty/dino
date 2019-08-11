
#include "Preparser.h"

vector<vector<Token*>*>& Preparser::Preparse(vector<Token*> tokens)
{
	auto preparsedTokens = new vector<vector<Token*>*>();
	auto currentLine = new vector<Token*>();

	for (unsigned int i = 0; i < tokens.size(); i++)
	{
		if (tokens[i]->_type == TT_MULTI_LINE_COMMENT || tokens[i]->_type == TT_WHITESPACE)
		{
			delete tokens[i];
			continue;
		}
		if (tokens[i]->_type == TT_SINGLE_LINE_COMMENT || tokens[i]->_type == TT_LINE_BREAK || tokens[i]->_type == TT_NEWLINE)	// TODO - make it so you can have multiple-line sections of code.
		{
			delete tokens[i];
			if (currentLine->size() > 0)
			{
				preparsedTokens->push_back(currentLine);
				currentLine = new vector<Token*>();
			}
			continue;
		}
		currentLine->push_back(tokens[i]);
	}

	return *preparsedTokens;
}
