#ifndef SCRIPTHANDLE_H
#define SCRIPTHANDLE_H

#ifndef ANGELSCRIPT_H 
// Avoid having to inform include path if header is already include before
#include "../angelscript.h"
#endif


BEGIN_AS_NAMESPACE

class ScriptHandle 
{
public:
	// Constructors
	ScriptHandle(asIScriptEngine *engine);
	ScriptHandle(void *ref, int refTypeId, asIScriptEngine *engine);

	// Memory management
	int AddRef() const;
	int Release() const;

	// Copy the stored value from another any object
	ScriptHandle &operator=(const ScriptHandle&);
	int CopyFrom(const ScriptHandle *other);

	// Store the value, either as variable type, integer number, or real number
	void Store(void *ref, int refTypeId);
	void Store(asINT64 &value);
	void Store(double &value);

	// Retrieve the stored value, either as variable type, integer number, or real number
	bool Retrieve(void *ref, int refTypeId) const;
	bool Retrieve(asINT64 &value) const;
	bool Retrieve(double &value) const;
	void Cast( void *outRef, int typeId );
	// Get the type id of the stored value
	int  GetTypeId() const;

	// GC methods
	int  GetRefCount();
	void SetFlag();
	bool GetFlag();
	void EnumReferences(asIScriptEngine *engine);
	void ReleaseAllHandles(asIScriptEngine *engine);

	ScriptString* 	script_TypeName();
	ScriptString* 	script_TypeNamespace();
	ScriptHandle* 	script_Property( ScriptString* propName );
	bool 			script_Primitive();
	
	ScriptHandle* 	Property( const char* propName );
	asIObjectType*	GetType();
	const char* 	TypeName();
	
	void SetCustom( const ScriptString& name, ScriptHandle* value );
	ScriptHandle* GetCustom( const ScriptString& name );
protected:
	virtual ~ScriptHandle();
	void FreeObject();

	mutable int refCount;
	asIScriptEngine *engine;

	// The structure for holding the values
    struct valueStruct
    {
        union
        {
            asINT64 valueInt;
            double  valueFlt;
            void   *valueObj;
        };
        int   typeId;
    };

	map<string,ScriptHandle*> custom;
	valueStruct value;
};

void RegisterScriptHandle(asIScriptEngine *engine);

END_AS_NAMESPACE

#endif
