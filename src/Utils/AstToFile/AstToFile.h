#pragma once

#include <sstream>
#include <fstream>
#include "../Unicode/utf8.h"
#include "../../Parser/AstNode.h"
#include "../../Decorator/DstNode.h"

using std::stringstream;
using std::ofstream;

/*
	Returns a string (graphviz format) representation of an AST.
	NOTE: The 'showline' option is currently redacted.
*/
string astToString(AST::Node* node);

/*
	Outputs a .gv (graphviz) representation of an AST to fileName.
	NOTE: The 'showline' option is currently redacted.
*/
void astToFile(string filename, AST::Node* ast);


/*
	Returns a string (graphviz format) representation of a DST.
	NOTE: The 'showline' option is currently redacted.
*/
string dstToString(DST::Node* node);

/*
	Outputs a .gv (graphviz) representation of a DST to fileName.
	NOTE: The 'showline' option is currently redacted.
*/
void dstToFile(string filename, DST::Node* dst);
