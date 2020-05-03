#pragma once

#include <string>
#include "llvm/IR/DIBuilder.h"

using std::string;

class SourceFile {
private:
    string _str;

    /* Debug Info File */
public:
    llvm::DIFile *_backendDIFile;
    
    SourceFile(string fullPath);

    string getFullPath();
    string getName();
    string getPath();
};