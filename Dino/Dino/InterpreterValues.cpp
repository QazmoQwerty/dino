#include "InterpreterValues.h"

TypeValue::TypeValue(string typeName, unordered_map<string, TypeDefinition> &types) : Value(typeName), _types(types)
{
	_variables = unordered_map<string, Value*>();
	if (_types.count(typeName) == 0)
		throw "nonexistant type";
	_typeDefinition = _types[typeName];
	for (auto i : _typeDefinition._variables)
	{
		Value* var = nullptr;
		if (i.second.type == "int") var = new IntValue();
		else if (i.second.type == "bool") var = new BoolValue();
		else if (i.second.type == "char") var = new CharValue();
		else if (i.second.type == "frac") var = new FracValue();
		else if (i.second.type == "string") var = new StringValue();
		else if (_types.count(i.second.type)) var = new PtrValue(i.second.type, NULL);
		//var = new TypeValue(i.second.type, _types);
		else throw "nonexistant type";
		var->setNotTemp();
		_variables[i.first] = var;
	}
}

Value * TypeValue::getVariable(string name, string scope)	// no public/private modifiers yet
{
	if (_typeDefinition._functions.count(name))
	{
		if (scope == _typeDefinition._name || hasModifier(_typeDefinition._functions[name].modifiers, "public") ||
			(!hasModifier(_typeDefinition._functions[name].modifiers, "private") && 'A' <= name[0] && name[0] <= 'Z'))
			return _typeDefinition._functions[name].value;	// if modifiers has public (or doesn't have private and name is uppercase)
		else throw "function is private";
	}
	if (_typeDefinition._properties.count(name))
	{
		if (scope == _typeDefinition._name || hasModifier(_typeDefinition._properties[name].modifiers, "public") ||
			(!hasModifier(_typeDefinition._properties[name].modifiers, "private") && 'A' <= name[0] && name[0] <= 'Z'))
			return _typeDefinition._properties[name].value;	// if modifiers has public (or doesn't have private and name is uppercase)
		else throw "property is private";
	}
	if (_variables.count(name))
	{
		if (scope == _typeDefinition._name || hasModifier(_typeDefinition._variables[name].modifiers, "public") ||
			(!hasModifier(_typeDefinition._variables[name].modifiers, "private") && 'A' <= name[0] && name[0] <= 'Z'))
			return _variables[name];	// if modifiers has public (or doesn't have private and name is uppercase)
		else throw "variable is private";
	}
	throw "variable does not exist";
}

bool hasModifier(vector<string> &modifiers, string modifier)
{
	return std::find(modifiers.begin(), modifiers.end(), modifier) != modifiers.end();
}

bool TypeValue::hasVariable(string name)
{
	return _typeDefinition._functions.count(name) || _variables.count(name);
}

void FuncValue::setValue(AST::Function * value)
{
	if (_returnType != "" && value->getReturnType().name != _returnType)
		throw "Error: Function pointer to different value";	// TODO - fix error msg
	_value = value; _returnType = value->getReturnType().name;
}
