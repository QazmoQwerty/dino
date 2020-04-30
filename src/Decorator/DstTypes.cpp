/*
	This file implements the bulk of the Type's (and inheriting classes') member functions.
	For implementations for the rest of DST's classes see DstNode.cpp.
*/
#include "DstNode.h"

#define createPrimitiveTypeSpec(name) (new TypeSpecifierType(new TypeDeclaration(unicode_string(name))))

namespace DST 
{
	PrimitiveTypeSpecifiers _builtinTypes;
	unordered_map<NamespaceDeclaration*, NamespaceType*> _namespaceTypes;

	void setup() {
		_builtinTypes._bool = createPrimitiveTypeSpec("bool");
		_builtinTypes._int = createPrimitiveTypeSpec("int");
		_builtinTypes._string = createPrimitiveTypeSpec("string");
		_builtinTypes._char = createPrimitiveTypeSpec("char");
		_builtinTypes._float = createPrimitiveTypeSpec("float");
		_builtinTypes._void = createPrimitiveTypeSpec("void");
		_builtinTypes._type = createPrimitiveTypeSpec("type");
		_builtinTypes._any = new TypeSpecifierType(new InterfaceDeclaration(new AST::InterfaceDeclaration(unicode_string("any"))));
		_builtinTypes._error = new TypeSpecifierType(new InterfaceDeclaration(new AST::InterfaceDeclaration(unicode_string("error"))));
		_builtinTypes._nullPtError = new TypeSpecifierType(new InterfaceDeclaration(new AST::InterfaceDeclaration(unicode_string("NullPtrError"))));
	}

	BasicType *getIntTy() 	 { return _builtinTypes._int    -> getBasicTy(); }
	BasicType *getBoolTy() 	 { return _builtinTypes._bool   -> getBasicTy(); }
	BasicType *getCharTy() 	 { return _builtinTypes._char   -> getBasicTy(); }
	BasicType *getVoidTy()   { return _builtinTypes._void	-> getBasicTy(); }
	BasicType *getFloatTy()  { return _builtinTypes._float  -> getBasicTy(); }
	BasicType *getTypeidTy() { return _builtinTypes._type   -> getBasicTy(); }
	BasicType *getAnyTy() 	 { return _builtinTypes._any    -> getBasicTy(); }
	BasicType *getStringTy() { return _builtinTypes._string -> getBasicTy(); }
	BasicType *getErrorTy()  { return _builtinTypes._error  -> getBasicTy(); }
	NullType  *getNullTy()   { return NullType::get(); }
	BasicType *getNullPtrErrTy()  { return _builtinTypes._nullPtError -> getBasicTy(); }


	InterfaceDeclaration *getAnyInterface() { return _builtinTypes._any->getInterfaceDecl(); }
	InterfaceDeclaration *getErrorInterface() { return _builtinTypes._error->getInterfaceDecl(); }

	template <class T>
	T* Type::as() { return (T*)getNonConstOf()->getNonPropertyOf(); }

	template<> PropertyType		 *Type::as<PropertyType>()  	{ return (PropertyType*)	 getNonConstOf(); 	 }
	template<> ConstType 		 *Type::as<ConstType>() 		{ return (ConstType*)		 getNonPropertyOf(); }
	template<> TypeList 		 *Type::as<TypeList>() 			{ return (TypeList*)		 getNonConstOf()->getNonPropertyOf(); }
	template<> NamespaceType 	 *Type::as<NamespaceType>() 	{ return (NamespaceType*)	 getNonConstOf()->getNonPropertyOf(); }
	template<> TypeSpecifierType *Type::as<TypeSpecifierType>() { return (TypeSpecifierType*)getNonConstOf()->getNonPropertyOf(); }
	template<> ArrayType 		 *Type::as<ArrayType>() 		{ return (ArrayType*)		 getNonConstOf()->getNonPropertyOf(); }
	template<> BasicType		 *Type::as<BasicType>() 		{ return (BasicType*)		 getNonConstOf()->getNonPropertyOf(); }
	template<> PointerType		 *Type::as<PointerType>() 		{ return (PointerType*)		 getNonConstOf()->getNonPropertyOf(); }

