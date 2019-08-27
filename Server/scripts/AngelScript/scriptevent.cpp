
#include "scriptevent.h"

vector< asIScriptContext* > EventContext;
uint CountWorkContext = 0;

#include "source/as_scriptengine.h"

static void EventTreeAddReference( EventTree* object )
{
	//Log( "event <%s> addref %i\n", object->name.c_str(), object->countreference + 1 );
	object->countreference++;
}
	
static void EventTreeRelease( EventTree* object )
{
	//Log( "event <%s> release %i\n", object->name.c_str(), object->countreference - 1 );
	if( --object->countreference == 0 )
	{
		object->clear();
		delete object;
	}
}

void PrintExceptionInfo(asIScriptContext* ctx, void* )
{
	// Determine the exception that occurred
	Log("Description: %s\n", ctx->GetExceptionString());
	// Determine the function where the exception occurred
	const asIScriptFunction *function = ctx->GetExceptionFunction();
	if( function )
	{
		Log("Function: %s\n", function->GetDeclaration());
		Log("Module: %s\n", function->GetModuleName());
		Log("Section: %s\n", function->GetScriptSectionName());
		// Determine the line number where the exception occurred
	}
	Log("Line: %d\n", ctx->GetExceptionLineNumber());
}

asIScriptContext* GetEventContext( )
{
	asIScriptContext* result = 0;
	if(  EventContext.size() < ++CountWorkContext )
	{
		result = ASEngine->CreateContext( );
		result->SetExceptionCallback(asFUNCTION(PrintExceptionInfo), nullptr, asCALL_CDECL);
		EventContext.push_back( result );
	}
	else result = EventContext[CountWorkContext - 1];
	return result;
}

void FreeContext( asIScriptContext* ctx )
{
	--CountWorkContext;
	ctx->Unprepare();
}

void EventFunction::clear( )
{

}

EventFunction::EventFunction( asIScriptFunction* func, uint prior )
{
	function = func;
	priority = prior;
}

void EventCustomFunction::clear( )
{
	function = nullptr;
	priority = 0;
}

bool EventFunction::run( ScriptString& script_eventName, ScriptHandle* handle, asIScriptContext* eventContext )
{
	eventContext->Prepare( function );
	eventContext->SetArgAddress( 0, &script_eventName );
	eventContext->SetArgObject( 1, handle );
	if( eventContext->Execute() == asEXECUTION_FINISHED )
	{
		switch( function->GetReturnTypeId() )
		{
			case asTYPEID_VOID: 
				return true;
				
			case asTYPEID_BOOL:
				return ( eventContext->GetReturnByte() != 0 );
				
			default: Log( "Error return typeId EventFunction\n" ); break;
		}
	}
	return false;
}

bool EventDelegate::run( ScriptString& script_eventName, ScriptHandle* handle, asIScriptContext* eventContext )
{
	eventContext->Prepare( function );
	eventContext->SetObject( object );
	eventContext->SetArgAddress( 0, &script_eventName );
	eventContext->SetArgObject( 1, handle );
	if( eventContext->Execute() == asEXECUTION_FINISHED )
	{
		switch( function->GetReturnTypeId() )
		{
			case asTYPEID_VOID: 
				return true;
				
			case asTYPEID_BOOL:
				return ( eventContext->GetReturnByte() != 0 );
				
			default: Log( "Error return typeId EventDelegate\n" ); break;
		}
	}
	return false;
}

EventDelegate::EventDelegate( asIScriptFunction* func, uint prior, void* obj, int tId ) : 
		EventFunction( func, priority )
{
	if( tId & asTYPEID_OBJHANDLE )
	{
		// Store the actual reference
		obj = *(void**)obj;
		tId &= ~asTYPEID_OBJHANDLE;
	}
	object = obj;
	objectId = tId;
	
	ASEngine->AddRefScriptObject(object, objectId);
}

void EventDelegate::clear( )
{
	if( object )
		ASEngine->ReleaseScriptObject(object, objectId);

	//Log( "Release delegate\n");
	object = nullptr;
	objectId = 0;
}

EventDelegate::~EventDelegate(){}
EventFunction::~EventFunction(){}

