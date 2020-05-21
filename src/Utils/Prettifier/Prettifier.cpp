#include "Prettifier.h"

namespace Prettifier {
    /*
        Opens the specified file and swaps the following operators with their unicode counterparts:

        old |  new
        ----|------
        *   |   ×
        /   |   ÷
        :=  |   ≡
        !=  |   ≠
        <=  |   ≤
        >=  |   ≥

        The function is 'smart': it won't replace characters inside string/char literals or inside comments.
    */
    void Prettify(string fileName) {
        std::ifstream t;
        t = std::ifstream(fileName);
        std::stringstream buffer;
        buffer << t.rdbuf();
        string str = buffer.str();

        string out = "";
        uint idx = 0;
        while (idx < str.size()) {
            char curr = str[idx++];

            if (idx >= str.size())
            {
                out += curr;
                break;
            }

            switch (curr) {
                default:
                    out += curr; 
                    break;
                case ':': 
                    if      (str[idx] == '=') { idx++; out += u8"≡"; } 
                    else out += curr;
                    break;
                case '<': 
                    if      (str[idx] == '=') { idx++; out += u8"≤"; } 
                    else if (str[idx] == '<') { idx++; out += "<<";  } 
                    else out += curr;
                    break;
                case '>': 
                    if      (str[idx] == '=') { idx++; out += u8"≥"; }
                    else if (str[idx] == '>') { idx++; out += ">>";  }
                    else out += curr;
                    break;
                case '!': 
                    if      (str[idx] == '=') { idx++; out += u8"≠"; } 
                    else out += curr;
                    break;
                case '*':
                    if      (str[idx] == '/') { idx++; out += "*/"; } 
                    else out += u8"×"; 
                    break;
                case '#': // '#' single line comment
                    out += "#";
                    while(idx < str.size() && str[idx] != '\n')
                        out += str[idx++];
                    break;
                case '/': switch(str[idx])
                {
                    default:
                        out += u8"÷"; 
                        break;
                    case '/':   // '//' single line comment
                        idx++;
                        out += "//";
                        while(idx < str.size() && str[idx] != '\n')
                            out += str[idx++];
                        break;
                    case '*':   // multi line comment
                        idx++;
                        out += "/*";
                        while(idx + 1 < str.size() && !(str[idx] == '*' && str[idx+1] == '/'))
                            out += str[idx++];
                        break;
                } break;
                case '"': case '\'':
                {
                    uint escCount = 0;
                    out += curr;

                    while (idx < str.size())
                    {
                        char c = str[idx++];
                        out += c;
                        if (c == curr && escCount%2 == 0)
                            break;
                        if (c == '\\')
                            escCount++;
                        else escCount = 0;
                    }
                    break;
                }                
            }

        }

        std::ofstream outF(fileName);
        outF << out;
        outF.close();
    }

}