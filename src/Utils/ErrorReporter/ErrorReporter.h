
#pragma once

#include <iostream>
#include <string>
#include <exception>
#include <limits>

#include "../TerminalColors.h"
#include "../TypeEnums.h"
#include "../../Lexer/Token.h"
#include "llvm/Support/raw_ostream.h"

#define POSITION_INFO_NONE PositionInfo{0, 0, 0, NULL}

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

    /*
        Report an INTERNAL error - these should never be shown to a user of the compiler (in theory at least).
    */
    static Error reportInternal(string msg, ErrorType errTy, PositionInfo pos = POSITION_INFO_NONE);
};

#define FATAL_ERROR(_string) throw ErrorReporter::reportInternal(std::string(_string) + " at " + __func__ + ":" + std::to_string(__LINE__) + " in " + __FILE__ , ERR_INTERNAL)

#define TODO FATAL_ERROR("TODO reached");
#define UNREACHABLE FATAL_ERROR("unreachable reached");
#define ASSERT(cond) if(!(cond)) FATAL_ERROR("assertion failed")