bool EventTree::run( ScriptString& script_eventName, ScriptHandle* handle, asIScriptContext* eventContext, bool listen )
{
	if( listen )
	{
		for( ASFunctionVecIt it = listenbool.begin( ), end = listenbool.end( ); it != end; it++ )
		{
			if( !(*it)->run( script_eventName, handle, eventContext ) )
				return false;
		}
		for( ASFunctionVecIt it = listenresult.begin( ), end = listenresult.end( ); it != end; it++ )
		{
			if( !(*it)->run( script_eventName, handle, eventContext ) )
				return false;
		}
	}
	else
	{
		for( ASFunctionVecIt it = eventsbool.begin( ), end = eventsbool.end( ); it != end; it++ )
		{
			if( !(*it)->run( script_eventName, handle, eventContext ) )
				return false;
		}
		for( ASFunctionVecIt it = eventsresult.begin( ), end = eventsresult.end( ); it != end; it++ )
		{
			if( !(*it)->run( script_eventName, handle, eventContext ) )
				return false;
		}
	}
	return true;
}
	
string EventTree::format( string& _name )
{
	string::size_type index = _name.find( "." );
	if( index++ != string::npos && index < _name.size() )
		return _name.substr( index, _name.size() - index );
	return "";
}
	
string EventTree::findsection( string& _name )
{
	string::size_type index = _name.find( "." );
	if( index != string::npos )
		return _name.substr( 0, index );
	return _name;
}

EventTree::EventTree( const string& _name )
{
	parent = nullptr;
	name = _name;
	countreference = 0;
}
	
EventTree::~EventTree()
{
	parent = nullptr;
	name = "";
}
	
bool EventTree::Run( ScriptString& script_eventName, ScriptHandle* handle, string& currentName, asIScriptContext* eventContext )
{
	if( !run( script_eventName, handle, eventContext, currentName != "" ) )
		return false;
	
	string section = findsection( currentName );
	currentName = format( currentName );
	
	auto iterbrunch = brunchs.find( section );
	if( iterbrunch != brunchs.end( ) )
		return iterbrunch->second->Run( script_eventName, handle, currentName, eventContext );
	return true;
}
	
void EventTree::clear( )
{
	//Log( "event <%s> clear\n", name.c_str() );
	for( auto it = brunchs.begin( ), end = brunchs.end( ); it != end; it++ )
	{
		it->second->clear( );
		delete (it->second);
	}
	
	for( ASFunctionVecIt it = eventsresult.begin( ), end = eventsresult.end( ); it != end; it++ )
	{
		(*it)->clear();
		delete (*it);
	}
	for( ASFunctionVecIt it = eventsbool.begin( ), end = eventsbool.end( ); it != end; it++ )
	{
		(*it)->clear();
		delete (*it);
	}
	
	eventsresult.clear();
	eventsbool.clear();
}
	
void EventTree::AddCallback( EventFunction* func, string& currentName  )
{
	if( currentName == "" )
	{
		if( func->function->GetReturnTypeId() == asTYPEID_BOOL )
		{
			uint index = 0;
			for( uint end = eventsbool.size( ); index < end; index++ )
				if( eventsbool[index]->priority > func->priority )
					break;
				
			eventsbool.insert( eventsbool.begin( ) + index, func );
		}
		else if( func->function->GetReturnTypeId() == asTYPEID_VOID )
		{
			uint index = 0;
			for( uint end = eventsresult.size( ); index < end; index++ )
				if( eventsresult[index]->priority > func->priority )
					break;
			
			eventsresult.insert( eventsresult.begin( ) + index, func );
		}
	}
	else
	{
		string section = findsection( currentName );
		currentName = format( currentName );
		
		auto iterbrunch = brunchs.find( section );
		if( iterbrunch == brunchs.end( ) )
		{
			EventTree* child = new EventTree( section );
			child->parent = this;
			brunchs[section] = child;
			iterbrunch = brunchs.find( section );
		}
		iterbrunch->second->AddCallback( func, currentName );
	}
}
	
