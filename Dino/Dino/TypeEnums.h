#pragma once

/*
	Types of tokens the lexer creates
*/
enum TokenType
{
	TT_WHITESPACE,
	TT_NEWLINE,
	TT_OPERATOR,
	TT_LINE_BREAK,
	TT_IDENTIFIER,
	TT_LITERAL,
	TT_SINGLE_LINE_COMMENT,
	TT_MULTI_LINE_COMMENT,
	TT_UNKNOWN,
};

enum ReturnType
{
	RT_LEFT,
	RT_RIGHT,
	RT_BOOLEAN,
	RT_VOID,
};

/*
	Types of literals the lexer proccesses
*/
enum LiteralType
{
	LT_BOOLEAN,
	LT_INTEGER,
	LT_FRACTION,
	LT_CHARACTER,
	LT_STRING,
	LT_FUNCTION,
	LT_TYPE,
	LT_NULL,
	LT_UNKNOWN,
};

/*
	Types of characters the lexer proccesses
*/
enum CharType
{
	CT_WHITESPACE,
	CT_LINE_BREAK,
	CT_NEWLINE,
	CT_LETTER,
	CT_DIGIT,
	CT_OPERATOR,
	CT_UNKNOWN,
};

/*
	Types of operators the lexer proccesses.
*/
enum OperatorType
{
	// Arithmetic Operators:
	OT_ADD,							//	+
	OT_SUBTRACT,					//	-
	OT_DIVIDE,						//	/
	OT_MULTIPLY,					//	*
	OT_MODULUS,						//	%
	OT_INCREMENT,					//	++
	OT_DECREMENT,					//	--
	OT_EXPONENTIATION,				//	^^

	// Relational Operators:
	OT_EQUAL,						//	==
	OT_NOT_EQUAL,					//	!=
	OT_GREATER,						//	>
	OT_SMALLER,						//	<
	OT_GREATER_EQUAL,				//	>=
	OT_SMALLER_EQUAL,				//	<=

	// Bitwise Operators:
	OT_BITWISE_AND,					//	&
	OT_BITWISE_OR,					//	?
	OT_BITWISE_XOR,					//	^
	OT_BITWISE_NOT,					//	~
	OT_BITWISE_SHIFT_LEFT,			//	<<
	OT_BITWISE_SHIFT_RIGHT,			//	>>

	// Logical Operators:
	OT_LOGICAL_AND,					//	&&
	OT_LOGICAL_OR,					//	||
	OT_LOGICAL_NOT,					//	!
					
	// Assignment Operators:
	OT_ASSIGN_EQUAL,				//	=
	OT_ASSIGN_ADD,					//	+=
	OT_ASSIGN_SUBTRACT,				//	-=
	OT_ASSIGN_MULTIPLY,				//	*=
	OT_ASSIGN_DIVIDE,				//	/=
	OT_ASSIGN_MODULUS,				//	%=
	OT_ASSIGN_SHIFT_LEFT,			//	<<=
	OT_ASSIGN_SHIFT_RIGHT,			//	>>=
	OT_ASSIGN_BITWISE_AND,			//	&=
	OT_ASSIGN_BITWISE_OR,			//	|=
	OT_ASSIGN_BITWISE_XOR,			//	^=

	// Brackets / Parenthesis:
	OT_PARENTHESIS_OPEN,			//	(
	OT_PARENTHESIS_CLOSE,			//	)
	OT_SQUARE_BRACKETS_OPEN,		//	[
	OT_SQUARE_BRACKETS_CLOSE,		//	]
	OT_CURLY_BRACES_OPEN,			//	{
	OT_CURLY_BRACES_CLOSE,			//	}

	// Comments:
	OT_SINGLE_LINE_COMMENT,			//	//
	OT_MULTI_LINE_COMMENT_OPEN,		//	/*
	OT_MULTI_LINE_COMMENT_CLOSE,	//	*/

	// Words:
	OT_WHILE,
	OT_DO,
	OT_FOR,
	OT_IF,
	OT_ELSE,
	OT_UNLESS,
	OT_IS,
	OT_RETURN,
	OT_DELETE,
	OT_TYPE,
	OT_INTERFACE,
	OT_NAMESPACE,
	OT_IMPLEMENTS,
	OT_NEW,
	OT_GET,
	OT_SET,
	OT_SWITCH,
	OT_CASE,
	OT_DEFAULT,


	// Misc:
	OT_AT,							//	@
	OT_SINGLE_QUOTE,				//	'
	OT_DOUBLE_QUOTE,				//	"
	OT_PERIOD,						//	.
	OT_COMMA,						//	,
	OT_COLON,						//	:
	OT_ESCAPE,						//	\ 
	OT_EOF,
	OT_UNKNOWN,
};

/*
	Types of exceptions (in all of the project)
*/
enum ExceptionType
{
	EXT_GENERAL,
	EXT_LEXER,
};

/*
	Types of AST statements 
*/
enum StatementType
{
	ST_SWITCH,
	ST_FOR_LOOP,
	ST_UNARY_ASSIGNMENT,
	ST_STATEMENT_BLOCK,
	ST_IF_THEN_ELSE,
	ST_WHILE_LOOP,
	ST_DO_WHILE_LOOP,
	ST_LIST,
	ST_EXP_STATEMENT_LIST,
	ST_VARIABLE_DECLARATION,
	ST_UNARY_OPERATION,
	ST_FUNCTION_DECLARATION,
	ST_ASSIGNMENT,
	ST_FUNCTION_CALL,
	ST_INCREMENT,
	ST_TYPE_DECLARATION,
	ST_INTERFACE_DECLARATION,
	ST_NAMESPACE_DECLARATION,
	ST_PROPERTY_DECLARATION,
	ST_UNKNOWN,
};

/*
	Types of AST expressions
*/
enum ExpressionType
{
	ET_UNARY_ASSIGNMENT,
	ET_VARIABLE_DECLARATION,
	ET_INCREMENT,
	ET_EXP_STATEMENT_LIST,
	ET_ASSIGNMENT,
	ET_LIST,
	ET_BINARY_OPERATION,
	ET_UNARY_OPERATION,
	ET_FUNCTION_CALL,
	ET_IDENTIFIER,
	ET_CONDITIONAL_EXPRESSION,
	ET_LITERAL,
	ET_FUNCTION_LITERAL,
	ET_UNKNOWN,
	ET_TYPE
};

enum ExactType 
{
	EXACT_BASIC,
	EXACT_ARRAY,
	EXACT_TYPELIST,
	EXACT_FUNCTION,
	EXACT_POINTER,
	EXACT_PROPERTY,
};