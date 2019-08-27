#include "scripthandle.h"
#include <new>
#include <assert.h>
#include <string.h>

BEGIN_AS_NAMESPACE

// We'll use the generic interface for the factories as we need the engine pointer
static void ScriptHandleFactory(asIScriptGeneric *gen)
{
	asIScriptEngine *engine = gen->GetEngine();
	*(ScriptHandle**)gen->GetAddressOfReturnLocation() = new ScriptHandle(engine);
}

static void ScriptHandleFactory2(asIScriptGeneric *gen)
{
	asIScriptEngine *engine = gen->GetEngine();
	void *ref = (void*)gen->GetArgAddress(0);
	int refType = gen->GetArgTypeId(0);

	*(ScriptHandle**)gen->GetAddressOfReturnLocation() = new ScriptHandle(ref,refType,engine);
}

static ScriptHandle &ScriptHandleAssignment(ScriptHandle *other, ScriptHandle *self)
{
	return *self = *other;
}


static ScriptHandle* CreateCScriptHandle() 						{ return new ScriptHandle(ASEngine); }
static ScriptHandle* CopyScriptHandle(const ScriptHandle &other)	
{
	ScriptHandle* handle = CreateCScriptHandle();
	handle->operator=(other);
	return handle; 
}

static ScriptHandle* FormatCScriptHandle(void *ref, int typeId)	
{
	return new ScriptHandle(ref, typeId, ASEngine);
}

static ScriptHandle* CreateScriptHandle( ScriptString* typeName, ScriptString* moduleName )
{	
	int typeId = 0;
	if( moduleName )
	{
		asIScriptModule *module = ASEngine->GetModule( moduleName->c_str() );
		if( module )
			typeId = module->GetTypeIdByDecl( typeName->c_str() );
	}
	else
		typeId = ASEngine->GetTypeIdByDecl( typeName->c_str() );
	switch( typeId )
	{
		case asTYPEID_BOOL           : // 1,
		{
			bool value = false;
			return FormatCScriptHandle( &value, typeId );
		}
		case asTYPEID_INT8           : // 2,
		{
			int8 value = 0;
			return FormatCScriptHandle( &value, typeId );
		}
		case asTYPEID_INT16          : // 3,
		{
			int16 value = 0;
			return FormatCScriptHandle( &value, typeId );
		}
		case asTYPEID_INT32          : // 4,
		{
			int value = 0;
			return FormatCScriptHandle( &value, typeId );
		}
		case asTYPEID_INT64          : // 5,
		{
			int64 value = 0;
			return FormatCScriptHandle( &value, typeId );
		}
		case asTYPEID_UINT8          : // 6,
		{
			uint8 value = 0;
			return FormatCScriptHandle( &value, typeId );
		}
		case asTYPEID_UINT16         : // 7,
		{
			uint16 value = 0;
			return FormatCScriptHandle( &value, typeId );
		}
		case asTYPEID_UINT32         : // 8,
		{
			uint value = 0;
			return FormatCScriptHandle( &value, typeId );
		}
		case asTYPEID_UINT64         : // 9,
		{
			uint64 value = 0;
			return FormatCScriptHandle( &value, typeId );
		}
		case asTYPEID_FLOAT          : // 10,
		{
			float value = 0.0f;
			return FormatCScriptHandle( &value, typeId );
		}
		case asTYPEID_DOUBLE         : // 11,
		{
			double value = 0.0;
			return FormatCScriptHandle( &value, typeId );
		}
		default:
		{
			asIObjectType *type = ASEngine->GetObjectTypeById( typeId );
			if( type )
			{				
				asIScriptObject *obj = reinterpret_cast<asIScriptObject*>(ASEngine->CreateScriptObject(typeId));
				if( obj )
				{
					ScriptHandle* ref = FormatCScriptHandle( obj, typeId );
					obj->Release();
					return ref;
				}
			}
		}
		break;
	}
	return nullptr;
}