	bool ArrayType::assignableTo(Type* type)
	{
		return type && type->writeable() && type->isArrayTy() && 
			   type->as<ArrayType>()->_length == _length && 
			   _valueType->assignableTo(type->as<ArrayType>()->_valueType);
	}

	NamespaceType *getNamespaceTy(NamespaceDeclaration *decl)
	{
		if (auto ret = _namespaceTypes[decl])
			return ret;
		else return _namespaceTypes[decl] = new NamespaceType(decl);
	}


	UnknownType *UnknownType::get() 
	{ 
		static UnknownType *ret = NULL;
		if (!ret) ret = new UnknownType();
		return ret;
	}

	TypeSpecifierType *TypeSpecifierType::get(InterfaceDeclaration *decl)
	{
		static unordered_map<InterfaceDeclaration*, TypeSpecifierType*> rets;
		if (rets.size() == 0)
		{
			rets[getAnyInterface()] = _builtinTypes._any;
			rets[getErrorInterface()] = _builtinTypes._error;
		}
		if (decl == _builtinTypes._error->getInterfaceDecl())	// TODO - clean this up
			return rets[decl] = _builtinTypes._error;
		if (auto ret = rets[decl])
			return ret;
		return rets[decl] = new TypeSpecifierType(decl);
	}

	TypeSpecifierType *TypeSpecifierType::get(TypeDeclaration *decl)
	{
		static unordered_map<TypeDeclaration*, TypeSpecifierType*> rets;
		if (rets.size() == 0)
		{
			rets[_builtinTypes._bool   -> getTypeDecl()] = _builtinTypes._bool;
			rets[_builtinTypes._char   -> getTypeDecl()] = _builtinTypes._char;
			rets[_builtinTypes._float  -> getTypeDecl()] = _builtinTypes._float;
			rets[_builtinTypes._int    -> getTypeDecl()] = _builtinTypes._int;
			rets[_builtinTypes._string -> getTypeDecl()] = _builtinTypes._string;
			rets[_builtinTypes._type   -> getTypeDecl()] = _builtinTypes._type;
			rets[_builtinTypes._void   -> getTypeDecl()] = _builtinTypes._void;
		}
		if (auto ret = rets[decl])
			return ret;
		return rets[decl] = new TypeSpecifierType(decl);
	}

	TypeList::TypeList(Type *ty, Type *append)  
	{
		if (append->isListTy())
			throw ErrorReporter::reportInternal("cannot append a list type to a list type", ERR_DECORATOR);

		if (ty->isListTy())
		{
			for (auto i : ((TypeList*)ty)->getTypes()) 	
				_types.push_back(i);
			_types.push_back(append);
			if (append->isListTy())
				for (auto i : ((TypeList*)append)->getTypes()) 	
					_types.push_back(i);
			_types.push_back(append);
		}
		else 
		{
			_types.push_back(ty);
			_types.push_back(append);
		}
	}

	TypeList *Type::appendType(Type *ty)
	{
		if (auto ret = _appends[ty])
			return ret;
		else return _appends[ty] = new TypeList(this, ty);
	}

	NamespaceType *NamespaceType::get(NamespaceDeclaration *decl)
	{
		if (auto ret = _namespaceTypes[decl])
			return ret;
		else return _namespaceTypes[decl] = new NamespaceType(decl);
	}

	PropertyType *PropertyType::get(Type *ret, bool hasGet, bool hasSet)
	{
		return ret->getPropertyOf(hasGet, hasSet);
	}

	PointerType *PointerType::get(Type *ty)
	{
		return ty->getPtrTo();
	}

	ConstType *ConstType::get(Type *ty)
	{
		return ty->getConstOf();
	}

	NullType *NullType::get()
	{
		static NullType *ret = NULL;
		if (!ret) ret = new NullType();
		return ret;
	}

