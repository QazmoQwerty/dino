
#include "Preprocessor.h"

vector<Token*>& Preprocessor::preprocess(vector<Token*> tokens)
{
	auto preparsedTokens = new vector<Token*>();

	//stack<OperatorType> brackets;

	for (Token * token : tokens)
	{
		if (token->_type == TT_MULTI_LINE_COMMENT || token->_type == TT_WHITESPACE)
		{
			delete token;
			continue;
		}
		if (token->_type == TT_SINGLE_LINE_COMMENT || token->_type == TT_NEWLINE)	// TODO - make it so you can have multiple-line sections of code.
		{
			bool b = false;
			if (preparsedTokens->size() != 0)
			{
				if (preparsedTokens->back()->_type == TT_IDENTIFIER || preparsedTokens->back()->_type == TT_LITERAL)
					b = true;
				if (preparsedTokens->back()->_type == TT_OPERATOR) {
					switch (((OperatorToken*)preparsedTokens->back())->_operator._type)
					{
						case(OT_RETURN):
						case(OT_INCREMENT):
						case(OT_DECREMENT):
						case(OT_CURLY_BRACES_CLOSE):
						case(OT_PARENTHESIS_CLOSE):
						case(OT_SQUARE_BRACKETS_CLOSE):
							b = true;
					}
				}
			}
			if (b)
			{
				token->_type = TT_LINE_BREAK;
				token->_data = "|";
			}
			else {
				delete token;
				continue;
			}
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
	/*Token* t = new Token();
	t->_data = "|";
	t->_line = preparsedTokens->back()->_line + 1;
	t->_type = TT_LINE_BREAK;
	preparsedTokens->push_back(t);*/
	auto eof = new OperatorToken;
	eof->_data = "EOF";
	eof->_type = TT_OPERATOR;
	eof->_line = preparsedTokens->back()->_line; // TODO: fix crush when file is empty.
	eof->_operator = { OT_EOF, "EOF", NULL, NULL };
	preparsedTokens->push_back(eof);
	return *preparsedTokens;
}
