/*
	This file implements the bulk of the DST::Type's (and inheriting classes') member functions.
	For implementations for the rest of DST's classes see DstNode.cpp.
*/
#include "DstNode.h"

DST::TypeSpecifierType::~TypeSpecifierType() { if (_typeDecl) delete _typeDecl; if (_interfaceDecl) delete _interfaceDecl; }

unicode_string DST::TypeSpecifierType::getTypeName()
{
	if (_typeDecl) return _typeDecl->getName();
	if (_interfaceDecl) return _interfaceDecl->getName();
	return unicode_string();
}

DST::Type * DST::TypeSpecifierType::getMemberType(unicode_string name)
{
	if (_typeDecl) return _typeDecl->getMemberType(name);
	if (_interfaceDecl) return _interfaceDecl->getMemberType(name);
	return NULL;
}

bool DST::PointerType::equals(Type * other)
{
	if (other == nullptr) 
		return false;

	switch (other->getExactType())
	{
		case EXACT_NULL:
			return true;

		case EXACT_TYPELIST:
			return equals(((TypeList*)other)->getTypes()[0]);
		
		case EXACT_PROPERTY:
			return equals(((PropertyType*)other)->getReturn());
		
		case EXACT_POINTER:
			if (_type->getExactType() != EXACT_SPECIFIER)
				return ((PointerType*)other)->_type->equals(_type);
			else if (auto inter = ((TypeSpecifierType*)(_type))->getInterfaceDecl())
			{
				auto otype = ((TypeSpecifierType*)((PointerType*)other)->_type)->getTypeDecl();
				auto ointer = ((TypeSpecifierType*)((PointerType*)other)->_type)->getInterfaceDecl();
				if (otype)
					return otype->implements(inter);
				else if (ointer)
					return inter->implements(ointer) || ointer->implements(inter);
				else return false;
			}
		default:
			return false;
	}
}

string DST::BasicType::toShortString() 
{ 
	if (!_base || _base->getExpressionType() == ET_IDENTIFIER)
		return getTypeId().to_string(); 
	else if (_base->getExpressionType() != ET_BINARY_OPERATION || ((AST::BinaryOperation*)_base)->getOperator()._type != OT_PERIOD)
		throw "internal error in DST::BasicType::toShortString()";
	stack<AST::Identifier*> ids;
	auto bo = ((AST::BinaryOperation*)_base);
	while (bo)
	{
		if (auto id = dynamic_cast<AST::Identifier*>(bo->getRight()))
			ids.push(id);
		else throw "internal error in DST::BasicType::toShortString()";
		if (bo->getLeft()->getExpressionType() == ET_BINARY_OPERATION && ((AST::BinaryOperation*)_base)->getOperator()._type == OT_PERIOD)
			bo = (AST::BinaryOperation*)bo->getLeft();
		else if (auto id = dynamic_cast<AST::Identifier*>(bo->getLeft()))
			ids.push(id);
		else throw "internal error in DST::BasicType::toShortString()";
	}
	string ret = ids.top()->getVarId().to_string();
	ids.pop();
	while (!ids.empty()) 
	{
		ret += "." + ids.top()->getVarId().to_string();
		ids.pop();
	}
	return ret;
}

string DST::PointerType::toShortString()
{
	return _type->toShortString() + "@";
}

DST::Type * DST::Type::getType()
{
	std::cout << "you probably didn't wanna call this func" << std::endl;
	return typeidTypePtr;
}

bool DST::BasicType::assignableTo(DST::Type *type)
{
	if (!type)
		return false;
	if (type->getExactType() == EXACT_PROPERTY)
		return ((DST::PropertyType*)type)->writeable() && assignableTo(((DST::PropertyType*)type)->getReturn());
	if (type->getExactType() != EXACT_BASIC) 
		return false;
	auto other = (DST::BasicType*)type;
	if (!other->_typeSpec) return false;
	if (_typeSpec->getTypeDecl())
		return other->_typeSpec->getTypeDecl() == _typeSpec->getTypeDecl();
	if (_typeSpec->getInterfaceDecl() && other->_typeSpec->getInterfaceDecl()) 
		return _typeSpec->getInterfaceDecl()->implements(other->_typeSpec->getInterfaceDecl());
	return false;
}

