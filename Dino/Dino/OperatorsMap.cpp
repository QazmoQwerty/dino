#include "OperatorsMap.h"

unordered_map<unicode_string, Operator, UnicodeHasherFunction> OperatorsMap::_map;
unordered_map<unicode_string, Operator, UnicodeHasherFunction> OperatorsMap::_wordsMap;

const unordered_map<unicode_string, Operator, UnicodeHasherFunction>& OperatorsMap::getOperators() { return _map; }

const unordered_map<unicode_string, Operator, UnicodeHasherFunction>& OperatorsMap::getWordOperators() { return _wordsMap; }

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
	//return op._binaryPrecedence == NONE && op._prefixPrecedence == NONE && op._postfixPrecedence == NONE;
	OperatorType keywordTypes[] = {
		OT_SWITCH,
		OT_CASE,
		OT_DEFAULT,
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
	return std::find(std::begin(keywordTypes), std::end(keywordTypes), op._type) != std::end(keywordTypes);
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

ReturnType OperatorsMap::getReturnType(OperatorType type)
{
	OperatorType leftTypes[] = {
		OT_ADD,							//	+
		OT_SUBTRACT,					//	-
		OT_DIVIDE,						//	/
		OT_MULTIPLY,					//	*
		OT_MODULUS,						//	%
		OT_INCREMENT,					//	++
		OT_DECREMENT,					//	--
		OT_BITWISE_AND,					//	&
		OT_BITWISE_OR,					//	?
		OT_BITWISE_XOR,					//	^
		OT_BITWISE_NOT,					//	~
		OT_BITWISE_SHIFT_LEFT,			//	<<
		OT_BITWISE_SHIFT_RIGHT,			//	>>
		OT_ASSIGN_EQUAL,				//	:=
		OT_ASSIGN_ADD,					//	+=
		OT_ASSIGN_SUBTRACT,				//	-=
		OT_ASSIGN_MULTIPLY,				//	*=
		OT_ASSIGN_DIVIDE,				//	/=
		OT_ASSIGN_MODULUS,				//	%=
		OT_ASSIGN_SHIFT_LEFT,			//	<<=
		OT_ASSIGN_SHIFT_RIGHT,			//	>>=
		OT_ASSIGN_BITWISE_AND,			//	&=
		OT_ASSIGN_BITWISE_OR,			//	?=
		OT_ASSIGN_BITWISE_XOR,			//	^=
	};
	if (std::find(std::begin(leftTypes), std::end(leftTypes), type) != std::end(leftTypes))
		return RT_LEFT;

	OperatorType logicalTypes[] = {
		OT_EQUAL,						//	=
		OT_NOT_EQUAL,					//	!=
		OT_GREATER,						//	>
		OT_SMALLER,						//	<
		OT_GREATER_EQUAL,				//	>=
		OT_SMALLER_EQUAL,				//	<=
		OT_IS,							//	is
		OT_LOGICAL_AND,					//	and
		OT_LOGICAL_OR,					//	or
		OT_LOGICAL_NOT,					//	not
	};
	if (std::find(std::begin(logicalTypes), std::end(logicalTypes), type) != std::end(logicalTypes))
		return RT_BOOLEAN;
	return RT_VOID;
}

/*
	Gets an OperatorType and searches _map for the corresponding operator string.
	NOTE: Unused function, might get deleted in the future.
*/
pair<const unicode_string, Operator> OperatorsMap::getOperatorByDefinition(OperatorType operatorType)
{
	for (auto t : OperatorsMap::getOperators())
		if (t.second._type == operatorType)
			return t;
	for (auto t : OperatorsMap::getWordOperators())
		if (t.second._type == operatorType)
			return t;
	return pair<const unicode_string, Operator>(unicode_string(""), { OT_UNKNOWN, unicode_string(""), NULL, NULL });
}

//#define SPECIAL(a) a, a, a

#define KEYWORD NULL, NONE, NONE, NONE
#define NON_PARSER NULL, NONE, NONE, NONE

#define UTF8(a) unicode_string(a)

/*
	Sets up values in _map.
	IMPORTANT: Function MUST be called once before using any other functions of this class.
*/
void OperatorsMap::setup()
{
	_map = unordered_map<unicode_string, Operator, UnicodeHasherFunction>();
	_wordsMap = unordered_map<unicode_string, Operator, UnicodeHasherFunction>();

	//																							/-----precedences-----\
	//							type						str				associativity		binary	prefix	postfix
	_map[UTF8("++")]		= { OT_INCREMENT,				UTF8("++"),		LEFT_TO_RIGHT,		NONE,	140,	150	 };
	_map[UTF8("--")]		= { OT_DECREMENT,				UTF8("--"),		LEFT_TO_RIGHT,		NONE,	140,	150  };
	_map[UTF8(".")]			= { OT_PERIOD,					UTF8("."),		LEFT_TO_RIGHT,		150,	NONE,	NONE };
	_map[UTF8("@")]			= { OT_AT,						UTF8("@"),		RIGHT_TO_LEFT,		NONE,	140,	140  };
	_map[UTF8("~")]			= { OT_BITWISE_NOT,				UTF8("~"),		RIGHT_TO_LEFT,		NONE,	130,	NONE };
	_wordsMap[UTF8("not")]	= { OT_LOGICAL_NOT,				UTF8("not"),	RIGHT_TO_LEFT,		NONE,	130,	NONE };
	_map[UTF8(u8"¬")]		= { OT_LOGICAL_NOT,				UTF8(u8"¬"),	RIGHT_TO_LEFT,		NONE,	130,	NONE };
	_wordsMap[UTF8("new")]	= { OT_NEW,						UTF8("new"),	RIGHT_TO_LEFT,		NONE,	130,	NONE };
	_map[UTF8("*")]			= { OT_MULTIPLY,				UTF8("*"),		LEFT_TO_RIGHT,		120,	NONE,	NONE };
	_map[UTF8(u8"×")]		= { OT_MULTIPLY,				UTF8(u8"×"),	LEFT_TO_RIGHT,		120,	NONE,	NONE };
	_map[UTF8("/")]			= { OT_DIVIDE,					UTF8("/"),		LEFT_TO_RIGHT,		120,	NONE,	NONE };
	_map[UTF8(u8"÷")]		= { OT_DIVIDE,					UTF8(u8"÷"),	LEFT_TO_RIGHT,		120,	NONE,	NONE };
	_map[UTF8("%")]			= { OT_MODULUS,					UTF8("%"),		LEFT_TO_RIGHT,		120,	NONE,	NONE };
	_map[UTF8("+")]			= { OT_ADD,						UTF8("+"),		LEFT_TO_RIGHT,		110,	140,	NONE };
	_map[UTF8("-")]			= { OT_SUBTRACT,				UTF8("-"),		LEFT_TO_RIGHT,		110,	140,	NONE };
	_map[UTF8("<<")]		= { OT_BITWISE_SHIFT_LEFT,		UTF8("<<"),		LEFT_TO_RIGHT,		100,	100,	NONE };
	_map[UTF8(">>")]		= { OT_BITWISE_SHIFT_RIGHT,		UTF8(">>"),		LEFT_TO_RIGHT,		100,	100,	NONE };
	_wordsMap[UTF8("is")]	= { OT_IS,						UTF8("is"),		LEFT_TO_RIGHT,		90,		NONE,	NONE };
	_map[UTF8("<")]			= { OT_SMALLER,					UTF8("<"),		LEFT_TO_RIGHT,		90,		NONE,	NONE };
	_map[UTF8("<=")]		= { OT_SMALLER_EQUAL,			UTF8("<="),		LEFT_TO_RIGHT,		90,		NONE,	NONE };
	_map[UTF8(u8"≤")]		= { OT_SMALLER_EQUAL,			UTF8(u8"≤"),	LEFT_TO_RIGHT,		90,		NONE,	NONE };
	_map[UTF8(">")]			= { OT_GREATER,					UTF8(">"),		LEFT_TO_RIGHT,		90,		NONE,	NONE };
	_map[UTF8(">=")]		= { OT_GREATER_EQUAL,			UTF8(">="),		LEFT_TO_RIGHT,		90,		NONE,	NONE };
	_map[UTF8(u8"≥")]		= { OT_GREATER_EQUAL,			UTF8(u8"≥"),	LEFT_TO_RIGHT,		90,		NONE,	NONE };
	_map[UTF8("=")]			= { OT_EQUAL,					UTF8("="),		LEFT_TO_RIGHT,		80,		NONE,	NONE };
	_map[UTF8("!=")]		= { OT_NOT_EQUAL,				UTF8("!="),		LEFT_TO_RIGHT,		80,		NONE,	NONE };
	_map[UTF8(u8"≠")]		= { OT_NOT_EQUAL,				UTF8(u8"≠"),	LEFT_TO_RIGHT,		80,		NONE,	NONE };
	_map[UTF8("&")]			= { OT_BITWISE_AND,				UTF8("&"),		LEFT_TO_RIGHT,		70,		140,	NONE };
	_map[UTF8("^")]			= { OT_BITWISE_XOR,				UTF8("^"),		LEFT_TO_RIGHT,		60,		NONE,	NONE };
	_map[UTF8("?")]			= { OT_BITWISE_OR,				UTF8("?"),		LEFT_TO_RIGHT,		50,		NONE,	NONE };
	_wordsMap[UTF8("and")]	= { OT_LOGICAL_AND,				UTF8(u8"∧"),	LEFT_TO_RIGHT,		40,		NONE,	NONE };
	_map[UTF8(u8"∧")]		= { OT_LOGICAL_AND,				UTF8("and"),	LEFT_TO_RIGHT,		40,		NONE,	NONE };
	_wordsMap[UTF8("or")]	= { OT_LOGICAL_OR,				UTF8("or"),		LEFT_TO_RIGHT,		30,		NONE,	NONE };
	_map[UTF8(u8"∨")]		= { OT_LOGICAL_OR,				UTF8(u8"∨"),	LEFT_TO_RIGHT,		30,		NONE,	NONE };
	_map[UTF8(",")]			= { OT_COMMA,					UTF8(","),		LEFT_TO_RIGHT,		20,		NONE,	NONE };
	_wordsMap[UTF8("if")]	= { OT_IF,						UTF8("if"),		LEFT_TO_RIGHT,		15,		NONE,	NONE };
	_map[UTF8(":=")]		= { OT_ASSIGN_EQUAL,			UTF8(":="),		RIGHT_TO_LEFT,		10,		NONE,	NONE };
	_map[UTF8(u8"≡")]		= { OT_ASSIGN_EQUAL,			UTF8(u8"≡"),	RIGHT_TO_LEFT,		10,		NONE,	NONE };
	_map[UTF8("+=")]		= { OT_ASSIGN_ADD,				UTF8("+="),		RIGHT_TO_LEFT,		10,		NONE,	NONE };
	_map[UTF8("-=")]		= { OT_ASSIGN_SUBTRACT,			UTF8("-="),		RIGHT_TO_LEFT,		10,		NONE,	NONE };
	_map[UTF8("*=")]		= { OT_ASSIGN_MULTIPLY,			UTF8("*="),		RIGHT_TO_LEFT,		10,		NONE,	NONE };
	_map[UTF8(u8"×=")]		= { OT_ASSIGN_MULTIPLY,			UTF8(u8"×="),	RIGHT_TO_LEFT,		10,		NONE,	NONE };
	_map[UTF8("/=")]		= { OT_ASSIGN_DIVIDE,			UTF8("/="),		RIGHT_TO_LEFT,		10,		NONE,	NONE };
	_map[UTF8(u8"÷=")]		= { OT_ASSIGN_DIVIDE,			UTF8(u8"÷="),	RIGHT_TO_LEFT,		10,		NONE,	NONE };
	_map[UTF8("%=")]		= { OT_ASSIGN_MODULUS,			UTF8("%="),		RIGHT_TO_LEFT,		10,		NONE,	NONE };
	_map[UTF8("<<=")]		= { OT_ASSIGN_SHIFT_LEFT,		UTF8("<<="),	RIGHT_TO_LEFT,		10,		NONE,	NONE };
	_map[UTF8(">>=")]		= { OT_ASSIGN_SHIFT_RIGHT,		UTF8(">>="),	RIGHT_TO_LEFT,		10,		NONE,	NONE };
	_map[UTF8("&=")]		= { OT_ASSIGN_BITWISE_AND,		UTF8("&="),		RIGHT_TO_LEFT,		10,		NONE,	NONE };
	_map[UTF8("?=")]		= { OT_ASSIGN_BITWISE_OR,		UTF8("?="),		RIGHT_TO_LEFT,		10,		NONE,	NONE };
	_map[UTF8("^=")]		= { OT_ASSIGN_BITWISE_XOR,		UTF8("^="),		RIGHT_TO_LEFT,		10,		NONE,	NONE };
	_map[UTF8(":")]			= { OT_COLON,					UTF8(":"),		LEFT_TO_RIGHT,		NONE,	NONE,	NONE };
	_map[UTF8("(")]			= { OT_PARENTHESIS_OPEN,		UTF8("("),		LEFT_TO_RIGHT,		150,	150,	150  };
	_map[UTF8(")")]			= { OT_PARENTHESIS_CLOSE,		UTF8(")"),		LEFT_TO_RIGHT,		0,		0,		0	 };
	_map[UTF8("[")]			= { OT_SQUARE_BRACKETS_OPEN,	UTF8("["),		LEFT_TO_RIGHT,		150,	150,	150  };
	_map[UTF8("]")]			= { OT_SQUARE_BRACKETS_CLOSE,	UTF8("]"),		LEFT_TO_RIGHT,		0,		0,		0	 };
	_map[UTF8("{")]			= { OT_CURLY_BRACES_OPEN,		UTF8("{"),		LEFT_TO_RIGHT,		150,	150,	150  };
	_map[UTF8("}")]			= { OT_CURLY_BRACES_CLOSE,		UTF8("}"),		LEFT_TO_RIGHT,		0,		0,		0	 };

	// Non-parser related operators:
	_map[UTF8("//")]		= { OT_SINGLE_LINE_COMMENT,			UTF8("//"),	NON_PARSER };
	_map[UTF8("/*")]		= { OT_MULTI_LINE_COMMENT_OPEN,		UTF8("/*"),	NON_PARSER };
	_map[UTF8("*/")]		= { OT_MULTI_LINE_COMMENT_CLOSE,	UTF8("*/"),	NON_PARSER };
	_map[UTF8("\"")]		= { OT_DOUBLE_QUOTE,				UTF8("\""),	NON_PARSER };
	_map[UTF8("\\")]		= { OT_DOUBLE_QUOTE,				UTF8("\""),	NON_PARSER };
	_map[UTF8("'")]			= { OT_SINGLE_QUOTE,				UTF8("'"),	NON_PARSER };

	// Word operators:
	_wordsMap[UTF8("while")]		= { OT_WHILE,		UTF8("while"),		KEYWORD };
	_wordsMap[UTF8("for")]			= { OT_FOR,			UTF8("for"),		KEYWORD };
	_wordsMap[UTF8("do")]			= { OT_DO,			UTF8("do"),			KEYWORD };
	_wordsMap[UTF8("switch")]		= { OT_SWITCH,		UTF8("switch"),		KEYWORD };
	_wordsMap[UTF8("case")]			= { OT_CASE,		UTF8("case"),		KEYWORD };
	_wordsMap[UTF8("default")]		= { OT_DEFAULT,		UTF8("default"),	KEYWORD };
	_wordsMap[UTF8("else")]			= { OT_ELSE,		UTF8("else"),		KEYWORD };
	_wordsMap[UTF8("unless")]		= { OT_UNLESS,		UTF8("unless"),		KEYWORD };
	_wordsMap[UTF8("return")]		= { OT_RETURN,		UTF8("return"),		KEYWORD };
	_wordsMap[UTF8("get")]			= { OT_GET,			UTF8("get"),		KEYWORD };
	_wordsMap[UTF8("set")]			= { OT_SET,			UTF8("set"),		KEYWORD };
	_wordsMap[UTF8("type")]			= { OT_TYPE,		UTF8("type"),		KEYWORD };
	_wordsMap[UTF8("delete")]		= { OT_DELETE,		UTF8("delete"),		KEYWORD };
	_wordsMap[UTF8("interface")]	= { OT_INTERFACE,	UTF8("interface"),	KEYWORD };
	_wordsMap[UTF8("namespace")]	= { OT_NAMESPACE,	UTF8("namespace"),	KEYWORD };
}
