#include "ErrorReporter.h"

vector<Error> ErrorReporter::errors;

void ErrorReporter::report(string msg, ErrorType errTy, PositionInfo pos, bool isFatal)
{
    errors.push_back({ msg, errTy, pos });
    if (isFatal)
        throw "[Aborting due to previous error]";
}

void ErrorReporter::showAll() 
{
    for (unsigned int i = 0; i < errors.size(); i++)
        show(errors[i]);
}

void ErrorReporter::show(Error &err) 
{
    llvm::errs() << "In file \"" << err.pos.file << "\", line " << err.pos.line << "\n";
    llvm::errs() << getLine(err.pos.file, err.pos.line) << "\n";
    for (int i = 0; i < err.pos.startPos; i++)
        llvm::errs() << " ";
    for (int i = 0; i < err.pos.endPos - err.pos.startPos; i++)
        llvm::errs() << "^";
    llvm::errs() << "\n";
    llvm::errs() << toString(err.errTy) << ": " << err.msg << "\n\n";
}

string ErrorReporter::toString(ErrorType type) 
{
    switch (type)
    {
        case ERR_LEXER: return "LexError";
        case ERR_PARSER: return "ParseError";
        case ERR_DECORATOR: return "DecorateError";
        case ERR_CODEGEN: return "CodeGenError";
        default: return "Error";
    }
}

string ErrorReporter::getLine(string fileName, int line)
{
    std::fstream file(fileName);
    file.seekg(std::ios::beg);
    for(int i=0; i < line - 1; ++i)
        file.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
    string ret;
    std::getline(file, ret);
    return ret;
}
