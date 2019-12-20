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
	std::string _msg;

public:
	//DinoException(const char* msg, ExceptionType type, int line) : _type(type), _line(line) {}
	DinoException(std::string msg, ExceptionType type, int line) : _msg(msg), _type(type), _line(line) {}

	string errorMsg() { return "line " + std::to_string(_line) + ": Error " + std::to_string(_type) + ": " + _msg.c_str(); }
	ExceptionType getType() const { return _type; }
	int getLine() const { return _line; }
	virtual const char* what() const throw() { return _msg.c_str(); }
};