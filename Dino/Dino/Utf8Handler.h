#pragma once
#include <string>
#include <vector>
//#define U_CHARSET_IS_UTF8 1
//
//#include <unicode/ustring.h>
//#include <unicode/stringpiece.h>
//#include <unicode/utypes.h>
//#include <unicode/uchar.h>

#include <fstream>
#include "utf8.h"

using std::string;
using std::vector;

class unicode_char {
private:
	utf8::uint32_t _val;
	
public:
	unicode_char(utf8::uint32_t val) { _val = val; }
	unicode_char(string str);

	inline bool operator==(const string& other) const { return to_string() == other; }
	inline bool operator==(const char& other) const { return _val == other; }

	inline bool operator==(const unicode_char& other) const { return _val == other._val; }
	inline bool operator!=(const unicode_char& other) const { return _val != other._val; }

	utf8::uint32_t getValue() const { return _val; }

	string to_string() const;
};

class unicode_string {
private:
	vector<unicode_char> _str;
public:
	unicode_string() { }

	void addString(string str)
	{
		string::iterator iter = str.begin();
		while (iter != str.end()) {
			utf8::uint32_t tok = utf8::next(iter, str.end());
			_str.push_back(unicode_char(tok));
		}
	}

	unicode_string(std::string str) { addString(str); }

	unicode_char& operator[](std::size_t idx) { return _str[idx]; }
	const unicode_char& operator[](std::size_t idx) const { return _str[idx]; }

	unicode_string& operator=(unicode_char other) {
		_str.clear();
		_str.push_back(other);
		return *this;
	}

	unicode_string& operator+=(unicode_char other) {
		_str.push_back(other);
		return *this;
	}

	unicode_string& operator=(string other) {
		_str.clear();
		addString(other);
		return *this;
	}

	bool operator==(const unicode_string& other) const
	{
		if (other.length() != length())
			return false;
		for (unsigned int i = 0; i < length(); i++)
			if (other[i] != _str[i])
				return false;
		return true;
	}

	bool operator==(const string other) const { return unicode_string(other) == *this; }

	unicode_string& operator+=(unicode_string& other) {
		for (auto i : other._str)
			_str.push_back(i);
		return *this;
	}

	unicode_string& operator+=(string other) {
		addString(other);
		return *this;
	}

	string to_string() const
	{
		string str;
		for (unicode_char c : _str)
			str += c.to_string();
		return str;
	}

	unsigned int length() const { return _str.size(); }
};

class UnicodeHasherFunction {
public:
	std::size_t operator()(const unicode_char& c) const {
		return std::hash<int>()(c.getValue());
	}

	std::size_t operator()(const unicode_string& c) const {
		return std::hash<string>()(c.to_string());
	}
};