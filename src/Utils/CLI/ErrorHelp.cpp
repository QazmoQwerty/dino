#include "CLI.h"

namespace CLI 
{
    bool isErrorCode(char* str)
    {
        if (strlen(str) != 5)
            return false;
        switch (str[0])
        {
            case 'e': case 'E': case 'w': case 'W': case 'n': case 'N':
                return '0' <= str[1] && str[1] <= '9' &&
                       '0' <= str[2] && str[2] <= '9' &&
                       '0' <= str[3] && str[3] <= '9';
            default: return false;
        }
        
    }

    void showErrorHelp(char* str)
    {
        uint num = (str[1] - '0') * 100 + (str[2] - '0') * 10 + (str[3] - '0');
        switch (str[0])
        {
            case 'e': case 'E': num += ERR_GENERAL;
            case 'w': case 'W': num += ERR_WARNING;
            case 'n': case 'N': num += ERR_NOTE;
            default: throw "no help article found for `" + string(str) + "`";
        }
        switch ((ErrorCode)num)
        {
            // for future support of things like `dino help E207`
            default: throw "no help article found for `" + string(str) + "`";
        }
    }
}