uint EventTree::EraseCallback( EventFunction* func, string& currentName )
{
	if( currentName == "" )
	{
		uint result = 0;
		if( func->function->GetReturnTypeId() == asTYPEID_BOOL )
		{
			for( uint index = 0, end = eventsbool.size( ); index < end; index++ )
				if( eventsbool[index]->function == func->function )
				{
					delete (*eventsbool.begin() + index);
					eventsbool.erase( eventsbool.begin() + index-- );
					result++;
					end--;
				}
		}
		else if( func->function->GetReturnTypeId() == asTYPEID_VOID )
		{
			for( uint index = 0, end = eventsresult.size( ); index < end; index++ )
				if( eventsresult[index]->function == func->function )
				{
					delete (*eventsresult.begin() + index);
					eventsresult.erase( eventsresult.begin() + index-- );
					result++;
					end--;
				}
		}
		return result;
	}
	else
	{
		string section = findsection( currentName );
		currentName = format( currentName );
		
		auto iterbrunch = brunchs.find( section );
		if( iterbrunch != brunchs.end( ) )
			return iterbrunch->second->EraseCallback( func, currentName );
	}
	return 0;
}
	
uint EventTree::EraseDelegate( EventDelegate* func, string& currentName )
{
	if( currentName == "" )
	{
		uint result = 0;
		if( func->function->GetReturnTypeId() == asTYPEID_BOOL )
		{
			for( uint index = 0, end = eventsbool.size( ); index < end; index++ )
			{
				EventDelegate* delegate = dynamic_cast<EventDelegate*>(eventsbool[index]);
				if( delegate && delegate->function == func->function && delegate->object == func->object)
				{
					delete (*eventsbool.begin() + index);
					eventsbool.erase( eventsbool.begin() + index-- );
					result++;
					end--;
				}
			}
		}
		else if( func->function->GetReturnTypeId() == asTYPEID_VOID )
		{
			for( uint index = 0, end = eventsresult.size( ); index < end; index++ )
			{
				EventDelegate* delegate = dynamic_cast<EventDelegate*>(eventsresult[index]);
				if( delegate->function == func->function && delegate->object == func->object )
				{
					delete (*eventsresult.begin() + index);
					eventsresult.erase( eventsresult.begin() + index-- );
					result++;
					end--;
				}
			}
		}
		return result;
	}
	else
	{
		string section = findsection( currentName );
		currentName = format( currentName );
		
		auto iterbrunch = brunchs.find( section );
		if( iterbrunch != brunchs.end( ) )
			return iterbrunch->second->EraseDelegate( func, currentName );
	}
	return 0;
}

void EventTree::script_AddCallback( ScriptString& script_eventName, asIScriptFunction* func, uint prior )
{
	string var_name = script_eventName.c_std_str();
	std::transform(var_name.begin(), var_name.end(), var_name.begin(), ::tolower);

	this->AddCallback( new EventFunction( func, prior ), var_name );
}

void EventTree::script_AddCustomCallback( ScriptString& script_eventName, EventCustomFunction* script_func, uint prior )
{
	string var_name = script_eventName.c_std_str();
	std::transform(var_name.begin(), var_name.end(), var_name.begin(), ::tolower);
	
	this->AddCallback( new EventCustomFunction( script_func, prior ), var_name );
}

uint EventTree::script_EraseCallback( ScriptString& script_eventName, asIScriptFunction* func )
{
	string var_name = script_eventName.c_std_str();
	std::transform(var_name.begin(), var_name.end(), var_name.begin(), ::tolower);

	return this->EraseCallback( new EventFunction( func, 0 ), var_name );
}

uint EventTree::script_EraseDelegate( ScriptString& script_eventName, ScriptString& script_methodname, void* object, int typeId )
{
	if( typeId & asTYPEID_OBJHANDLE )
	{
		// Store the actual reference
		object = *(void**)object;
		typeId &= ~asTYPEID_OBJHANDLE;
	}
	uint result = 0;
	asIObjectType* type = ASEngine->GetObjectTypeById(typeId);
	if( type )
	{
		string  methoddeclar = script_methodname.c_std_str() + "( string&in, Handle@+ )";
		asIScriptFunction* func = type->GetMethodByDecl( string( "void " + methoddeclar ).c_str() );
		if( func )
		{
			string var_name = script_eventName.c_std_str();
			std::transform(var_name.begin(), var_name.end(), var_name.begin(), ::tolower);
			result += this->EraseDelegate( &EventDelegate( func, 0, object, typeId ), var_name );
		}
		
		func = type->GetMethodByDecl( string( "bool " + methoddeclar ).c_str() );
		if( func )
		{
			string var_name = script_eventName.c_std_str();
			std::transform(var_name.begin(), var_name.end(), var_name.begin(), ::tolower);
			result += this->EraseDelegate( &EventDelegate( func, 0, object, typeId ), var_name );
		}
	}
	return result;
}

