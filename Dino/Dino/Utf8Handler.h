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

	inline bool operator==(const string& other) { return to_string() == other; }

	inline bool operator==(const unicode_char& other) { return _val == other._val; }
	inline bool operator!=(const unicode_char& other) { return _val != other._val; }

	string to_string()
	{
		string str;
		unsigned char u[5] = { 0, 0, 0, 0, 0 };
		unsigned char* end = utf8::append(_val, u);
		for (auto i = u; i < end; i++) {
			str += *i;
		}
		return str;
	}
};

class unicode_string {
private:
	vector<unicode_char> _str;
public:
	unicode_string(std::string str) 
	{
		//_str = str;
		string::iterator iter = str.begin();
		while (iter != str.end()) {
			utf8::uint32_t tok = utf8::next(iter, str.end());
			_str.push_back(unicode_char(tok));
		}
	}

	unicode_char& operator[](std::size_t idx) { return _str[idx]; }
	const unicode_char& operator[](std::size_t idx) const { return _str[idx]; }

	string to_string() 
	{
		string str;
		for (unicode_char c : _str)
			str += c.to_string();
		return str;
	}

	unsigned int length() { return _str.size(); }
};