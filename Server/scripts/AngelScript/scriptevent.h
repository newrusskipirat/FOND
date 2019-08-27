

class EventTree;
class EventFunction;

typedef vector< EventFunction* >                 	ASFunctionVec;
typedef vector< EventFunction* >::const_iterator 	ASFunctionVecIt;

typedef map<string,EventTree*>					  		EventBrunch;

template <class T>  void AngelScript_AddReference( T* object );
template <class T>  void AngelScript_Release( T* object );
	
class EventFunction
{
public:
	EventFunction( asIScriptFunction* func, uint priority );
	~EventFunction();
	
	virtual bool run( ScriptString& script_eventName, ScriptHandle* handle, asIScriptContext* eventContext );
	virtual void clear( );
	bool script_run( ScriptHandle* handle );
	
	uint				priority;
	asIScriptFunction*	function;
};

class EventDelegate : public EventFunction
{
public:
	EventDelegate( asIScriptFunction* func, uint priority, void* obj, int tId );
	~EventDelegate();
	
	bool run( ScriptString& script_eventName, ScriptHandle* handle, asIScriptContext* eventContext ) override;
	void clear( ) override;
	
	void*   		  	object;
	int 			  	objectId;
};

class EventCustomFunction : public EventFunction
{
public:
	EventCustomFunction( asIScriptFunction* func );
	EventCustomFunction( EventCustomFunction* func, uint priority );
	
	bool run( ScriptString& script_eventName, ScriptHandle* handle, asIScriptContext* eventContext ) override;
	void clear( ) override;
	
	int					countreference;
};

class EventTree
{
private:
	ASFunctionVec 	eventsresult; 	// список каллбэков результатов
	ASFunctionVec 	eventsbool; 	// список каллбэков условий
	ASFunctionVec 	listenresult; 	// список каллбэков результатов
	ASFunctionVec 	listenbool; 	// список каллбэков условий
	EventBrunch 	brunchs;		// ветки узла
	EventTree*      parent;
	
	bool run( ScriptString& script_eventName, ScriptHandle* handle, asIScriptContext* eventContext, bool listen );
	string format( string& _name );
	string findsection( string& _name );
	EventTree* get( string& currentName );
	
	bool Run( ScriptString& script_eventName, ScriptHandle* handle, string& currentName, asIScriptContext* eventContext );
	
	void AddListen( EventFunction* func, string& currentName );
	uint EraseListen( EventFunction* func, string& currentName );
	uint EraseListenDelegate( EventDelegate* func, string& currentName );
	
	void AddCallback( EventFunction* func, string& currentName );
	uint EraseCallback( EventFunction* func, string& currentName );
	uint EraseDelegate( EventDelegate* func, string& currentName );
	
public:
	EventTree( const string& _name );
	~EventTree();
	
	void script_AddCustomCallback( ScriptString& script_eventName, EventCustomFunction* script_func, uint prior );
	void script_AddCallback( ScriptString& script_eventName, asIScriptFunction* func, uint prior );
	bool script_AddDelegate( ScriptString& script_eventName, ScriptString& script_methodname, void* object, int typeId, uint prior );
	uint script_EraseCallback( ScriptString& script_eventName, asIScriptFunction* func );
	uint script_EraseDelegate( ScriptString& script_eventName, ScriptString& script_methodname, void* object, int typeId );
	
	void script_Listen( ScriptString& script_eventName, asIScriptFunction* func, uint prior );
	bool script_ListenDelegate( ScriptString& script_eventName, ScriptString& script_methodname, void* object, int typeId, uint prior );
	uint script_EraseListen( ScriptString& script_eventName, asIScriptFunction* func );
	uint script_EraseListenDelegate( ScriptString& script_eventName, ScriptString& script_methodname, void* object, int typeId );
	
	bool script_Run( ScriptString& script_eventName, ScriptHandle* handle );
	bool script_RunUnknow( ScriptString& script_eventName, void* ref, int typeId );
	bool script_RunNull( ScriptString& script_eventName );
	uint script_ScanCallback( ScriptString& script_nameEvent, ScriptString& script_nameFunc, uint prior );
	EventTree* Get( ScriptString& script_eventName );
	EventTree* script_Parent( );
	
	EventTree* script_At( uint index );
	uint script_Length();

	static void Register( );
	static void script_Clear();
	static void script_Prepare();
	static EventTree* EventTree::Create( ScriptString& script_name );
	
	ScriptString* script_Name();
	
	void clear( );
	
	string			name;
	int				countreference;
};