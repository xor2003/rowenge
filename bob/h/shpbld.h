//------------------------------------------------------------------------------
//Filename       shpbld.h
//System         
//Author         Paul.   
//Date           Mon 22 Jan 1996
//Description    
//------------------------------------------------------------------------------
#pragma pack(push,1)	// self-assert: dosdefs-era headers require pack(1)
									// regardless of ambient packing - class/file-layout ODR.
#ifndef	SHPBLD_Included
#define	SHPBLD_Included

#define	DEFAULT_SHPBLD 0

	#include	"DOSDefs.h"
	#include	"3DDefs.h"
	#include	"myANGLES.h"
	#include	"worldinc.h"
	#include	"AnimData.h"									//RJS 10Feb97

	#include	"3DInstr.h"
	#include	"imagemap.h"

class	ShapeBuild
{
	UByteP	currentp;

	public:

		CON		ShapeBuild();
		DES	   ~ShapeBuild();

		UByteP	BuildCloud(ImageMapNumber, ImageMapNumber, SWord);
		void	DeleteCloud();

	private:

		void 	CreateDoPoint(UByteP& ,SWord ,SWord ,SWord , SWord );
		void 	CreateDoCreateBumpPoly(UByteP& ,ImageMapNumber );
		void 	CreateDoCopyBVert(UByteP& ,SWord , SWord , SWord );
		void 	CreateDoDrawBPoly(UByteP& );
		void 	CreateDoReturn(UByteP& );

		void 	CreateDoCreateIPoly(UByteP& ,ImageMapNumber );
		void 	CreateDoCopyIVert(UByteP& ,SWord , SWord , SWord );
		void 	CreateDoDrawIPoly(UByteP& );
	protected:
};

extern class ShapeBuild Shape_Build;

#pragma pack(pop)
#endif
