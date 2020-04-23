#include "OperatorsMap.h"

unordered_map<unicode_string, Operator, UnicodeHasherFunction> OperatorsMap::_map;
unordered_map<unicode_string, Operator, UnicodeHasherFunction> OperatorsMap::_wordsMap;

const unordered_map<unicode_string, Operator, UnicodeHasherFunction>& OperatorsMap::getOperators() { return _map; }

const unordered_map<unicode_string, Operator, UnicodeHasherFunction>& OperatorsMap::getWordOperators() { return _wordsMap; }

bool OperatorsMap::isWord(OperatorType type)
{
	switch(type) {
		case OT_WHILE: case OT_DO: case OT_FOR: case OT_IF: case OT_ELSE: case OT_LOGICAL_AND: case OT_LOGICAL_OR: case OT_LOGICAL_NOT: case OT_CONST: case OT_TRY: case OT_CATCH:
		case OT_TYPE: case OT_INTERFACE: case OT_NAMESPACE: case OT_NEW: case OT_DELETE: case OT_BREAK: case OT_CONTINUE: case OT_GET: case OT_SET: case OT_IMPLEMENTS: case OT_INCLUDE: case OT_IMPORT:
			return true;
		default: 
			return false;
	}
}

bool OperatorsMap::isUnary(OperatorType type)
{
	switch (type) 
	{
		case OT_ADD: case OT_SUBTRACT: case OT_LOGICAL_NOT: case OT_BITWISE_NOT: case OT_INCREMENT: case OT_DECREMENT: case OT_PARENTHESIS_OPEN: case OT_TRY: case OT_CATCH:
		case OT_CURLY_BRACES_OPEN: case OT_WHILE: case OT_DO: case OT_IF: case OT_ELSE: case OT_UNLESS: case OT_IS: case OT_AS: case OT_FOR: case OT_RETURN: case OT_THROW:
		case OT_DELETE: case OT_BREAK: case OT_CONTINUE: case OT_TYPE: case OT_INTERFACE: case OT_NAMESPACE: case OT_NEW: case OT_AT: case OT_CONST: case OT_INCLUDE: case OT_IMPORT: case OT_EXTERN:
			return true;
		default: 
			return false;
	}
}

bool OperatorsMap::isBinary(OperatorType type)
{
	switch(type) 
	{
		case OT_INCREMENT: case OT_DECREMENT: case OT_LOGICAL_NOT: case OT_BITWISE_NOT:
			return false;
		default: 
			return true;
	}
}

bool OperatorsMap::isKeyword(Operator op)
{
	switch (op._type) 
	{
		case OT_SWITCH: case OT_CASE: case OT_DEFAULT: case OT_WHILE: case OT_FOR:case OT_DO: case OT_IF: case OT_ELSE: case OT_UNLESS: case OT_EXTERN: case OT_TRY: case OT_CATCH:
		case OT_RETURN: case OT_TYPE: case OT_INTERFACE: case OT_NAMESPACE: case OT_DELETE: case OT_BREAK: case OT_CONTINUE: case OT_GET: case OT_SET: case OT_CONST: case OT_INCLUDE: case OT_IMPORT: case OT_THROW:
			return true;
		default: 
			return false;
	}
}

bool OperatorsMap::isAssignment(OperatorType type)
{
	switch (type) 
	{
		case OT_ASSIGN_EQUAL: case OT_ASSIGN_ADD: case OT_ASSIGN_DIVIDE: case OT_ASSIGN_MULTIPLY: case OT_ASSIGN_SUBTRACT: case OT_ASSIGN_MODULUS:
		case OT_ASSIGN_BITWISE_AND: case OT_ASSIGN_BITWISE_OR: case OT_ASSIGN_BITWISE_XOR: case OT_ASSIGN_SHIFT_LEFT: case OT_ASSIGN_SHIFT_RIGHT:
			return true;
		default: 
			return false;
	}
}

ReturnType OperatorsMap::getReturnType(OperatorType type)
{
	switch (type) 
	{
		case OT_ADD:						//	+
		case OT_SUBTRACT:					//	-
		case OT_DIVIDE:						//	/
		case OT_MULTIPLY:					//	*
		case OT_MODULUS:					//	%
		case OT_INCREMENT:					//	++
		case OT_DECREMENT:					//	--
		case OT_BITWISE_AND:				//	&
		case OT_BITWISE_OR:					//	?
		case OT_BITWISE_XOR:				//	^
		case OT_BITWISE_NOT:				//	~
		case OT_BITWISE_SHIFT_LEFT:			//	<<
		case OT_BITWISE_SHIFT_RIGHT:		//	>>
		case OT_ASSIGN_EQUAL:				//	:=
		case OT_ASSIGN_ADD:					//	+=
		case OT_ASSIGN_SUBTRACT:			//	-=
		case OT_ASSIGN_MULTIPLY:			//	*=
		case OT_ASSIGN_DIVIDE:				//	/=
		case OT_ASSIGN_MODULUS:				//	%=
		case OT_ASSIGN_SHIFT_LEFT:			//	<<=
		case OT_ASSIGN_SHIFT_RIGHT:			//	>>=
		case OT_ASSIGN_BITWISE_AND:			//	&=
		case OT_ASSIGN_BITWISE_OR:			//	?=
		case OT_ASSIGN_BITWISE_XOR:			//	^=
			return RT_LEFT;

		case OT_EQUAL:						//	=
		case OT_NOT_EQUAL:					//	!=
		case OT_GREATER:					//	>
		case OT_SMALLER:					//	<
		case OT_GREATER_EQUAL:				//	>=
		case OT_SMALLER_EQUAL:				//	<=
		case OT_IS:							//	is
		case OT_AS:							// 	as
		case OT_LOGICAL_AND:				//	and
		case OT_LOGICAL_OR:					//	or
		case OT_LOGICAL_NOT:				//	not
			return RT_BOOLEAN;

		case OT_SQUARE_BRACKETS_OPEN:
			return RT_ARRAY;

		default: 
			return RT_VOID;
	}
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
	return pair<const unicode_string, Operator>(unicode_string(""), { OT_UNKNOWN, unicode_string(""), NON_ASSCOCIATIVE, 0 });
}