EventTree* EventTree::get( string& currentName )
{
	if( currentName == "" )
		return this;
	else
	{
		string section = findsection( currentName );
		currentName = format( currentName );
		
		auto iterbrunch = brunchs.find( section );
		if( iterbrunch == brunchs.end( ) )
			return nullptr;
		return iterbrunch->second->get( currentName );
	}
}

EventTree* EventTree::Get( ScriptString& script_eventName )
{
	string var_name = script_eventName.c_std_str();
	std::transform(var_name.begin(), var_name.end(), var_name.begin(), ::tolower);
	return get( var_name );
}

bool EventTree::script_AddDelegate( ScriptString& script_eventName, ScriptString& script_methodname, void* object, int typeId, uint prior )
{
	if( typeId & asTYPEID_OBJHANDLE )
	{
		// Store the actual reference
		object = *(void**)object;
		typeId &= ~asTYPEID_OBJHANDLE;
	}
	bool result = false;
	asIObjectType* type = ASEngine->GetObjectTypeById(typeId);
	if( type )
	{
		string  methoddeclar = script_methodname.c_std_str() + "( string&in, Handle@+ )";
		asIScriptFunction* func = type->GetMethodByDecl( string( "void " + methoddeclar ).c_str() );
		if( func )
		{
			string var_name = script_eventName.c_std_str();
			std::transform(var_name.begin(), var_name.end(), var_name.begin(), ::tolower);
			this->AddCallback( new EventDelegate( func, prior, object, typeId ), var_name );
			result = true;
		}
		
		func = type->GetMethodByDecl( string( "bool " + methoddeclar ).c_str() );
		if( func )
		{
			string var_name = script_eventName.c_std_str();
			std::transform(var_name.begin(), var_name.end(), var_name.begin(), ::tolower);
			this->AddCallback( new EventDelegate( func, prior, object, typeId ), var_name );
			result = true;
		}
	}
	return result;
}

bool EventTree::script_Run( ScriptString& script_eventName, ScriptHandle* handle )
{
	asIScriptContext* eventContext = GetEventContext( );
	string var_name = script_eventName.c_std_str();
	std::transform(var_name.begin(), var_name.end(), var_name.begin(), ::tolower);
	bool result = this->Run( script_eventName, handle, var_name, eventContext );
	FreeContext( eventContext );
	return result;
}

bool EventTree::script_RunUnknow( ScriptString& script_eventName, void* ref, int typeId )
{
	ScriptHandle* handle = FormatCScriptHandle( ref, typeId );
	bool result = this->script_Run( script_eventName, handle );
	handle->Release();
	return result;
}

void EventTree::script_Clear()
{	
	for( uint index = 0, end = EventContext.size( ); index < end; index++ )
		EventContext[index]->Release( );

	EventContext.clear();
}

void EventTree::script_Prepare( )
{
	
}

ScriptString* EventTree::script_Name()
{
	return &ScriptString::Create( name.c_str() );
}

EventTree* EventTree::script_At( uint index )
{
	if( index >= brunchs.size() )
		return nullptr;
	auto it = brunchs.begin( );
	std::advance(it, index);
	return it->second;
}

EventTree* EventTree::script_Parent( )
{
	return parent;
}

uint EventTree::script_Length()
{
	return brunchs.size();
}

EventTree* EventTree::Create( ScriptString& script_name )
{
	string var_name = script_name.c_std_str();
	std::transform(var_name.begin(), var_name.end(), var_name.begin(), ::tolower);
	return new EventTree( var_name );
}

