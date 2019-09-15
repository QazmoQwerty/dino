#include "OperatorsMap.h"

unordered_map<string, Operator> OperatorsMap::_map;
unordered_map<string, Operator> OperatorsMap::_wordsMap;

const unordered_map<string, Operator>& OperatorsMap::getOperators() { return _map; }

const unordered_map<string, Operator>& OperatorsMap::getWordOperators() { return _wordsMap; }

bool OperatorsMap::isWord(OperatorType type)
{
	OperatorType wordTypes[] = {
		OT_WHILE,
		OT_FOR,
		OT_IF,
		OT_ELSE,
	};
	return std::find(std::begin(wordTypes), std::end(wordTypes), type) != std::end(wordTypes);
}

bool OperatorsMap::isUnary(OperatorType type)
{
	OperatorType unaryTypes[] = {
		OT_ADD,
		OT_SUBTRACT,
		OT_LOGICAL_NOT,
		OT_BITWISE_NOT,
		OT_INCREMENT,
		OT_DECREMENT,
		OT_PARENTHESIS_OPEN,
		OT_CURLY_BRACES_OPEN,
		OT_WHILE,
		OT_IF,
		OT_ELSE,
	};
	return std::find(std::begin(unaryTypes), std::end(unaryTypes), type) != std::end(unaryTypes);
}

bool OperatorsMap::isBinary(OperatorType type)
{
	OperatorType nonBinaryTypes[] = {
		OT_INCREMENT,
		OT_DECREMENT,
		OT_LOGICAL_NOT,
		OT_BITWISE_NOT,
	};
	return std::find(std::begin(nonBinaryTypes), std::end(nonBinaryTypes), type) == std::end(nonBinaryTypes);
}

bool OperatorsMap::isAssignment(OperatorType type)
{
	OperatorType assignmentTypes[] = {
		OT_ASSIGN_EQUAL,
		OT_ASSIGN_ADD,
		OT_ASSIGN_DIVIDE,
		OT_ASSIGN_MULTIPLY,
		OT_ASSIGN_SUBTRACT,
		OT_ASSIGN_MODULUS,
		OT_ASSIGN_BITWISE_AND,
		OT_ASSIGN_BITWISE_OR,
		OT_ASSIGN_BITWISE_XOR,
		OT_ASSIGN_SHIFT_LEFT,
		OT_ASSIGN_SHIFT_RIGHT,
	};
	return std::find(std::begin(assignmentTypes), std::end(assignmentTypes), type) != std::end(assignmentTypes);
}

/*
	Gets an OperatorType and searches _map for the corresponding operator string.
	NOTE: Unused function, might get deleted in the future.
*/
pair<const string, Operator> OperatorsMap::getOperatorByDefinition(OperatorType operatorType)
{
	for (auto t : OperatorsMap::getOperators())
		if (t.second._type == operatorType)
			return t;
	return pair<const string, Operator>("", { OT_UNKNOWN, "", NULL, NULL });
}

