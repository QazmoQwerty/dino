#pragma once

#include <sstream>
#include <fstream>
#include "Unicode/utf8.h"
#include "../Parser/AstNode.h"
#include "../Decorator/DstNode.h"

using std::stringstream;
using std::ofstream;

/*
	Returns a string (graphviz format) representation of an AST.
	NOTE: The 'showline' option is currently redacted.
*/
string astToString(AST::Node* node, bool showLine)
{
	static int nullCount = -1;
	if (node == NULL || node == nullptr)
		return "";
	stringstream ss;
	int id = node->getNodeId();
	/*int line = node->getLine();
	// if (showLine)
	// 	ss << id << " [label=\"line " << line << "\n" << node->toString() << "\"];";
	else */ss << id << " [label=\"" << node->toString() << "\"];";
	for (auto child : node->getChildren()) 
	{
		if (child == NULL) 
		{
			ss << id << "->" << nullCount << ';';
			ss << nullCount-- << " [label=\"<NULL>\"];";
		}
		else ss << id << "->" << child->getNodeId() << ';';
	}
	ss << '\n';
	for (auto child : node->getChildren())
		ss << astToString(child, showLine);
	return ss.str();
}

/*
	Outputs a .gv (graphviz) representation of an AST to fileName.
	NOTE: The 'showline' option is currently redacted.
*/
void astToFile(string filename, AST::Node* ast, bool showLine)
{
	ofstream file;
	file.open(filename);
	file << utf8::bom;	// make sure output file is utf8 encoded
	file << "digraph G {\n";
	file << astToString(ast, showLine);
	file << '}';
	file.close();
}


/****************************************/

/*
	Returns a string (graphviz format) representation of a DST.
	NOTE: The 'showline' option is currently redacted.
*/
string dstToString(DST::Node* node, bool showLine)
{
	static int nullCount = -1;
	if (node == NULL || node == nullptr)
		return "";
	stringstream ss;
	int id = node->getNodeId();
	/* int line = node->getLine();
	// if (showLine)
	// 	ss << id << " [label=\"line " << line << "\n" << node->toString() << "\"];";
	else*/ ss << id << " [label=\"" << node->toString() << "\"];";
	for (auto child : node->getChildren())
	{
		if (child == NULL)
		{
			ss << id << "->" << nullCount << ';';
			ss << nullCount-- << " [label=\"<NULL>\"];";
		}
		else ss << id << "->" << child->getNodeId() << ';';
	}
	ss << '\n';
	for (auto child : node->getChildren())
		ss << dstToString(child, showLine);
	return ss.str();
}

/*
	Outputs a .gv (graphviz) representation of a DST to fileName.
	NOTE: The 'showline' option is currently redacted.
*/
void dstToFile(string filename, DST::Node* dst, bool showLine)
{
	ofstream file;
	file.open(filename);
	//file << utf8::bom;	// make sure output file is utf8 encoded - needed in windows
	file << "digraph G {\n";
	file << dstToString(dst, showLine);
	file << '}';
	file.close();
}
