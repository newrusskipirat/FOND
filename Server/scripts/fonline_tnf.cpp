#include "fonline_tnf.h"
#include "AngelScript/scripthandle/scripthandle.cpp"
#include "AngelScript/scriptfilesystem.cpp"

// Extern data definition
_GlobalVars GlobalVars;

// Slot/parameters allowing
EXPORT bool allowSlot_Hand1( uint8, Item &, Critter &, Critter & toCr );

// Parameters Get behavior
EXPORT int getParam_Strength( CritterMutual & cr, uint );
EXPORT int getParam_Perception( CritterMutual & cr, uint );
EXPORT int getParam_Endurance( CritterMutual & cr, uint );
EXPORT int getParam_Charisma( CritterMutual & cr, uint );
EXPORT int getParam_Intellegence( CritterMutual & cr, uint );
EXPORT int getParam_Agility( CritterMutual & cr, uint );
EXPORT int getParam_Luck( CritterMutual & cr, uint );
EXPORT int getParam_Hp( CritterMutual & cr, uint );
EXPORT int getParam_MaxLife( CritterMutual & cr, uint );
EXPORT int getParam_MaxAp( CritterMutual & cr, uint );
EXPORT int getParam_Ap( CritterMutual & cr, uint );
EXPORT int getParam_MaxMoveAp( CritterMutual & cr, uint );
EXPORT int getParam_MoveAp( CritterMutual & cr, uint );
EXPORT int getParam_MaxWeight( CritterMutual & cr, uint );
EXPORT int getParam_Sequence( CritterMutual & cr, uint );
EXPORT int getParam_MeleeDmg( CritterMutual & cr, uint );
EXPORT int getParam_HealingRate( CritterMutual & cr, uint );
EXPORT int getParam_CriticalChance( CritterMutual & cr, uint );
EXPORT int getParam_MaxCritical( CritterMutual & cr, uint );
EXPORT int getParam_Ac( CritterMutual & cr, uint );
EXPORT int getParam_DamageResistance( CritterMutual& cr, uint index );
EXPORT int getParam_DamageThreshold( CritterMutual& cr, uint index );
EXPORT int getParam_RadiationResist( CritterMutual & cr, uint );
EXPORT int getParam_PoisonResist( CritterMutual & cr, uint );
EXPORT int getParam_Timeout( CritterMutual& cr, uint index );
EXPORT int getParam_Reputation( CritterMutual& cr, uint index );
EXPORT void changedParam_Reputation( CritterMutual& cr, uint index, int oldValue );

// Extended methods
EXPORT bool Critter_IsInjured( CritterMutual& cr );
EXPORT bool Critter_IsDmgEye( CritterMutual& cr );
EXPORT bool Critter_IsDmgLeg( CritterMutual& cr );
EXPORT bool Critter_IsDmgTwoLeg( CritterMutual& cr );
EXPORT bool Critter_IsDmgArm( CritterMutual& cr );
EXPORT bool Critter_IsDmgTwoArm( CritterMutual& cr );
EXPORT bool Critter_IsAddicted( CritterMutual& cr );
EXPORT bool Critter_IsOverweight( CritterMutual& cr );
EXPORT bool Item_Weapon_IsHtHAttack( Item& item, uint8 mode );
EXPORT bool Item_Weapon_IsGunAttack( Item& item, uint8 mode );
EXPORT bool Item_Weapon_IsRangedAttack( Item& item, uint8 mode );

// Callbacks
uint GetUseApCost( CritterMutual& cr, Item& item, uint8 mode );
uint GetAttackDistantion( CritterMutual& cr, Item& item, uint8 mode );

// Generic stuff
int  GetNightPersonBonus();
uint GetAimApCost( int hitLocation );
uint GetAimHit( int hitLocation );
uint GetMultihex( CritterMutual& cr );

// tnf server
#ifdef __SERVER
EXPORT uint Map_GetTile( Map& map, uint16 tx, uint16 ty );
EXPORT uint Map_GetRoof( Map& map, uint16 tx, uint16 ty );
EXPORT bool Map_SetTile( Map& map, uint16 tx, uint16 ty, uint picHash );
EXPORT bool Map_SetRoof( Map& map, uint16 tx, uint16 ty, uint picHash );

EXPORT uint Critter_GetItemTransferCount( Critter& cr );
EXPORT void Critter_GetIp( Critter& cr, ScriptArray* array );
#endif // __SERVER

/************************************************************************/
/*                          TNF - includes                              */
/************************************************************************/

# include <windows.h>
#include "revenge.h"
#include <algorithm>

#ifdef __CLIENT
# include "q_sprites.h"
# include "q_sprites.cpp"

# pragma comment(lib, "user32.lib")

bool CBPaste( ScriptString& str )
{
    HGLOBAL hglb;
    LPTSTR  lptstr;

    if( !IsClipboardFormatAvailable( CF_TEXT ) || !OpenClipboard( NULL ) )
        return false;

    hglb = GetClipboardData( CF_TEXT );
    if( hglb )
    {
        lptstr = (LPTSTR) GlobalLock( hglb );
        if( lptstr )
        {
            str = lptstr;
            GlobalUnlock( lptstr );
        }
    }
    CloseClipboard();
    return str.length() > 0;
}

uint GetHardware()
{
    SYSTEM_INFO siSysInfo;

    // Копируем информацию о железе в структуру SYSTEM_INFO.

    GetSystemInfo( &siSysInfo );
    uint hardwareId = siSysInfo.dwOemId;

    // Отображаем содержимое структуры SYSTEM_INFO.

    /*printf("Hardware information: \n");
       printf("  OEM ID: %u\n", siSysInfo.dwOemId);
       printf("  Number of processors: %u\n",
       siSysInfo.dwNumberOfProcessors);
       printf("  Page size: %u\n", siSysInfo.dwPageSize);
       printf("  Processor type: %u\n", siSysInfo.dwProcessorType);
       printf("  Minimum application address: %lx\n",
       siSysInfo.lpMinimumApplicationAddress);
       printf("  Maximum application address: %lx\n",
       siSysInfo.lpMaximumApplicationAddress);
       printf("  Active processor mask: %u\n",
       siSysInfo.dwActiveProcessorMask);*/

    return hardwareId;
}

#endif

#include "qmap_tools.h"
#include "qmap_tools.cpp"

#ifdef __SERVER
void RunClientScript( Critter* cr, ScriptString* funcName, int p0, int p1, int p2, ScriptString* p3, ScriptArray* p4 )
{
    _asm {
        mov eax, ENGINE_PTR_FUNC_Cl_RunClientScript
        push    p4
        push    p3
        push    p2
        push    p1
        push    p0
        push    funcName
        push    cr
        call eax
        add esp, 0x1C
    }
    return;
}
#endif
#include "AngelScript/scriptevent.cpp"
#include "AngelScript/scripttree.cpp"

/************************************************************************/
/* Initialization                                                       */
/************************************************************************/

// In this functions (DllMain and DllLoad) all global variables is NOT initialized, use FONLINE_DLL_ENTRY instead
#if defined ( FO_WINDOWS )
int __stdcall DllMain( void* module, unsigned long reason, void* reserved ) { return 1; }
#elif defined ( FO_LINUX )
void __attribute__( ( constructor ) ) DllLoad()   {}
void __attribute__( ( destructor ) )  DllUnload() {}
#endif

