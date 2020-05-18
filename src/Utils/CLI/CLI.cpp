#include "CLI.h"

namespace CLI 
{
    cmdOptions getCmdOptions(int argc, char *argv[])
    {
        cmdOptions options;
        try 
        {
            if (argc <= 1) 
                showHelp();

            if (!strcmp(argv[1], "help")) showHelp(argc, argv);
            else if (!strcmp(argv[1], "version")) showVersion(argc, argv);  
            else if (!strcmp(argv[1], "fmt"))
            {
                if (argc != 3)
                    throw "incorrect usage.\nUsage:\n        dino fmt [file]";
                Prettifier::Prettify(argv[2]);
                exit(0);
            }    
            else if (!strcmp(argv[1], "run"))
            {
                if (argc != 3)
                    throw "incorrect usage.\nUsage:\n        dino run [file]";
                options.fileName = argv[2];
                options.run = true;
            }    
            else if (!strcmp(argv[1], "build"))
            {
                for (int i = 2; i < argc; i++) 
                {
                    if (strlen(argv[i]) && argv[i][0] == '-') 
                    {
                        if      (!strcmp(argv[i], "-v"))		options.verbose 			= true;
                        else if (!strcmp(argv[i], "-lex")) 		options.showLexerOutput 	= true;
                        else if (!strcmp(argv[i], "-ast")) 		options.outputAstFiles 	    = true;
                        else if (!strcmp(argv[i], "-lineAst")) 	options.showLineAST 		= true;
                        else if (!strcmp(argv[i], "-noGC")) 	options.noGC 				= true;
                        else if (!strcmp(argv[i], "-g")) 		options.emitDebugInfo 		= true;
                        else if (!strcmp(argv[i], "-O0") || !strcmp(argv[i], "-O1") || !strcmp(argv[i], "-O2") 
                                || !strcmp(argv[i], "-O3") || !strcmp(argv[i], "-Os") || !strcmp(argv[i], "-Oz") || !strcmp(argv[i], "-Od"))	
                            options.optLevel = argv[i];
                        else if (!strcmp(argv[i], "-bc"))
                        {
                            options.outputBc = true;
                            if (++i >= argc)
                                throw "missing file name after '-bc'";
                            else options.bcFileName = argv[i];
                        }	
                        else if (!strcmp(argv[i], "-o"))
                        {
                            options.outputExe = true;
                            if (++i >= argc)
                                throw "missing file name after '-o'";
                            else options.exeFileName = argv[i];
                        }
                        else if (!strcmp(argv[i], "-lib"))
                        {
                            options.outputLib = true;
                            if (++i >= argc)
                                throw "missing file name after '-lib'";
                            else options.libFileName = argv[i];
                        }
                        else if (!strcmp(argv[i], "-ll"))
                        {
                            options.outputLl = true;
                            if (++i >= argc)
                                throw "missing file name after '-ll'";
                            else options.llFileName = argv[i];
                        }
                        else throw "unknown flag \"" + string(argv[i]) + "\"";
                    }
                    else if (options.fileName)
                        throw "more than one input file specified";
                    else options.fileName = argv[i];
                }
                if (!options.fileName)
                    throw "incorrect usage, missing input file\nTry \"dino help build\" for more info.";
            }
            else throw "unknown command `" + string(argv[1]) + "`.\nTry `dino help` for a list of commands.";
        } 
        catch (ErrorReporter::Error err) { ErrorReporter::showAll(); exit(1); }
        catch (string s)       { llvm::errs() << FRED(BOLD("Error: ")) << s << "\n"; exit(1); }
        catch (const char * c) { llvm::errs() << FRED(BOLD("Error: ")) << c << "\n"; exit(1); }
        return options;
    }

    string runCmd(string cmd, bool printOutput) // if print output is false, nothing will be printed untill the entire command is done
    {
        std::string result = "";
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) throw std::runtime_error("popen() failed in getOutputFromCmd");
        try {
            while (!feof(pipe)) {
                char c;
                if ((c=getc(pipe)) != EOF)
                {
                    result += c;
                    
                    if (printOutput)
                    {
                        std::cout << c;
                        std::cout.flush();
                    }
                }
            }
        } catch (...) {
            pclose(pipe);
            throw;
        }
        pclose(pipe);
        return result;
    }

    void showVersion(int argc, char* argv[])
    {
        if (argc > 2)
        {
            llvm::errs() << "usage: dino version\n";
            exit(0);
        }
        llvm::errs() << DINO_VERSION << "\n";
        exit(0);
    }

    void showHelp(int argc, char* argv[]) 
    {
        if (argc <= 2)
        {
            llvm::errs() 
                << "Usage:\n" 
                << "        dino <command> [arguments]\n\n" 
                << "The commands are:\n\n"
                << "        build      compile and Dino code\n"
                << "        run        compile and run Dino code\n"
                << "        fmt        format source code\n"
                << "        version    print Dino version\n\n"
                << "For more info on a specific command, try `dino help <command>`\n\n";
        }
        else for (int i = 2; i < argc; i++) 
        {
            if (!strcmp(argv[i], "help"))
            {
                llvm::errs() << "Usage: dino help <command>\n"
                             << "Shows help/options for a particular command.\n";
            }
            else if (!strcmp(argv[i], "version"))
            {
                llvm::errs() << "Usage: dino version\n"
                             << "Shows the current version of Dino installed.\n";
            }
            else if (!strcmp(argv[i], "run")) 
            {
                llvm::errs() << "Usage: dino run <file>\n"
                             << "Compiles and runs a Dino program.\n";
            }
            else if (!strcmp(argv[i], "fmt"))
            {
                llvm::errs() << "Usage: dino fmt <file>\n"
                             << "The command `fmt` formats Dino code by swapping all non-unicode operators with their unicode counterparts.\n"
                             << "For examples, '!=' turns into '≠'\n";
            }
            else if (!strcmp(argv[i], "build"))
            {
                llvm::errs()
                    << "Usage: dino build <file> [options]\n"
                    << "Compiles a Dino program.\n"
                    << "Options:"
                    << "    -o [filepath]      output an executable file to filepath\n"
                    << "    -ll [filepath]     output a .ll file to filepath\n"
                    << "    -bc [filepath]     output a .bc file to filepath\n" 
                    << "    -lib [dirpath]     build as a library to dirpath\n"
                    << "    -O[0/1/2/3/s/z/d]  optimization levels, see 'opt -help', the default, '-Od', means no optimizations\n"
                    << "    -g                 generate debug info\n"
                    << "    -v                 verbose: show ongoing compilation status\n"
                    << "    -noGC              turn off garbage collection\n"
                    << "    -lex               prints the output of the lexer\n"
                    << "    -ast               output a .gv file of the AST and DST\n" 
                    << "    -lineAst           show line numbers in the AST and DST files\n";
            }
            else if (isErrorCode(argv[i]))
                showErrorHelp(argv[i]);
            else throw "no help article found for `" + string(argv[i]) + "`";
        }
        exit(0);
    }
}