uint EventTree::script_ScanCallback( ScriptString& script_nameEvent, ScriptString& script_nameFunc, uint prior )
{
	uint result = 0;
	//EventTree* tree = this->Get( script_nameEvent );
	//if( tree )
	{
		string var_name = script_nameEvent.c_std_str();
		std::transform(var_name.begin(), var_name.end(), var_name.begin(), ::tolower);
		asCScriptEngine* scriptEngine = (asCScriptEngine*)ASEngine;
		size_t len = scriptEngine->scriptModules.GetLength();
		
		string methoddeclar = script_nameFunc.c_std_str() + "( string&in, Handle@+ )";
		
		for( uint i = 0; i < len; i++ )
		{
			asCModule* module = scriptEngine->scriptModules[i];
			if( module )
			{
				asIScriptFunction *func = module->GetFunctionByDecl( string( "void " + methoddeclar ).c_str() );
				if( func )
				{
					result++;
					this->AddCallback( new EventFunction( func, prior ), string( var_name ) );
				}
				func = module->GetFunctionByDecl( string( "bool " + methoddeclar ).c_str() );
				if( func )
				{
					result++;
					this->AddCallback( new EventFunction( func, prior ), string( var_name ) );
				}
			}
		}
	}
	//else
	//	Log( "Узел ивентов не найден\n" );
	return result;
}

static bool ScriptCallbackTemplate( asIObjectType* baseot )
{
	return strcmp( baseot->GetSubType()->GetName(), "_builtin_function_" ) == 0;
}

static EventCustomFunction* ScriptCallbackFactory( asIObjectType *baseot, asIScriptFunction *func )
{
	return new EventCustomFunction( func );
}

EventCustomFunction::EventCustomFunction( asIScriptFunction* func ) :
	EventFunction( func, 0 )
{
	countreference = 1;
}

EventCustomFunction::EventCustomFunction( EventCustomFunction* func, uint prior ) :
	EventFunction( func->function, prior )
{
	countreference = 1;
}

void AngelScript_AddReferenceEventCustomFunction( EventCustomFunction* object )
{
	object->countreference++;
}

void AngelScript_ReleaseEventCustomFunction( EventCustomFunction* object )
{
	if( --object->countreference == 0 )
	{
		object->clear();
		delete object;
	}
}

bool EventCustomFunction::run( ScriptString& script_eventName, ScriptHandle* handle, asIScriptContext* eventContext )
{
	eventContext->Prepare( function );
	uint argid = 0;
	for( uint paramindex = 0, endindex = function->GetParamCount(); paramindex < endindex; paramindex++ )
	{
		ScriptHandle* handle_arg = handle->Property( "error" );
		if( !handle_arg )
		{
			Log( "No find argument\n <%s>\n <%s>\n", function->GetDeclaration( true, false ), function->GetDeclaration( false, false ) );
			eventContext->Unprepare( );
			return true;
		}
	}
	//	eventContext->SetArgAddress( 0, &script_eventName );
	//	eventContext->SetArgObject( 1, handle );
	if( eventContext->Execute() == asEXECUTION_FINISHED )
	{
		switch( function->GetReturnTypeId() )
		{
			case asTYPEID_VOID: 
				return true;
				
			case asTYPEID_BOOL:
				return ( eventContext->GetReturnByte() != 0 );
				
			default: break;
		}
	}
	Log( "Error return typeId EventDelegate\n" );
	return false;
}

