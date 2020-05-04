#include "SourceFile.h"

SourceFile::SourceFile(string str) : _str(str), _backendDIFile(NULL) {}

// TODO!
string SourceFile::getFullPath() 
{
    return _str; 
}

string SourceFile::getName() { return "Test2.dino"; }
string SourceFile::getPath() { return "/home/magshimim/Desktop/dino/Examples"; }