#pragma once

#include "AstNode.h"
#include <sstream>
#include <fstream>

using std::stringstream;
using std::ofstream;

string astToString(AST::Node* node)
{
	if (node == NULL || node == nullptr)
		return "";
	stringstream ss;
	int id = node->getNodeId();
	ss << id << " [label=\"" << node->toString() << "\"];";
	for (auto child : node->getChildren())
		ss << id << "->" << child->getNodeId() << '|';
	ss << '\n';
	for (auto child : node->getChildren())
		ss << astToString(child);
	return ss.str();
}

void astToFile(string filename, AST::Node* ast)
{
	ofstream file;
	file.open(filename);
	file << "digraph G {\n";
	file << astToString(ast);
	file << '}';
	file.close();
}