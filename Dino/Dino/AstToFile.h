#pragma once

#include "AstNode.h"
#include <sstream>
#include <fstream>

using std::stringstream;
using std::ofstream;

string astToString(AST::Node* node)
{
	stringstream ss;
	int id = node->getNodeId();
	ss << id << "[label=\"" << node->toString() << "\";";
	for (auto child : node->getChildren())
		ss << id << "->" << child->getNodeId() << ';';
	ss << '\n';
	for (auto child : node->getChildren())
		ss << astToString(child);
	return ss.str();
}

void AstToFile(string fileName, AST::Node* ast)
{
	ofstream file;
	file.open("AstDisplay.gv");
	file << "digraph G {\n";
	file << astToString(ast);
	file << '}';
	file.close();
}