#include "ErrorReporter.h"

namespace ErrorReporter 
{
    vector<string> splitLines(const string& str)
    {
        vector<string> strings;

        string::size_type pos = 0;
        string::size_type prev = 0;
        while ((pos = str.find("\n", prev)) != string::npos)
        {
            strings.push_back(str.substr(prev, pos - prev));
            prev = pos + 1;
        }

        // To get the last substring (or only, if delimiter is not found)
        strings.push_back(str.substr(prev));

        return strings;
    }

    void Error::show()
    {
        if (notes.size() == 0)
            return showBasic();

        for (auto note : notes)
            if (note.pos.file == pos.file && note.pos.line == pos.line)
                goto normal;

        showBasic();
        for (auto note : notes)
            if (note.pos.file != pos.file)
                note.show();
        return;

        normal:

        std::cout << color(tyToString() + ": ") << BOLD(msg) << "\n";
        if (pos.file == NULL)
            return;
        // uncomment this for uglier, but clickable links
        // std::cout << color(" --> ") << "./" << pos.file->getOriginalPath() << ":" << pos.line << ":" << pos.startPos << "\n";
        for (uint i = 1; i < std::to_string(pos.line).size(); i++) std::cout << " ";
        std::cout << color(" --> ") << pos.file->getOriginalPath() << "\n";

        bool isFirst = true;
        for (auto note : notes)
        {
            if (note.pos.file == pos.file && note.pos.line < pos.line && note.msg == "")
            {
                if (isFirst)
                {
                    printIndent();
                    std::cout << "\n";
                    isFirst = false;
                }
                string line = getLine(pos.file->getOriginalPath(), note.pos.line);
                auto tmp = note.errTy;
                note.errTy = errTy;
                note.printIndent(true);
                note.errTy = tmp;
                std::cout << line << "\n";
                bool isFirstB = true;
                for (auto currLine : splitLines(note.subMsg))
                {
                    printIndent();
                    for (int i = 0; i < note.pos.startPos; i++)
                        std::cout << (line[i] == '\t' ? "\t" : " ");
                    
                    if (isFirstB)
                    {
                        for (int i = 0; i < note.pos.endPos - note.pos.startPos; i++)
                            std::cout << note.color("^");
                        std::cout << note.color(" ");
                        isFirstB = false;
                    }
                    std::cout << note.color(currLine);
                    std::cout << "\n";
                }
            }
        }

        string line = getLine(pos.file->getOriginalPath(), pos.line);

        
        if (subMsg != "")
        {
            for (auto currLine : splitLines(subMsg))
            {
                printIndent();
                for (int i = 0; i < pos.startPos; i++)
                    std::cout << (line[i] == '\t' ? "\t" : " ");
                std::cout << color(currLine);
                std::cout << "\n";
            }
        }
        
        printIndent();

        for (int i = 0; i < pos.startPos; i++)
            std::cout << (line[i] == '\t' ? "\t" : " ");
        for (int i = 0; i < pos.endPos - pos.startPos; i++)
            std::cout << color("v");
        
        std::cout << "\n";
        printIndent(true);
        std::cout << line << "\n";

        sortSecondaries();
        for (auto note : notes)
        {
            if (note.pos.file != pos.file)
                note.show();
            else if (note.pos.line == pos.line)
            {
                printIndent();
                for (int i = 0; i < note.pos.startPos; i++)
                    std::cout << (line[i] == '\t' ? "\t" : " ");
                for (int i = 0; i < note.pos.endPos - note.pos.startPos; i++)
                    std::cout << note.color("^");
                std::cout << " " << note.color(note.subMsg) << "\n";
            }
            else if (note.pos.line > pos.line)
            {
                string line = getLine(pos.file->getOriginalPath(), note.pos.line);
                auto tmp = note.errTy;
                note.errTy = errTy;
                note.printIndent(true);
                note.errTy = tmp;
                std::cout << line << "\n";
                bool isFirst = true;
                for (auto currLine : splitLines(note.subMsg))
                {
                    printIndent();
                    for (int i = 0; i < note.pos.startPos; i++)
                        std::cout << (line[i] == '\t' ? "\t" : " ");
                    
                    if (isFirst)
                    {
                        for (int i = 0; i < note.pos.endPos - note.pos.startPos; i++)
                            std::cout << note.color("^");
                        std::cout << note.color(" ");
                        isFirst = false;
                    }
                    std::cout << note.color(currLine);
                    std::cout << "\n";
                }
            }
        }
    }

