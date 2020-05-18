#pragma once

#include <stdio.h>
#include <sys/stat.h>
#include <fstream>
#include <iostream> 
#include <sstream> 

#define DINO_VERSION "Dino 0.1.0"

#include "../../Lexer/Lexer.h"
#include "../../Parser/Parser.h"
#include "../../Decorator/Decorator.h"
#include "../../CodeGenerator/CodeGenerator.h"
#include "../AstToFile/AstToFile.h"
#include "../Prettifier/Prettifier.h"
#include "../Unicode/Utf8Handler.h"
#include "../LibFileWriter/LibFileWriter.h"

namespace CLI 
{
    typedef struct cmdOptions 
    {
        const char *fileName = NULL;
        bool showHelp = false;
        bool showVersion = false;
        bool verbose = false;
        bool showLexerOutput = false; 
        bool outputAstFiles = false; 
        bool showLineAST = false;
        bool showIR = false;
        bool outputBc = false;
        const char *bcFileName = NULL;
        bool outputLl = false;
        const char *llFileName = NULL;
        bool outputExe = false;
        const char *exeFileName = NULL;
        bool outputLib = false;
        const char *libFileName = NULL;
        char *optLevel = NULL;
        bool prettify = false;
        bool noGC = false;
        bool run = false;
        bool emitDebugInfo = false;
    } cmdOptions;

    void showHelp(int argc = 0, char* argv[] = NULL);

    bool isErrorCode(char* str);
    void showErrorHelp(char* str);

    void showVersion(int argc = 0, char* argv[] = NULL);

    // if print output is false, nothing will be printed untill the entire command is done
    string runCmd(string cmd, bool printOutput = true); 

    cmdOptions getCmdOptions(int argc, char *argv[]);
}