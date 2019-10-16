#include "OperatorsMap.h"

unordered_map<string, Operator> OperatorsMap::_map;
unordered_map<string, Operator> OperatorsMap::_wordsMap;

const unordered_map<string, Operator>& OperatorsMap::getOperators() { return _map; }

const unordered_map<string, Operator>& OperatorsMap::getWordOperators() { return _wordsMap; }

bool OperatorsMap::isWord(OperatorType type)
{
	OperatorType wordTypes[] = {
		OT_WHILE,
		OT_DO,
		OT_FOR,
		OT_IF,
		OT_ELSE, 
		OT_LOGICAL_AND,
		OT_LOGICAL_OR,
		OT_LOGICAL_NOT,
		OT_TYPE,
		OT_INTERFACE,
		OT_NAMESPACE,
		OT_NEW,
		OT_DELETE,
		OT_GET,
		OT_SET,
		OT_IMPLEMENTS,
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
		OT_DO,
		OT_IF,
		OT_ELSE,
		OT_UNLESS,
		OT_IS,
		OT_FOR,
		OT_RETURN,
		OT_DELETE,
		OT_TYPE,
		OT_INTERFACE,
		OT_NAMESPACE,
		OT_NEW,
		OT_AT,
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

bool OperatorsMap::isKeyword(Operator op)
{
	return op._binaryPrecedence == NONE && op._prefixPrecedence == NONE && op._postfixPrecedence == NONE;

	/*OperatorType keywordTypes[] = {
		OT_WHILE,
		OT_FOR,
		OT_DO,
		OT_IF,
		OT_ELSE,
		OT_UNLESS,
		OT_RETURN,
		OT_TYPE,
		OT_INTERFACE,
		OT_NAMESPACE,
		OT_DELETE,
		OT_GET,
		OT_SET,
	};
	return std::find(std::begin(keywordTypes), std::end(keywordTypes), type) != std::end(keywordTypes);*/
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
	for (auto t : OperatorsMap::getWordOperators())
		if (t.second._type == operatorType)
			return t;
	return pair<const string, Operator>("", { OT_UNKNOWN, "", NULL, NULL });
}

//#define SPECIAL(a) a, a, a

#define KEYWORD NULL, NONE, NONE, NONE
#define NON_PARSER NULL, NONE, NONE, NONE

/*
	Sets up values in _map.
	IMPORTANT: Function MUST be called once before using any other functions of this class.
*/
void OperatorsMap::setup()
{
	_map = unordered_map<string, Operator>();
	_wordsMap = unordered_map<string, Operator>();

	//				  type						str		associativity		binary	prefix	postfix
	_map["++"]	= { OT_INCREMENT,				"++",	LEFT_TO_RIGHT,		NONE,	140,	150	 };
	_map["--"]	= { OT_DECREMENT,				"--",	LEFT_TO_RIGHT,		NONE,	140,	150  };
	_map["."]	= { OT_PERIOD,					".",	LEFT_TO_RIGHT,		150,	NONE,	NONE };
	_map["@"]	= { OT_AT,						"@",	RIGHT_TO_LEFT,		NONE,	140,	140  };
	_map["~"]	= { OT_BITWISE_NOT,				"~",	RIGHT_TO_LEFT,		NONE,	130,	NONE };
	_wordsMap["not"] = { OT_LOGICAL_NOT,		"not",	RIGHT_TO_LEFT,		NONE,	130,	NONE };
	_wordsMap["new"] = { OT_NEW,				"new",	RIGHT_TO_LEFT,		NONE,	130,	NONE };
	_map["*"]	= { OT_MULTIPLY,				"*",	LEFT_TO_RIGHT,		120,	NONE,	NONE };
	_map["/"]	= { OT_DIVIDE,					"/",	LEFT_TO_RIGHT,		120,	NONE,	NONE };
	_map["%"]	= { OT_MODULUS,					"%",	LEFT_TO_RIGHT,		120,	NONE,	NONE };
	_map["+"]	= { OT_ADD,						"+",	LEFT_TO_RIGHT,		110,	140,	NONE };
	_map["-"]	= { OT_SUBTRACT,				"-",	LEFT_TO_RIGHT,		110,	140,	NONE };
	_map["<<"]	= { OT_BITWISE_SHIFT_LEFT,		"<<",	LEFT_TO_RIGHT,		100,	100,	NONE };
	_map[">>"]	= { OT_BITWISE_SHIFT_RIGHT,		"<<",	LEFT_TO_RIGHT,		100,	100,	NONE };
	_wordsMap["is"] = { OT_IS,					"is",	LEFT_TO_RIGHT,		90,		NONE,	NONE };
	_map["<"]	= { OT_SMALLER,					"<",	LEFT_TO_RIGHT,		90,		NONE,	NONE };
	_map["<="]	= { OT_SMALLER_EQUAL,			"<=",	LEFT_TO_RIGHT,		90,		NONE,	NONE };
	_map[">"]	= { OT_GREATER,					">",	LEFT_TO_RIGHT,		90,		NONE,	NONE };
	_map[">="]	= { OT_GREATER_EQUAL,			">=",	LEFT_TO_RIGHT,		90,		NONE,	NONE };
	_map["=="]	= { OT_EQUAL,					"==",	LEFT_TO_RIGHT,		80,		NONE,	NONE };
	_map["!="]	= { OT_NOT_EQUAL,				"!=",	LEFT_TO_RIGHT,		80,		NONE,	NONE };
	_map["&"]	= { OT_BITWISE_AND,				"&",	LEFT_TO_RIGHT,		70,		140,	NONE };
	_map["^"]	= { OT_BITWISE_XOR,				"^",	LEFT_TO_RIGHT,		60,		NONE,	NONE };
	_map["?"]	= { OT_BITWISE_OR,				"?",	LEFT_TO_RIGHT,		50,		NONE,	NONE };
	_wordsMap["and"] = { OT_LOGICAL_AND,		"and",	LEFT_TO_RIGHT,		40,		NONE,	NONE };
	_wordsMap["or"] = { OT_LOGICAL_OR,			"or",	LEFT_TO_RIGHT,		30,		NONE,	NONE };
	_map[","]	= { OT_COMMA,					",",	LEFT_TO_RIGHT,		20,		NONE,	NONE };
	_map["="]	= { OT_ASSIGN_EQUAL,			"=",	RIGHT_TO_LEFT,		10,		NONE,	NONE };
	_map["+="]	= { OT_ASSIGN_ADD,				"+=",	RIGHT_TO_LEFT,		10,		NONE,	NONE };
	_map["-="]	= { OT_ASSIGN_SUBTRACT,			"-=",	RIGHT_TO_LEFT,		10,		NONE,	NONE };
	_map["*="]	= { OT_ASSIGN_MULTIPLY,			"*=",	RIGHT_TO_LEFT,		10,		NONE,	NONE };
	_map["/="]	= { OT_ASSIGN_DIVIDE,			"/=",	RIGHT_TO_LEFT,		10,		NONE,	NONE };
	_map["%="]	= { OT_ASSIGN_MODULUS,			"%=",	RIGHT_TO_LEFT,		10,		NONE,	NONE };
	_map["<<="]	= { OT_ASSIGN_SHIFT_LEFT,		"<<=",	RIGHT_TO_LEFT,		10,		NONE,	NONE };
	_map[">>="]	= { OT_ASSIGN_SHIFT_RIGHT,		">>=",	RIGHT_TO_LEFT,		10,		NONE,	NONE };
	_map["&="]	= { OT_ASSIGN_BITWISE_AND,		"&=",	RIGHT_TO_LEFT,		10,		NONE,	NONE };
	_map["?="]	= { OT_ASSIGN_BITWISE_OR,		"?=",	RIGHT_TO_LEFT,		10,		NONE,	NONE };
	_map["^="]	= { OT_ASSIGN_BITWISE_XOR,		"^=",	RIGHT_TO_LEFT,		10,		NONE,	NONE };
	_map[":"]	= { OT_COLON,					":",	LEFT_TO_RIGHT,		NONE,	NONE,	NONE };
	_map["("]	= { OT_PARENTHESIS_OPEN,		"(",	LEFT_TO_RIGHT,		150,	150,	150  };
	_map[")"]	= { OT_PARENTHESIS_CLOSE,		")",	LEFT_TO_RIGHT,		0,		0,		0	 };
	_map["["]	= { OT_SQUARE_BRACKETS_OPEN,	"[",	LEFT_TO_RIGHT,		150,	150,	150  };
	_map["]"]	= { OT_SQUARE_BRACKETS_CLOSE,	"]",	LEFT_TO_RIGHT,		0,		0,		0	 };
	_map["{"]	= { OT_CURLY_BRACES_OPEN,		"{",	LEFT_TO_RIGHT,		150,	150,	150  };
	_map["}"]	= { OT_CURLY_BRACES_CLOSE,		"}",	LEFT_TO_RIGHT,		0,		0,		0	 };

	// Non-parser related operators:
	_map["//"] = { OT_SINGLE_LINE_COMMENT,		"//",	NON_PARSER };
	_map["/*"] = { OT_MULTI_LINE_COMMENT_OPEN,	"/*",	NON_PARSER };
	_map["*/"] = { OT_MULTI_LINE_COMMENT_CLOSE,	"*/",	NON_PARSER };
	_map["\""] = { OT_DOUBLE_QUOTE,				"\"",	NON_PARSER };
	_map["\\"] = { OT_DOUBLE_QUOTE,				"\"",	NON_PARSER };
	_map["'"]  = { OT_SINGLE_QUOTE,				"'",	NON_PARSER };

	
	_wordsMap["while"]	= { OT_WHILE,	"while",	KEYWORD };
	_wordsMap["for"]	= { OT_FOR,		"for",		KEYWORD };
	_wordsMap["do"]		= { OT_DO,		"do",		KEYWORD };
	_wordsMap["if"]		= { OT_IF,		"if",		KEYWORD };
	_wordsMap["else"]	= { OT_ELSE,	"else",		KEYWORD };
	_wordsMap["unless"] = { OT_UNLESS,	"unless",	KEYWORD };
	_wordsMap["return"] = { OT_RETURN,	"return",	KEYWORD };
	_wordsMap["get"]	= { OT_GET,		"get",		KEYWORD };
	_wordsMap["set"]	= { OT_SET,		"set",		KEYWORD };
	_wordsMap["type"]	= { OT_TYPE,	"type",		KEYWORD };
	_wordsMap["delete"] = { OT_DELETE,	"delete",	KEYWORD };
	_wordsMap["interface"]	= { OT_INTERFACE,	"interface",	KEYWORD };
	_wordsMap["namespace"]	= { OT_NAMESPACE,	"namespace",	KEYWORD };

	//_wordsMap["implements"] = { OT_IMPLEMENTS,	"implements",	KEYWORD };

	//_map = unordered_map<string, Operator>();

	////				  type						str		associativity	precedence
	//_map["++"]	=	{ OT_INCREMENT,				"++",	LEFT_TO_RIGHT,		150 };	
	//_map["--"]	=	{ OT_DECREMENT,				"--",	LEFT_TO_RIGHT,		150 };
	//_map["("]	=	{ OT_PARENTHESIS_OPEN,		"(",	LEFT_TO_RIGHT,		150 };
	//_map[")"]	=	{ OT_PARENTHESIS_CLOSE,		")",	LEFT_TO_RIGHT,		0 };
	//_map["["]	=	{ OT_SQUARE_BRACKETS_OPEN,	"[",	LEFT_TO_RIGHT,		150 };
	//_map["]"]	=	{ OT_SQUARE_BRACKETS_CLOSE, "]",	LEFT_TO_RIGHT,		0 };
	//_map["{"]	=	{ OT_CURLY_BRACES_OPEN,		"{",	LEFT_TO_RIGHT,		150 };
	//_map["}"]	=	{ OT_CURLY_BRACES_CLOSE,	"}",	LEFT_TO_RIGHT,		0 };
	//_map["."]	=	{ OT_PERIOD,				".",	LEFT_TO_RIGHT,		150 };
	//_map["^^"]	=	{ OT_EXPONENTIATION,		"^^",	RIGHT_TO_LEFT,		140 };
	////_map["!"]	=	{ OT_LOGICAL_NOT,			"!",	RIGHT_TO_LEFT,		130 };
	//_map["~"]	=	{ OT_BITWISE_NOT,			"~",	RIGHT_TO_LEFT,		130 };
	//_map["*"]	=	{ OT_MULTIPLY,				"*",	LEFT_TO_RIGHT,		120 };
	//_map["/"]	=	{ OT_DIVIDE,				"/",	LEFT_TO_RIGHT,		120 };
	//_map["%"]	=	{ OT_MODULUS,				"%",	LEFT_TO_RIGHT,		120 };
	//_map["+"]	=	{ OT_ADD,					"+",	LEFT_TO_RIGHT,		110 };
	//_map["-"]	=	{ OT_SUBTRACT,				"-",	LEFT_TO_RIGHT,		110 };
	//_map["<<"]	=	{ OT_BITWISE_SHIFT_LEFT,	"<<",	LEFT_TO_RIGHT,		100 };
	//_map["<"]	=	{ OT_SMALLER,				"<",	LEFT_TO_RIGHT,		90 };
	//_map["<="]	=	{ OT_SMALLER_EQUAL,			"<=",	LEFT_TO_RIGHT,		90 };
	//_map[">"]	=	{ OT_GREATER,				">",	LEFT_TO_RIGHT,		90 };
	//_map[">="]	=	{ OT_GREATER_EQUAL,			">=",	LEFT_TO_RIGHT,		90 };
	//_map["=="]	=	{ OT_EQUAL,					"==",	LEFT_TO_RIGHT,		80 };
	//_map["!="]	=	{ OT_NOT_EQUAL,				"!=",	LEFT_TO_RIGHT,		80 };
	//_map["&"]	=	{ OT_BITWISE_AND,			"&",	LEFT_TO_RIGHT,		70 };
	//_map["^"]	=	{ OT_BITWISE_XOR,			"^",	LEFT_TO_RIGHT,		60 };
	//_map["?"]	=	{ OT_BITWISE_OR,			"?",	LEFT_TO_RIGHT,		50 };
	////_map["&&"]	=	{ OT_LOGICAL_AND,			"&&",	LEFT_TO_RIGHT,		40 };
	////_map["??"]	=	{ OT_LOGICAL_OR,			"??",	LEFT_TO_RIGHT,		30 };
	//_map["="]	=	{ OT_ASSIGN_EQUAL,			"=",	RIGHT_TO_LEFT,		20 };
	//_map["+="]	=	{ OT_ASSIGN_ADD,			"+=",	RIGHT_TO_LEFT,		20 };
	//_map["-="]	=	{ OT_ASSIGN_SUBTRACT,		"-=",	RIGHT_TO_LEFT,		20 };
	//_map["*="]	=	{ OT_ASSIGN_MULTIPLY,		"*=",	RIGHT_TO_LEFT,		20 };
	//_map["/="]	=	{ OT_ASSIGN_DIVIDE,			"/=",	RIGHT_TO_LEFT,		20 };
	//_map["%="]	=	{ OT_ASSIGN_MODULUS,		"%=",	RIGHT_TO_LEFT,		20 };
	//_map["<<="]	=	{ OT_ASSIGN_SHIFT_LEFT,		"<<=",	RIGHT_TO_LEFT,		20 };
	//_map[">>="]	=	{ OT_ASSIGN_SHIFT_RIGHT,	">>=",	RIGHT_TO_LEFT,		20 };
	//_map["&="]	=	{ OT_ASSIGN_BITWISE_AND,	"&=",	RIGHT_TO_LEFT,		20 };
	//_map["?="]	=	{ OT_ASSIGN_BITWISE_OR,		"?=",	RIGHT_TO_LEFT,		20 };
	//_map["^="]	=	{ OT_ASSIGN_BITWISE_XOR,	"^=",	RIGHT_TO_LEFT,		20 };
	//_map[","]	=	{ OT_COMMA,					",",	LEFT_TO_RIGHT,		10 };
	//_map[":"]	=	{ OT_COLON,					":",	LEFT_TO_RIGHT,		NULL };

	//// Non-parser related operators:
	//_map["//"]	=	{ OT_SINGLE_LINE_COMMENT,		"//",	NULL,		NULL };
	//_map["/*"]	=	{ OT_MULTI_LINE_COMMENT_OPEN,	"/*",	NULL,		NULL };
	//_map["*/"]	=	{ OT_MULTI_LINE_COMMENT_CLOSE,	"*/",	NULL,		NULL };
	//_map["'"]	=	{ OT_SINGLE_QUOTE,				"'",	NULL,		NULL };
	//_map["\""]	=	{ OT_DOUBLE_QUOTE,				"\"",	NULL,		NULL };
	//_map["\\"]	=	{ OT_DOUBLE_QUOTE,				"\"",	NULL,		NULL };

	//_wordsMap = unordered_map<string, Operator>();
	//_wordsMap["while"] = { OT_WHILE, "while", NULL, NULL };
	//_wordsMap["for"] = { OT_FOR, "for", NULL, NULL };
	//_wordsMap["do"] = { OT_DO, "do", NULL, NULL };
	//_wordsMap["if"] = { OT_IF, "if", NULL, NULL };
	//_wordsMap["else"] = { OT_ELSE, "else", NULL, NULL };
	//_wordsMap["unless"] = { OT_UNLESS, "unless", NULL, NULL };
	//_wordsMap["return"] = { OT_RETURN, "return", NULL, NULL };
	//_wordsMap["delete"] = { OT_DELETE, "delete", NULL, NULL };

	//_wordsMap["type"] = { OT_TYPE, "type", NULL, NULL };
	//_wordsMap["implements"] = { OT_IMPLEMENTS, "implements", NULL, NULL };
	//_wordsMap["interface"] = { OT_INTERFACE, "interface", NULL, NULL };
	//_wordsMap["namespace"] = { OT_NAMESPACE, "namespace", NULL, NULL };
	//_wordsMap["get"] = { OT_GET, "get", NULL, NULL };
	//_wordsMap["set"] = { OT_SET, "set", NULL, NULL };

	//_wordsMap["not"]	=	{ OT_LOGICAL_NOT,			"not",	RIGHT_TO_LEFT,		130 };
	//_wordsMap["and"]	=	{ OT_LOGICAL_AND,			"and",	LEFT_TO_RIGHT,		40 };
	//_wordsMap["or"]		=	{ OT_LOGICAL_OR,			"or",	LEFT_TO_RIGHT,		30 };
	//_wordsMap["new"]	=	{ OT_NEW,					"new",	RIGHT_TO_LEFT,		130 };
	//_wordsMap["is"]		=	{ OT_IS, "is", NULL, NULL };
}
