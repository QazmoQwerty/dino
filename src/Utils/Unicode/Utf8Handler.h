#pragma once
#include <string>
#include <vector>

#include <fstream>
#include "utf8.h"
// #include "../ErrorReporter/ErrorReporter.h"

using std::string;
using std::vector;

/*
	A single UTF8 unicode character.
	Character is saved as an uint32.
*/
class unicode_char {
public:
	/* Returns unicode code point value. */
	utf8::uint32_t getValue() const { return _val; }

	/* Default constructor */
	unicode_char() : _val(0) { }

	/* Simple constructor which gets a unicode code point. */
	unicode_char(utf8::uint32_t val) { _val = val; }

	/*
		Gets a single unicode character and sets value to that character.
		Constructor will throw an exception if str is more than one unicode characters long.
	*/
	unicode_char(string str);

	/* Returns unicode letter in std::string form. */
	string to_string() const;

	/* 
		Returns whether character is a utf8 uppercase character.
		FIXME: currently only checks for values from 'A'-'Z', needs to be updated.
	*/
	bool isUpper() { return 'A' <= _val && _val <= 'Z'; }

	/* ----------  Operators ---------- */

	inline bool operator==(const string& other) const { return to_string() == other; }
	inline bool operator!=(const string& other) const { return to_string() != other; }

	inline bool operator==(const utf8::uint32_t& other) const { return _val == other; }
	inline bool operator!=(const utf8::uint32_t& other) const { return _val != other; }

	inline bool operator==(const unicode_char& other) const { return _val == other._val; }
	inline bool operator!=(const unicode_char& other) const { return _val != other._val; }

private:
	utf8::uint32_t _val;
};

/*
	A UTF8 unicode string.
*/
class unicode_string {
public:
	/* Creates an empty unicode string */
	unicode_string() { }

	/* Gets a UTF8 encoded string and creates a unicode string from it */
	unicode_string(std::string str) : _str(str) { }

	/* Returns unicode string as std::string */
	const string& to_string() const;

	/* 
		Number of unicode characters in string.
		NOTE: does not include null-terminator.
	*/
	size_t length();

	/* 
		Number of unicode characters in string.
		NOTE: does not include null-terminator.
	*/
	size_t length() const;

	/* ----------  Operators ---------- */

		  unicode_char& operator[](std::size_t idx);
	const unicode_char operator[](std::size_t idx) const;

	unicode_string& operator=(unicode_char other);
	unicode_string& operator+=(unicode_char other);

	unicode_string& operator+=(unicode_string& other);
	bool operator==(const unicode_string& other) const;

	unicode_string& operator=(string other);
	unicode_string& operator+=(string other);
	bool operator==(const string other) const;
	bool operator!=(const string other) const;

private:
	string _str;
	vector<unicode_char> _chars;
	void recalculate();
};

/*
	Class to hash unicode_strings and unicode_chars (needed for std::unordered_map).
*/
class UnicodeHasherFunction {
public:
	std::size_t operator()(const unicode_char& c) const { return std::hash<int>()(c.getValue()); }
	std::size_t operator()(const unicode_string& c) const { return std::hash<string>()(c.to_string()); }
};