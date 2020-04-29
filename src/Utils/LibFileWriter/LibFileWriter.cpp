#include "LibFileWriter.h"

void LibFileWriter::Write(string fileName, string bcFileName, DST::Program *node) 
{   
    ofstream stream;
	stream.open(fileName);
    for (auto i : node->getNamespaces()) 
        if (i.first != ".")
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
    WriteIndentCount(stream, indentCount);
    stream << "}\n";
}

void LibFileWriter::Write(ofstream &stream, DST::ConstDeclaration *node) 
{
    if (node->getExpression()->getExpressionType() != ET_LITERAL || ((DST::Literal*)node->getExpression())->getLiteralType() == LT_FUNCTION)
        throw ErrorReporter::report("Only literal values are currently supported in library const declarations", ERR_CODEGEN, node->getPosition());
    stream << "const " << node->getName().to_string() << " ≡ " << ((DST::Literal*)node->getExpression())->toShortString() << "\n";
}

void LibFileWriter::Write(ofstream &stream, int indentCount, DST::TypeDeclaration *node) 
{
    stream << "type " << node->getName().to_string();
    if (node->getInterfaces().size()) 
    {
        stream << " is ";
        bool first = true;
        for (auto i : node->getInterfaces())  // TODO - what if the interface which we are implementing is not in the current scope?
        {
            if (first)
            {
                stream << i->getName().to_string();
                first = false;
            } 
            else stream << ", " << i->getName().to_string();
        }         
    }
    stream << " {\n";
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
    WriteIndentCount(stream, indentCount);
    stream << "}\n";
}

void LibFileWriter::Write(ofstream &stream, int indentCount, DST::InterfaceDeclaration *node) 
{
    stream << "interface " << node->getName().to_string();
    if (node->getImplements().size()) 
    {
        stream << " is ";
        bool first = 0;
        for (auto i : node->getImplements())  // TODO - what if the interface which we are implementing is not in the current scope?
        {
            if (first)
            {
                stream << i->getName().to_string();
                first = false;
            } 
            else stream << ", " << i->getName().to_string();
        }         
    }
    stream << " {\n";
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
    WriteIndentCount(stream, indentCount);
    stream << "}\n";
}

void LibFileWriter::Write(ofstream &stream, DST::VariableDeclaration *node) 
{
    stream << node->getType()->toShortString() << " " << node->getVarId().to_string();
}

void LibFileWriter::Write(ofstream &stream, int indentCount, DST::PropertyDeclaration *node) 
{
    if (node->getReturnType()->getExactType() == EXACT_PROPERTY)
        stream << node->getReturnType()->as<DST::PropertyType>()->getReturn()->toShortString() << " " << node->getName().to_string();
    
    if (!node->getGet() && !node->getSet())
    {
        auto propTy = node->getReturnType()->as<DST::PropertyType>();
        stream << ((propTy->hasGet() && propTy->hasSet()) ? " { get | set }\n" 
                : propTy->hasGet() ? ": get\n" : ": set\n");
        return;
    }

    if (node->getGet() && node->getSet())
    {
        stream << " {\n";
        WriteIndentCount(stream, indentCount + 1);
    }
    else stream << ": ";

    if (node->getGet())
    {
        stream << "get: extern \"" << node->_llvmGetFuncId << "\"\n";
        if (node->getSet())
            WriteIndentCount(stream, indentCount + 1);
    }
    
    if (node->getSet())
        stream << "set: extern \"" << node->_llvmSetFuncId << "\"\n";

    if (node->getGet() && node->getSet())
    {
        WriteIndentCount(stream, indentCount);
        stream << "}\n";
    }
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
    if (node->getContent() == NULL)
        stream << ")\n";
    else stream << "): extern \"" << node->_llvmFuncId << "\"\n";
}