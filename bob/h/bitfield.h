//------------------------------------------------------------------------------
//Filename       bitfield.h
//System
//Author         Jim Taylor
//Date           Sun 26 Nov 1995
//Description    A bitfields class.
//				In principal, this is a container class...
//1) Macro MAKEFIELD (srctype,minval,maxval)
//		this defines a srctypeField type, and operations possible on that type:
//			srcField|=src		sets a bit	-does not commute
//			srcField<<src		sets a bit	-commutes: srcField<<src<<src works
//			srcField%=src		clears a bit
//			srcField^=src		xors a bit
//			srcField|=srcField	logical or of all bits
//			srcField&=srcField	logical and of all bits
//			srcField^=srcField	logical xor of all bits
//			+srcField			any bits set? true/false
//			srcField>=srcField	set contains set? true/false
//			srcField<=srcField	set contains set? true/false
//			srcField==srcField	set contains set? true/false
//			srcField[src]		set contains bit? true/false
//			srcField>=src		set contains bit? true/false
//			srcField.Count()	count bits set
//		Any other logical op should be illegal!
////JIM 15/02/00
//			srcField=src,src,src,src
//			srcField=0					zeros all bits
//			srcField=-1					sets all bits
//			Use (srcField::Make(),src,src,src) as fn parameter
//
//------------------------------------------------------------------------------
#pragma pack(push,1)	// self-assert: dosdefs-era headers require pack(1)
									// regardless of ambient packing - class/file-layout ODR.
#ifndef	BITFIELD_Included
#define	BITFIELD_Included
/*#ifndef	assert
	#define assert(txt,str)
#pragma warnmsg("Assert is non functional!!!")
#endif
*/
	#include	"hardpasm.h"

//#undef assert
//#include "myerror.h"
#include <cassert>

#define	DEFAULT_BITFIELD 0

//template	<class	enumtype,enumtype minval,enumtype maxval> class	MakeField;
//#define	MAKEFIELD(src,minval,maxval)		\
//	typedef	MakeField<src,minval,maxval>	src##Field;