void EventTree::Register( )
{
	class _result_angelscript_registration
	{
		public:
		_result_angelscript_registration& operator=( int other )
		{
			assert( other >= 0 );
			return *this;
		}
	} result;
	
	result = ASEngine->RegisterFuncdef("void EventFuncdefVoid( string&in event, Handle@+ handle )");
	result = ASEngine->RegisterFuncdef("bool EventFuncdefBool( string&in event, Handle@+ handle )");
    result = ASEngine->RegisterGlobalFunction( "void ClearEventSystem( )", asFUNCTION( EventTree::script_Clear ), asCALL_CDECL );
    result = ASEngine->RegisterGlobalFunction( "void PrepareEventSystem( )", asFUNCTION( EventTree::script_Prepare ), asCALL_CDECL );
	
	result = ASEngine->RegisterObjectType( "Callback<class T>", sizeof( EventCustomFunction ), asOBJ_REF | asOBJ_TEMPLATE );
	result = ASEngine->RegisterObjectBehaviour( "Callback<T>", asBEHAVE_ADDREF, "void f()", asFUNCTION( AngelScript_AddReferenceEventCustomFunction ), asCALL_CDECL_OBJFIRST );
    result = ASEngine->RegisterObjectBehaviour( "Callback<T>", asBEHAVE_RELEASE, "void f()", asFUNCTION( AngelScript_ReleaseEventCustomFunction ), asCALL_CDECL_OBJFIRST );
	
	result = ASEngine->RegisterObjectBehaviour( "Callback<T>", asBEHAVE_TEMPLATE_CALLBACK, "bool f(int&in)", asFUNCTION( ScriptCallbackTemplate ), asCALL_CDECL);
	result = ASEngine->RegisterObjectBehaviour( "Callback<T>", asBEHAVE_FACTORY, "Callback<T>@ f( int&in, T&in callback )", asFUNCTION( ScriptCallbackFactory ), asCALL_CDECL);
	
	result = ASEngine->RegisterObjectType( "EventTree", sizeof( EventTree ), asOBJ_REF );
	
	result = ASEngine->RegisterObjectBehaviour( "EventTree", asBEHAVE_ADDREF, "void f()", asFUNCTION( EventTreeAddReference ), asCALL_CDECL_OBJFIRST );
    result = ASEngine->RegisterObjectBehaviour( "EventTree", asBEHAVE_RELEASE, "void f()", asFUNCTION( EventTreeRelease ), asCALL_CDECL_OBJFIRST );
	
    result = ASEngine->RegisterObjectMethod( "EventTree", "bool Run( string&in name , Handle@+ handle = null )", asMETHOD(EventTree, script_Run), asCALL_THISCALL );
    result = ASEngine->RegisterObjectMethod( "EventTree", "bool Run( string&in name , ?&in )", asMETHOD(EventTree, script_RunUnknow), asCALL_THISCALL );
	result = ASEngine->RegisterObjectMethod( "EventTree", "void SetCallback( string& name, EventFuncdefVoid@+ callback, uint priority = 0 )", asMETHOD(EventTree,script_AddCallback ), asCALL_THISCALL );
	result = ASEngine->RegisterObjectMethod( "EventTree", "void SetCallback( string& name, EventFuncdefBool@+ callback, uint priority = 0 )", asMETHOD(EventTree,script_AddCallback ), asCALL_THISCALL );
	result = ASEngine->RegisterObjectMethod( "EventTree", "void SetCallback( string& name, Callback<T>@+ callback, uint priority = 0 )", asMETHOD(EventTree,script_AddCustomCallback ), asCALL_THISCALL );
	result = ASEngine->RegisterObjectMethod( "EventTree", "bool SetCallback( string& name, string& method, const ?&in delegate, uint priority = 0 )", asMETHOD(EventTree,script_AddDelegate ), asCALL_THISCALL );
	result = ASEngine->RegisterObjectMethod( "EventTree", "uint EraseCallback( string& name, EventFuncdefVoid@+ callback )", asMETHOD(EventTree,script_EraseCallback ), asCALL_THISCALL );
	result = ASEngine->RegisterObjectMethod( "EventTree", "uint EraseCallback( string& name, EventFuncdefBool@+ callback )", asMETHOD(EventTree,script_EraseCallback ), asCALL_THISCALL );
	result = ASEngine->RegisterObjectMethod( "EventTree", "uint EraseCallback( string& name, string& method, const ?&in delegate )", asMETHOD(EventTree,script_EraseDelegate ), asCALL_THISCALL );

	result = ASEngine->RegisterObjectMethod( "EventTree", "void SetListen( string& name, EventFuncdefVoid@+ callback, uint priority = 0 )", asMETHOD(EventTree,script_Listen ), asCALL_THISCALL );
	result = ASEngine->RegisterObjectMethod( "EventTree", "void SetListen( string& name, EventFuncdefBool@+ callback, uint priority = 0 )", asMETHOD(EventTree,script_Listen ), asCALL_THISCALL );
	result = ASEngine->RegisterObjectMethod( "EventTree", "bool SetListen( string& name, string& method, const ?&in delegate, uint priority = 0 )", asMETHOD(EventTree,script_ListenDelegate ), asCALL_THISCALL );
	result = ASEngine->RegisterObjectMethod( "EventTree", "uint EraseListen( string& name, EventFuncdefVoid@+ callback )", asMETHOD(EventTree,script_EraseListen ), asCALL_THISCALL );
	result = ASEngine->RegisterObjectMethod( "EventTree", "uint EraseListen( string& name, EventFuncdefBool@+ callback )", asMETHOD(EventTree,script_EraseListen ), asCALL_THISCALL );
	result = ASEngine->RegisterObjectMethod( "EventTree", "uint EraseListen( string& name, string& method, const ?&in delegate )", asMETHOD(EventTree,script_EraseListenDelegate ), asCALL_THISCALL );

	result = ASEngine->RegisterObjectMethod( "EventTree", "uint ScanCallback( string& nameEvent, string& nameFunc, uint priority = 0 )", asMETHOD(EventTree,script_ScanCallback ), asCALL_THISCALL );

	result = ASEngine->RegisterObjectMethod( "EventTree", "uint length( )", asMETHOD( EventTree, script_Length ), asCALL_THISCALL );
	result = ASEngine->RegisterObjectMethod( "EventTree", "EventTree@ at( uint index )", asMETHOD( EventTree,script_At ), asCALL_THISCALL );
	
	result = ASEngine->RegisterObjectMethod( "EventTree", "string@ get_Name( )", asMETHOD(EventTree,script_Name ), asCALL_THISCALL );
	result = ASEngine->RegisterObjectMethod( "EventTree", "EventTree@ get_Parent( )", asMETHOD(EventTree,script_Parent ), asCALL_THISCALL );
	result = ASEngine->RegisterObjectMethod( "EventTree", "EventTree@ opIndex( const string&in name )", asMETHOD(EventTree,Get ), asCALL_THISCALL );
	
	ASEngine->SetDefaultNamespace( "EventTree" );
    result = ASEngine->RegisterGlobalFunction( "::EventTree@+ Create( const ::string&in name )", asFUNCTION( EventTree::Create ), asCALL_CDECL );
    ASEngine->SetDefaultNamespace( "" );
}
	
