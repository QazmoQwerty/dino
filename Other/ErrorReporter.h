
#pragma once

#include <iostream>
#include <string>
#include <exception>
#include <limits>

#include "TypeEnums.h"
#include "../Lexer/Token.h"
#include "llvm/Support/raw_ostream.h"

using std::string;
using std::exception;

typedef struct Error {
    string msg;
    ErrorType errTy;
    PositionInfo pos;
} Error;

class ErrorReporter {
private:
    static vector<Error> errors;
    static string toString(ErrorType type);
    static string getLine(string fileName, int line);
public:
    static void showAll();
    static void show(Error &err);
    static void report(string msg, ErrorType errTy, PositionInfo pos, bool isFatal = false);
};