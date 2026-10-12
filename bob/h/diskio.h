//------------------------------------------------------------------------------
//Filename       diskio.h
//System         
//Author         Paul.   
//Date           Thu 6 Jul 1995
//Description    
//------------------------------------------------------------------------------
#pragma pack(push,1)	// self-assert: dosdefs-era headers require pack(1)
									// regardless of ambient packing - class/file-layout ODR.
#ifndef	diskio_Included
#define	diskio_Included

class	DiskIO
{
	public:
		char	*DiskIO::LoadNumberedFile(short);
		void	DiskIO::SaveNumberedFile(short, char *, short);

	private:

	protected:

};

#pragma pack(pop)
#endif
