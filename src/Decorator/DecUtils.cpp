#include "Decorator.h"

DST::Node *Decorator::decorate(AST::Node * node)
{
	if (node == nullptr)
		return NULL;
	if (node->isExpression())
		return decorate(dynamic_cast<AST::Expression*>(node));
	else return decorate(dynamic_cast<AST::Statement*>(node));
}

void Decorator::createErrorInterfaceType()
{

	auto astNamespace = new AST::NamespaceDeclaration();
	astNamespace->setName(unicode_string("."));
	_universalNs = new DST::NamespaceDeclaration(astNamespace);

	auto interfaceDecl1 = new AST::InterfaceDeclaration();
	interfaceDecl1->setName(unicode_string(ERROR_TYPE_NAME));
	auto interfaceDecl2 = new DST::InterfaceDeclaration(interfaceDecl1);

	auto varDecl = new AST::VariableDeclaration();
	varDecl->setVarId(unicode_string("Msg"));
	auto propDecl = new AST::PropertyDeclaration(varDecl);
	auto stringTy = new DST::BasicType(getPrimitiveType("int"));
	auto propTy = new DST::PropertyType(stringTy, true, false);
	auto decPropDecl = new DST::PropertyDeclaration(propDecl, NULL, NULL, propTy);

	interfaceDecl2->addDeclaration(decPropDecl, new DST::PropertyType(stringTy, true, false));

	auto ty = _variables[0][unicode_string(ERROR_TYPE_NAME)] = new DST::TypeSpecifierType(interfaceDecl2);

	_universalNs->addMember(interfaceDecl2->getName(), interfaceDecl2, ty);
	_currentProgram->addNamespace(_universalNs);
}	

DST::TypeSpecifierType *Decorator::getPrimitiveType(std::string name)
{
	auto ret = (DST::TypeSpecifierType*)_variables[0][(unicode_string(name))];
	if (ret == nullptr)
		throw "primitive type " + name + " does not exist!";
	return ret;
}

DST::Type * Decorator::evalType(AST::Expression * node)
{
	auto ret = decorate(node);
	if (ret->getExpressionType() != ET_TYPE)
		throw ErrorReporter::report("expected a type", ERR_DECORATOR, ret->getPosition());
	return (DST::Type*)ret;
}

bool Decorator::isCondition(DST::Expression * node)
{
	return node && node->getType()->getExactType() == EXACT_BASIC
		&& dynamic_cast<DST::BasicType*>(node->getType())->getTypeId() == CONDITION_TYPE;
}

void Decorator::leaveBlock()
{
	for (auto i : _variables[currentScope()])
		_toDelete.push_back(i.second);
	_variables.pop_back();
}

void Decorator::clear()
{
	leaveBlock();
	for (auto i : _toDelete)
		if (i != nullptr)
		{
			try
			{
				delete i;
			}
			catch (...)
			{
				// continue;
			}
		}
}
