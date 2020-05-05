/*
	This file implements the bulk of the Type's (and inheriting classes') member functions.
	For implementations for the rest of DST's classes see DstNode.cpp.
*/
#include "DstNode.h"

#define createPrimitiveTypeSpec(name) (new ValueType(new TypeDeclaration(unicode_string(name))))

namespace DST 
{
	PrimitiveTypes _builtinTypes;

	void setup() {
		_builtinTypes._i8 = createPrimitiveTypeSpec("i8");
		_builtinTypes._i16 = createPrimitiveTypeSpec("i16");
		_builtinTypes._i32 = createPrimitiveTypeSpec("int");
		_builtinTypes._i64 = createPrimitiveTypeSpec("i64");
		_builtinTypes._i128 = createPrimitiveTypeSpec("i128");

		_builtinTypes._u8 = createPrimitiveTypeSpec("u8");
		_builtinTypes._u16 = createPrimitiveTypeSpec("u16");
		_builtinTypes._u32 = createPrimitiveTypeSpec("uint");
		_builtinTypes._u64 = createPrimitiveTypeSpec("u64");
		_builtinTypes._u128 = createPrimitiveTypeSpec("u128");

		_builtinTypes._c8 = createPrimitiveTypeSpec("char");
		_builtinTypes._c32 = createPrimitiveTypeSpec("uchar");

		_builtinTypes._f16 = createPrimitiveTypeSpec("f16");
		_builtinTypes._f32 = createPrimitiveTypeSpec("float");
		_builtinTypes._f64 = createPrimitiveTypeSpec("f64");
		_builtinTypes._f128 = createPrimitiveTypeSpec("f128");

		_builtinTypes._bool = createPrimitiveTypeSpec("bool");
		_builtinTypes._string = createPrimitiveTypeSpec("string");
		_builtinTypes._void = createPrimitiveTypeSpec("void");
		_builtinTypes._type = createPrimitiveTypeSpec("type");
		_builtinTypes._any = new InterfaceType(new InterfaceDeclaration(new AST::InterfaceDeclaration(unicode_string("any"))));
		_builtinTypes._error = new InterfaceType(new InterfaceDeclaration(new AST::InterfaceDeclaration(unicode_string("error"))));
		_builtinTypes._nullPtrError = createPrimitiveTypeSpec("NullPointerError");
	}

	// int types
	BasicType *geti8Ty()   { return _builtinTypes._i8; };
	BasicType *geti16Ty()  { return _builtinTypes._i16; };
	BasicType *geti32Ty()  { return _builtinTypes._i32; };
	BasicType *geti64Ty()  { return _builtinTypes._i64; };
	BasicType *geti128Ty() { return _builtinTypes._i128; };

	BasicType *getu8Ty()   { return _builtinTypes._u8  ; };
	BasicType *getu16Ty()  { return _builtinTypes._u16 ; };
	BasicType *getu32Ty()  { return _builtinTypes._u32 ; };
	BasicType *getu64Ty()  { return _builtinTypes._u64 ; };
	BasicType *getu128Ty() { return _builtinTypes._u128; };

	BasicType *getf16Ty()  { return _builtinTypes._f16; };
	BasicType *getf32Ty()  { return _builtinTypes._f32; };
	BasicType *getf64Ty()  { return _builtinTypes._f64; };
	BasicType *getf128Ty() { return _builtinTypes._f128; };

	// char types
	BasicType *getc8Ty()	{ return _builtinTypes._c8; };
	BasicType *getc32Ty()	{ return _builtinTypes._c32; };

	// other types/aliases
	BasicType *getIntTy() 	      { return geti32Ty(); }
	BasicType *getUnsignedIntTy() { return getu32Ty(); }
	BasicType *getCharTy() 	      { return getc8Ty(); }
	BasicType *getFloatTy()       { return getf32Ty(); }
	BasicType *getBoolTy() 	      { return _builtinTypes._bool; }
	BasicType *getVoidTy()        { return _builtinTypes._void; }
	BasicType *getTypeidTy()      { return _builtinTypes._type; }
	BasicType *getAnyTy() 	      { return _builtinTypes._any ; }
	BasicType *getStringTy()      { return _builtinTypes._string; }
	BasicType *getErrorTy()       { return _builtinTypes._error; }
	NullType  *getNullTy()        { return NullType::get(); }
	BasicType *getNullPtrErrTy()  { return _builtinTypes._nullPtrError; }


	InterfaceDeclaration *getAnyInterface() { return _builtinTypes._any->getInterfaceDecl(); }
	InterfaceDeclaration *getErrorInterface() { return _builtinTypes._error->getInterfaceDecl(); }

	template <class T>
	T* Type::as() { return (T*)getNonConstOf()->getNonPropertyOf(); }

