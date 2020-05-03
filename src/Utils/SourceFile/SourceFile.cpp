#include "SourceFile.h"

SourceFile::SourceFile(string str) : _str(str), _backendDIFile(NULL) {}

// TODO!
string SourceFile::getFullPath() 
{
    return _str; 
}

string SourceFile::getName() { return _str; }
string SourceFile::getPath() { return "."; }