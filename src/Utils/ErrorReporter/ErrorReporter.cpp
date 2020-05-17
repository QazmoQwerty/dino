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
            if (note.pos.file != pos.file|| note.pos.line >= pos.line)
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
            if (note.pos.file == pos.file && note.pos.line == pos.line)
            {
                printIndent();
                for (int i = 0; i < note.pos.startPos; i++)
                    std::cout << (line[i] == '\t' ? "\t" : " ");
                for (int i = 0; i < note.pos.endPos - note.pos.startPos; i++)
                    std::cout << note.color("^");
                std::cout << " " << note.color(note.subMsg) << "\n";
            }
            else note.show();
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
    }

    void Error::sortSecondaries()
    {
        std::sort(
            std::begin(notes), std::end(notes), 
            [](Error a, Error b) {return a.pos.startPos > b.pos.startPos; }
        );
    }

    vector<Error> errors;

    Error& report(string msg, uint errCode, Position pos, bool isFatal)
    {
        errors.push_back(Error(msg, errCode, pos));
        if (isFatal)
            throw "[Aborting due to previous error]";
        return errors.back();
    }

    Error& report(string msg, string subMsg, uint errCode, Position pos, bool isFatal)
    {
        errors.push_back(Error(msg, subMsg, errCode, pos));
        if (isFatal)
            throw "[Aborting due to previous error]";
        return errors.back();
    }

    Error& reportInternal(string msg, uint errCode, Position pos)
    {
        errors.push_back({ msg, errCode, pos });
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
        switch (errTy)
        {
            case INTERNAL: return "Internal Error";
            case WARNING: return "Warning";
            case NOTE: return "Note";
            default: return "Error";
        }
    }

    string Error::color(string str)
    {
        switch (errTy)
        {
            default: return BOLD(FRED(str));
            case WARNING: return BOLD(FBLU(str));
            case NOTE: return BOLD(FBLK(str));
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

    void show(Error &err) 
    {
        // if (err.pos.file != NULL)
        // {   
        //     std::cout << color(" --> ", err) << BOLD("`" + err.pos.file->getOriginalPath() + "`, line " + std::to_string(err.pos.line)) "\n";
        //     printIndent(err);
        //     std::cout << "\n";
        //     string line = getLine(err.pos.file->getOriginalPath(), err.pos.line);
        //     printIndent(err, true);
        //     std::cout << line << "\n";
        //     printIndent(err);
        //     for (int i = 0; i < err.pos.startPos; i++)
        //     {
        //         if (line[i] == '\t')
        //             std::cout << "\t";    
        //         else std::cout << " ";
        //     }
        //     for (int i = 0; i < err.pos.endPos - err.pos.startPos; i++)
        //         std::cout << color("^", err);
        //     std::cout << "\n";
        // }

        // std::cout << color(toString(err) + ": ", err) << err.msg << "\n";        
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