	template<> PropertyType		 *Type::as<PropertyType>()  	{ return (PropertyType*)	 getNonConstOf(); 	 }
	template<> ConstType 		 *Type::as<ConstType>() 		{ return (ConstType*)		 getNonPropertyOf(); }
	template<> TypeList 		 *Type::as<TypeList>() 			{ return (TypeList*)		 getNonConstOf()->getNonPropertyOf(); }
	template<> NamespaceType 	 *Type::as<NamespaceType>() 	{ return (NamespaceType*)	 getNonConstOf()->getNonPropertyOf(); }
	template<> ArrayType 		 *Type::as<ArrayType>() 		{ return (ArrayType*)		 getNonConstOf()->getNonPropertyOf(); }
	template<> BasicType		 *Type::as<BasicType>() 		{ return (BasicType*)		 getNonConstOf()->getNonPropertyOf(); }
	template<> PointerType		 *Type::as<PointerType>() 		{ return (PointerType*)		 getNonConstOf()->getNonPropertyOf(); }
	template<> ValueType 		 *Type::as<ValueType>() 		{ return (ValueType*)		 getNonConstOf()->getNonPropertyOf(); }
	template<> InterfaceType 	 *Type::as<InterfaceType>() 	{ return (InterfaceType*)	 getNonConstOf()->getNonPropertyOf(); }
	template<> FunctionType 	 *Type::as<FunctionType>() 		{ return (FunctionType*)	 getNonConstOf()->getNonPropertyOf(); }
	template<> EnumType *Type::as<EnumType>() { return (EnumType*)getNonConstOf()->getNonPropertyOf(); }

	bool Type::isFloatTy()
	{
		auto t = getNonPropertyOf()->getNonConstOf();
		return t == getf16Ty() || t == getf32Ty() || t == getf64Ty() || t == getf128Ty();
	}

	bool Type::isBoolTy() 
	{
		return getNonPropertyOf()->getNonConstOf() == getBoolTy();
	}

	bool Type::isCharTy()
	{
		auto t = getNonPropertyOf()->getNonConstOf();
		return t == getc8Ty() || t == getc32Ty();
	}

	bool Type::isIntTy()
	{
		auto t = getNonPropertyOf()->getNonConstOf();
		return t == geti8Ty() || t == geti16Ty() || t == geti32Ty() || t == geti64Ty() || t == geti128Ty() || 
			   t == getu8Ty() || t == getu16Ty() || t == getu32Ty() || t == getu64Ty() || t == getu128Ty();
	}

	bool Type::isEnumerable()
	{
		return isIntTy() || isFloatTy() || isBoolTy() || isCharTy();
	}

	bool Type::isSigned()
	{
		auto t = getNonPropertyOf()->getNonConstOf();
		return t == geti8Ty() || t == geti16Ty() || t == geti32Ty() || t == geti64Ty() || t == geti128Ty();
	}

	bool ArrayType::assignableTo(Type* type)
	{
		return type && type->writeable() && type->isArrayTy() && 
			   type->as<ArrayType>()->_length == _length && 
			   _valueType->assignableTo(type->as<ArrayType>()->_valueType);
	}

	bool FunctionType::assignableTo(DST::Type *type)
	{
		if (!type || !type->writeable() || !type->isFuncTy())
			return false;
		auto other = type->as<DST::FunctionType>();
		if (!_return->assignableTo(other->_return) || other->_parameters.size() != _parameters.size())
			return false;
		for (uint i = 0; i < _parameters.size(); i++)
			if (!_parameters[i]->assignableTo(other->_parameters[i]))
				return false;
		return true;
	}

	bool InterfaceType::implements(InterfaceType* type)
	{
		return this->_decl->implements(type->getInterfaceDecl());
	}

	bool ValueType::implements(InterfaceType* type)
	{
		return this->_decl->implements(type->getInterfaceDecl());
	}

	bool UnsetGenericType::implements(InterfaceType* type)
	{
		if (type->getInterfaceDecl() == getAnyInterface())
			return true;
		for (auto i : _implements)	
			if (i->implements(type->getInterfaceDecl()))
				return true;
		return false;
	}

	bool UnsetGenericType::assignableTo(Type* type)
	{
		return equals(type);
	}


	UnknownType *UnknownType::get() 
	{ 
		static UnknownType *ret = NULL;
		if (!ret) ret = new UnknownType();
		return ret;
	}

	BasicType *BasicType::get(InterfaceDeclaration *decl)
	{
		return InterfaceType::get(decl);
	}

	BasicType *BasicType::get(TypeDeclaration *decl)
	{
		return ValueType::get(decl);
	}

	Type *InterfaceType::getMember(unicode_string name) { return _decl->getMemberType(name); }
	Type *ValueType::getMember(unicode_string name) { return _decl->getMemberType(name); }

