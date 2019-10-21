#pragma once

#include <iostream>
#include <string>
#include <exception>

#include "TypeEnums.h"

using std::string;
using std::exception;

class DinoException : public exception {
	ExceptionType _type;
	int _line;

public:
	DinoException(const char* msg, ExceptionType type, int line) : std::exception(msg), _type(type), _line(line) {}

	string errorMsg() { return "line " + std::to_string(_line) + ": Error " + std::to_string(_type) + ": " + what(); }
	ExceptionType getType() const { return _type; }
	int getLine() const { return _line; }
};