double StringToFloat( ScriptString& str ){ return ::atof(str.c_str()); }
void ClearLog( ScriptString& log ){ Log( "%s\n", log.c_str() ); }

#ifdef __SERVER

template< typename T >
void script_DataDescriptorSetter( T& engineClass, asIScriptObject* value )
{
	if( *GlobalVars.__StartServerVersion != engineClass.Data.ScriptClass.WorldVersion )
	{
		engineClass.Data.ScriptClass.DescriptorReference = NULL;
		engineClass.Data.ScriptClass.WorldVersion = *GlobalVars.__StartServerVersion;
	}
	
	if(engineClass.Data.ScriptClass.DescriptorReference)
	{
		__try
		{
			engineClass.Data.ScriptClass.DescriptorReference->Release();
		}
		__except( EXCEPTION_EXECUTE_HANDLER )
        {
            engineClass.Data.ScriptClass.DescriptorReference = NULL;
			engineClass.Data.ScriptClass.WorldVersion = *GlobalVars.__StartServerVersion;
			Log( "Exception<script_DataDescriptorSetter> :: DescriptorReference is not valid.\n" );
        }
	}
	
	engineClass.Data.ScriptClass.DescriptorReference = value;
}

template< typename T >
asIScriptObject* script_DataDescriptorGetter( T& engineClass )
{
	if( *GlobalVars.__StartServerVersion != engineClass.Data.ScriptClass.WorldVersion )
	{
		engineClass.Data.ScriptClass.DescriptorReference = NULL;
		engineClass.Data.ScriptClass.WorldVersion = *GlobalVars.__StartServerVersion;
	}
	
	if(engineClass.Data.ScriptClass.DescriptorReference)
	{
		__try
		{
			engineClass.Data.ScriptClass.DescriptorReference->AddRef();
		}
		__except( EXCEPTION_EXECUTE_HANDLER )
        {
            engineClass.Data.ScriptClass.DescriptorReference = NULL;
			engineClass.Data.ScriptClass.WorldVersion = *GlobalVars.__StartServerVersion;
			Log( "Exception<script_DataDescriptorGetter> :: DescriptorReference is not valid.\n" );
			return NULL;
        }
		return engineClass.Data.ScriptClass.DescriptorReference;
	}
    return NULL;
}

asIScriptObject* map_DataDescriptorGetter( Map& engineClass ) { return script_DataDescriptorGetter< Map >( engineClass ); }
void map_DataDescriptorSetter( Map& engineClass, asIScriptObject* value ) { script_DataDescriptorSetter< Map >( engineClass, value ); }

asIScriptObject* location_DataDescriptorGetter( Location& engineClass ) { return script_DataDescriptorGetter< Location >( engineClass ); }
void location_DataDescriptorSetter( Location& engineClass, asIScriptObject* value ) { script_DataDescriptorSetter< Location >( engineClass, value ); }

asIScriptObject* Critter_GetGroupDescriptor( Critter& critter )
{
	if( *GlobalVars.__StartServerVersion != critter.GroupDescriptor.WorldVersion )
	{
		critter.GroupDescriptor.DescriptorReference = NULL;
		critter.GroupDescriptor.WorldVersion = *GlobalVars.__StartServerVersion;
	}
	
	if(critter.GroupDescriptor.DescriptorReference)
	{
		__try
		{
			critter.GroupDescriptor.DescriptorReference->AddRef();
		}
		__except( EXCEPTION_EXECUTE_HANDLER )
        {
            critter.GroupDescriptor.DescriptorReference = NULL;
			critter.GroupDescriptor.WorldVersion = *GlobalVars.__StartServerVersion;
			Log( "Exception<Map_GetScriptDescriptor> :: DescriptorReference is not valid.\n" );
			return NULL;
        }
		return critter.GroupDescriptor.DescriptorReference;
	}
    return NULL;
}

void Critter_SetGroupDescriptor(  Critter& critter, asIScriptObject* value )
{
	if( *GlobalVars.__StartServerVersion != critter.GroupDescriptor.WorldVersion )
	{
		critter.GroupDescriptor.DescriptorReference = NULL;
		critter.GroupDescriptor.WorldVersion = *GlobalVars.__StartServerVersion;
	}
	
	if(critter.GroupDescriptor.DescriptorReference)
	{
		__try
		{
			critter.GroupDescriptor.DescriptorReference->Release();
		}
		__except( EXCEPTION_EXECUTE_HANDLER )
        {
            critter.GroupDescriptor.DescriptorReference = NULL;
			critter.GroupDescriptor.WorldVersion = *GlobalVars.__StartServerVersion;
			Log( "Exception<Map_SetScriptDescriptor> :: DescriptorReference is not valid.\n" );
        }
	}
	
	critter.GroupDescriptor.DescriptorReference = value;
}
#endif

void scriptapi_Exception( ScriptString *message )
{
	asIScriptContext* ctx = ScriptGetActiveContext( );
	if( ctx ) ctx->SetException( message->c_str( ) );
}