void RegisterScriptHandle(asIScriptEngine *engine)
{
	int r;
	r = engine->RegisterObjectType("Handle", sizeof(ScriptHandle), asOBJ_REF | asOBJ_GC); assert( r >= 0 );

	// We'll use the generic interface for the constructor as we need the engine pointer
	r = engine->RegisterObjectBehaviour("Handle", asBEHAVE_FACTORY, "Handle@ f()", asFUNCTION(ScriptHandleFactory), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("Handle", asBEHAVE_FACTORY, "Handle@ f(?&in)", asFUNCTION(ScriptHandleFactory2), asCALL_GENERIC); assert( r >= 0 );

	r = engine->RegisterObjectBehaviour("Handle", asBEHAVE_ADDREF, "void f()", asMETHOD(ScriptHandle,AddRef), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("Handle", asBEHAVE_RELEASE, "void f()", asMETHOD(ScriptHandle,Release), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("Handle", "Handle &opAssign(Handle&in)", asFUNCTION(ScriptHandleAssignment), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("Handle", "void store(?&in)", asMETHODPR(ScriptHandle,Store,(void*,int),void), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("Handle", "void store(int64&in)", asMETHODPR(ScriptHandle,Store,(asINT64&),void), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("Handle", "void store(double&in)", asMETHODPR(ScriptHandle,Store,(double&),void), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("Handle", "bool retrieve(?&out)", asMETHODPR(ScriptHandle,Retrieve,(void*,int) const,bool), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("Handle", "bool retrieve(int64&out)", asMETHODPR(ScriptHandle,Retrieve,(asINT64&) const,bool), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("Handle", "bool retrieve(double&out)", asMETHODPR(ScriptHandle,Retrieve,(double&) const,bool), asCALL_THISCALL); assert( r >= 0 );

	// Register GC behaviours
	r = engine->RegisterObjectBehaviour("Handle", asBEHAVE_GETREFCOUNT, "int f()", asMETHOD(ScriptHandle,GetRefCount), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("Handle", asBEHAVE_SETGCFLAG, "void f()", asMETHOD(ScriptHandle,SetFlag), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("Handle", asBEHAVE_GETGCFLAG, "bool f()", asMETHOD(ScriptHandle,GetFlag), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("Handle", asBEHAVE_ENUMREFS, "void f(int&in)", asMETHOD(ScriptHandle,EnumReferences), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("Handle", asBEHAVE_RELEASEREFS, "void f(int&in)", asMETHOD(ScriptHandle,ReleaseAllHandles), asCALL_THISCALL); assert( r >= 0 );
	
	r = engine->RegisterObjectBehaviour( "Handle", asBEHAVE_REF_CAST, "void f(?&out)", asMETHODPR(ScriptHandle, Cast, (void *, int), void), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod( "Handle", "string@ get_TypeName( ) const", asMETHOD(ScriptHandle, script_TypeName), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod( "Handle", "string@ get_TypeNamespace( ) const", asMETHOD(ScriptHandle, script_TypeNamespace), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod( "Handle", "Handle@+ get_Property( string@ name ) const", asMETHOD(ScriptHandle, script_Property), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod( "Handle", "bool get_IsPrimitive( ) const", asMETHOD(ScriptHandle, script_Primitive), asCALL_THISCALL); assert( r >= 0 );
	
	r = engine->RegisterObjectMethod( "Handle", "void set_custom( const string&in name, Handle@ value )", asMETHOD(ScriptHandle,SetCustom ), asCALL_THISCALL ); assert( r >= 0 );
	r = engine->RegisterObjectMethod( "Handle", "Handle@ get_custom( const string&in name )", asMETHOD(ScriptHandle,GetCustom ), asCALL_THISCALL ); assert( r >= 0 );
	
    engine->SetDefaultNamespace( "Handle" );
	r = engine->RegisterGlobalFunction( "::Handle@ Create( )", asFUNCTION( CreateCScriptHandle ), asCALL_CDECL ); assert( r >= 0 );
	r = engine->RegisterGlobalFunction( "::Handle@ Create( const ::Handle&in )", asFUNCTION( CopyScriptHandle ), asCALL_CDECL ); assert( r >= 0 );
	r = engine->RegisterGlobalFunction( "::Handle@ Create( const ?&in )", asFUNCTION( FormatCScriptHandle ), asCALL_CDECL ); assert( r >= 0 );
	r = engine->RegisterGlobalFunction( "::Handle@ Create( ::string&in type, ::string@ module = null )", asFUNCTION( CreateScriptHandle ), asCALL_CDECL ); assert( r >= 0 );
    engine->SetDefaultNamespace( "" );
}

ScriptHandle &ScriptHandle::operator=(const ScriptHandle &other)
{
	// Hold on to the object type reference so it isn't destroyed too early
	if( other.value.valueObj && (other.value.typeId & asTYPEID_MASK_OBJECT) )
	{
		asIObjectType *ot = engine->GetObjectTypeById(other.value.typeId);
		if( ot )
			ot->AddRef();
	}

	FreeObject();

	value.typeId = other.value.typeId;
	if( value.typeId & asTYPEID_OBJHANDLE )
	{
		// For handles, copy the pointer and increment the reference count
		value.valueObj = other.value.valueObj;
		engine->AddRefScriptObject(value.valueObj, value.typeId);
	}
	else if( value.typeId & asTYPEID_MASK_OBJECT )
	{
		// Create a copy of the object
		value.valueObj = engine->CreateScriptObjectCopy(other.value.valueObj, value.typeId);
	}
	else
	{
		// Primitives can be copied directly
		value.valueInt = other.value.valueInt;
	}

	return *this;
}

int ScriptHandle::CopyFrom(const ScriptHandle *other)
{
	if( other == 0 ) return asINVALID_ARG;

	*this = *(ScriptHandle*)other;

	return 0;
}

ScriptHandle::ScriptHandle(asIScriptEngine *engine)
{
	this->engine = engine;
	refCount = 1;

	value.typeId = 0;
	value.valueInt = 0;

	// Notify the garbage collector of this object
	engine->NotifyGarbageCollectorOfNewObject(this, engine->GetObjectTypeByName("Handle"));		
}

ScriptHandle::ScriptHandle(void *ref, int refTypeId, asIScriptEngine *engine)
{
	this->engine = engine;
	refCount = 1;

	value.typeId = 0;
	value.valueInt = 0;

	// Notify the garbage collector of this object
	engine->NotifyGarbageCollectorOfNewObject(this, engine->GetObjectTypeByName("Handle"));		

	Store(ref, refTypeId);
}

ScriptHandle::~ScriptHandle()
{
	FreeObject();
	
	for( auto it = custom.begin( ), end = custom.end( ); it != end; it++ )
		it->second->Release();
	custom.clear();
}

void ScriptHandle::Store(void *ref, int refTypeId)
{
	// Hold on to the object type reference so it isn't destroyed too early
	if( *(void**)ref && (refTypeId & asTYPEID_MASK_OBJECT) )
	{
		asIObjectType *ot = engine->GetObjectTypeById(refTypeId);
		if( ot )
			ot->AddRef();
	}

	FreeObject();

	value.typeId = refTypeId;
	if( value.typeId & asTYPEID_OBJHANDLE )
	{
		// We're receiving a reference to the handle, so we need to dereference it
		value.valueObj = *(void**)ref;
		engine->AddRefScriptObject(value.valueObj, value.typeId);
	}
	else if( value.typeId & asTYPEID_MASK_OBJECT )
	{
		// Create a copy of the object
		value.valueObj = engine->CreateScriptObjectCopy(ref, value.typeId);
	}
	else
	{
		// Primitives can be copied directly
		value.valueInt = 0;

		// Copy the primitive value
		// We receive a pointer to the value.
		int size = engine->GetSizeOfPrimitiveType(value.typeId);
		memcpy(&value.valueInt, ref, size);
	}
}

void ScriptHandle::Store(double &ref)
{
	Store(&ref, asTYPEID_DOUBLE);
}

void ScriptHandle::Store(asINT64 &ref)
{
	Store(&ref, asTYPEID_INT64);
}


bool ScriptHandle::Retrieve(void *ref, int refTypeId) const
{
	if( refTypeId & asTYPEID_OBJHANDLE )
	{
		// Is the handle type compatible with the stored value?

		// A handle can be retrieved if the stored type is a handle of same or compatible type
		// or if the stored type is an object that implements the interface that the handle refer to.
		if( (value.typeId & asTYPEID_MASK_OBJECT) && 
			engine->IsHandleCompatibleWithObject(value.valueObj, value.typeId, refTypeId) )
		{
			engine->AddRefScriptObject(value.valueObj, value.typeId);
			*(void**)ref = value.valueObj;

			return true;
		}
	}
	else if( refTypeId & asTYPEID_MASK_OBJECT )
	{
		// Is the object type compatible with the stored value?

		// Copy the object into the given reference
		if( value.typeId == refTypeId )
		{
			engine->CopyScriptObject(ref, value.valueObj, value.typeId);

			return true;
		}
	}
	else
	{
		// Is the primitive type compatible with the stored value?

		if( value.typeId == refTypeId )
		{
			int size = engine->GetSizeOfPrimitiveType(refTypeId);
			memcpy(ref, &value.valueInt, size);
			return true;
		}

		// We know all numbers are stored as either int64 or double, since we register overloaded functions for those
		if( value.typeId == asTYPEID_INT64 && refTypeId == asTYPEID_DOUBLE )
		{
			*(double*)ref = double(value.valueInt);
			return true;
		}
		else if( value.typeId == asTYPEID_DOUBLE && refTypeId == asTYPEID_INT64 )
		{
			*(asINT64*)ref = asINT64(value.valueFlt);
			return true;
		}
	}

	return false;
}

bool ScriptHandle::Retrieve(asINT64 &value) const
{
	return Retrieve(&value, asTYPEID_INT64);
}

bool ScriptHandle::Retrieve(double &value) const
{
	return Retrieve(&value, asTYPEID_DOUBLE);
}

int ScriptHandle::GetTypeId() const
{
	return value.typeId;
}

void ScriptHandle::FreeObject()
{
    // If it is a handle or a ref counted object, call release
	if( value.typeId & asTYPEID_MASK_OBJECT )
	{
		// Let the engine release the object
		engine->ReleaseScriptObject(value.valueObj, value.typeId);

		// Release the object type info
		asIObjectType *ot = engine->GetObjectTypeById(value.typeId);
		if( ot )
			ot->Release();

		value.valueObj = 0;
		value.typeId = 0;
	}

    // For primitives, there's nothing to do
}


void ScriptHandle::EnumReferences(asIScriptEngine *engine)
{
	// If we're holding a reference, we'll notify the garbage collector of it
	if( value.valueObj && (value.typeId & asTYPEID_MASK_OBJECT) )
	{
		engine->GCEnumCallback(value.valueObj);

		// The object type itself is also garbage collected
		asIObjectType *ot = engine->GetObjectTypeById(value.typeId);
		if( ot )
			engine->GCEnumCallback(ot);
	}
}

void ScriptHandle::ReleaseAllHandles(asIScriptEngine * /*engine*/)
{
	FreeObject();
}

int ScriptHandle::AddRef() const
{
	// Increase counter and clear flag set by GC
	refCount = (refCount & 0x7FFFFFFF) + 1;
	return refCount;
}

int ScriptHandle::Release() const
{
	// Now do the actual releasing (clearing the flag set by GC)
	refCount = (refCount & 0x7FFFFFFF) - 1;
	if( refCount == 0 )
	{
		delete this;
		return 0;
	}

	return refCount;
}

int ScriptHandle::GetRefCount()
{
	return refCount & 0x7FFFFFFF;
}

void ScriptHandle::SetFlag()
{
	refCount |= 0x80000000;
}

bool ScriptHandle::GetFlag()
{
	return (refCount & 0x80000000) ? true : false;
}

const char* ScriptHandle::TypeName()
{
	if( value.valueObj )
	{
		if( GetType() )
			return GetType()->GetName( );
		else if( value.typeId )
		{
			switch( value.typeId )
			{
				case asTYPEID_VOID           : // 0,
					break;
				case asTYPEID_BOOL           : // 1,
					return "bool";
				case asTYPEID_INT8           : // 2,
					return "int8";
				case asTYPEID_INT16          : // 3,
					return "int16";
				case asTYPEID_INT32          : // 4,
					return "int";
				case asTYPEID_INT64          : // 5,
					return "int64";
				case asTYPEID_UINT8          : // 6,
					return "uint8";
				case asTYPEID_UINT16         : // 7,
					return "uint16";
				case asTYPEID_UINT32         : // 8,
					return "uint";
				case asTYPEID_UINT64         : // 9,
					return "uint64";
				case asTYPEID_FLOAT          : // 10,
					return "float";
				case asTYPEID_DOUBLE         : // 11,
					return "double";
				default: break;
			}
		}
	}
	return "null";
}

ScriptString* ScriptHandle::script_TypeName()
{
	return &ScriptString::Create( TypeName() );
}

ScriptString* ScriptHandle::script_TypeNamespace()
{
	if( value.valueObj && GetType() )
		return &ScriptString::Create( GetType()->GetNamespace( ) );
	else return &ScriptString::Create( "" );
}

ScriptHandle* ScriptHandle::script_Property( ScriptString* propName )
{
	if( propName )
		return Property( propName->c_str() );
	return nullptr;
}

ScriptHandle* ScriptHandle::Property( const char* propName )
{
	if( value.valueObj && propName )
	{
		asIScriptObject *object = reinterpret_cast<asIScriptObject*>(value.valueObj);
		if( object )
			for( uint i = 0, iEnd = object->GetPropertyCount(); i < iEnd; i++ )
				if( strcmp( object->GetPropertyName(i), propName ) == 0 )
					return FormatCScriptHandle( object->GetAddressOfProperty(i), object->GetPropertyTypeId(i) );
	}
	return nullptr;	
}

asIObjectType *ScriptHandle::GetType()
{
	return engine->GetObjectTypeById(value.typeId);
}

bool ScriptHandle::script_Primitive( )
{
	return ( value.typeId > 0 && !(value.typeId & asTYPEID_MASK_OBJECT) );
}

void ScriptHandle::Cast(void *outRef, int typeId)
{
	Retrieve( outRef, typeId );
}

void ScriptHandle::SetCustom( const ScriptString& name, ScriptHandle* value )
{
	ScriptHandle* old = GetCustom( name );
	if( old )
		old->Release();
	custom[name.c_std_str()] = value;
}

ScriptHandle* ScriptHandle::GetCustom( const ScriptString& name )
{
	auto it = custom.find( name.c_std_str() );
	if( it == custom.end( ) )
		return nullptr;
	return it->second;
}

END_AS_NAMESPACE