#ifndef SCRIPTANY_H
#define SCRIPTANY_H

#include "angelscript.h"

class ScriptAny
{
public:
    #ifdef FONLINE_DLL
    static ScriptAny& Create()
    {
        static int typeId = ASEngine->GetTypeIdByDecl( "any" );
        ScriptAny* scriptAny = (ScriptAny*) ASEngine->CreateScriptObject( typeId );
        return *scriptAny;
    }
protected:
    #endif

    // Constructors
    ScriptAny();
    ScriptAny( const ScriptAny& );
    ScriptAny( asIScriptEngine* engine );
    ScriptAny( void* ref, int refTypeId, asIScriptEngine* engine );

public:
    // Memory management
    virtual void AddRef() const;
    virtual void Release() const;

    // Copy the stored value from another any object
    ScriptAny& operator=( const ScriptAny& other )
    {
        Assign( other );
        return *this;
    }
    virtual void Assign( const ScriptAny& other );
    virtual int  CopyFrom( const ScriptAny* other );

    // Store the value, either as variable type, integer number, or real number
    virtual void Store( void* ref, int refTypeId );
    virtual void Store( asINT64& value );
    virtual void Store( double& value );

    // Retrieve the stored value, either as variable type, integer number, or real number
    virtual bool Retrieve( void* ref, int refTypeId ) const;
    virtual bool Retrieve( asINT64& value ) const;
    virtual bool Retrieve( double& value ) const;

    // Get the type id of the stored value
    virtual int GetTypeId() const;

    // GC methods
    virtual int  GetRefCount();
    virtual void SetFlag();
    virtual bool GetFlag();
    virtual void EnumReferences( asIScriptEngine* engine );
    virtual void ReleaseAllHandles( asIScriptEngine* engine );

protected:
    virtual ~ScriptAny();
    virtual void FreeObject();

    mutable int      refCount;
    asIScriptEngine* engine;

    // The structure for holding the values
    struct valueStruct
    {
        union
        {
            asINT64 valueInt;
            double  valueFlt;
            void*   valueObj;
        };
        int typeId;
    };

    valueStruct value;

public:

	static ScriptAny* FormatCScriptHandle(void *ref, int typeId)	
	{
		ScriptAny* any = &Create();
		any->Store( ref, typeId );
		return any;
	}

	asIObjectType *GetType()
	{
		return engine->GetObjectTypeById(value.typeId);
	}
	
	const char* TypeName()
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

	void Cast(void *outRef, int typeId)
	{
		Retrieve( outRef, typeId );
	}

	ScriptAny* Property( const char* propName )
	{
		if( value.valueObj && propName )
		{
			asIScriptObject *object = reinterpret_cast<asIScriptObject*>(value.valueObj);
			if( object )
				for( asUINT i = 0, iEnd = object->GetPropertyCount(); i < iEnd; i++ )
					if( strcmp( object->GetPropertyName(i), propName ) == 0 )
						return FormatCScriptHandle( object->GetAddressOfProperty(i), object->GetPropertyTypeId(i) );
		}
		return nullptr;	
	}

	ScriptString* script_TypeName()
	{
		return &ScriptString::Create( TypeName() );
	}

	ScriptString* script_TypeNamespace()
	{
		if( value.valueObj && GetType() )
			return &ScriptString::Create( GetType()->GetNamespace( ) );
		else return &ScriptString::Create( "" );
	}

	ScriptAny* script_Property( ScriptString* propName )
	{
		if( propName )
			return Property( propName->c_str() );
		return nullptr;
	}

	bool script_Primitive( )
	{
		return ( value.typeId > 0 && !(value.typeId & asTYPEID_MASK_OBJECT) );
	}
};

#ifndef FONLINE_DLL
void RegisterScriptAny( asIScriptEngine* engine );
#endif

#endif
