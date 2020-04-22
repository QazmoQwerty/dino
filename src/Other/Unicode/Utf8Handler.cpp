#include "Utf8Handler.h"

/* ---------- unicode_char ---------- */

unicode_char::unicode_char(string str)
{
	unicode_string s = unicode_string(str);
	if (s.length() > sizeof(_val))
		throw "error in unicode_char(string)";
	_val = s[0]._val;
}

string unicode_char::to_string() const
{
	string str;
	unsigned char u[5] = { 0, 0, 0, 0, 0 };
	unsigned char* end = utf8::append(_val, u);
	for (auto i = u; i < end; i++)
		str += *i;
	return str;
}


/* --------- unicode_string --------- */

const string &unicode_string::to_string() const
{
	return _str;
}

void unicode_string::recalculate()
{
	_chars.clear();
	string::iterator iter = _str.begin();
	while (iter != _str.end()) {
		utf8::uint32_t tok = utf8::next(iter, _str.end());
		_chars.push_back(unicode_char(tok));
	}
}

unicode_string & unicode_string::operator=(unicode_char other)
{
	_str = other.to_string();
	_chars.clear();
	return *this;
}

unicode_string & unicode_string::operator+=(unicode_char other)
{
	_str += other.to_string();
	_chars.clear();
	return *this;
}

unicode_string & unicode_string::operator=(string other)
{
	_str = other;
	_chars.clear();
	return *this;
}

bool unicode_string::operator==(const unicode_string & other) const
{
	return other._str == _str;
}

bool unicode_string::operator==(const string other) const 
{
	return _str == other;
}

bool unicode_string::operator!=(const string other) const 
{
	return !operator==(other);
}

unicode_string & unicode_string::operator+=(unicode_string & other)
{
	_str += other.to_string();
	_chars.clear();
	return *this;
}

unicode_string & unicode_string::operator+=(string other) { 
	_str += other;
	_chars.clear();
	return *this;
}

unicode_char& unicode_string::operator[](std::size_t idx) {
	recalculate();
	return _chars[idx];
}

const unicode_char unicode_string::operator[](std::size_t idx) const {
	string::const_iterator iter = _str.begin();
	for (unsigned int i = 0; iter != _str.end(); i++) {
		utf8::uint32_t tok = utf8::next(iter, _str.end());
		if (i == idx) 
			return unicode_char(tok);
	}
	throw NULL;
}

size_t unicode_string::length() {
	recalculate();
	return _chars.size();
}

size_t unicode_string::length() const {
	string::const_iterator iter = _str.begin();
	size_t n = 0;
	for (n = 0; iter != _str.end(); n++)
		utf8::next(iter, _str.end());
	return n;
}