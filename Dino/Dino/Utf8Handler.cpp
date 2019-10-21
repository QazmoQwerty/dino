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

string unicode_string::to_string() const
{
	string str;
	for (unicode_char c : _str)
		str += c.to_string();
	return str;
}

void unicode_string::addString(string str)
{
	string::iterator iter = str.begin();
	while (iter != str.end()) {
		utf8::uint32_t tok = utf8::next(iter, str.end());
		_str.push_back(unicode_char(tok));
	}
}

unicode_string & unicode_string::operator=(unicode_char other)
{
	_str.clear();
	_str.push_back(other);
	return *this;
}

unicode_string & unicode_string::operator+=(unicode_char other)
{
	_str.push_back(other);
	return *this;
}

unicode_string & unicode_string::operator=(string other)
{
	_str.clear();
	addString(other);
	return *this;
}

bool unicode_string::operator==(const unicode_string & other) const
{
	if (other.length() != length())
		return false;
	for (unsigned int i = 0; i < length(); i++)
		if (other[i] != _str[i])
			return false;
	return true;
}

bool unicode_string::operator==(const string other) const 
{
	return unicode_string(other) == *this;
}

unicode_string & unicode_string::operator+=(unicode_string & other)
{
	for (auto i : other._str)
		_str.push_back(i);
	return *this;
}

unicode_string & unicode_string::operator+=(string other) { 
	addString(other);
	return *this;
}