//#define SPECIAL(a) a, a, a

#define KEYWORD NON_ASSCOCIATIVE, NONE, NONE, NONE
#define NON_PARSER NON_ASSCOCIATIVE, NONE, NONE, NONE

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
	_wordsMap[UTF8("as")]	= { OT_AS,						UTF8("as"),		LEFT_TO_RIGHT,		135,	NONE,	NONE };
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
	_map[UTF8("#")]			= { OT_SINGLE_LINE_COMMENT,			UTF8("#"),	NON_PARSER };
	_map[UTF8("/*")]		= { OT_MULTI_LINE_COMMENT_OPEN,		UTF8("/*"),	NON_PARSER };
	_map[UTF8("*/")]		= { OT_MULTI_LINE_COMMENT_CLOSE,	UTF8("*/"),	NON_PARSER };
	_map[UTF8("\"")]		= { OT_DOUBLE_QUOTE,				UTF8("\""),	NON_PARSER };
	_map[UTF8("\\")]		= { OT_DOUBLE_QUOTE,				UTF8("\""),	NON_PARSER };
	_map[UTF8("'")]			= { OT_SINGLE_QUOTE,				UTF8("'"),	NON_PARSER };
	_map[UTF8("..")]		= { OT_IGNORE_LINEBREAK,			UTF8(".."),	NON_PARSER };

	// Word operators:
	_wordsMap[UTF8("while")]		= { OT_WHILE,		UTF8("while"),		KEYWORD };
	_wordsMap[UTF8("for")]			= { OT_FOR,			UTF8("for"),		KEYWORD };
	_wordsMap[UTF8("do")]			= { OT_DO,			UTF8("do"),			KEYWORD };
	_wordsMap[UTF8("switch")]		= { OT_SWITCH,		UTF8("switch"),		KEYWORD };
	_wordsMap[UTF8("case")]			= { OT_CASE,		UTF8("case"),		KEYWORD };
	_wordsMap[UTF8("default")]		= { OT_DEFAULT,		UTF8("default"),	KEYWORD };
	_wordsMap[UTF8("else")]			= { OT_ELSE,		UTF8("else"),		KEYWORD };
	_wordsMap[UTF8("try")]			= { OT_TRY,			UTF8("try"),		KEYWORD };
	_wordsMap[UTF8("catch")]		= { OT_CATCH,		UTF8("catch"),		KEYWORD };
	_wordsMap[UTF8("unless")]		= { OT_UNLESS,		UTF8("unless"),		KEYWORD };
	_wordsMap[UTF8("return")]		= { OT_RETURN,		UTF8("return"),		KEYWORD };
	_wordsMap[UTF8("throw")]		= { OT_THROW,		UTF8("throw"),		KEYWORD };
	_wordsMap[UTF8("get")]			= { OT_GET,			UTF8("get"),		KEYWORD };
	_wordsMap[UTF8("set")]			= { OT_SET,			UTF8("set"),		KEYWORD };
	_wordsMap[UTF8("type")]			= { OT_TYPE,		UTF8("type"),		KEYWORD };
	_wordsMap[UTF8("include")]		= { OT_INCLUDE,		UTF8("include"),	KEYWORD };
	_wordsMap[UTF8("import")]		= { OT_IMPORT,		UTF8("import"),		KEYWORD };
	_wordsMap[UTF8("delete")]		= { OT_DELETE,		UTF8("delete"),		KEYWORD };
	_wordsMap[UTF8("interface")]	= { OT_INTERFACE,	UTF8("interface"),	KEYWORD };
	_wordsMap[UTF8("namespace")]	= { OT_NAMESPACE,	UTF8("namespace"),	KEYWORD };
	_wordsMap[UTF8("const")]		= { OT_CONST,		UTF8("const"),		KEYWORD };
	_wordsMap[UTF8("extern")]		= { OT_EXTERN,		UTF8("extern"),		KEYWORD };
	_wordsMap[UTF8("continue")]		= { OT_CONTINUE,	UTF8("continue"),	KEYWORD };
	_wordsMap[UTF8("break")]		= { OT_BREAK,		UTF8("break"),		KEYWORD };
}