void EventTree::AddListen( EventFunction* func, string& currentName  )
{
	if( currentName == "" )
	{
		if( func->function->GetReturnTypeId() == asTYPEID_BOOL )
		{
			uint index = 0;
			for( uint end = listenbool.size( ); index < end; index++ )
				if( listenbool[index]->priority > func->priority )
					break;
				
			listenbool.insert( listenbool.begin( ) + index, func );
		}
		else if( func->function->GetReturnTypeId() == asTYPEID_VOID )
		{
			uint index = 0;
			for( uint end = listenresult.size( ); index < end; index++ )
				if( listenresult[index]->priority > func->priority )
					break;
			
			listenresult.insert( listenresult.begin( ) + index, func );
		}
	}
	else
	{
		string section = findsection( currentName );
		currentName = format( currentName );
		
		auto iterbrunch = brunchs.find( section );
		if( iterbrunch == brunchs.end( ) )
		{
			EventTree* child = new EventTree( section );
			child->parent = this;
			brunchs[section] = child;
			iterbrunch = brunchs.find( section );
		}
		iterbrunch->second->AddListen( func, currentName );
	}
}
	
uint EventTree::EraseListen( EventFunction* func, string& currentName )
{
	if( currentName == "" )
	{
		uint result = 0;
		if( func->function->GetReturnTypeId() == asTYPEID_BOOL )
		{
			for( uint index = 0, end = listenbool.size( ); index < end; index++ )
				if( listenbool[index]->function == func->function )
				{
					delete (*listenbool.begin() + index);
					listenbool.erase( listenbool.begin() + index-- );
					result++;
					end--;
				}
		}
		else if( func->function->GetReturnTypeId() == asTYPEID_VOID )
		{
			for( uint index = 0, end = listenresult.size( ); index < end; index++ )
				if( listenresult[index]->function == func->function )
				{
					delete (*listenresult.begin() + index);
					listenresult.erase( listenresult.begin() + index-- );
					result++;
					end--;
				}
		}
		return result;
	}
	else
	{
		string section = findsection( currentName );
		currentName = format( currentName );
		
		auto iterbrunch = brunchs.find( section );
		if( iterbrunch != brunchs.end( ) )
			return iterbrunch->second->EraseListen( func, currentName );
	}
	return 0;
}