bool DST::TypeList::assignableTo(DST::Type *type) 
{
	if (!type) 
		return false;
	if (type->getExactType() == EXACT_PROPERTY)
		return ((DST::PropertyType*)type)->writeable() && assignableTo(((DST::PropertyType*)type)->getReturn());
	if (type->getExactType() != EXACT_TYPELIST) 
		return size() == 1 && _types[0]->assignableTo(type);
	auto other = ((DST::TypeList*)type);
	if (other->_types.size() != _types.size())
		return false;
	for (unsigned int i = 0; i < _types.size(); i++)
		if (!_types[i]->assignableTo(other->_types[i]))
			return false;
	return true;
}

bool DST::NullType::assignableTo(DST::Type *type) 
{
	if (!type)
		return false;
	switch (type->getExactType()) 
	{
		case EXACT_PROPERTY:
			return ((DST::PropertyType*)type)->writeable() && assignableTo(((DST::PropertyType*)type)->getReturn());
		case EXACT_BASIC:
			return ((DST::BasicType*)type)->getTypeSpecifier()->getInterfaceDecl();
		case EXACT_POINTER: case EXACT_FUNCTION: case EXACT_ARRAY:
			return true;
		default: return false;
	}
}

bool DST::PointerType::assignableTo(DST::Type *type)
{
	if (!type)
		return false;
	if (type->getExactType() == EXACT_PROPERTY)
		return ((DST::PropertyType*)type)->writeable() && assignableTo(((DST::PropertyType*)type)->getReturn());
	if (type->getExactType() == EXACT_BASIC && _type->getExactType() == EXACT_BASIC && ((DST::BasicType*)_type)->getTypeSpecifier()->getTypeDecl())
		return ((DST::BasicType*)type)->getTypeSpecifier()->getInterfaceDecl() &&
				((DST::BasicType*)_type)->getTypeSpecifier()->getTypeDecl()->implements(((DST::BasicType*)type)->getTypeSpecifier()->getInterfaceDecl());
	return type->getExactType() == EXACT_POINTER && _type->assignableTo(((DST::PointerType*)type)->_type);
}

bool DST::FunctionType::equals(Type * other)
{
	if (other->getExactType() == EXACT_TYPELIST && ((DST::TypeList*)other)->size() == 1)
		return equals(((DST::TypeList*)other)->getTypes()[0]);
	if (other->getExactType() != EXACT_FUNCTION)
		return false;
	auto othr = (FunctionType*)other;
	return _returns->equals(othr->_returns) && _parameters->equals(othr->_parameters);
}

string DST::FunctionType::toShortString()
{
	return _returns->toShortString() + "(" + _parameters->toShortString() + ")";
}



void DST::TypeList::addType(Type * type)
{
	if (type->getExactType() == EXACT_TYPELIST)
		for (auto i : ((TypeList*)type)->getTypes())
			_types.push_back(i);
	else _types.push_back(type);
}

bool DST::TypeList::equals(Type * other)
{
	// if (other->getExactType() == EXACT_BASIC)
	// 	return _types.size() == 1 && _types[0]->equals(other);
	if (_types.size() == 1)
		return _types[0]->equals(other);
	if (other->getExactType() != EXACT_TYPELIST)
		return false;
	auto othr = (TypeList*)other;
	if (_types.size() != othr->_types.size())
		return false;
	for (unsigned int i = 0; i < _types.size(); i++)
		if (!_types[i]->equals(othr->_types[i]))
			return false;
	return true;
}

string DST::TypeList::toShortString()
{
	string str = "(";
	for (unsigned int i = 0; i < _types.size(); i++)
	{
		if (i > 0)
			str += ", ";
		str += _types[i]->toShortString();
	}
	str += ")";
	return str;
}

bool DST::PropertyType::equals(Type * other)
{
	return _return->equals(other);
}

string DST::PropertyType::toShortString()
{
	return _return->toShortString() + ((_hasGet && _hasSet) ? "{get|set}" : _hasGet ? "{get}" : "{set}");
}

DST::Type *DST::BasicType::getMember(unicode_string id) 
{ 
	return _typeSpec->getMemberType(id); 
}

unicode_string DST::BasicType::getTypeId() { return _typeSpec->getTypeName(); }