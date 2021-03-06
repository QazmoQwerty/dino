#pragma once

#include <string>
#include <iostream>
#include "llvm/IR/DIBuilder.h"

#include <stdio.h>
// #include <conio.h>
#include <stdlib.h>
// #include <direct.h>

using std::string;

class SourceFile {
private:

    string _path;
public:
    /* Debug Info File */
    llvm::DIFile *_backendDIFile;
    
    SourceFile(string path);

    string getOriginalPath();
    string getName();
    string getPath();
};