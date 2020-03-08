#include "LibFileWriter.h"

void LibFileWriter::Write(string fileName, string bcFileName, DST::Program *node) 
{   
    ofstream stream;
	stream.open(fileName);
    stream << "import \"" << bcFileName << "\"\n\n";
    for (auto i : node->getNamespaces()) 
        Write(stream, 0, i.second);
    stream.close();
}

void LibFileWriter::WriteIndentCount(ofstream &stream, int count) 
{
    while (count--) 
        stream << "\t";
}

void LibFileWriter::Write(ofstream &stream, int indentCount, DST::NamespaceDeclaration *node) 
{
    stream << "namespace " << node->getName().to_string() << " {\n";
    for (auto i : node->getMembers())
    {
        WriteIndentCount(stream, indentCount + 1);
        switch (i.second.first->getStatementType())
        {
            case ST_NAMESPACE_DECLARATION:  Write(stream, indentCount + 1, (DST::NamespaceDeclaration*)i.second.first); break;
            case ST_CONST_DECLARATION:      Write(stream, (DST::ConstDeclaration*)i.second.first);                      break;
            case ST_FUNCTION_DECLARATION:   Write(stream, (DST::FunctionDeclaration*)i.second.first);                   break;
            case ST_INTERFACE_DECLARATION:  Write(stream, indentCount + 1, (DST::InterfaceDeclaration*)i.second.first); break;
            case ST_VARIABLE_DECLARATION:   Write(stream, (DST::VariableDeclaration*)i.second.first);   stream << "\n"; break;
            case ST_PROPERTY_DECLARATION:   Write(stream, indentCount + 1, (DST::PropertyDeclaration*)i.second.first);  break;
            case ST_TYPE_DECLARATION:       Write(stream, indentCount + 1, (DST::TypeDeclaration*)i.second.first);      break;
            default: throw "this shouldn't be possible...";
        }
    }
    stream << "}\n";
}

void LibFileWriter::Write(ofstream &stream, DST::ConstDeclaration *node) 
{
    // TODO
    throw DinoException("LibFileWriter implementation for const declarations is missing", EXT_GENERAL, node->getLine());
}

void LibFileWriter::Write(ofstream &stream, int indentCount, DST::TypeDeclaration *node) 
{
    stream << "type " << node->getName().to_string() << " {\n";
    for (auto i : node->getMembers())
    {
        WriteIndentCount(stream, indentCount + 1);
        switch (i.second.first->getStatementType())
        {
            case ST_CONST_DECLARATION:      Write(stream, (DST::ConstDeclaration*)i.second.first);                      break;
            case ST_VARIABLE_DECLARATION:   Write(stream, (DST::VariableDeclaration*)i.second.first);   stream << "\n"; break;
            case ST_FUNCTION_DECLARATION:   Write(stream, (DST::FunctionDeclaration*)i.second.first);                   break;
            case ST_PROPERTY_DECLARATION:   Write(stream, indentCount + 1, (DST::PropertyDeclaration*)i.second.first);  break;
            default: throw "this shouldn't be possible...";
        }
    }
    stream << "}\n";
}

void LibFileWriter::Write(ofstream &stream, int indentCount, DST::InterfaceDeclaration *node) 
{
    stream << "interface " << node->getName().to_string() << " {\n";
    for (auto i : node->getDeclarations())
    {
        WriteIndentCount(stream, indentCount + 1);
        switch (i.second.first->getStatementType())
        {
            case ST_CONST_DECLARATION:      Write(stream, (DST::ConstDeclaration*)i.second.first);      break;
            case ST_FUNCTION_DECLARATION:   Write(stream, (DST::FunctionDeclaration*)i.second.first);   break;
            case ST_PROPERTY_DECLARATION:   Write(stream, indentCount + 1, (DST::PropertyDeclaration*)i.second.first);   break;
            default: throw "this shouldn't be possible...";
        }
    }
    stream << "}\n";
}

void LibFileWriter::Write(ofstream &stream, DST::VariableDeclaration *node) 
{
    stream << node->getType()->toShortString() << " " << node->getVarId().to_string();
}

void LibFileWriter::Write(ofstream &stream, int indentCount, DST::PropertyDeclaration *node) 
{
    stream << node->getReturnType()->toShortString() << " " << node->getName().to_string();
    
    if (node->getGet() && node->getSet())
        stream << " {\n";

    

    // string llvmGetFuncId = node->getName().to_string() + ".get";
    if (node->getGet())
        stream << "get: extern \"" << node->_llvmGetFuncId << "\"\n";
    
    // string llvmSetFuncId = node->getName().to_string() + ".set";
    if (node->getSet())
        stream << "set: extern \"" << node->_llvmSetFuncId << "\"\n";

    if (node->getGet() && node->getSet())
        stream << "}\n";
}

void LibFileWriter::Write(ofstream &stream, DST::FunctionDeclaration *node) 
{
    stream << node->getVarDecl()->getType()->toShortString() << " " << node->getVarDecl()->getVarId().to_string() << "(";
    for (unsigned int i = 0; i < node->getParameters().size(); i++) 
    {
        if (i) stream << ", ";
        Write(stream, node->getParameters()[i]);
    }
    // string llvmFuncId = node->getVarDecl()->getVarId().to_string();
    stream << "): extern \"" << node->_llvmFuncId << "\"\n";
}