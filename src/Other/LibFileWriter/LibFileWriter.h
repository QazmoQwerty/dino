#include "../../Decorator/DstNode.h"
#include <sstream>
#include <fstream>

using std::stringstream;
using std::ofstream;

namespace LibFileWriter 
{
    void WriteIndentCount(ofstream &stream, int count);
    void Write(string fileName, string bcFileName, DST::Program *node);
    void Write(ofstream &stream, int indentCount, DST::NamespaceDeclaration *node);
    void Write(ofstream &stream, DST::ConstDeclaration *node);
    void Write(ofstream &stream, DST::FunctionDeclaration *node);
    void Write(ofstream &stream, int indentCount, DST::InterfaceDeclaration *node);
    void Write(ofstream &stream, DST::VariableDeclaration *node);
    void Write(ofstream &stream, int indentCount, DST::PropertyDeclaration *node);
    void Write(ofstream &stream, int indentCount, DST::TypeDeclaration *node);
}