#include "ErrorReporter.h"

vector<Error> ErrorReporter::errors;

Error ErrorReporter::report(string msg, ErrorType errTy, PositionInfo pos, bool isFatal)
{
    errors.push_back({ msg, errTy, pos });
    if (isFatal)
        throw "[Aborting due to previous error]";
    return errors.back();
}

Error ErrorReporter::reportInternal(string msg, ErrorType errTy, PositionInfo pos)
{
    errors.push_back({ msg, errTy, pos });
    return errors.back();
}

void ErrorReporter::showAll() 
{
    for (uint i = 0; i < errors.size(); i++)
    {
        if (i) llvm::errs() << "\n";
        show(errors[i]);
    }
}

void ErrorReporter::show(Error &err) 
{
    if (err.pos.file != NULL)
    {   
        llvm::errs() << BOLD(FRED(" --> ")) << BOLD("\"" << err.pos.file->getOriginalPath() << "\": line " << err.pos.line) << BOLD(FRED("\n  | \n"));
        string line = getLine(err.pos.file->getOriginalPath(), err.pos.line);
        llvm::errs() << BOLD(FRED("  | ")) << line << BOLD(FRED("\n  | "));
        for (int i = 0; i < err.pos.startPos; i++)
        {
            if (line[i] == '\t')
                llvm::errs() << "\t";    
            else llvm::errs() << " ";
        }
        for (int i = 0; i < err.pos.endPos - err.pos.startPos; i++)
            llvm::errs() << BOLD(FRED("^"));
        llvm::errs() << "\n";
    }
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
        case ERR_INTERNAL: return "Internal Error";
        default: return "Error";
    }
}

string ErrorReporter::getLine(string fileName, int line)
{
    std::fstream file(fileName);
    file.seekg(std::ios::beg);
    for (int i=0; i < line - 1; ++i)
        file.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
    string ret;
    std::getline(file, ret);
    return ret;
}
