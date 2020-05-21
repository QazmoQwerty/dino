#include "AstToFile.h"

/*
	Returns a string (graphviz format) representation of an AST.
*/
string astToString(AST::Node* node)
{
	static long nullCount = -1;
	if (node == NULL) return "";
	stringstream ss;
	ss << (unsigned long)node << " [label=\"" << node->toString() << "\"];";
	for (auto child : node->getChildren()) 
	{
		if (child == NULL) 
		{
			ss << (unsigned long)node << "->" << nullCount << ';';
			ss << (unsigned long)nullCount-- << " [label=\"<NULL>\"];";
		}
		else ss << (unsigned long)node << "->" << (unsigned long)child << ';';
	}
	ss << '\n';
	for (auto child : node->getChildren())
		ss << astToString(child);
	return ss.str();
}

/*
	Outputs a .gv (graphviz) representation of an AST to fileName.
*/
void astToFile(string filename, AST::Node* ast)
{
	ofstream file;
	file.open(filename);
	file << utf8::bom;	// make sure output file is utf8 encoded
	file << "digraph G {\n";
	file << astToString(ast);
	file << '}';
	file.close();
}


/****************************************/

/*
	Returns a string (graphviz format) representation of a DST.
*/
string dstToString(DST::Node* node)
{
	static int nullCount = -1;
	if (node == NULL || node == nullptr)
		return "";
	stringstream ss;
	int id = node->getNodeId();
	ss << id << " [label=\"" << node->toString() << "\"];";
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
		ss << dstToString(child);
	return ss.str();
}

/*
	Outputs a .gv (graphviz) representation of a DST to fileName.
*/
void dstToFile(string filename, DST::Node* dst)
{
	ofstream file;
	file.open(filename);
	//file << utf8::bom;	// make sure output file is utf8 encoded - needed in windows
	file << "digraph G {\n";
	file << dstToString(dst);
	file << '}';
	file.close();
}