/*
	Sets up values in _map.
	IMPORTANT: Function MUST be called once before using any other functions of this class.
*/
void OperatorsMap::setup()
{
	/*
		TODO - operators to add: 
		  - different operators for pre/post-fix increment/decrement ("++"/"--"), since they have differenc precedence and associativity
		  - special comparison operators (like ".==.")
		  - do we need operators for backslash ("\") and colon (":")?
	*/

	_map = unordered_map<string, Operator>();

	//				  type						str		associativity	precedence
	_map["++"]	=	{ OT_INCREMENT,				"++",	LEFT_TO_RIGHT,		150 };	
	_map["--"]	=	{ OT_DECREMENT,				"--",	LEFT_TO_RIGHT,		150 };
	_map["("]	=	{ OT_PARENTHESIS_OPEN,		"(",	LEFT_TO_RIGHT,		150 };
	_map[")"]	=	{ OT_PARENTHESIS_CLOSE,		")",	LEFT_TO_RIGHT,		0 };
	_map["["]	=	{ OT_SQUARE_BRACKETS_OPEN,	"[",	LEFT_TO_RIGHT,		150 };
	_map["]"]	=	{ OT_SQUARE_BRACKETS_CLOSE, "]",	LEFT_TO_RIGHT,		0 };
	_map["{"]	=	{ OT_CURLY_BRACES_OPEN,		"{",	LEFT_TO_RIGHT,		150 };
	_map["}"]	=	{ OT_CURLY_BRACES_CLOSE,	"}",	LEFT_TO_RIGHT,		0 };
	_map["."]	=	{ OT_PERIOD,				".",	LEFT_TO_RIGHT,		150 };
	_map["^^"]	=	{ OT_EXPONENTIATION,		"^^",	RIGHT_TO_LEFT,		140 };
	_map["!"]	=	{ OT_LOGICAL_NOT,			"!",	RIGHT_TO_LEFT,		130 };
	_map["~"]	=	{ OT_BITWISE_NOT,			"~",	RIGHT_TO_LEFT,		130 };
	_map["*"]	=	{ OT_MULTIPLY,				"*",	LEFT_TO_RIGHT,		120 };
	_map["/"]	=	{ OT_DIVIDE,				"/",	LEFT_TO_RIGHT,		120 };
	_map["%"]	=	{ OT_MODULUS,				"%",	LEFT_TO_RIGHT,		120 };
	_map["+"]	=	{ OT_ADD,					"+",	LEFT_TO_RIGHT,		110 };
	_map["-"]	=	{ OT_SUBTRACT,				"-",	LEFT_TO_RIGHT,		110 };
	_map["<<"]	=	{ OT_BITWISE_SHIFT_LEFT,	"<<",	LEFT_TO_RIGHT,		100 };
	_map["<"]	=	{ OT_SMALLER,				"<",	LEFT_TO_RIGHT,		90 };
	_map["<="]	=	{ OT_SMALLER_EQUAL,			"<=",	LEFT_TO_RIGHT,		90 };
	_map[">"]	=	{ OT_GREATER,				">",	LEFT_TO_RIGHT,		90 };
	_map[">="]	=	{ OT_GREATER_EQUAL,			">=",	LEFT_TO_RIGHT,		90 };
	_map["=="]	=	{ OT_EQUAL,					"==",	LEFT_TO_RIGHT,		80 };
	_map["!="]	=	{ OT_NOT_EQUAL,				"!=",	LEFT_TO_RIGHT,		80 };
	_map["&"]	=	{ OT_BITWISE_AND,			"&",	LEFT_TO_RIGHT,		70 };
	_map["^"]	=	{ OT_BITWISE_XOR,			"^",	LEFT_TO_RIGHT,		60 };
	_map["|"]	=	{ OT_BITWISE_OR,			"|",	LEFT_TO_RIGHT,		50 };
	_map["&&"]	=	{ OT_LOGICAL_AND,			"&&",	LEFT_TO_RIGHT,		40 };
	_map["||"]	=	{ OT_LOGICAL_OR,			"||",	LEFT_TO_RIGHT,		30 };
	_map["="]	=	{ OT_ASSIGN_EQUAL,			"=",	RIGHT_TO_LEFT,		20 };
	_map["+="]	=	{ OT_ASSIGN_ADD,			"+=",	RIGHT_TO_LEFT,		20 };
	_map["-="]	=	{ OT_ASSIGN_SUBTRACT,		"-=",	RIGHT_TO_LEFT,		20 };
	_map["*="]	=	{ OT_ASSIGN_MULTIPLY,		"*=",	RIGHT_TO_LEFT,		20 };
	_map["/="]	=	{ OT_ASSIGN_DIVIDE,			"/=",	RIGHT_TO_LEFT,		20 };
	_map["%="]	=	{ OT_ASSIGN_MODULUS,		"%=",	RIGHT_TO_LEFT,		20 };
	_map["<<="]	=	{ OT_ASSIGN_SHIFT_LEFT,		"<<=",	RIGHT_TO_LEFT,		20 };
	_map[">>="]	=	{ OT_ASSIGN_SHIFT_RIGHT,	">>=",	RIGHT_TO_LEFT,		20 };
	_map["&="]	=	{ OT_ASSIGN_BITWISE_AND,	"&=",	RIGHT_TO_LEFT,		20 };
	_map["|="]	=	{ OT_ASSIGN_BITWISE_OR,		"|=",	RIGHT_TO_LEFT,		20 };
	_map["^="]	=	{ OT_ASSIGN_BITWISE_XOR,	"^=",	RIGHT_TO_LEFT,		20 };
	_map[","]	=	{ OT_COMMA,					",",	LEFT_TO_RIGHT,		10 };

	// Non-parser related operators:
	_map["//"]	=	{ OT_SINGLE_LINE_COMMENT,		"//",	NULL,		NULL };
	_map["/*"]	=	{ OT_MULTI_LINE_COMMENT_OPEN,	"/*",	NULL,		NULL };
	_map["*/"]	=	{ OT_MULTI_LINE_COMMENT_CLOSE,	"*/",	NULL,		NULL };
	_map["'"]	=	{ OT_SINGLE_QUOTE,				"'",	NULL,		NULL };
	_map["\""]	=	{ OT_DOUBLE_QUOTE,				"\"",	NULL,		NULL };
	_map["\\"]	=	{ OT_DOUBLE_QUOTE,				"\"",	NULL,		NULL };

	_wordsMap = unordered_map<string, Operator>();
	_wordsMap["while"] = { OT_WHILE, "while", NULL, NULL };
	_wordsMap["if"] = { OT_IF, "if", NULL, NULL };
	_wordsMap["else"] = { OT_ELSE, "else", NULL, NULL };
}
