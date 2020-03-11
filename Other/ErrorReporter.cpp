#include "ErrorReporter.h"

vector<Error> ErrorReporter::errors;

Error ErrorReporter::report(string msg, ErrorType errTy, PositionInfo pos, bool isFatal)
{
    errors.push_back({ msg, errTy, pos });
    if (isFatal)
        throw "[Aborting due to previous error]";
    return errors.back();
}

void ErrorReporter::showAll() 
{
    for (unsigned int i = 0; i < errors.size(); i++)
    {
        if (i) llvm::errs() << "\n";
        show(errors[i]);
    }
}

void ErrorReporter::show(Error &err) 
{
    llvm::errs() << BOLD("In file \"" << err.pos.file << "\", line " << err.pos.line) << "\n";
    string line = getLine(err.pos.file, err.pos.line);
    llvm::errs() << line << "\n";
    for (int i = 0; i < err.pos.startPos; i++)
    {
        if (line[i] == '\t')
            llvm::errs() << "\t";    
        else llvm::errs() << " ";
    }
    for (int i = 0; i < err.pos.endPos - err.pos.startPos; i++)
        llvm::errs() << BOLD(FRED("^"));
    llvm::errs() << "\n";
    llvm::errs() << BOLD(FRED(toString(err.errTy) << ": ") << err.msg) << "\n";
}

string ErrorReporter::toString(ErrorType type) 
{
    switch (type)
    {
        // case ERR_LEXER: return "LexError";
        // case ERR_PARSER: return "ParseError";
        // case ERR_DECORATOR: return "DecorateError";
        // case ERR_CODEGEN: return "CodeGenError";
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