FONLINE_DLL_ENTRY( isCompiler )
{	
	ASEngine->SetEngineProperty( asEP_REQUIRE_ENUM_SCOPE, 1 );
	RegisterScriptHandle( ASEngine );
	RegisterScriptFileSystem( );
	EventTree::Register( );
	ScriptTree::Register( );
	// RegisterScriptGrid( );
    #ifdef __CLIENT	
    RegisterNativeSprites( ASEngine, isCompiler );

    ASEngine->RegisterGlobalFunction( "bool CBPaste(string&)", asFUNCTION( CBPaste ), asCALL_CDECL );
    ASEngine->RegisterGlobalFunction( "uint GetHardware()", asFUNCTION( GetHardware ), asCALL_CDECL );
    #endif

    RegisterQmapTools( ASEngine, isCompiler );
	
	ASEngine->RegisterInterface("Descriptor");
	
	/*ASEngine->RegisterObjectBehaviour( "any", asBEHAVE_REF_CAST, "void f(?&out)", asMETHODPR(ScriptAny, Cast, (void *, int), void), asCALL_THISCALL);
	ASEngine->RegisterObjectMethod( "any", "string@ get_TypeName( ) const", asMETHOD(ScriptAny, script_TypeName), asCALL_THISCALL);
	ASEngine->RegisterObjectMethod( "any", "string@ get_TypeNamespace( ) const", asMETHOD(ScriptAny, script_TypeNamespace), asCALL_THISCALL);
	ASEngine->RegisterObjectMethod( "any", "any@+ get_Property( string@ name ) const", asMETHOD(ScriptAny, script_Property), asCALL_THISCALL);
	ASEngine->RegisterObjectMethod( "any", "bool get_IsPrimitive( ) const", asMETHOD(ScriptAny, script_Primitive), asCALL_THISCALL);*/
	
    ASEngine->RegisterGlobalFunction( "void ClearLog( ::string&in )", asFUNCTION( ClearLog ), asCALL_CDECL );
    #ifdef __SERVER
    // ASEngine->RegisterObjectMethod("Critter", "void RunClientScript2(string& funcName, int p0, int p1, int p2, string@+ p3, int[]@+ p4)", asFUNCTION(RunClientScript), asCALL_CDECL_OBJFIRST);
    //ASEngine->RegisterGlobalFunction( "void AddStartCallback(string&, string&)", asFUNCTION( AddStartCallback ), asCALL_CDECL );
    //ASEngine->RegisterGlobalFunction( "void AddInitCallback(string&, string&)", asFUNCTION( AddInitCallback ), asCALL_CDECL );
    //ASEngine->RegisterGlobalFunction( "void CallStartCallbacks()", asFUNCTION( CallStartCallbacks ), asCALL_CDECL );
    //ASEngine->RegisterGlobalFunction( "void CallInitCallbacks()", asFUNCTION( CallInitCallbacks ), asCALL_CDECL );
	
    ASEngine->RegisterGlobalFunction( "void Exception( string@+ )", asFUNCTION( scriptapi_Exception ), asCALL_CDECL );
	
	if( ASEngine->RegisterObjectProperty( "Critter", "uint16 KindIndex", offsetof( Critter, KindIndex ) ) < 0 )   Log( "Error register property: uint Critter::KindIndex\n" );
	if( ASEngine->RegisterObjectProperty( "Critter", "uint16 KindQIndex", offsetof( Critter, KindQIndex ) ) < 0 )   Log( "Error register property: uint Critter::KindQIndex\n" );	
	if( ASEngine->RegisterObjectProperty( "Critter", "int KindHierarchy", offsetof( Critter, KindHierarchy ) ) < 0 )   Log( "Error register property: uint Critter::KindHierarchy\n" );	
	if( ASEngine->RegisterObjectProperty( "Critter", "uint16 LastWorldX", offsetof( Critter, LastWorldX ) ) < 0 )   Log( "Error register property: uint16 Critter::LastWorldX\n" );	
	if( ASEngine->RegisterObjectProperty( "Critter", "uint16 LastWorldY", offsetof( Critter, LastWorldY ) ) < 0 )   Log( "Error register property: uint16 Critter::LastWorldY\n" );	
	if( ASEngine->RegisterObjectProperty( "Critter", "const uint16 LastHexX", offsetof( Critter, LastHexX ) ) < 0 )   Log( "Error register property: uint16 Critter::LastHexX\n" );	
	if( ASEngine->RegisterObjectProperty( "Critter", "const uint16 LastHexY", offsetof( Critter, LastHexY ) ) < 0 )   Log( "Error register property: uint16 Critter::LastHexY\n" );	
	
	if( ASEngine->RegisterObjectMethod( "Map", "uint CountPlayers()", asMETHOD( Map, CountPlayers ), asCALL_THISCALL ) < 0 ) Log( "Error register: uint Map::CountPlayers\n" );	
	if( ASEngine->RegisterObjectMethod( "Map", "uint CountNpc()", asMETHOD( Map, CountNpc ), asCALL_THISCALL ) < 0 ) Log( "Error register: uint Map::CountNpc\n" );	
	if( ASEngine->RegisterObjectMethod( "Map", "uint CountCritters()", asMETHOD( Map, CountCritters ), asCALL_THISCALL ) < 0 ) Log( "Error register: uint Map::CountCritters\n" );
	
	
	if( ASEngine->RegisterObjectMethod( "Map", "void set_Index( uint value )", asMETHOD( Map, script_SetIndex ), asCALL_THISCALL ) < 0 ) Log( "Error register: uint Map::script_SetIndex\n" );
	if( ASEngine->RegisterObjectMethod( "Map", "uint get_Index( )", asMETHOD( Map, script_GetIndex ), asCALL_THISCALL ) < 0 ) Log( "Error register: uint Map::script_GetIndex\n" );
	
	if( ASEngine->RegisterObjectProperty( "Scenery", "uint UID", offsetof( MapObject, UID ) ) < 0 )
        Log( "uint Scenery::UID\n" );
	if( ASEngine->RegisterObjectMethod( "Scenery", "int get_Param( uint index ) const", asMETHOD( MapObject, SceneryGetParam ), asCALL_THISCALL ) < 0 )
        Log( "Error registering Scenery::get_Param( uint )\n" );
	if( ASEngine->RegisterObjectMethod( "string", "float ToFloat( )", asFUNCTION( StringToFloat ), asCALL_CDECL_OBJFIRST ) < 0 )
        Log( "Error registering string::ToFloat( )\n" );
	

	if( ASEngine->RegisterObjectMethod( "Critter", "void set_GroupDescriptor( Descriptor@ value )", asFUNCTION( Critter_SetGroupDescriptor ), asCALL_CDECL_OBJFIRST ) < 0 )
        Log( "Error registering Critter::set_GroupDescriptor()\n" );
    if( ASEngine->RegisterObjectMethod( "Critter", "Descriptor@ get_GroupDescriptor()", asFUNCTION( Critter_GetGroupDescriptor ), asCALL_CDECL_OBJFIRST ) < 0 )
        Log( "Error registering Critter::get_GroupDescriptor()\n" );

	if( ASEngine->RegisterObjectMethod( "Map", "void set_ScriptDescriptor( Descriptor@ value )", asFUNCTION( map_DataDescriptorSetter ), asCALL_CDECL_OBJFIRST ) < 0 )
        Log( "Error registering Map::set_ScriptDescriptor()\n" );
    if( ASEngine->RegisterObjectMethod( "Map", "Descriptor@ get_ScriptDescriptor()", asFUNCTION( map_DataDescriptorGetter ), asCALL_CDECL_OBJFIRST ) < 0 )
        Log( "Error registering Map::get_ScriptDescriptor()\n" );

	if( ASEngine->RegisterObjectMethod( "Location", "uint get_HashStaticName( ) const", asMETHOD( Location, script_GetHashStaticName ), asCALL_THISCALL ) < 0 )
        Log( "Error registering Location::get_HashStaticName()\n" );
	if( ASEngine->RegisterObjectMethod( "Location", "void set_HashStaticName( uint )", asMETHOD( Location, script_SetHashStaticName ), asCALL_THISCALL ) < 0 )
        Log( "Error registering Location::set_HashStaticName()\n" );
	
	if( ASEngine->RegisterObjectMethod( "Location", "void set_ScriptDescriptor( Descriptor@ value )", asFUNCTION( location_DataDescriptorSetter ), asCALL_CDECL_OBJFIRST ) < 0 )
        Log( "Error registering Location::set_ScriptDescriptor()\n" );
    if( ASEngine->RegisterObjectMethod( "Location", "Descriptor@ get_ScriptDescriptor()", asFUNCTION( location_DataDescriptorGetter ), asCALL_CDECL_OBJFIRST ) < 0 )
        Log( "Error registering Location::get_ScriptDescriptor()\n" );

    if( ASEngine->RegisterObjectMethod( "Map", "string@+ get_Name() const", asMETHOD( Map, script_GetName ), asCALL_THISCALL )  < 0 )
        Log( "Error registering Map::script_GetName()\n" );
    if( ASEngine->RegisterObjectMethod( "Map", "uint GetEventFuncId( uint index ) const", asMETHOD( Map, GetEventFuncId ), asCALL_THISCALL )  < 0 )
        Log( "Error registering Map::GetEventFuncId( uint )\n" );
	
    #endif

    if( isCompiler )
        return;

    // Register callbacks
    FOnline->GetUseApCost = &GetUseApCost;
    FOnline->GetAttackDistantion = &GetAttackDistantion;

    // Register script global vars
    memset( &GlobalVars, 0, sizeof( GlobalVars ) );
    for( asUINT i = 0; i < ASEngine->GetGlobalPropertyCount(); i++ )
    {
        const char* name;
        void*       ptr;
        if( ASEngine->GetGlobalPropertyByIndex( i, &name, NULL, NULL, NULL, NULL, &ptr ) < 0 )
            continue;

        #define REGISTER_GLOBAL_VAR( type, gvar ) \
            else if( !strcmp( # gvar, name ) )    \
                GlobalVars.gvar = (type*) ptr
        REGISTER_GLOBAL_VAR( int, __CurX );
        REGISTER_GLOBAL_VAR( int, __CurY );
        REGISTER_GLOBAL_VAR( uint, __HitAimEyes );
        REGISTER_GLOBAL_VAR( uint, __HitAimHead );
        REGISTER_GLOBAL_VAR( uint, __HitAimGroin );
        REGISTER_GLOBAL_VAR( uint, __HitAimTorso );
        REGISTER_GLOBAL_VAR( uint, __HitAimArms );
        REGISTER_GLOBAL_VAR( uint, __HitAimLegs );
        REGISTER_GLOBAL_VAR( bool, __Zombies );
#ifdef __SERVER
        REGISTER_GLOBAL_VAR( uint, __StartServerVersion );
#endif
    }
}

/************************************************************************/
/* Slot/parameters allowing                                             */
/************************************************************************/

EXPORT bool allowSlot_Hand1( uint8, Item&, Critter&, Critter& toCr )
{
    return toCr.Params[ PE_AWARENESS ] != 0;
}

/************************************************************************/
/* Parameters Get behaviors                                             */
/************************************************************************/

EXPORT int getParam_Strength( CritterMutual& cr, uint )
{
    int val = cr.Params[ ST_STRENGTH ] + cr.Params[ ST_STRENGTH_EXT ];
    if( cr.Params[ PE_ADRENALINE_RUSH ] && getParam_Timeout( cr, TO_BATTLE ) && // Adrenaline rush perk
        cr.Params[ ST_CURRENT_HP ] <= ( cr.Params[ ST_MAX_LIFE ] + cr.Params[ ST_STRENGTH ] + cr.Params[ ST_ENDURANCE ] * 2 ) / 2 )
        val++;
    return CLAMP( val, 1, 10 );
}

EXPORT int getParam_Perception( CritterMutual& cr, uint )
{
    int val = ( cr.Params[ DAMAGE_EYE ] ? 1 : cr.Params[ ST_PERCEPTION ] + cr.Params[ ST_PERCEPTION_EXT ] );
    if( cr.Params[ TRAIT_NIGHT_PERSON ] )
        val += GetNightPersonBonus();
    return CLAMP( val, 1, 10 );
}

EXPORT int getParam_Endurance( CritterMutual& cr, uint )
{
    int val = cr.Params[ ST_ENDURANCE ] + cr.Params[ ST_ENDURANCE_EXT ];
    return CLAMP( val, 1, 10 );
}

EXPORT int getParam_Charisma( CritterMutual& cr, uint )
{
    int val = cr.Params[ ST_CHARISMA ] + cr.Params[ ST_CHARISMA_EXT ];
    return CLAMP( val, 1, 10 );
}

EXPORT int getParam_Intellegence( CritterMutual& cr, uint )
{
    int val = cr.Params[ ST_INTELLECT ] + cr.Params[ ST_INTELLECT_EXT ];
    if( cr.Params[ TRAIT_NIGHT_PERSON ] )
        val += GetNightPersonBonus();
    return CLAMP( val, 1, 10 );
}

EXPORT int getParam_Agility( CritterMutual& cr, uint )
{
    int val = cr.Params[ ST_AGILITY ] + cr.Params[ ST_AGILITY_EXT ];
    return CLAMP( val, 1, 10 );
}

EXPORT int getParam_Luck( CritterMutual& cr, uint )
{
    int val = cr.Params[ ST_LUCK ] + cr.Params[ ST_LUCK_EXT ];
    return CLAMP( val, 1, 10 );
}

EXPORT int getParam_Hp( CritterMutual& cr, uint )
{
    return cr.Params[ ST_CURRENT_HP ];
}

/* from TLA
   EXPORT int getParam_MaxLife(CritterMutual& cr, uint)
   {
        int val = cr.Params[ST_MAX_LIFE] + cr.Params[ST_MAX_LIFE_EXT] + cr.Params[ST_STRENGTH] + cr.Params[ST_ENDURANCE] * 2;
        return CLAMP(val, 1, 9999);
   }*/

EXPORT int getParam_MaxLife( CritterMutual& cr, uint )
{
    // int val = cr.Params[ST_MAX_LIFE] + cr.Params[ST_MAX_LIFE_EXT] + cr.Params[ST_STRENGTH] + cr.Params[ST_ENDURANCE] * 2;
    int val = cr.Params[ ST_MAX_LIFE ] + cr.Params[ ST_MAX_LIFE_EXT ] + cr.Params[ ST_STRENGTH ] * 4 + cr.Params[ ST_ENDURANCE ] * 8; // Roleplay
    return CLAMP( val, 1, 9999 );
}

EXPORT int getParam_MaxAp( CritterMutual& cr, uint )
{
    int val = cr.Params[ ST_ACTION_POINTS ] + cr.Params[ ST_ACTION_POINTS_EXT ] + getParam_Agility( cr, 0 ) / 2;
    return CLAMP( val, 1, 9999 );
}

EXPORT int getParam_Ap( CritterMutual& cr, uint )
{
    int val = cr.Params[ ST_CURRENT_AP ];
    val /= AP_DIVIDER;
    return CLAMP( val, -9999, 9999 );
}

EXPORT int getParam_MaxMoveAp( CritterMutual& cr, uint )
{
    int val = cr.Params[ ST_MAX_MOVE_AP ];
    return CLAMP( val, 0, 9999 );
}

EXPORT int getParam_MoveAp( CritterMutual& cr, uint )
{
    int val = cr.Params[ ST_MOVE_AP ];
    return CLAMP( val, 0, 9999 );
}

/* from TLA
   EXPORT int getParam_MaxWeight(CritterMutual& cr, uint)
   {
        int val = max(cr.Params[ST_CARRY_WEIGHT] + cr.Params[ST_CARRY_WEIGHT_EXT], 0);
        val += CONVERT_GRAMM(25 + getParam_Strength(cr, 0) * (25 - cr.Params[TRAIT_SMALL_FRAME] * 10));
        return CLAMP(val, 0, 2000000000);
   }*/

EXPORT int getParam_MaxWeight( CritterMutual& cr, uint )
{
    int val = cr.Params[ ST_CARRY_WEIGHT ] + cr.Params[ ST_CARRY_WEIGHT_EXT ];
    val +=  ( getParam_Strength( cr, 0 ) * 10 + ( cr.Params[ TRAIT_SMALL_FRAME ] ? 0 : 20 ) ) * 1000;

/*	//Рассчет скоростей на ходу.
   #ifdef __SERVER
        if(FOnline->GameTimeTick < cr.PrevHexTick+2000)
        {
                int weight = cr.GetItemsWeight();

                int oldWalk = cr.Params[MODE_NO_WALK],
                walk = 0,
                oldRun = cr.Params[MODE_NO_RUN],
                run = 0;

                if(weight>val) walk=1;
                if(weight*2>val) run=1;

                if(walk!=oldWalk)
                {
                        cr.Params[MODE_NO_WALK]=walk;
                        cr.ParamsChanged.push_back(MODE_NO_WALK);
                        cr.ParamsIsChanged[MODE_NO_WALK]=true;
                        FOnline->CritterChangeParameter(cr, MODE_NO_WALK);
                }
                if(run!=oldRun)
                {
                        cr.Params[MODE_NO_RUN]=run;
                        cr.ParamsChanged.push_back(MODE_NO_RUN);
                        cr.ParamsIsChanged[MODE_NO_RUN]=true;
                        FOnline->CritterChangeParameter(cr, MODE_NO_RUN);
                }
        }
   #endif
 */
    return CLAMP( val, 0, 2000000000 );
}

EXPORT int getParam_Sequence( CritterMutual& cr, uint )
{
    int val = cr.Params[ ST_SEQUENCE ] + cr.Params[ ST_SEQUENCE_EXT ] + getParam_Perception( cr, 0 ) * 2;
    return CLAMP( val, 0, 9999 );
}

EXPORT int getParam_MeleeDmg( CritterMutual& cr, uint )
{
    int strength = getParam_Strength( cr, 0 );
    int val = cr.Params[ ST_MELEE_DAMAGE ] + cr.Params[ ST_MELEE_DAMAGE_EXT ] + ( strength > 6 ? strength - 5 : 1 );
    return CLAMP( val, 1, 9999 );
}

EXPORT int getParam_HealingRate( CritterMutual& cr, uint )
{
    int e = getParam_Endurance( cr, 0 );
    int val = cr.Params[ ST_HEALING_RATE ] + cr.Params[ ST_HEALING_RATE_EXT ] + max( 1, e / 3 );
    return CLAMP( val, 0, 9999 );
}

EXPORT int getParam_CriticalChance( CritterMutual& cr, uint )
{
    int val = cr.Params[ ST_CRITICAL_CHANCE ] + cr.Params[ ST_CRITICAL_CHANCE_EXT ] + getParam_Luck( cr, 0 );
    return CLAMP( val, 0, 100 );
}

EXPORT int getParam_MaxCritical( CritterMutual& cr, uint )
{
    int val = cr.Params[ ST_MAX_CRITICAL ] + cr.Params[ ST_MAX_CRITICAL_EXT ];
    return CLAMP( val, -100, 100 );
}

/* from TLA
   EXPORT int getParam_Ac(CritterMutual& cr, uint)
   {
        int val = cr.Params[ST_ARMOR_CLASS] + cr.Params[ST_ARMOR_CLASS_EXT] + getParam_Agility(cr, 0) + cr.Params[ST_TURN_BASED_AC];
        Item* armor = cr.ItemSlotArmor;
        if(armor->GetId() && armor->IsArmor()) val += armor->Proto->Armor_AC * (100 - armor->GetDeteriorationProc()) / 100;
        return CLAMP(val, 0, 90);
   }*/

EXPORT int getParam_Ac( CritterMutual& cr, uint )
{
    // int val = cr.Params[ST_ARMOR_CLASS] + cr.Params[ST_ARMOR_CLASS_EXT] + getParam_Agility(cr, 0) + cr.Params[ST_TURN_BASED_AC]; //TLA
    int         val = cr.Params[ ST_ARMOR_CLASS ] + cr.Params[ ST_ARMOR_CLASS_EXT ] + ( getParam_Agility( cr, 0 ) * 5 ) + cr.Params[ ST_TURN_BASED_AC ]; // Roleplay
    const Item* armor = cr.ItemSlotArmor;
    // if(armor->GetId() && armor->IsArmor()) val += armor->Proto->Armor_AC * (100 - armor->GetWearProc()) / 100; //TLA
    if( armor->GetId() && armor->IsArmor() )
        val -= armor->Proto->Armor_AC;                                        // Roleplay
    return CLAMP( val, 0, 90 );
}

EXPORT int getParam_DamageResistance( CritterMutual& cr, uint index )
{
    int         dmgType = index - ST_NORMAL_RESIST + 1;

    const Item* armor = cr.ItemSlotArmor;
    int         val = 0;
    int         drVal = 0;
    switch( dmgType )
    {
    case DAMAGE_NORMAL:
        val = cr.Params[ ST_NORMAL_RESIST ]  + cr.Params[ ST_NORMAL_RESIST_EXT ];
        drVal = armor->Proto->Armor_DRNormal;
        break;
    case DAMAGE_LASER:
        val = cr.Params[ ST_LASER_RESIST ]   + cr.Params[ ST_LASER_RESIST_EXT ];
        drVal = armor->Proto->Armor_DRLaser;
        break;
    case DAMAGE_FIRE:
        val = cr.Params[ ST_FIRE_RESIST ]    + cr.Params[ ST_FIRE_RESIST_EXT ];
        drVal = armor->Proto->Armor_DRFire;
        break;
    case DAMAGE_PLASMA:
        val = cr.Params[ ST_PLASMA_RESIST ]  + cr.Params[ ST_PLASMA_RESIST_EXT ];
        drVal = armor->Proto->Armor_DRPlasma;
        break;
    case DAMAGE_ELECTR:
        val = cr.Params[ ST_ELECTRO_RESIST ] + cr.Params[ ST_ELECTRO_RESIST_EXT ];
        drVal = armor->Proto->Armor_DRElectr;
        break;
    case DAMAGE_EMP:
        val = cr.Params[ ST_EMP_RESIST ]     + cr.Params[ ST_EMP_RESIST_EXT ];
        drVal = armor->Proto->Armor_DREmp;
        break;
    case DAMAGE_EXPLODE:
        val = cr.Params[ ST_EXPLODE_RESIST ] + cr.Params[ ST_EXPLODE_RESIST_EXT ];
        drVal = armor->Proto->Armor_DRExplode;
        break;
    case DAMAGE_UNCALLED:
    default:
        break;
    }

    if( armor->GetId() && armor->IsArmor() )
        val += drVal * ( 100 - armor->GetDeteriorationProc() ) / 100;

    if( dmgType == DAMAGE_EMP )
        return CLAMP( val, 0, 999 );
    return CLAMP( val, 0, 90 );
}

EXPORT int getParam_DamageThreshold( CritterMutual& cr, uint index )
{
    int         dmgType = index - ST_NORMAL_ABSORB + 1;

    const Item* armor = cr.ItemSlotArmor;
    int         val = 0;
    int         dtVal = 0;
    switch( dmgType )
    {
    case DAMAGE_NORMAL:
        val = cr.Params[ ST_NORMAL_ABSORB ]  + cr.Params[ ST_NORMAL_ABSORB_EXT ];
        dtVal = armor->Proto->Armor_DTNormal;
        break;
    case DAMAGE_LASER:
        val = cr.Params[ ST_LASER_ABSORB ]   + cr.Params[ ST_LASER_ABSORB_EXT ];
        dtVal = armor->Proto->Armor_DTLaser;
        break;
    case DAMAGE_FIRE:
        val = cr.Params[ ST_FIRE_ABSORB ]    + cr.Params[ ST_FIRE_ABSORB_EXT ];
        dtVal = armor->Proto->Armor_DTFire;
        break;
    case DAMAGE_PLASMA:
        val = cr.Params[ ST_PLASMA_ABSORB ]  + cr.Params[ ST_PLASMA_ABSORB_EXT ];
        dtVal = armor->Proto->Armor_DTPlasma;
        break;
    case DAMAGE_ELECTR:
        val = cr.Params[ ST_ELECTRO_ABSORB ] + cr.Params[ ST_ELECTRO_ABSORB_EXT ];
        dtVal = armor->Proto->Armor_DTElectr;
        break;
    case DAMAGE_EMP:
        val = cr.Params[ ST_EMP_ABSORB ]     + cr.Params[ ST_EMP_ABSORB_EXT ];
        dtVal = armor->Proto->Armor_DTEmp;
        break;
    case DAMAGE_EXPLODE:
        val = cr.Params[ ST_EXPLODE_ABSORB ] + cr.Params[ ST_EXPLODE_ABSORB_EXT ];
        dtVal = armor->Proto->Armor_DTExplode;
        break;
    case DAMAGE_UNCALLED:
    default:
        break;
    }

    if( armor->GetId() && armor->IsArmor() )
        val += dtVal * ( 100 - armor->GetDeteriorationProc() ) / 100;

    return CLAMP( val, 0, 999 );
}

EXPORT int getParam_RadiationResist( CritterMutual& cr, uint )
{
    int val = cr.Params[ ST_RADIATION_RESISTANCE ] + cr.Params[ ST_RADIATION_RESISTANCE_EXT ] + getParam_Endurance( cr, 0 ) * 2;
    return CLAMP( val, 0, 95 );
}

EXPORT int getParam_PoisonResist( CritterMutual& cr, uint )
{
    int val = cr.Params[ ST_POISON_RESISTANCE ] + cr.Params[ ST_POISON_RESISTANCE_EXT ] + getParam_Endurance( cr, 0 ) * 5;
    return CLAMP( val, 0, 95 );
}

EXPORT int getParam_Timeout( CritterMutual& cr, uint index )
{
    return (uint) cr.Params[ index ] > FOnline->FullSecond ? (uint) cr.Params[ index ] - FOnline->FullSecond : 0;
}

EXPORT int getParam_Reputation( CritterMutual& cr, uint index )
{
    #ifdef __SERVER
    if( cr.Params[ index ] == 0x80000000 )
    {
        FOnline->CritterChangeParameter( cr, index );
        const_cast< int* >( cr.Params )[ index ] = 0;
    }
    #else
    if( cr.Params[ index ] == 0x80000000 )
        return 0;
    #endif
    return cr.Params[ index ];
}

EXPORT void changedParam_Reputation( CritterMutual& cr, uint index, int oldValue )
{
    if( oldValue == 0x80000000 )
        const_cast< int* >( cr.Params )[ index ] += 0x80000000;
}

/************************************************************************/
/* Extended methods                                                     */
/************************************************************************/

EXPORT bool Critter_IsInjured( CritterMutual& cr )
{
    return Critter_IsDmgArm( cr ) || Critter_IsDmgLeg( cr ) || Critter_IsDmgEye( cr );
}

EXPORT bool Critter_IsDmgEye( CritterMutual& cr )
{
    return cr.Params[ DAMAGE_EYE ] != 0;
}

EXPORT bool Critter_IsDmgLeg( CritterMutual& cr )
{
    return cr.Params[ DAMAGE_RIGHT_LEG ] || cr.Params[ DAMAGE_LEFT_LEG ];
}

EXPORT bool Critter_IsDmgTwoLeg( CritterMutual& cr )
{
    return cr.Params[ DAMAGE_RIGHT_LEG ] && cr.Params[ DAMAGE_LEFT_LEG ];
}

EXPORT bool Critter_IsDmgArm( CritterMutual& cr )
{
    return cr.Params[ DAMAGE_RIGHT_ARM ] || cr.Params[ DAMAGE_LEFT_ARM ];
}

EXPORT bool Critter_IsDmgTwoArm( CritterMutual& cr )
{
    return cr.Params[ DAMAGE_RIGHT_ARM ] && cr.Params[ DAMAGE_LEFT_ARM ];
}

EXPORT bool Critter_IsAddicted( CritterMutual& cr )
{
    for( uint i = ADDICTION_BEGIN; i <= ADDICTION_END; i++ )
        if( cr.Params[ i ] )
            return true;
    return false;
}

EXPORT bool Critter_IsOverweight( CritterMutual& cr )
{
    // Calculate inventory items weight
    uint w = 0;
    for( ItemVecIt it = cr.InvItems.begin(), end = cr.InvItems.end(); it != end; ++it )
        w += ( *it )->GetWeight();

    return w > (uint) getParam_MaxWeight( cr, 0 );
}

EXPORT bool Item_Weapon_IsHtHAttack( Item& item, uint8 mode )
{
    if( !item.IsWeapon() || !item.WeapIsUseAviable( mode & 7 ) )
        return false;
    int skill = SKILL_OFFSET( item.Proto->Weapon_Skill[ mode & 7 ] );
    return skill == SK_UNARMED || skill == SK_MELEE_WEAPONS;
}

EXPORT bool Item_Weapon_IsGunAttack( Item& item, uint8 mode )
{
    if( !item.IsWeapon() || !item.WeapIsUseAviable( mode & 7 ) )
        return false;
    int skill = SKILL_OFFSET( item.Proto->Weapon_Skill[ mode & 7 ] );
    return skill == SK_SMALL_GUNS || skill == SK_BIG_GUNS || skill == SK_ENERGY_WEAPONS;
}

EXPORT bool Item_Weapon_IsRangedAttack( Item& item, uint8 mode )
{
    if( !item.IsWeapon() || !item.WeapIsUseAviable( mode & 7 ) )
        return false;
    int skill = SKILL_OFFSET( item.Proto->Weapon_Skill[ mode & 7 ] );
    return skill == SK_SMALL_GUNS || skill == SK_BIG_GUNS || skill == SK_ENERGY_WEAPONS || skill == SK_THROWING;
}

/************************************************************************/
/* Callbacks                                                            */
/************************************************************************/

uint GetUseApCost( CritterMutual& cr, Item& item, uint8 mode )
{
    uint8 use = mode & 0xF;
    uint8 aim = mode >> 4;
    int   apCost = 1;

    if( use == USE_USE )
    {
        if( TB_BATTLE_TIMEOUT_CHECK( getParam_Timeout( cr, TO_BATTLE ) ) )
            apCost = FOnline->TbApCostUseItem;
        else
            apCost = FOnline->RtApCostUseItem;
    }
    else if( use == USE_RELOAD )
    {
        if( TB_BATTLE_TIMEOUT_CHECK( getParam_Timeout( cr, TO_BATTLE ) ) )
            apCost = FOnline->TbApCostReloadWeapon;
        else
            apCost = FOnline->RtApCostReloadWeapon;

        if( item.IsWeapon() && item.Proto->Weapon_Perk == WEAPON_PERK_FAST_RELOAD )
            apCost--;
    }
    else if( use >= USE_PRIMARY && use <= USE_THIRD && item.IsWeapon() )
    {
        int  skill = item.Proto->Weapon_Skill[ use ];
        bool hthAttack = Item_Weapon_IsHtHAttack( item, mode );
        bool rangedAttack = Item_Weapon_IsRangedAttack( item, mode );

        apCost = item.Proto->Weapon_ApCost[ use ];
        if( aim )
            apCost += GetAimApCost( aim );
        if( hthAttack && cr.Params[ PE_BONUS_HTH_ATTACKS ] )
            apCost--;
        if( rangedAttack && cr.Params[ PE_BONUS_RATE_OF_FIRE ] )
            apCost--;
        if( cr.Params[ TRAIT_FAST_SHOT ] && !hthAttack )
            apCost--;
    }

    if( apCost < 1 )
        apCost = 1;
    return apCost;
}

uint GetAttackDistantion( CritterMutual& cr, Item& item, uint8 mode )
{
    uint8 use = mode & 0xF;
    int   dist = item.Proto->Weapon_MaxDist[ use ];
    int   strength = getParam_Strength( cr, 0 );
    if( item.Proto->Weapon_Skill[ use ] == SKILL_OFFSET( SK_THROWING ) )
        dist = min( dist, 3 * min( 10, strength + 2 * cr.Params[ PE_HEAVE_HO ] ) );
    if( Item_Weapon_IsHtHAttack( item, mode ) && cr.Params[ MODE_RANGE_HTH ] )
        dist++;
    dist += GetMultihex( cr );
    if( dist < 0 )
        dist = 0;
    return dist;
}

/************************************************************************/
/* Generic stuff                                                        */
/************************************************************************/

int GetNightPersonBonus()
{
    if( FOnline->Hour < 6 || FOnline->Hour > 18 )
        return 1;
    if( FOnline->Hour == 6 && FOnline->Minute == 0 )
        return 1;
    if( FOnline->Hour == 18 && FOnline->Minute > 0 )
        return 1;
    return -1;
}

uint GetAimApCost( int hitLocation )
{
    switch( hitLocation )
    {
    case HIT_LOCATION_TORSO:
        return FOnline->ApCostAimTorso;
    case HIT_LOCATION_EYES:
        return FOnline->ApCostAimEyes;
    case HIT_LOCATION_HEAD:
        return FOnline->ApCostAimHead;
    case HIT_LOCATION_LEFT_ARM:
    case HIT_LOCATION_RIGHT_ARM:
        return FOnline->ApCostAimArms;
    case HIT_LOCATION_GROIN:
        return FOnline->ApCostAimGroin;
    case HIT_LOCATION_RIGHT_LEG:
    case HIT_LOCATION_LEFT_LEG:
        return FOnline->ApCostAimLegs;
    case HIT_LOCATION_NONE:
    case HIT_LOCATION_UNCALLED:
    default:
        break;
    }
    return 0;
}

uint GetAimHit( int hitLocation )
{
    switch( hitLocation )
    {
    case HIT_LOCATION_TORSO:
        return *GlobalVars.__HitAimTorso;
    case HIT_LOCATION_EYES:
        return *GlobalVars.__HitAimEyes;
    case HIT_LOCATION_HEAD:
        return *GlobalVars.__HitAimHead;
    case HIT_LOCATION_LEFT_ARM:
    case HIT_LOCATION_RIGHT_ARM:
        return *GlobalVars.__HitAimArms;
    case HIT_LOCATION_GROIN:
        return *GlobalVars.__HitAimGroin;
    case HIT_LOCATION_RIGHT_LEG:
    case HIT_LOCATION_LEFT_LEG:
        return *GlobalVars.__HitAimLegs;
    case HIT_LOCATION_NONE:
    case HIT_LOCATION_UNCALLED:
    default:
        break;
    }
    return 0;
}

uint GetMultihex( CritterMutual& cr )
{
    int mh = cr.Multihex;
    if( mh < 0 )
        mh = FOnline->CritterTypes[ cr.BaseType ].Multihex;
    return CLAMP( mh, 0, MAX_HEX_OFFSET );
}


/************************************************************************/
/*                         TNF - common                                 */
/************************************************************************/

EXPORT void Item_SetInvPic( Item& item, uint hash )
{
    const_cast< uint& >( item.Data.PicInvHash ) = hash;
    // item.Update();
    return;
}

EXPORT void Item_SetMapPic( Item& item, uint hash )
{
    const_cast< uint& >( item.Data.PicMapHash ) = hash;
    return;
}

/************************************************************************/
/*                         TNF - server                                 */
/************************************************************************/

#ifdef __SERVER
/*
EXPORT bool Critter_SetAccess(Critter& cr, int access)
   {
        if(cr.CritterIsNpc) return false;
        Client* cl = (Client*)&cr;

        cl->Access = 1 << (access);
        return true;
   }*/

uint GetTiles( Map& map, uint16 hexX, uint16 hexY, bool is_roof, vector< uint >& finded )
{
    ProtoMap::TileVec& tiles = const_cast< ProtoMap::TileVec& >( map.Proto->Tiles );

    for( uint i = 0, j = tiles.size(); i < j; i++ )
    {
        if( tiles[ i ].HexX != hexX || tiles[ i ].HexY != hexY || tiles[ i ].IsRoof != is_roof )
            continue;
        finded.push_back( tiles[ i ].NameHash );
    }
    return finded.size();
}

EXPORT uint Map_GetTiles( Map& map, uint16 hexX, uint16 hexY, bool is_roof, ScriptArray& array )
{
    vector< uint > finded;

    if( array.GetElementSize() != 4 )
        return 0;

    uint delta = GetTiles( map, hexX, hexY, is_roof, finded );
    if( delta == 0 )
        return 0;

    uint size = array.GetSize();
    array.Grow( delta );
    memcpy( array.GetBuffer() + size * 4, &( finded[ 0 ] ), delta * 4 );

    return delta;
}

EXPORT uint Map_GetTile( Map& map, uint16 tx, uint16 ty )
{
    vector< uint > finded;
	tx *= 2;
	ty *= 2;
	// Для справки, принято считать что тайл фаллаута занимает квадрат в 2х2 гекса, таким образом для того чтобы удостоверится что тайл не найден не из-за косяка мапперов нужно проверять все четыре гекса.
    if( GetTiles( map, tx, ty, false, finded ) == 0 )
		if( GetTiles( map, tx + 1, ty, false, finded ) == 0 )
			if( GetTiles( map, tx, ty + 1, false, finded ) == 0 )
				if( GetTiles( map, tx + 1, ty + 1, false, finded ) == 0 )
					return 0;
    return finded[ 0 ];
}

EXPORT uint Map_GetRoof( Map& map, uint16 tx, uint16 ty )
{
    // if(map.IsNotValid) return 0;
    // ProtoMap* pMap = map.Proto;
    // if(pMap->(Header.MaxHexX/2)<tx || pMap->(Header.MaxHexY/2)<ty) return 0;
    // return pMap->GetRoof(tx, ty);

    vector< uint > finded;

    if( GetTiles( map, tx * 2, ty * 2, true, finded ) != 1 )
        return 0;

    return finded[ 0 ];
}

EXPORT bool Map_SetTile( Map& map, uint16 tx, uint16 ty, uint picHash )
{
    // if(map.IsNotValid) return 0;
    // ProtoMap* pMap = map.Proto;
    // if(pMap->(Header.MaxHexX/2)<tx || pMap->(Header.MaxHexY/2)<ty) return 0;
    // pMap->SetTile(tx, ty, picHash);
    // return true;
    return false;
}

EXPORT bool Map_SetRoof( Map& map, uint16 tx, uint16 ty, uint picHash )
{
    // if(map.IsNotValid) return 0;
    // ProtoMap* pMap = map.Proto;
    // if(pMap->(Header.MaxHexX/2)<tx || pMap->(Header.MaxHexY/2)<ty) return 0;
    // pMap->SetRoof(tx, ty, picHash);
    // return true;
    return false;
}

EXPORT uint Critter_GetItemTransferCount( Critter& cr )
{
    return cr.ItemTransferCount;
}

EXPORT void Critter_GetIp( Critter& cr, ScriptArray* array )
{
    array->Resize( MAX_STORED_IP );
    const uint* p = cr.DataExt->PlayIp;
    memcpy( array->GetBuffer(), p, MAX_STORED_IP * 4 );
}

// pm added

EXPORT void Critter_SetWorldPos( CritterMutual& cr, uint16 x, uint16 y ) // pm added
{
    const_cast< uint16& >( cr.WorldX ) = x;
    const_cast< uint16& >( cr.WorldY ) = y;
	
	const_cast< uint16& >( cr.LastWorldX ) = x;
	const_cast< uint16& >( cr.LastWorldY ) = y;
}

/*EXPORT uint Item_GetDurability(Item& item)
   {
        return item.Durability;
   }

   EXPORT void Item_SetDurability(Item& item, uint8 value)
   {
        item.Durability = value;
        return;
   }*/

EXPORT int Map_GetScenParam( Map& map, uint16 tx, uint16 ty, uint protoId, uint8 num )
{
    if( num < 0 || num > 10 )
        return -1;

    const MapObjectVec& obj = map.Proto->SceneriesVec;

    for( MapObjectVecIt it = obj.begin(); it < obj.end(); ++it )
    {
        if( ( *it )->MapX != tx || ( *it )->MapY != ty || ( *it )->ProtoId != protoId )
            continue;
        return ( *it )->MScenery.Param[ num ];
    }

    return -1;
}

EXPORT void Map_GetTransferParams( Map& map, uint16& startx, uint16& starty, uint16& endx, uint16& endy, uint protoId )
{
    const MapObjectVec& obj = map.Proto->SceneriesVec;

    for( MapObjectVecIt it = obj.begin(); it < obj.end(); ++it )
    {
		if( ( *it )->ProtoId == protoId )
		{
			if( ( ( *it )->MapX < startx || startx == 0 ) )
				startx = ( *it )->MapX;
			
			if( ( ( *it )->MapX > endx || endx == 0 ) )
				endx = ( *it )->MapX;
			
			if( ( ( *it )->MapY < starty || starty == 0 ) )
				starty = ( *it )->MapY;
			
			if( ( ( *it )->MapY > endy || endy == 0 ) )
				endy = ( *it )->MapY;
		}
    }
}
/*
   EXPORT int Map_GetLightValue(Map& map, uint8[] light)
   {
        return 0;
   }*/


/*
   EXPORT uint8 Item_GetDurability( Item& item )
   {
    return item.Data.Rate;
   }

   EXPORT uint8 Item_SetDurability( Item& item, uint8 val )
   {
    item.Data.Rate = val;
    return 0;
   }
 */

#endif // __SERVER

#ifdef __CLIENT

EXPORT int Map_GetScenParam( uint16 tx, uint16 ty, uint protoId, uint8 num )
{
    if( num < 0 || num > 10 )
        return -1;
    /*
       MapObjectVec &obj = map.Proto->SceneriesVec;

       for(MapObjectVecIt it=obj.begin(); it<obj.end(); ++it)
       {
            if((*it)->MapX!=tx || (*it)->MapY!=ty || (*it)->ProtoId!=protoId) continue;
            return (*it)->MScenery.Param[num];
       }*/

    uint    width = FOnline->ClientMapWidth;
    Field   field = FOnline->ClientMap[ ty * width + tx ];

    ItemVec items = field.Items;

    for( ItemVecIt it = items.begin(); it < items.end(); ++it )
    {
        const ProtoItem* proto = ( *it )->Proto;
        if( proto->ProtoId != protoId || ( *it )->Accessory != 2 )
            continue;                                                            // 2 == ACCESSORY_HEX

        // Map @ map = GetMap((*it)->ACC_HEX.MapId);
        return proto->StartValue[ num ];
        //		Data.ScriptValues[num];
    }

    return -1;
}

EXPORT uint8 Item_GetDurability( Item& item )
{
    return item.Data.Rate;
}

#endif // __CLIENT
template< typename T1, typename T2 >
bool GetMyFloderFiles( T1& dirName, T2& outStr )
{
    WIN32_FIND_DATA ffd;
    HANDLE          hFind = INVALID_HANDLE_VALUE;

    hFind = FindFirstFile( dirName.c_str(), &ffd );

    if( INVALID_HANDLE_VALUE == hFind )
        return false;

    do
    {
        if( ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
            continue;

        outStr += "\n";
        outStr += ffd.cFileName;
    }
    while( FindNextFile( hFind, &ffd ) != 0 );
    FindClose( hFind );
    return true;
}

template< typename T1, typename T2 >
bool GetMyFloders( T1& dirName, T2& outStr )
{
    WIN32_FIND_DATA ffd;
    HANDLE          hFind = INVALID_HANDLE_VALUE;

    hFind = FindFirstFile( dirName.c_str(), &ffd );

    if( INVALID_HANDLE_VALUE == hFind )
        return false;

    do
    {
        if( ( ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
            continue;
		
        outStr += "\n";
        outStr += ffd.cFileName;
    }
    while( FindNextFile( hFind, &ffd ) != 0 );
    FindClose( hFind );
    return true;
}

EXPORT bool GetMyFolderFiles( ScriptString& dirName, ScriptString& outStr )
{
    return GetMyFloderFiles< ScriptString, ScriptString  >( dirName, outStr );
}

EXPORT bool GetMyFolders( ScriptString& dirName, ScriptString& outStr )
{
    return GetMyFloders< ScriptString, ScriptString  >( dirName, outStr );
}
