#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>

using std::map;
using std::string;
using std::exception;


namespace Prettifier 
{
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
        TODO - currently only treats '//' single line comments, need to add treatment for '#' single line comments.
    */
    void Prettify(string fileName);
}