#pragma once

#include <iostream>
#include <string>
#include <exception>

#include "TypeEnums.h"

using std::string;
using std::exception;

class DinoException : public exception {
	int _line;
	const string _info;

public:
	DinoException(const char* msg, int line, const string info = "") : 
		std::exception(msg), _line(line), _info(info) {}

	int getLine() const { return _line; }
	const string getInfo() const { return _info; }
};