	TypeList *TypeList::get(vector<Type*> tys)
	{
		if (tys.size() < 2)
			throw ErrorReporter::reportInternal("type list must have at least 2 types", ERR_DECORATOR);
		auto ret = tys[0]->appendType(tys[1]);
		for (unsigned int i = 2; i < tys.size(); i++)
			ret = ret->appendType(tys[i]);
		return ret;
	}

	BasicType *BasicType::get(TypeSpecifierType *spec) 
	{
		return spec->getBasicTy();
	}

	ArrayType *ArrayType::get(Type *elems, size_t len)
	{
		return elems->getArrayOf(len);
	}

	ArrayType *ArrayType::get(Type *elems, Expression *lenExp)
	{
		return elems->getArrayOf(lenExp);
	}

	FunctionType *FunctionType::get(Type* ret, vector<Type*> params)
	{
		static unordered_map<Type*, map<vector<Type*>, FunctionType*>> rets;
		ret = ret->getConstOf();
		if (rets.count(ret))
		{
			if (rets[ret].count(params))
				return rets[ret][params];
			return rets[ret][params] = new FunctionType(ret, params);
		}
		rets[ret] = {};
		return rets[ret][params] = new FunctionType(ret, params);
	}

	/////////////////////////////////////////////////////////////////////////////////

	TypeSpecifierType::~TypeSpecifierType() { }

	unicode_string TypeSpecifierType::getTypeName()
	{
		if (_typeDecl) return _typeDecl->getName();
		if (_interfaceDecl) return _interfaceDecl->getName();
		return unicode_string();
	}

	Type * TypeSpecifierType::getMemberType(unicode_string name)
	{
		if (_typeDecl) return _typeDecl->getMemberType(name);
		if (_interfaceDecl) return _interfaceDecl->getMemberType(name);
		return NULL;
	}

	ArrayType *Type::getArrayOf(size_t size)
	{
		if (auto ret = _arrayTypes[size])
			return ret;
		return _arrayTypes[size] = new ArrayType(this, size);
	}

	ArrayType *Type::getArrayOf(Expression *exp)
	{
		// TODO - Dumb temporarty hack that will need to be replaced pretty soon
		if (auto ret = _dynArrayTypes[exp])
			return ret;
		return _dynArrayTypes[exp] = new ArrayType(this, exp);
	}

	PropertyType *Type::getPropertyOf(bool hasGet, bool hasSet)
	{
		auto constRet = getConstOf();
		if (hasGet && hasSet)
		{
			if (constRet->_getSetPropTy == NULL)
				constRet->_getSetPropTy = new PropertyType(this, hasGet, hasSet);
			return constRet->_getSetPropTy;
		}
		if (hasGet)
		{
			if (constRet->_getPropTy == NULL)
				constRet->_getPropTy = new PropertyType(this, hasGet, hasSet);
			return constRet->_getPropTy;
		}
		if (hasSet)
		{
			if (constRet->_setPropTy == NULL)
				constRet->_setPropTy = new PropertyType(this, hasGet, hasSet);
			return constRet->_setPropTy;
		}
		throw ErrorReporter::report("a property must have get/set!", ERR_INTERNAL, {0, 0, 0, ""});
	}

	ConstType *Type::getConstOf()
	{
		if (isConstTy())
			return (ConstType*)this;
		if (_constOf == NULL)
			_constOf = new ConstType(this);
		return _constOf;
	}

	PointerType *Type::getPtrTo()
	{
		if (_ptrTo == NULL)
			_ptrTo = new PointerType(this);
		return _ptrTo;
	}

	BasicType *TypeSpecifierType::getBasicTy()
	{
		if (_basicTy == NULL)
			_basicTy = new BasicType(this);
		return _basicTy;
	}

