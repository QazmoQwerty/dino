#include "Utf8Handler.h"

unicode_char::unicode_char(string str)
{
	if (str.length() > sizeof(_val))
		throw "error in unicode_char(string)";
	_val = unicode_string(str)[0]._val;
}

string unicode_char::to_string() const
{
	string str;
	unsigned char u[5] = { 0, 0, 0, 0, 0 };
	unsigned char* end = utf8::append(_val, u);
	for (auto i = u; i < end; i++) {
		str += *i;
	}
	return str;
}