    void Error::showBasic()
    {
        if (msg != "")
            std::cout << color(tyToString() + ": ") << BOLD(msg) << "\n";
        if (pos.file == NULL)
            return;
        // uncomment this for uglier, but clickable links
        // std::cout << color(" --> ") << "./" << pos.file->getOriginalPath() << ":" << pos.line << ":" << pos.startPos << "\n";
        for (uint i = 1; i < std::to_string(pos.line).size(); i++) std::cout << " ";
        std::cout << color(" --> ") << pos.file->getOriginalPath() << "\n";

        printIndent();
        std::cout << "\n";
        for (auto note : notes)
        {
            if (note.pos.file == pos.file && note.pos.line < pos.line && note.msg == "")
            {
                string line = getLine(pos.file->getOriginalPath(), note.pos.line);
                auto tmp = note.errTy;
                note.errTy = errTy;
                note.printIndent(true);
                note.errTy = tmp;
                std::cout << line << "\n";
                bool isFirst = true;
                for (auto currLine : splitLines(note.subMsg))
                {
                    printIndent();
                    for (int i = 0; i < note.pos.startPos; i++)
                        std::cout << (line[i] == '\t' ? "\t" : " ");
                    
                    if (isFirst)
                    {
                        for (int i = 0; i < note.pos.endPos - note.pos.startPos; i++)
                            std::cout << note.color("^");
                        std::cout << note.color(" ");
                        isFirst = false;
                    }
                    std::cout << note.color(currLine);
                    std::cout << "\n";
                }
            }
        }

        string line = getLine(pos.file->getOriginalPath(), pos.line);
        printIndent(true);
        std::cout << line << "\n";
        printIndent();
        for (int i = 0; i < pos.startPos; i++)
            std::cout << (line[i] == '\t' ? "\t" : " ");
        for (int i = 0; i < pos.endPos - pos.startPos; i++)
            std::cout << color("^");
        
        if (subMsg != "")
        {
            bool isFirst = true;
            for (auto currLine : splitLines(subMsg))
            {
                if (isFirst)
                {
                    std::cout << " " << color(currLine);
                    isFirst = false;
                }
                else
                {
                    std::cout << "\n";
                    printIndent();
                    for (int i = 0; i < pos.startPos; i++)
                        std::cout << (line[i] == '\t' ? "\t" : " ");
                    std::cout << color(currLine);
                }
                
            }
            

        }
        std::cout << "\n";
        for (auto note : notes)
        {
            if (note.pos.file == pos.file && note.pos.line > pos.line)
            {
                string line = getLine(pos.file->getOriginalPath(), note.pos.line);
                auto tmp = note.errTy;
                note.errTy = errTy;
                note.printIndent(true);
                note.errTy = tmp;
                std::cout << line << "\n";
                bool isFirst = true;
                for (auto currLine : splitLines(note.subMsg))
                {
                    printIndent();
                    for (int i = 0; i < note.pos.startPos; i++)
                        std::cout << (line[i] == '\t' ? "\t" : " ");
                    
                    if (isFirst)
                    {
                        for (int i = 0; i < note.pos.endPos - note.pos.startPos; i++)
                            std::cout << note.color("^");
                        std::cout << note.color(" ");
                        isFirst = false;
                    }
                    std::cout << note.color(currLine);
                    std::cout << "\n";
                }
            }
        }

    }

    bool hasHelpArticle(ErrorCode errTy)
    {
        return true;    // TODO
    }

    void Error::sortSecondaries()
    {
        std::sort(
            std::begin(notes), std::end(notes), 
            [](Error a, Error b) {return a.pos.startPos > b.pos.startPos; }
        );
    }

    vector<Error> errors;

    Error& report(string msg, ErrorCode errCode, Position pos)
    {
        errors.push_back(Error(msg, errCode, pos));
        errors.back().show();
        std::cout << "\n";
        return errors.back();
    }

    void reportAbort() 
    {
        errors.push_back(Error("aborting due to previous error", ERR_NOTE, POS_NONE));
        errors.back().show();
    }

    Error& report(string msg, string subMsg, ErrorCode errCode, Position pos)
    {
        errors.push_back(Error(msg, subMsg, errCode, pos));
        errors.back().show();
        std::cout << "\n";
        return errors.back();
    }

    Error& reportInternal(string msg, ErrorCode errCode, Position pos)
    {
        errors.push_back({ msg, errCode, pos });
        errors.back().show();
        std::cout << "\n";
        return errors.back();
    }

    void showAll() 
    {
        for (uint i = 0; i < errors.size(); i++)
        {
            if (i) std::cout << "\n";
            errors[i].show();
        }
    }

    string Error::tyToString() 
    {
        if (ERR_GENERAL <= errTy && errTy < ERR_WARNING)
            return "error(E" + std::to_string(errTy).substr(1) + ")";
        else if (ERR_WARNING <= errTy && errTy < ERR_NOTE)
            return "warning(W" + std::to_string(errTy).substr(1) + ")";
        else if (ERR_NOTE < errTy && errTy < ERR_UNKNOWN)
            return "note(N" + std::to_string(errTy).substr(1) + ")";
        else if (errTy == ERR_NOTE)
            return "note";
        return "internal error";
    }

    string Error::color(string str)
    {
        switch (errTy)
        {
            default: return BOLD(FRED(str));
            case ERR_WARNING: return BOLD(FYEL(str));
            case ERR_NOTE: return BOLD(FBLK(str));
        }
    }

    void Error::printIndent(bool showLine) {
        if (showLine)
        {
            auto str = " " + std::to_string(pos.line) + " ";
            std::cout << color(str);
        }
        else 
        {
            for (uint i = 0; i < std::to_string(pos.line).size() + 2; i++)
                std::cout << " ";
        }
        std::cout << color("| ");
    }

    string getLine(string fileName, int line)
    {
        std::fstream file(fileName);
        file.seekg(std::ios::beg);
        for (int i=0; i < line - 1; ++i)
            file.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
        string ret;
        std::getline(file, ret);
        return ret;
    }
}