uint EventTree::EraseListenDelegate( EventDelegate* func, string& currentName )
{
	if( currentName == "" )
	{
		uint result = 0;
		if( func->function->GetReturnTypeId() == asTYPEID_BOOL )
		{
			for( uint index = 0, end = listenbool.size( ); index < end; index++ )
			{
				EventDelegate* delegate = dynamic_cast<EventDelegate*>(listenbool[index]);
				if( delegate && delegate->function == func->function && delegate->object == func->object)
				{
					delete (*listenbool.begin() + index);
					listenbool.erase( listenbool.begin() + index-- );
					result++;
					end--;
				}
			}
		}
		else if( func->function->GetReturnTypeId() == asTYPEID_VOID )
		{
			for( uint index = 0, end = listenresult.size( ); index < end; index++ )
			{
				EventDelegate* delegate = dynamic_cast<EventDelegate*>(listenresult[index]);
				if( delegate->function == func->function && delegate->object == func->object )
				{
					delete (*listenresult.begin() + index);
					listenresult.erase( listenresult.begin() + index-- );
					result++;
					end--;
				}
			}
		}
		return result;
	}
	else
	{
		string section = findsection( currentName );
		currentName = format( currentName );
		
		auto iterbrunch = brunchs.find( section );
		if( iterbrunch != brunchs.end( ) )
			return iterbrunch->second->EraseListenDelegate( func, currentName );
	}
	return 0;
}
	
void EventTree::script_Listen( ScriptString& script_eventName, asIScriptFunction* func, uint prior )
{
	string var_name = script_eventName.c_std_str();
	std::transform(var_name.begin(), var_name.end(), var_name.begin(), ::tolower);

	this->AddListen( new EventFunction( func, prior ), var_name );
}

bool EventTree::script_ListenDelegate( ScriptString& script_eventName, ScriptString& script_methodname, void* object, int typeId, uint prior )
{
	if( typeId & asTYPEID_OBJHANDLE )
	{
		// Store the actual reference
		object = *(void**)object;
		typeId &= ~asTYPEID_OBJHANDLE;
	}
	bool result = false;
	asIObjectType* type = ASEngine->GetObjectTypeById(typeId);
	if( type )
	{
		string  methoddeclar = script_methodname.c_std_str() + "( string&in, Handle@+ )";
		asIScriptFunction* func = type->GetMethodByDecl( string( "void " + methoddeclar ).c_str() );
		if( func )
		{
			string var_name = script_eventName.c_std_str();
			std::transform(var_name.begin(), var_name.end(), var_name.begin(), ::tolower);
			this->AddListen( new EventDelegate( func, prior, object, typeId ), var_name );
			result = true;
		}
		
		func = type->GetMethodByDecl( string( "bool " + methoddeclar ).c_str() );
		if( func )
		{
			string var_name = script_eventName.c_std_str();
			std::transform(var_name.begin(), var_name.end(), var_name.begin(), ::tolower);
			this->AddListen( new EventDelegate( func, prior, object, typeId ), var_name );
			result = true;
		}
	}
	return result;
}

uint EventTree::script_EraseListen( ScriptString& script_eventName, asIScriptFunction* func )
{
	string var_name = script_eventName.c_std_str();
	std::transform(var_name.begin(), var_name.end(), var_name.begin(), ::tolower);

	return this->EraseListen( new EventFunction( func, 0 ), var_name );
}

uint EventTree::script_EraseListenDelegate( ScriptString& script_eventName, ScriptString& script_methodname, void* object, int typeId )
{
	if( typeId & asTYPEID_OBJHANDLE )
	{
		// Store the actual reference
		object = *(void**)object;
		typeId &= ~asTYPEID_OBJHANDLE;
	}
	uint result = 0;
	asIObjectType* type = ASEngine->GetObjectTypeById(typeId);
	if( type )
	{
		string  methoddeclar = script_methodname.c_std_str() + "( string&in, Handle@+ )";
		asIScriptFunction* func = type->GetMethodByDecl( string( "void " + methoddeclar ).c_str() );
		if( func )
		{
			string var_name = script_eventName.c_std_str();
			std::transform(var_name.begin(), var_name.end(), var_name.begin(), ::tolower);
			result += this->EraseListenDelegate( &EventDelegate( func, 0, object, typeId ), var_name );
		}
		
		func = type->GetMethodByDecl( string( "bool " + methoddeclar ).c_str() );
		if( func )
		{
			string var_name = script_eventName.c_std_str();
			std::transform(var_name.begin(), var_name.end(), var_name.begin(), ::tolower);
			result += this->EraseListenDelegate( &EventDelegate( func, 0, object, typeId ), var_name );
		}
	}
	return result;
}