template	<class	src,int minval,int maxval> class	MakeField
{
	friend 	MakeField;
//public:	class	Make;
public:
enum	fieldsize	{	MAX=+maxval,MIN=+minval,
						RANGE=(+MAX- +MIN)+1,
						BYTES=(+MAX- +MIN+8)/8
					};
/*private:*/
void	errorcheck(int val,int min,int max) const
	{assert ((val<=max && val>=min))/* ,"Bit field index out of range")*/;}
char	dataspace[BYTES];
public:
/*------------------------------
/ operator |=
/ set a bit in the bitfield
/
/------------------------------*/
MakeField&	operator	|=(src	s)
{
	errorcheck(s,MIN,MAX);
	BITSET(dataspace,(int)s-(int)MIN);
	return(*this);
}
/*------------------------------
/ operator <<
/ set a bit in the bitfield
/
/------------------------------*/
MakeField&	operator	<<(src	s)
{
	errorcheck(s,MIN,MAX);
	BITSET(dataspace,(int)s-(int)MIN);
	return(*this);
}

/*------------------------------
/ operator |=
/ set a bit in the bitfield
/
/------------------------------*/
MakeField&	operator	=(const int s)
{
	assert((s==0)||(s==-1));//,"Can set to 0 or -1 only");
	int i;
	for (i=BYTES;i--;)
		dataspace[i]=(char)s;
	return(*this);
}

/*------------------------------
/ operator %=
/ clear a bit in the bitfield
/ this is the remainder function
/------------------------------*/
MakeField&	operator	%=(src	s)
{
	errorcheck(s,MIN,MAX);
	BITRESET(dataspace,(int)s-(int)MIN);
	return(*this);
}

/*------------------------------
/ operator ^=
/ not a bit in the bitfield
/
/------------------------------*/
MakeField&	operator	^=(src	s)
{
	errorcheck(s,MIN,MAX);
	BITCOMP(dataspace,(int)s-(int)MIN);
	return(*this);
}

/*------------------------------
/ operator |=
/ or together two bit fields
/ 'destroys' the target field
/------------------------------*/
MakeField& operator	|=(const MakeField& s)
{
	int i;
	for (i=(int)BYTES;i--;)
		dataspace[i]|=s.dataspace[i];
	return(*this);
}

/*------------------------------
/ operator &=
/ and together two bit fields
/ 'destroys' the target field
/------------------------------*/
MakeField& operator	&=(const MakeField& s)
{
	int i;
	for (i=(int)BYTES;i--;)
		dataspace[i]&=s.dataspace[i];
	return(*this);
}

/*------------------------------
/ operator ^=
/ xor together two bit fields
/ 'destroys' the target field
/------------------------------*/
MakeField& operator	^=(const MakeField& s)
{
	int i;
	for (i=(int)BYTES;i--;)
		dataspace[i]^=s.dataspace[i];
	return(*this);
}
/*------------------------------
/ operator -=
/ xor together two bit fields
/ 'destroys' the target field
/------------------------------*/
MakeField& operator	-=(const MakeField& s)
{
	int i;
	for (i=(int)BYTES;i--;)
		dataspace[i]&=~s.dataspace[i];
	return(*this);
}

/*------------------------------
/ operator int
/ result from an int conversion
/ specifically when used in an if
/------------------------------*/
/* THIS IS TOO DANGEROUS BY HALF!!!
 Any missing operation is defaulting to this conversion
operator	int	()
{
	int i;
	for (i=BYTES;i--;)
		if(dataspace[i])	return(TRUE);
	return(FALSE);
}
*/
/*------------------------------
/ operator int
/ result from an int conversion
/ specifically when used in an if
/------------------------------*/
bool	operator	+()	const
{
	int i;
	for (i=(int)BYTES;i--;)
		if(dataspace[i])	return(true);
	return(false);
}
/*------------------------------
/ operator >=
/ this contains s,
/ i.e. there would be no change if
/ you performed an OR operation
/------------------------------*/
bool	operator	>=(const MakeField& s) const
{
	int i;
	for (i=(int)BYTES;i--;)
		if (	(dataspace[i]&s.dataspace[i])
			!=	s.dataspace[i]	)
			return(false);
	return(true);
}
/*------------------------------
/ operator <=
/ s contains this,
/ i.e. there would be no change if
/ you performed an OR operation
/------------------------------	*/
bool	operator	<=(const MakeField& s) const
{
	int i;
	for (i=(int)BYTES;i--;)
		if (	(dataspace[i]&s.dataspace[i])
			!=	dataspace[i]	)
			return(false);
	return(true);
}

/*------------------------------
/ operator ==
/ s contains this,
/ i.e. there would be no change if
/ you performed an OR operation
/------------------------------*/
bool	operator	==(const MakeField& s) const
{
	int i;
	for (i=(int)BYTES;i--;)
		if (dataspace[i]!=s.dataspace[i])
			return(false);
	return(true);
}
/*------------------------------
/ operator []
/ element s is set test
/------------------------------*/
bool	operator	[] (const src s) const
{
	errorcheck(s,MIN,MAX);
	return BITTEST(dataspace,(int)s-(int)MIN);
}

	friend	bool	operator	>=
		(const MakeField& f,src s);

/*------------------------------
/ Count()
/ number of elements set
/------------------------------*/
int	Count() const
{
	int	rv=0;
	char* l="\0\1\1\2" "\1\2\2\3" "\1\2\2\3" "\2\3\3\4";
	for (int i=(int)BYTES;i--;)
		rv+=l[dataspace[i]&15]+l[(dataspace[i]>>4)&15];
	return rv;
}
/*------------------------------
/ Assignment from constructable version:
/ number of elements set
/------------------------------*/
//Make&
MakeField&	operator =(src x)
{
	operator=(0);
	operator|=(x);
	return *this;
}

static MakeField	Make(const int s=0)
{
	MakeField rv;
	rv=s;
	return rv;
}

	/* class end*/
};	/* class end*/


/*---------------------------------------------------*/


//----------------------------------------------------------------------------
//
#pragma pack(pop)
#endif