	InterfaceType *InterfaceType::get(InterfaceDeclaration *decl)
	{
		static unordered_map<InterfaceDeclaration*, InterfaceType*> rets;
		if (rets.size() == 0)
		{
			rets[getAnyInterface()] = (InterfaceType*)_builtinTypes._any;
			rets[getErrorInterface()] = (InterfaceType*)_builtinTypes._error;
		}
		if (decl == _builtinTypes._error->getInterfaceDecl())	// TODO - clean this up
			return rets[decl] = (InterfaceType*)_builtinTypes._error;
		if (auto ret = rets[decl])
			return ret;
		return rets[decl] = new InterfaceType(decl);
	}

	ValueType *ValueType::get(TypeDeclaration *decl)
	{
		#define ADD_TO_MAP(ty) rets[_builtinTypes.ty   -> getTypeDecl()] = (ValueType*)_builtinTypes.ty;
		static unordered_map<TypeDeclaration*, ValueType*> rets;
		if (rets.size() == 0)
		{
			ADD_TO_MAP(_i8);
			ADD_TO_MAP(_i16);
			ADD_TO_MAP(_i32);
			ADD_TO_MAP(_i64);
			ADD_TO_MAP(_i128);
			ADD_TO_MAP(_f32);
			ADD_TO_MAP(_f64);
			ADD_TO_MAP(_f128);
			ADD_TO_MAP(_c8);
			ADD_TO_MAP(_c32);
			ADD_TO_MAP(_bool);
			ADD_TO_MAP(_string);
			ADD_TO_MAP(_type);
			ADD_TO_MAP(_void);
			ADD_TO_MAP(_nullPtrError);
		}
		if (auto ret = rets[decl])
			return ret;
		return rets[decl] = new ValueType(decl);
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
		static unordered_map<NamespaceDeclaration*, NamespaceType*> _namespaceTypes;
		if (auto ret = _namespaceTypes[decl])
			return ret;
		else return _namespaceTypes[decl] = new NamespaceType(decl);
	}

	EnumType *EnumType::get(EnumDeclaration *decl)
	{
		static unordered_map<EnumDeclaration*, EnumType*> _namespaceTypes;
		if (auto ret = _namespaceTypes[decl])
			return ret;
		else return _namespaceTypes[decl] = new EnumType(decl);
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
		for (uint i = 2; i < tys.size(); i++)
			ret = ret->appendType(tys[i]);
		return ret;
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

	unicode_string InterfaceType::getTypeName()
	{
		return _decl->getName();
	}

	unicode_string ValueType::getTypeName()
	{
		return _decl->getName();
	}

	ArrayType *Type::getArrayOf(size_t size)
	{
		if (auto ret = _arrayTypes[size])
			return ret;
		return _arrayTypes[size] = new ArrayType(this, size);
	}

	ArrayType *Type::getArrayOf(Expression *exp)
	{
		// TODO - Dumb temporary hack that will need to be replaced pretty soon
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
		throw ErrorReporter::report("a property must have get/set!", ERR_INTERNAL, POSITION_INFO_NONE);
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

	string PointerType::toShortString()
	{
		return _type->toShortString() + "@";
	}

	Type * Type::getType()
	{
		UNREACHABLE
	}

	string EnumType::toShortString()
	{
		return _decl->getName().to_string();
	}

	bool InterfaceType::assignableTo(Type *type)
	{
		if (!type || !type->writeable() || !type->isInterfaceTy())
			return false;
		return _decl->implements(type->as<InterfaceType>()->_decl);
	}

	bool ValueType::assignableTo(Type *type)
	{
		if (!type || !type->writeable() || !type->isValueTy())
			return false;
		return this == type->as<ValueType>();
	}

	bool TypeList::assignableTo(Type *type) 
	{
		if (!type || !type->writeable() || !type->isListTy()) 
			return false;
		auto other = type->as<TypeList>();
		if (other->_types.size() != _types.size())
			return false;
		for (uint i = 0; i < _types.size(); i++)
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
		// if (_type->isValueTy() && type->isInterfaceTy())
		// 	return _type->as<ValueType>()->getTypeDecl()->implements(type->as<InterfaceType>()->getInterfaceDecl());
		if (type->isInterfaceTy())
			return _type->implements(type->as<InterfaceType>());
		return type->isPtrTy() && _type->assignableTo(type->as<PointerType>()->_type);
	}

	Type *UnsetGenericType::getMember(unicode_string name)
	{
		for (auto i : _implements)
			if (auto ret = i->getMemberType(name))
				return ret;
		return NULL;
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
		for (uint i = 0; i < _types.size(); i++)
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

	bool Type::comparableTo(Type *other)
	{
		return this->readable() && other->readable() && (
			   this->getNonPropertyOf()->getNonConstOf()->assignableTo(other->getNonPropertyOf()->getNonConstOf()) ||
			   other->getNonPropertyOf()->getNonConstOf()->assignableTo(this->getNonPropertyOf()->getNonConstOf())
		);
	}
}