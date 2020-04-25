
#pragma once

#include <iostream>
#include <string>
#include <exception>
#include <limits>

#include "../TerminalColors.h"
#include "../TypeEnums.h"
#include "../../Lexer/Token.h"
#include "llvm/Support/raw_ostream.h"

#define POSITION_INFO_NONE {0, 0, 0, "" }

using std::string;
using std::exception;

/* Represents dino compile time errors */
typedef struct Error {
    string msg;
    ErrorType errTy;
    PositionInfo pos;
} Error;

/*
    Static class which contains a vector of Errors. 
    Any compile errors/warnings found will be reported here.
*/
class ErrorReporter {
private:
    static vector<Error> errors;
    static string toString(ErrorType type);
    static string getLine(string fileName, int line);
public:
    /* Pretty-prints all errors reported so far */
    static void showAll();

    /* Pretty-prints an Error. */
    static void show(Error &err);

    /* 
        Create and save an error/warning.
        Returns the Error created.
        If isFatal is (true), the function will throw a value.
    */
    static Error report(string msg, ErrorType errTy, PositionInfo pos, bool isFatal = false);
};