	string BasicType::toShortString() 
	{ 
		// TODO - add scope!
		return getTypeId().to_string(); 
		// else if (_base->getExpressionType() != ET_BINARY_OPERATION || ((AST::BinaryOperation*)_base)->getOperator()._type != OT_PERIOD)
		// 	throw "internal error in BasicType::toShortString()";
		// stack<AST::Identifier*> ids;
		// auto bo = ((AST::BinaryOperation*)_base);
		// while (bo)
		// {
		// 	if (auto id = dynamic_cast<AST::Identifier*>(bo->getRight()))
		// 		ids.push(id);
		// 	else throw "internal error in BasicType::toShortString()";
		// 	if (bo->getLeft()->getExpressionType() == ET_BINARY_OPERATION && ((AST::BinaryOperation*)_base)->getOperator()._type == OT_PERIOD)
		// 		bo = (AST::BinaryOperation*)bo->getLeft();
		// 	else if (auto id = dynamic_cast<AST::Identifier*>(bo->getLeft()))
		// 		ids.push(id);
		// 	else throw "internal error in BasicType::toShortString()";
		// }
		// string ret = ids.top()->getVarId().to_string();
		// ids.pop();
		// while (!ids.empty()) 
		// {
		// 	ret += "." + ids.top()->getVarId().to_string();
		// 	ids.pop();
		// }
		// return ret;
	}

	string PointerType::toShortString()
	{
		return _type->toShortString() + "@";
	}

	Type * Type::getType()
	{
		std::cout << "you probably didn't wanna call this func" << std::endl;
		return getTypeidTy();
	}

	bool BasicType::assignableTo(Type *type)
	{
		if (!type || !type->writeable() || !type->isBasicTy())
			return false;
		auto other = type->as<BasicType>();
		if (!other->_typeSpec) return false;
		if (_typeSpec->getTypeDecl())
			return other->_typeSpec->getTypeDecl() == _typeSpec->getTypeDecl();
		if (_typeSpec->getInterfaceDecl() && other->_typeSpec->getInterfaceDecl()) 
			return _typeSpec->getInterfaceDecl()->implements(other->_typeSpec->getInterfaceDecl());
		return false;
	}

	bool TypeList::assignableTo(Type *type) 
	{
		if (!type || !type->writeable() || !type->isListTy()) 
			return false;
		auto other = type->as<TypeList>();
		if (other->_types.size() != _types.size())
			return false;
		for (unsigned int i = 0; i < _types.size(); i++)
			if (!_types[i]->assignableTo(other->_types[i]))
				return false;
		return true;
	}

	bool NullType::assignableTo(Type *type) 
	{
		return type && type->writeable() && (
			type->isInterfaceTy() || type->isPtrTy() || 
			type->isFuncTy() 	  || type->isArrayTy()
		);
	}

	bool PointerType::assignableTo(Type *type)
	{
		if (!type || !type->writeable())
			return false;
		if (_type->isValueTy() && type->isInterfaceTy())
			return _type->as<BasicType>()->getTypeSpecifier()->getTypeDecl()->implements(type->as<BasicType>()->getTypeSpecifier()->getInterfaceDecl());
		return type->isPtrTy() && _type->assignableTo(type->as<PointerType>()->_type);
	}

	string FunctionType::toShortString()
	{
		string ret = _return->toShortString() + "(";
		bool first = true;
		for (auto i : _parameters)
		{
			if (first)
				first = false;
			else ret += ", ";
			ret += i->toShortString();
		}
		return  ret + ")";
	}

	string TypeList::toShortString()
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

	string PropertyType::toShortString()
	{
		return _return->toShortString() + ((_hasGet && _hasSet) ? "{get|set}" : _hasGet ? "{get}" : "{set}");
	}

	Type *BasicType::getMember(unicode_string id) 
	{ 
		return _typeSpec->getMemberType(id); 
	}

	unicode_string BasicType::getTypeId() { return _typeSpec->getTypeName(); }

	bool Type::comparableTo(Type *other)
	{
		return this->readable() && other->readable() && (
			   this->getNonPropertyOf()->getNonConstOf()->assignableTo(other->getNonPropertyOf()->getNonConstOf()) ||
			   other->getNonPropertyOf()->getNonConstOf()->assignableTo(this->getNonPropertyOf()->getNonConstOf())
		);
	}
}