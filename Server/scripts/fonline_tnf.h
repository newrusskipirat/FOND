#ifndef __FONLINE_TNF__
#define __FONLINE_TNF__

// Script constants
#define SKIP_PRAGMAS
#include "_defines.fos"

// Disable macro redefinition warning
#pragma warning (push)
#pragma warning (disable : 4005)
#include "fonline.h"
#pragma warning (pop)

// Script global variables
struct _GlobalVars
{
    int*  __CurX;
    int*  __CurY;
    uint* __HitAimEyes;
    uint* __HitAimHead;
    uint* __HitAimGroin;
    uint* __HitAimTorso;
    uint* __HitAimArms;
    uint* __HitAimLegs;
	bool* __Zombies;

#ifdef __SERVER
	uint* __StartServerVersion;
#endif
} extern GlobalVars;

#endif // __FONLINE_TNF__
