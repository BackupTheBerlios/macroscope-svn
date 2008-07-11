/* GDCHART 0.11.3dev  GDCHART.C  11 Mar 2003 */
/* Copyright Bruce Verderaime 1998-2004 */

/* vi:set tabstop=4 */

// adapting for C++
/*-
 * Copyright 2008 Guram Dukashvili
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
//---------------------------------------------------------------------------
#include <adicpp/ksys.h>
#if HAVE_GD_H
#include <gd.h>
#include <gdfontg.h>
#include <gdfontl.h>
#include <gdfontmb.h>
#include <gdfonts.h>
#include <gdfontt.h>
#else
#define HAVE_GD_H 1
#include <adicpp/gd/gd.h>
#include <adicpp/gd/gdfontg.h>
#include <adicpp/gd/gdfontl.h>
#include <adicpp/gd/gdfontmb.h>
#include <adicpp/gd/gdfonts.h>
#include <adicpp/gd/gdfontt.h>
#endif
//------------------------------------------------------------------------------
namespace ksys {
//------------------------------------------------------------------------------
#if !defined (__GNUC__)  || defined (__STRICT_ANSI__)
double	**allocate_array_double( int nrs, int ncs );
char	**allocate_array_char( int nrs, int ncs );
int		**allocate_array_int( int nrs, int ncs );
#define CREATE_ARRAY1( arr, type, nels )		type *arr = (type*)malloc( (nels) * sizeof(type) )
												/* array2 can't do ptr (*) types */
#define CREATE_ARRAY2( arr, type, nrs, ncs )	type **arr = allocate_array_##type( nrs, ncs )

#define FREE_ARRAY1( arr )						if( arr ) { free( (void*)arr ); arr = NULL; } else
#define FREE_ARRAY2( arr )						if( arr )						\
													{							\
													if( arr[0] )				\
														free( (void*)arr[0] );	\
													free( (void*)arr );			\
													arr = NULL;					\
													}							\
												else

/* ----- avoid alloc, let the stack do the work ----- */
#else

#define CREATE_ARRAY1( arr, type, nels )		type arr[ nels ]
#define CREATE_ARRAY2( arr, type, nrs, ncs )	type arr[ nrs ][ ncs ]
#define FREE_ARRAY1( arr )
#define FREE_ARRAY2( arr )

#endif
#if !defined (__GNUC__)  || defined (__STRICT_ANSI__)
/*F***************************************************************************
*
* Function Name:         allocate_array_double
*
* Function Prototype:    double	** allocate_array_double (int nr, int nc)
*
* Allocates memory for a two-dimensional array of doubles
* referenced as arr[r][c]
*
***************************************************************************
*
*   access array as arr[i1=0-nr][i2=0-nc] = *(&arr[0][0]+nr*i1+i2)
*                       row      col  makes fewest pointer array elements
*                                     assuming Ncol>>Nrow
*  Ex: NC=2
*           _______                  __________
*    **arr |arr[0] |------>arr[0]-->|arr[0][0] |
*          |_______|                |__________|
*          |arr[1] |-----           |arr[0][1] |
*          |_______|     |          |__________|
*          |  .    |      --------->|arr[1][0] |
*          |  .    |                |__________|
*          |_______|                |arr[1][1] |
*          |arr[NR-1]
*          |       |
*         pointer array             data array
*
******************************************************************************
*
* Parameters :
*
* Name               Type           Use       Description
* ----               ----           ---       -----------
* nr                 int          Input     Number of rows
* nc                 int          Input     Number of columns
*
******************************************************************************
*
* Function Return:   double** with allocated memory all set up as double[][].
*
******************************************************************************
*
* Author:  Don Fasen
* Date  :  12/18/98
* Modified: Darren Kent 20/03/2002
*
****************************************************************************F*/
static double **
allocate_array_double( int	nr,		/* number of pointer array elements */
					  int	nc )	/* larger number for min memory usage */
{
	int		i;
	double	**arr;

	/* Try to allocate memory for the pointer array, 'nr' pointers-to-double: */
	arr = (double**)malloc( nr * sizeof(double*) );
	/* no test for success ala glib, i.e., if you're outa memory, ... */
#ifdef DBUG
	if( arr == NULL )
		{
		perror( "Unable to malloc pointer array in allocate_array_double()\n" );
		exit( 2 );
		}
#endif
	
	/* Now allocate memory for the data array, 'nr * nc' doubles: */
	arr[0] = (double*)malloc( nr * nc * sizeof(double) );
#ifdef DBUG
	if( arr == NULL )
		{
		perror( "Unable to malloc data array in allocate_array_double()\n" );
		exit( 2 );
		}
#endif
	/* fill in the array pointers to point to successive columns. */
	/* Don't need to do the first (zeroth) one, because it was set up by the data malloc. */
	/* Note that adding 1 to (double*)a.arr[0] adds sizeof(double*) automatically. */

	for (i = 1; i < nr; i++) 
		arr[i] = arr[0] + i*nc;

	return arr;

} /* End of FUNCTION allocate_array_double */

/***********************************************************************************
*
* Same Implementation for a Char array.
*
************************************************************************************/
static char **
allocate_array_char( int	nr,
					 int	nc )
{
	int		i;
	char	**arr;

	/* Try to allocate memory for the pointer array, 'nr' pointers-to-chars: */
	arr = (char**)malloc( nr * sizeof(char*) );
	/* no test for success ala glib, i.e., if you're outa memory, ... */
#ifdef DBUG
	if( arr == NULL )
		{
		perror( "Unable to malloc pointer array in allocate_array_char()\n" );
		exit( 2 );
		}
#endif
	
	/* Now allocate memory for the data array, 'nr * nc' char: */
	arr[0] = (char*)malloc( nr * nc * sizeof(char) );
#ifdef DBUG
	if( arr == NULL )
		{
		perror( "Unable to malloc data array in allocate_array_char()\n" );
		exit( 2 );
		}
#endif
	/* fill in the array pointers to point to successive columns. */
	/* Don't need to do the first (zeroth) one, because it was set up by the data malloc. */
	/* Note that adding 1 to (char*)a.arr[0] adds sizeof(char*) automatically. */

	for (i = 1; i < nr; i++) 
		arr[i] = arr[0] + i*nc;

	return arr;

} /* End of FUNCTION allocate_array_char */

/***********************************************************************************
*
*  Same Implementation for a Int array.
*
************************************************************************************/
static int **
allocate_array_int( int	nr,
					int	nc )

{
	int		i;
	int		**arr;

	/* Try to allocate memory for the pointer array, 'nr' pointers-to-ints: */
	arr = (int**)malloc( nr * sizeof(int*) );
	/* no test for success ala glib, i.e., if you're outa memory, ... */
#ifdef DBUG
	if( arr == NULL )
		{
		perror( "Unable to malloc pointer array in allocate_array_int()\n" );
		exit( 2 );
		}
#endif
	
	/* Now allocate memory for the data array, 'nr * nc' int: */
	arr[0] = (int*)malloc( nr * nc * sizeof(int) );
#ifdef DBUG
	if( arr == NULL )
		{
		perror( "Unable to malloc data array in allocate_array_int()\n" );
		exit( 2 );
		}
#endif
	/* fill in the array pointers to point to successive columns. */
	/* Don't need to do the first (zeroth) one, because it was set up by the data malloc. */
	/* Note that adding 1 to (int*)a.arr[0] adds sizeof(int*) automatically. */

	for (i = 1; i < nr; i++) 
		arr[i] = arr[0] + i*nc;

	return arr;

} /* End of FUNCTION allocate_array_int */

#endif /* !defined (__GNUC__)  || defined (__STRICT_ANSI__) */
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif

#define GDC_NOVALUE			-DBL_MAX
#define GDC_NULL			GDC_NOVALUE

#define ABS( x )			( (x)<0.0? -(x): (x) )
#define MAX( x, y )			( (x)>(y)?(x):(y) )
#define MIN( x, y )			( (x)<(y)?(x):(y) ) 
#define TO_RAD(o)			( (o)/360.0*(2.0*M_PI) )

#define GDC_NOCOLOR			0x1000000L
#define GDC_DFLTCOLOR		0x2000000L
#define PVRED               0x00FF0000
#define PVGRN               0x0000FF00
#define PVBLU               0x000000FF
#define l2gdcal( c )        ((c)&PVRED)>>16 , ((c)&PVGRN)>>8 , ((c)&0x000000FF)
#define l2gdshd( c )        (((c)&PVRED)>>16)/2 , (((c)&PVGRN)>>8)/2 , (((c)&0x000000FF))/2

#define HIGHSET		0
#define LOWSET		1
#define CLOSESET	2
#define MAX_NOTE_LEN		19
#define GDC_INTERP_VALUE	(GDC_NOVALUE/2.0)	/* must set GDC_interpolations */
#define GDC_INTERP			((GDC_interpolations=TRUE),GDC_INTERP_VALUE)
/* scaled translation onto graph */
#define PX( x )		(int)( xorig + (setno*xdepth_3D) + (x)*xscl )
#define PY( y )		(int)( yorig - (setno*ydepth_3D) + (y)*yscl )
#define PV( y )		(int)( vyorig - (setno*ydepth_3D) + (y)*vyscl )
#define ABS( x )			( (x)<0.0? -(x): (x) )
#define MAX( x, y )			( (x)>(y)?(x):(y) )
#define MIN( x, y )			( (x)<(y)?(x):(y) ) 
#define TO_RAD(o)			( (o)/360.0*(2.0*M_PI) )

#define _clrallocate( rawclr, bgc )														\
							( (_gdccfoo2=rawclr==GDC_DFLTCOLOR? _gdcntrst(bgc): rawclr),	\
							  (_gdccfoo1=colorExact(l2gdcal(_gdccfoo2))) != -1?	\
								_gdccfoo1:													\
								colorsTotal() == gdMaxColors?						\
								   colorClosest(l2gdcal(_gdccfoo2)):				\
								   colorAllocate(l2gdcal(_gdccfoo2)) )
#define _clrshdallocate( rawclr, bgc )													\
							( (_gdccfoo2=rawclr==GDC_DFLTCOLOR? _gdcntrst(bgc): rawclr),	\
							  (_gdccfoo1=colorExact(l2gdshd(_gdccfoo2))) != -1?	\
								_gdccfoo1:													\
								colorsTotal() == gdMaxColors?						\
									colorClosest(l2gdshd(_gdccfoo2)):				\
									colorAllocate(l2gdshd(_gdccfoo2)) )

#define clrallocate( rawclr )		_clrallocate( rawclr, GDC_BGColor )
#define clrshdallocate( rawclr )	_clrshdallocate( rawclr, GDC_BGColor )

#define _gdcntrst(bg)		( ((bg)&0x800000?0x000000:0xFF0000)|	\
							  ((bg)&0x008000?0x000000:0x00FF00)|	\
							  ((bg)&0x000080?0x000000:0x0000FF) )
/* ------------------------------------------------------------------ *\
 * count (natural) substrings (new line sep)
\* ------------------------------------------------------------------ */
static short cnt_nl( char	*nstr, int		*len )			/* strlen - max seg */
{
	short	c           = 1;
	short	max_seg_len = 0;
	short	tmplen      = 0;

	if( !nstr )
		{
		if( len )
			*len = 0;
		return 0;
		}
	while( *nstr )
		{
		if( *nstr == '\n' )
			{
			++c;
			max_seg_len = MAX( tmplen, max_seg_len );
			tmplen = 0;
			}
		else
			++tmplen;
		++nstr;
		}

	if( len )
		*len = MAX( tmplen, max_seg_len );		/* don't forget last seg */
	return c;
}
/* ------------------------------------------------------------------- */
/* ------------------------------------------------------------------------ */
/* ----------------------------------------------------------------- */
/* -- convert a double to a printable string, in form:             -- */
/* --	W N/D                                                     -- */
/* -- where W is whole, N is numerator, D is denominator          -- */
/* -- the frac N/D is one of 2nds, 4,8,16,32,64,128,256ths        -- */
/* -- if cannot convert, return str of the double                  -- */
/* ----------------------------------------------------------------- */

#define EPSILON		((1.0/256.0)/2.0)
#define GET_DEC(x)	( (x) - (double)(int)(x) )

char * GDChart::price_to_str(
        double	price,
			  int	*numorator,
			  int	*demoninator,
			  int	*decimal,
			  char	*fltfmt )			/* printf fmt'ing str */
{
	int			whole = (int)price;
	double		dec   = GET_DEC( price ),
				numr;
	/* double		pow( double, double ); */

	/* caller doesn't want fractions */
	if( fltfmt )
		{
		sprintf( rtn, fltfmt, price );
		*numorator = *demoninator = *decimal = 0;
		return rtn;
		}

	numr = dec * 256;
	/* check if we have a perfect fration in 256ths */
	{	
		double	rdec = GET_DEC( numr );

		if( rdec < EPSILON )
			;							/* close enough to frac */
		else if( (1-rdec) < EPSILON )	/* just over but close enough */
			++numr;
		else							/* no frac match */
		{
			sprintf( rtn, "%f", price );
			*numorator = *demoninator = *decimal = 0;
			return rtn;
		}
	}

	/* now have numr 256ths */
	/* resolve down */
	if( numr != 0 )
		{
		int	cnt = 8;

		while( (double)(numr)/2.0 == (double)(int)(numr/2) )
			{
			numr /= 2;
			--cnt;
			}

		/* don't want both whole AND numerator to be - */
		if( whole<0 && numr<0.0 )
			numr = -numr;
		*numorator = (int)numr;
		*demoninator = (int)pow(2.0, (double)cnt);
		*decimal = whole;
		sprintf( rtn, "%d %d/%d", whole,
								  (int)numr,
								  *demoninator );
		}
	else
		{
		*numorator = *demoninator = 0;
		*decimal = whole;
		sprintf( rtn, "%d", whole );
		}

    return rtn;
}
/* ------------------------------------------------------------------------ */
long GDChart::get_uniq_color()
{
	return 0x123454;
}
/* ------------------------------------------------------------------------ */
struct GDChart::fnt_sz_t GDChart::GDCfnt_sz(
       char					*s,
		   enum GDC_font_size	gdfontsz,
		   char					*ftfont,
		   double				ftfptsz,
		   double				rad,		/* w,h still relative to horiz. */
		   char					**sts )
{
	struct fnt_sz_t	rtnval;
	int				len;
	char			*err = NULL;

#ifdef HAVE_LIBFREETYPE
	if( ftfont && ftfptsz )
		{
		int		brect[8];

		/* obtain brect so that we can size the image */
		if( (err = gdImageStringFT( (gdImagePtr)NULL,
									&brect[0],
									0,
									ftfont,
									ftfptsz,
									0.0,	/* rad, */	/* always match angled size??? */
									0,
									0,
									s)) == NULL )
			{
			rtnval.h = brect[1] - brect[7];
			rtnval.w = brect[2] - brect[0];
			if( sts )	*sts = err;
			return rtnval;
			}
		}
#endif

	rtnval.h = cnt_nl(s,&len) * GDC_fontc[gdfontsz].h;
	rtnval.w = len * GDC_fontc[gdfontsz].w;
	if( sts )	*sts = err;
	return rtnval;
}
/* ------------------------------------------------------------------ */
/* gd out a string with '\n's
 * handle FTs (TTFs) and gd fonts
 * gdImageString() draws from the upper left;
 * gdImageStringFT() draws from lower left (one font height, even with '\n's)! >:-|
\* ------------------------------------------------------------------ */
int GDChart::GDCImageStringNL(
				  struct GDC_FONT_T	*f,
				  char				*ftfont,
				  double			ftptsz,
				  double			rad,
				  int				x,
				  int				y,
				  char				*str,
				  int				clr,
				  GDC_justify_t		justify,
				  char				**sts )
{
	int		retval = 0;
	char	*err   = NULL;

#ifdef HAVE_LIBFREETYPE
	/* TODO: honor justifies */
	if( ftfont && ftptsz ) 
		{
		/* need one line height */
		/* remember last one (will likely be the same) */
		/*	is this needed? */
		/*	gdImageStringFT() utilizes some caching */
		/*	saves a couple doubleing point trig calls */
		static int		f1hgt = 0;
		static double	xs,
						ys;
		static double	lftptsz = 0.0;
		static char		*lftfont = (char*)-1;

		if( !f1hgt ||
			( lftfont != ftfont || lftptsz != ftptsz ) )
			{
			f1hgt = GDCfnt_sz( "Aj",
							   0,
							   ftfont,
							   ftptsz,
							   rad,
							   NULL ).h;
			xs = (double)f1hgt * sin(rad);
			ys = (double)(f1hgt-1) * cos(rad);
			}
		x += (int)xs;
		y += (int)ys;
		if( (err = gdImageStringFT( im,
									(int*)NULL,
									clr,
									ftfont,
									ftptsz,
									rad,
									x,
									y,
									str)) == NULL )
			{
			if( sts )	*sts = err;
			return 0;
			}
		else
			{
			/* TTF failed */
			retval = 1;
			/* fall through - default to gdFonts */
			/* reinstate upper left reference */
			x -= (int)xs;
			y -= (int)ys;
			}
		}
#endif

	{
	int		i;
	int		len;
	int     max_len;
	short   strs_num = cnt_nl( str, &max_len );
	CREATE_ARRAY1( sub_str, char, max_len+1 );	/* char sub_str[max_len+1]; */

	len      = -1;
	strs_num = -1;
	i = -1;
	do
		{
		++i;
		++len;
		sub_str[len] = *(str+i);
		if( *(str+i) == '\n' ||
			*(str+i) == '\0' )
			{
			int	xpos;

			sub_str[len] = '\0';
			++strs_num;
			switch( justify )
			  {
			  case GDC_JUSTIFY_LEFT:	xpos = 0;					break;
			  case GDC_JUSTIFY_RIGHT:	xpos = f->w*(max_len-len);	break;
			  case GDC_JUSTIFY_CENTER:
			  default:					xpos = f->w*(max_len-len)/2;
			  }
			if( rad == 0.0 )
				string(
							   f->f,
							   x + xpos,
							   y + (f->h-1)*strs_num,
							   sub_str,
							   clr );
			else /* if( rad == M_PI/2.0 ) */
				stringUp(
								 f->f,
								 x + (f->h-1)*strs_num,
								 y - xpos,
								 sub_str,
								 clr );
			len = -1;
			}
		}
	while( *(str+i) );
	}

	if( sts )	*sts = err;
	return retval;
}
/* ------------------------------------------------------------------- *\ 
 * convert from enum GDC_font_size to gd fonts
 * for now load them all
 *	#defines and #ifdefs might enable loading only needed fonts
 *	gd2.0 is to be built as a shared obj.
\* ------------------------------------------------------------------- */
void GDChart::load_font_conversions()
{
  GDC_fontc[GDC_pad].f     = fontTiny();
	GDC_fontc[GDC_TINY].f    = fontTiny();
	GDC_fontc[GDC_SMALL].f   = fontSmall();
	GDC_fontc[GDC_MEDBOLD].f = fontMediumBold();
	GDC_fontc[GDC_LARGE].f   = fontLarge();
	GDC_fontc[GDC_GIANT].f   = fontGiant();
}
//------------------------------------------------------------------------------

#define SET_RECT( gdp, x1, x2, y1, y2 )	gdp[0].x = gdp[3].x = x1,	\
										gdp[0].y = gdp[1].y = y1,	\
										gdp[1].x = gdp[2].x = x2,	\
										gdp[2].y = gdp[3].y = y2


#define SET_3D_POLY( gdp, x1, x2, y1, y2, xoff, yoff )						\
								gdp[0].x  = x1,        gdp[0].y = y1,		\
								gdp[1].x  = x1+(xoff), gdp[1].y = y1-yoff,	\
								gdp[2].x  = x2+(xoff), gdp[2].y = y2-yoff,	\
								gdp[3].x  = x2,        gdp[3].y = y2
/* ------------------------------------------------------------------------- */
void GDChart::
out_err( int			IMGWIDTH,
		 int			IMGHEIGHT,
		 FILE			*fptr,
		 unsigned long	BGColor,
		 unsigned long	LineColor,
		 char			*err_str )
{

	int			lineclr;
	int			bgclr;


  if( image_ == NULL ) create(IMGWIDTH,IMGHEIGHT);
	bgclr    = colorAllocate(l2gdcal(BGColor) );
	lineclr  = colorAllocate(l2gdcal(LineColor) );

	string(
    fontMediumBold(),
				   IMGWIDTH/2 - GDC_fontc[GDC_MEDBOLD].w*strlen(err_str)/2,
				   IMGHEIGHT/3,
				   err_str,
				   lineclr );

	/* usually GDC_generate_img is used in conjunction with hard or hold options */
	if( GDC_generate_img )
		{
		fflush(fptr);			/* clear anything buffered  */
		switch( GDC_image_type )
			{
#ifdef HAVE_JPEG
			case GDC_JPEG:	gdImageJpeg( im, fptr, GDC_jpeg_quality );	break;
#endif
			case GDC_WBMP:	wbmp(lineclr, fptr );			break;
			case GDC_GIF:	gif(fptr);						break;
			case GDC_PNG:
			default:		png(fptr );
			}
		}
}
//------------------------------------------------------------------------------
/* vals in pixels */
/* ref is front plane */
/* allows for intersecting 3D lines      */
/*  (also used for single 3D lines >:-Q  */
int GDChart::qcmpr( const void *a, const void *b )
{
  if( ((struct YS*)a)->y2 < ((struct YS*)b)->y2 ) return 1;
	  if( ((struct YS*)a)->y2 > ((struct YS*)b)->y2 ) return -1;
	  return 0;
}
void GDChart::draw_3d_line(
			  int			y0,
			  int			x1,
			  int			x2,
			  int			y1[],
			  int			y2[],
			  int			xdepth,
			  int			ydepth,
			  int			num_sets,
			  int			clr[],
			  int			clrshd[] )
{
  gdImagePtr im = (gdImagePtr) image_;
#define F(x,i)	(int)( (double)((x)-x1)*slope[i]+(double)y1[i] )
	double		depth_slope  = xdepth==0? DBL_MAX: (double)ydepth/(double)xdepth;
	CREATE_ARRAY1( slope, double,     num_sets );		/* double		slope[num_sets] */
	CREATE_ARRAY1( lnclr, int,       num_sets );		/* int			slope[num_sets] */
	CREATE_ARRAY1( shclr, int,       num_sets );		/* int			slope[num_sets] */
	CREATE_ARRAY1( ypts,  struct YS, num_sets );		/* struct YS	slope[num_sets] */
	int			i;
	int			x;
	gdPoint		poly[4];

	for( i=0; i<num_sets; ++i )
		{
		/* lnclr[i] = clr[i]; */
		/* shclr[i] = clrshd[i]; */
		slope[i] = x2==x1? DBL_MAX: (double)(y2[i]-y1[i])/(double)(x2-x1);
		}

	for( x=x1+1; x<=x2; ++x )
		{
		for( i=0; i<num_sets; ++i )						/* load set of points */
			{
			ypts[i].y1    = F(x-1,i);
			ypts[i].y2    = F(x,i);
			ypts[i].lnclr = clr[i];
			ypts[i].shclr = clrshd[i];
			ypts[i].slope = slope[i];
			}											/* sorted "lowest" first */
		qsort( ypts, num_sets, sizeof(struct YS), qcmpr );
														/* put out in that order */
		for( i=0; i<num_sets; ++i )
			{											/* top */
			SET_3D_POLY( poly, x-1, x, ypts[i].y1, ypts[i].y2, xdepth, ydepth );
			filledPolygon( poly, 4,			/* depth_slope ever < 0 ? */
								  -ypts[i].slope>depth_slope? ypts[i].shclr: ypts[i].lnclr );
			if( x == x1+1 )								/* edging */
				line(
							 x-1, ypts[i].y2,
							 x-1+xdepth, ypts[i].y2-ydepth,
							 -ypts[i].slope<=depth_slope? ypts[i].shclr: ypts[i].lnclr );
			}
		}
	FREE_ARRAY1( slope );
	FREE_ARRAY1( lnclr );
	FREE_ARRAY1( shclr );
	FREE_ARRAY1( ypts );
}

/* ------------------------------------------------------------------------- */
/* vals in pixels */
/* ref is front plane */
void GDChart::draw_3d_area(
			 int				x1,
			 int				x2,
			 int				y0,			/* drawn from 0 */
			 int				y1,
			 int				y2,
			 int				xdepth,
			 int				ydepth,
			 int				clr,
			 int				clrshd )
{
  gdImagePtr		im = (gdImagePtr) image_;
	gdPoint     poly[4];
	int			y_intercept = 0;									/* if xdepth || ydepth */

	if( xdepth || ydepth )
		{
		double		line_slope   = x2==x1?    DBL_MAX: (double)-(y2-y1) / (double)(x2-x1);
		double		depth_slope  = xdepth==0? DBL_MAX: (double)ydepth/(double)xdepth;

		y_intercept = (y1 > y0 && y2 < y0) ||						/* line crosses y0 */
					   (y1 < y0 && y2 > y0)?
							(int)((1.0/ABS(line_slope))*(double)(ABS(y1-y0)))+x1:
							0;										/* never */

																	/* edging along y0 depth */
		line( x1+xdepth, y0-ydepth, x2+xdepth, y0-ydepth, clrshd );

		SET_3D_POLY( poly, x1, x2, y1, y2, xdepth, ydepth );		/* top */
		filledPolygon( poly, 4, line_slope>depth_slope? clrshd: clr );

		SET_3D_POLY( poly, x1, x2, y0, y0, xdepth, ydepth+1 );	/* along y axis */
		filledPolygon( poly, 4, clr );

		SET_3D_POLY( poly, x2, x2, y0, y2, xdepth, ydepth );		/* side */
		filledPolygon( poly, 4, clrshd );

		if( y_intercept )
			line( y_intercept,        y0,
							 y_intercept+xdepth, y0-ydepth, clrshd );	/* edging */
		line( x1, y0, x1+xdepth, y0-ydepth, clrshd );	/* edging */
		line( x2, y0, x2+xdepth, y0-ydepth, clrshd );	/* edging */

		/* SET_3D_POLY( poly, x2, x2, y0, y2, xdepth, ydepth );	// side */
		/* filledPolygon( poly, 4, clrshd ); */

		line( x1, y1, x1+xdepth, y1-ydepth, clrshd );	/* edging */
		line( x2, y2, x2+xdepth, y2-ydepth, clrshd );	/* edging */
		}

	if( y1 == y2 )												/* bar rect */
		SET_RECT( poly, x1, x2, y0, y1 );							/* front */
	else
		{
		poly[0].x = x1;	poly[0].y = y0;
		poly[1].x = x2;	poly[1].y = y0;
		poly[2].x = x2;	poly[2].y = y2;
		poly[3].x = x1;	poly[3].y = y1;
		}
	filledPolygon( poly, 4, clr );

	line( x1, y0, x2, y0, clrshd );			/* edging along y0 */

	if( (xdepth || ydepth) &&								/* front edging only on 3D */
		(y1<y0 || y2<y0) )									/* and only above y0 */
		{
		if( y1 > y0 && y2 < y0 )							/* line crosses from below y0 */
			line( y_intercept, y0, x2, y2, clrshd );
		else
		if( y1 < y0 && y2 > y0 )							/* line crosses from above y0 */
			line( x1, y1, y_intercept, y0, clrshd );
		else												/* completely above */
			line( x1, y1, x2, y2, clrshd );
		}
}

/* ------------------------------------------------------------------------- */
/* vals in pixels */
/* ref is front plane */
void GDChart::draw_3d_bar(
			 int				x1,
			 int				x2,
			 int				y0,
			 int				yhigh,
			 int				xdepth,
			 int				ydepth,
			 int				clr,
			 int				clrshd )
{
   gdImagePtr			im = (gdImagePtr) image_;
#define SET_3D_BAR( gdp, x1, x2, y1, y2, xoff, yoff )						\
								gdp[0].x  = x1,        gdp[0].y = y1,		\
								gdp[1].x  = x1+(xoff), gdp[1].y = y1-yoff,	\
								gdp[2].x  = x2+(xoff), gdp[2].y = y2-yoff,	\
								gdp[3].x  = x2,        gdp[3].y = y2

	gdPoint     poly[4];
	int			usd = MIN( y0, yhigh );								/* up-side-down bars */


	if( xdepth || ydepth )
		{
		if( y0 != yhigh )											/* 0 height? */
			{
			SET_3D_BAR( poly, x2, x2, y0, yhigh, xdepth, ydepth );	/* side */
			filledPolygon( poly, 4, clrshd );
			}

		SET_3D_BAR( poly, x1, x2, usd, usd, xdepth, ydepth );		/* top */
		filledPolygon( poly, 4, clr );
		}

	SET_RECT( poly, x1, x2, y0, yhigh );							/* front */
	filledPolygon( poly, 4, clr );

	if( xdepth || ydepth )
		line( x1, usd, x2, usd, clrshd );
}

/* ------------------------------------------------------------------------- */
int GDChart::barcmpr( const void *a, const void *b )
{
  if( ((struct BS*)a)->y2 < ((struct BS*)b)->y2 ) return -1;
	  if( ((struct BS*)a)->y2 > ((struct BS*)b)->y2 ) return 1;
	  return 0;
}
/* ------------------------------------------------------------------------- */
/* simple two-point linear interpolation */
/* attempts between first, then nearest */
void GDChart::do_interpolations( int		num_points,
				   int		interp_point,
				   double	vals[] )
{
	int		i, j;
	double	v1 = GDC_NOVALUE,
			v2 = GDC_NOVALUE;
	int		p1 = -1,
			p2 = -1;

	/* find backwards */
	for( i=interp_point-1; i>=0 && p1==-1; --i )
		if( vals[i] != GDC_NOVALUE && vals[i] != GDC_INTERP_VALUE )
			{
			v1 = vals[i];
			p1 = i;
			}
	/* find forwards */
	for( j=interp_point+1; j<num_points && p2==-1; ++j )
		if( vals[j] != GDC_NOVALUE && vals[j] != GDC_INTERP_VALUE )
			{
			v2 = vals[j];
			p2 = j;
			}
	/* no forward sample, find backwards */
	for( ; i>=0 && p2==-1; --i )
		if( vals[i] != GDC_NOVALUE && vals[i] != GDC_INTERP_VALUE )
			{
			v2 = vals[i];
			p2 = i;
			}
	/* no backwards sample, find forwards */
	for( ; j<num_points && p1==-1; ++j )
		if( vals[j] != GDC_NOVALUE && vals[j] != GDC_INTERP_VALUE )
			{
			v1 = vals[j];
			p1 = j;
			}
	if( p1==-1 || p2==-1 ||							/* need both */
		p1 == p2 )									/* idiot */
		{
		vals[interp_point] = GDC_NOVALUE;
		return;
		}

	/* Point-slope formula */
	vals[interp_point] = ((v2-v1)/(double)(p2-p1)) * (double)(interp_point-p1) + v1;
	return;
}

/* ========================================================================= */
/* little error checking  0: ok,                      */
/*                     -ret: error no graph output    */
/*                      ret: error graph out          */
/* watch out for # params and array sizes==num_points */
/* ------------------------------------------------------------------------- */
/* original var arg interface */
int GDChart::out_graph(
       int		IMGWIDTH,		/* no check for a image that's too small to fit */
		   int		IMGHEIGHT,		/* needed info (labels, etc), could core dump */
		   FILE			*img_fptr,		/* open file pointer (img out) */
		   int      type1,
		   int			num_points,     /* points along x axis (even iterval) */
										/*	all arrays dependant on this */
		   char			*xlbl[],		/* array of xlabels */
		   int			num_sets,
						... )
{
  GDC_CHART_T	type = (GDC_CHART_T) type1;
	char	do_hlc = ( type == GDC_HILOCLOSE        ||
					   type == GDC_3DHILOCLOSE      ||
					   type == GDC_3DCOMBO_HLC_BAR  ||
					   type == GDC_3DCOMBO_HLC_AREA ||
					   type == GDC_COMBO_HLC_BAR    ||
					   type == GDC_COMBO_HLC_AREA );

	char	do_fb  = ( type == GDC_FLOATINGBAR ||
					   type == GDC_3DFLOATINGBAR );

	char	do_vol = ( type == GDC_COMBO_HLC_BAR   ||
					   type == GDC_COMBO_HLC_AREA  ||
					   type == GDC_COMBO_LINE_BAR  ||
					   type == GDC_COMBO_LINE_AREA ||
					   type == GDC_COMBO_LINE_LINE ||
					   type == GDC_3DCOMBO_HLC_BAR ||
					   type == GDC_3DCOMBO_HLC_AREA||
					   type == GDC_3DCOMBO_LINE_BAR||
					   type == GDC_3DCOMBO_LINE_AREA ||
					   type == GDC_3DCOMBO_LINE_LINE );

	int		num_arrays = num_sets * (do_hlc? 3:
									 do_fb?  2: 1);

	CREATE_ARRAY1( data, double, num_arrays*num_points );	/* double data[num_arrays*num_points]  */
	double	*combo_data = (double*)NULL;

	va_list	ap;
	int		i,
			rtn;

	va_start( ap, num_sets );
	for( i=0; i<num_arrays; ++i )
		memcpy( data+i*num_points, va_arg(ap, double*), num_points*sizeof(double) );
	if( do_vol )
		combo_data = va_arg(ap, double*);
	va_end(ap);

	rtn =  GDC_out_graph( IMGWIDTH,
						  IMGHEIGHT,
						  img_fptr,
						  type,
						  num_points,
						  xlbl,
						  num_sets,
						  data,
						  combo_data );
	FREE_ARRAY1( data );

	return rtn;
}

/* ------------------------------------------------------------------------- */
/* multi array interface */
int GDChart::GDC_out_graph(
         int		IMGWIDTH,		/* no check for a img that's too small to fit */
			   int		IMGHEIGHT,		/* needed info (labels, etc), could core dump */
			   FILE			*img_fptr,		/* open file pointer (img out) */
			   int      type1,
			   int			num_points,     /* points along x axis (even iterval) */
											/*	all arrays dependant on this */
			   char			*xlbl[],		/* array of xlabels */
			   int			num_sets,
			   double		*data,			/* (double*) cast on multi-dim array (num_sets > 1) */
			   double		*combo_data )	/* only used on COMBO chart types */
{
  GDC_CHART_T	type = (GDC_CHART_T) type1;
	int			i, j;//, k;

	int			graphwidth;
	int			grapheight;
	GD bg_img;

	double		xorig, yorig, vyorig;
	double		yscl     = 0.0;
	double		vyscl    = 0.0;
	double		xscl     = 0.0;
	double		vhighest = -DBL_MAX;
	double		vlowest  = DBL_MAX;
	double		highest  = -DBL_MAX;
	double		lowest   = DBL_MAX;
	//gdPoint     volpoly[4];

	char		do_vol = ( type == GDC_COMBO_HLC_BAR   ||		/* aka: combo */
						   type == GDC_COMBO_HLC_AREA  ||
						   type == GDC_COMBO_LINE_BAR  ||
						   type == GDC_COMBO_LINE_AREA ||
						   type == GDC_COMBO_LINE_LINE ||
						   type == GDC_3DCOMBO_HLC_BAR ||
						   type == GDC_3DCOMBO_HLC_AREA||
						   type == GDC_3DCOMBO_LINE_BAR||
						   type == GDC_3DCOMBO_LINE_AREA ||
						   type == GDC_3DCOMBO_LINE_LINE );
	char		threeD = ( type == GDC_3DAREA          ||
						   type == GDC_3DLINE          ||
						   type == GDC_3DBAR           ||
						   type == GDC_3DFLOATINGBAR   ||
						   type == GDC_3DHILOCLOSE     ||
						   type == GDC_3DCOMBO_HLC_BAR ||
						   type == GDC_3DCOMBO_HLC_AREA||
						   type == GDC_3DCOMBO_LINE_BAR||
						   type == GDC_3DCOMBO_LINE_AREA ||
						   type == GDC_3DCOMBO_LINE_LINE );
	char		num_groups = num_sets;							/* set before num_sets gets adjusted */
	char		set_depth = ( GDC_stack_type == GDC_STACK_DEPTH )? num_groups:
																   1;
	char		do_bar = ( type == GDC_3DBAR           ||		/* offset X objects to leave */
						   type == GDC_BAR             ||		/*  room at X(0) and X(n) */
						   type == GDC_3DFLOATINGBAR   ||		/*  i.e., not up against Y axes */
						   type == GDC_FLOATINGBAR);
	char		do_ylbl_fractions = 							/* %f format not given, or */
						 ( !GDC_ylabel_fmt ||					/*  format doesn't have a %,g,e,E,f or F */
						   strlen(GDC_ylabel_fmt) == strcspn(GDC_ylabel_fmt,"%geEfF") );
	double		ylbl_interval  = 0.0;
	int			xlbl_hgt       = 0;
	int			xdepth_3Dtotal = 0;
	int			ydepth_3Dtotal = 0;
	int			xdepth_3D      = 0;		/* affects PX() */
	int			ydepth_3D      = 0;		/* affects PY() and PV() */
	int			hlf_barwdth	   = 0;		/* half bar widths */
	int			hlf_hlccapwdth = 0;		/* half cap widths for HLC_I_CAP and DIAMOND */
	int			annote_len     = 0,
				annote_hgt     = 0;

	/* args */
	int			setno = 0;				/* affects PX() and PY() */
	CREATE_ARRAY1( uvals, double *, type == GDC_HILOCLOSE        ||
								   type == GDC_3DHILOCLOSE      ||
								   type == GDC_3DCOMBO_HLC_BAR  ||
								   type == GDC_3DCOMBO_HLC_AREA ||
								   type == GDC_COMBO_HLC_BAR    ||
								   type == GDC_COMBO_HLC_AREA?  num_sets *= 3:	/* 1 more last set is vol */
								   type == GDC_FLOATINGBAR      ||
								   type == GDC_3DFLOATINGBAR?   num_sets *= 2:
								   type == GDC_COMBO_LINE_BAR   ||
								   type == GDC_3DCOMBO_LINE_BAR ||
								   type == GDC_3DCOMBO_LINE_AREA||
								   type == GDC_3DCOMBO_LINE_LINE||
								   type == GDC_COMBO_LINE_AREA  ||
								   type == GDC_COMBO_LINE_LINE? num_sets:		/* 1 more last set is vol */
																num_sets );
	CREATE_ARRAY1( ExtVolColor, int, num_points );				/* int	ExtVolColor[num_points],           */
	CREATE_ARRAY2( ExtColor,    int, num_sets, num_points );	/*		ExtColor[num_sets][num_points],    */
	CREATE_ARRAY2( ExtColorShd, int, threeD?num_sets:1,			/*		ExtColorShd[num_sets][num_points]; */
									 threeD?num_points:1 );		/* shade colors only with 3D */
	double		*uvol;

	int			BGColor,
				LineColor,
				PlotColor,
				GridColor,
				VolColor,
				//ThumbDColor,
				//ThumbLblColor,
				//ThumbUColor,
/*				ArrowDColor, */
/*				ArrowUColor, */
				AnnoteColor;
#ifdef HAVE_LIBFREETYPE
	char		*gdc_title_font	      = GDC_title_font;			/* for convienience  */
	char		*gdc_ytitle_font      = GDC_ytitle_font;		/* in func calls */
	char		*gdc_xtitle_font      = GDC_xtitle_font;
/*	char		*gdc_yaxis_font	      = GDC_yaxis_font; */
	char		*gdc_xaxis_font	      = GDC_xaxis_font;
	double		gdc_title_ptsize      = GDC_title_ptsize;
	double		gdc_ytitle_ptsize     = GDC_ytitle_ptsize;
	double		gdc_xtitle_ptsize     = GDC_xtitle_ptsize;
/*	double		gdc_yaxis_ptsize      = GDC_yaxis_ptsize; */
	double		gdc_xaxis_ptsize      = GDC_xaxis_ptsize;
	double		gdc_xaxis_rad         = TO_RAD( GDC_xaxis_angle );
	char		*gdc_annotation_font  = GDC_annotation_font;
	double		gdc_annotation_ptsize = GDC_annotation_ptsize;

#else
	char		*gdc_title_font		 = NULL;
	char		*gdc_ytitle_font	 = NULL;
	char		*gdc_xtitle_font	 = NULL;
/*	char		*gdc_yaxis_font		 = NULL; */
	char		*gdc_xaxis_font		 = NULL;
	double		gdc_title_ptsize	 = 0.0;
	double		gdc_ytitle_ptsize	 = 0.0;
	double		gdc_xtitle_ptsize	 = 0.0;
/*	double		gdc_yaxis_ptsize	 = 0.0; */
	double		gdc_xaxis_ptsize	 = 0.0;
	double		gdc_xaxis_rad        = GDC_xaxis_angle==90.0? M_PI/2.0: 0.0;
	char		*gdc_annotation_font = NULL;
	double		gdc_annotation_ptsize=0.0;
#endif
	double		sin_xangle = 1.0,								/* calc only when&if needed */
				cos_xangle = 0.0;

	/* idiot checks */
	if( IMGWIDTH<=0 || IMGHEIGHT<=0 || (!img_fptr && GDC_generate_img) )
		{
		FREE_ARRAY1( uvals );
		FREE_ARRAY1( ExtVolColor );
		FREE_ARRAY2( ExtColor );
		FREE_ARRAY2( ExtColorShd );
		return -1;
		}
	if( num_points <= 0 )
		{
		out_err( IMGWIDTH, IMGHEIGHT, img_fptr, GDC_BGColor, GDC_LineColor, "No Data Available" );
		FREE_ARRAY1( uvals );
		FREE_ARRAY1( ExtVolColor );
		FREE_ARRAY2( ExtColor );
		FREE_ARRAY2( ExtColorShd );
		return 1;
		}

	if( image_ == NULL ) create( IMGWIDTH, IMGHEIGHT );
	load_font_conversions();
	if( GDC_thumbnail ){
    GDC_grid = GDC_TICK_NONE;
		GDC_xaxis = GDC_TICK_NONE;
		GDC_yaxis = GDC_TICK_NONE;
	}

	/* ----- get args ----- */
	for( i=0; i<num_sets; ++i )
		uvals[i] = data+i*num_points;
	if( do_vol )
		if( !combo_data )
			{
			out_err( IMGWIDTH, IMGHEIGHT, img_fptr, GDC_BGColor, GDC_LineColor, "No Combo Data Available" );
			FREE_ARRAY1( uvals );
			FREE_ARRAY1( ExtVolColor );
			FREE_ARRAY2( ExtColor );
			FREE_ARRAY2( ExtColorShd );
			return -2;
			}
		else
			uvol = combo_data;

	/* ----- calculate interpretations first ----- */
	if( GDC_interpolations )
		{
		for( i=0; i<num_sets; ++i )
			for( j=0; j<num_points; ++j )
				if( uvals[i][j] == GDC_INTERP_VALUE )
					{
					do_interpolations( num_points, j, uvals[i] );
					}
		if( do_vol )
			for( j=0; j<num_points; ++j )
				if( uvol[j] == GDC_INTERP_VALUE )
					{
					do_interpolations( num_points, j, uvol );
					}
		}

	/* ----- highest & lowest values ----- */
	if( GDC_stack_type == GDC_STACK_SUM ) 		/* need to walk sideways */
		for( j=0; j<num_points; ++j )
			{
			double	set_sum = 0.0;
			for( i=0; i<num_sets; ++i )
				if( uvals[i][j] != GDC_NOVALUE )
					{
					set_sum += uvals[i][j];
					highest = MAX( highest, set_sum );
					lowest  = MIN( lowest,  set_sum );
					}
			}
	else
	if( GDC_stack_type == GDC_STACK_LAYER )		/* need to walk sideways */
		for( j=0; j<num_points; ++j )
			{
			double	neg_set_sum = 0.0,
					pos_set_sum = 0.0;
			for( i=0; i<num_sets; ++i )
				if( uvals[i][j] != GDC_NOVALUE )
					if( uvals[i][j] < 0.0 )
						neg_set_sum += uvals[i][j];
					else
						pos_set_sum += uvals[i][j];
			lowest  = MIN( lowest,  MIN(neg_set_sum,pos_set_sum) );
			highest = MAX( highest, MAX(neg_set_sum,pos_set_sum) );
			}
	else
		for( i=0; i<num_sets; ++i )
			for( j=0; j<num_points; ++j )
				if( uvals[i][j] != GDC_NOVALUE )
					{
					highest = MAX( uvals[i][j], highest );
					lowest  = MIN( uvals[i][j], lowest );
					}
	if( GDC_scatter )
	  for( i=0; i<GDC_num_scatter_pts; ++i )
		{
		highest = MAX( (GDC_scatter+i)->val, highest );
		lowest  = MIN( (GDC_scatter+i)->val, lowest  );
		}
	if( do_vol )								/* for now only one combo set allowed */
		{
		/* vhighest = 1.0; */
		/* vlowest  = 0.0; */
		for( j=0; j<num_points; ++j )
			if( uvol[j] != GDC_NOVALUE )
				{
				vhighest = MAX( uvol[j], vhighest );
				vlowest  = MIN( uvol[j], vlowest );
				}
		if( vhighest == -DBL_MAX )				/* no values */
			vhighest = 1.0;						/* for scaling, need a range */
		if( vlowest == DBL_MAX )
			vlowest = 0.0;
		if( type == GDC_COMBO_LINE_BAR    ||
			type == GDC_COMBO_HLC_BAR     ||
			type == GDC_COMBO_LINE_AREA   ||
			type == GDC_COMBO_HLC_AREA    ||
			type == GDC_3DCOMBO_LINE_BAR  ||
			type == GDC_3DCOMBO_LINE_AREA ||
			type == GDC_3DCOMBO_HLC_BAR   ||
			type == GDC_3DCOMBO_HLC_AREA )
		if( vhighest < 0.0 )
			vhighest = 0.0;
		else
		if( vlowest > 0.0 )
			vlowest = 0.0;						/* bar, area should always start at 0 */
		}

	if( lowest == DBL_MAX )
		lowest = 0.0;
	if( highest == -DBL_MAX )
		highest = 1.0;							/* need a range */
	if( type == GDC_AREA  ||					/* bars and area should always start at 0 */
		type == GDC_BAR   ||
		type == GDC_3DBAR ||
		type == GDC_3DAREA )
		if( highest < 0.0 )
			highest = 0.0;
		else
		if( lowest > 0.0 )						/* negs should be drawn from 0 */
			lowest = 0.0;

	if( GDC_requested_ymin != GDC_NOVALUE && GDC_requested_ymin < lowest )
		lowest = GDC_requested_ymin;
	if( GDC_requested_ymax != GDC_NOVALUE && GDC_requested_ymax > highest )
		highest = GDC_requested_ymax;

	/* ----- graph height and width within the img height width ----- */
	/* grapheight/height is the actual size of the scalable graph */
	{
	int	title_hgt  = GDC_title? 2				/* title? horizontal text line(s) */
								+ GDCfnt_sz(GDC_title,GDC_title_size,gdc_title_font,gdc_title_ptsize,0.0,NULL).h
								+ 2:
								2;
	int	xlabel_hgt = 0;
	int	xtitle_hgt = GDC_xtitle? 1+GDCfnt_sz(GDC_xtitle,GDC_xtitle_size,gdc_xtitle_font,gdc_xtitle_ptsize,0.0,NULL).h+1: 0;
	int	ytitle_hgt = GDC_ytitle? 1+GDCfnt_sz(GDC_ytitle,GDC_ytitle_size,gdc_ytitle_font,gdc_ytitle_ptsize,M_PI/2.0,NULL).h+1: 0;
	int	vtitle_hgt = do_vol&&GDC_ytitle2? 1+GDCfnt_sz(GDC_ytitle2,GDC_ytitle_size,gdc_ytitle_font,gdc_ytitle_ptsize,M_PI/2.0,NULL).h+1: 0;
	int	ylabel_wth = 0;
	int	vlabel_wth = 0;

	int	xtics      = GDC_ticks && (GDC_grid||GDC_xaxis)? 1+2: 0;
	int	ytics      = GDC_ticks && (GDC_grid||GDC_yaxis)? 1+3: 0;
	int	vtics      = GDC_ticks && (GDC_yaxis&&do_vol)? 3+1: 0;


#define	HYP_DEPTH	( (double)((IMGWIDTH+IMGHEIGHT)/2) * ((double)GDC_3d_depth)/100.0 )
#define RAD_DEPTH	( (double)GDC_3d_angle*2*M_PI/360 )
	xdepth_3D      = threeD? (int)( cos(RAD_DEPTH) * HYP_DEPTH ): 0;
	ydepth_3D      = threeD? (int)( sin(RAD_DEPTH) * HYP_DEPTH ): 0;
	xdepth_3Dtotal = xdepth_3D*set_depth;
	ydepth_3Dtotal = ydepth_3D*set_depth;
	annote_hgt     = GDC_annotation && *(GDC_annotation->note)?
						1 +											/* space to note */
						(1+GDCfnt_sz( GDC_annotation->note,GDC_annotation_font_size,
					                  gdc_annotation_font,gdc_annotation_ptsize,0.0,NULL ).h) +
						1 +											/* space under note */
						2: 0;										/* space to chart */
	annote_len     = GDC_annotation && *(GDC_annotation->note)?
						GDCfnt_sz( GDC_annotation->note,GDC_annotation_font_size,
				        	       gdc_annotation_font,gdc_annotation_ptsize,0.0,NULL ).w:
						0;

	/* find length of "longest" (Y) xaxis label */
	/* find the average "width" (X) xaxis label */
	/*	avg method fails when 2 or 3 very wide are consecutive, with the rest being thin */
	/*	this is most evident with horizontal (0deg) xlabels */
	/*	assume in this case they are quite uniform, e.g., dates */
	/* find affects on graphwidth/xorig of wildly overhanging angled labels */
	if( GDC_xaxis && xlbl )
		{
		int		biggest = -INT_MAX,
				widest  = -INT_MAX;
#ifdef HAVE_LIBFREETYPE
		if( gdc_xaxis_rad!=M_PI/2.0 && gdc_xaxis_font && gdc_xaxis_ptsize )
			{
			sin_xangle = sin( gdc_xaxis_rad ),
			cos_xangle = cos( gdc_xaxis_rad );
			}
#endif

		for( i=0; i<num_points; ++i )
			{
			int	len = 0,
				wdth = 0;
			if( !GDC_xlabel_ctl ||
				(GDC_xlabel_ctl && GDC_xlabel_ctl[i]) )
				{
				char	*sts;
				struct fnt_sz_t	lftsz = GDCfnt_sz( xlbl[i], GDC_xaxisfont_size,
												   gdc_xaxis_font, gdc_xaxis_ptsize, gdc_xaxis_rad,
												   &sts );

				if( gdc_xaxis_rad == M_PI/2.0 ||			/* no need to do the doubleing point math */
					(sts && *sts) )							/* FT fail status, used default gdfont */
					{
					#ifdef DEBUG
					fprintf( stderr, "TTF/FT failure: %s\n", sts );
					#endif
					len  = lftsz.w;
					wdth = lftsz.h;
					}
				else
				if( gdc_xaxis_rad == 0.0 )			/* protect /0 */
					{								/* reverse when horiz. */
					len  = lftsz.h;
					wdth = lftsz.w;
					}
				else									/* length & width due to the angle */
					{
					len  = (int)( (double)lftsz.w * sin_xangle + (double)lftsz.h * cos_xangle );
					wdth = (int)( (double)lftsz.h / sin_xangle );
					}
				}
			biggest = MAX( len, biggest );				/* last seg */
			widest  = MAX( wdth, widest );				/* last seg */
			}
		xlbl_hgt = 1+ widest +1;
		xlabel_hgt   = 1+ biggest +1;
		}

	grapheight = IMGHEIGHT - ( xtics          +
							   xtitle_hgt     +
							   xlabel_hgt     +
							   title_hgt      +
							   annote_hgt     +
							   ydepth_3Dtotal +
							   2 );
	if( GDC_hard_size && GDC_hard_grapheight )				/* user wants to use his */
		grapheight = GDC_hard_grapheight;
	GDC_hard_grapheight = grapheight;
															/* before width can be known... */
	/* ----- y labels intervals ----- */
	{
	double	tmp_highest;
															/* possible y gridline points */
	#define	NUM_YPOINTS	(sizeof(ypoints_2f) / sizeof(double))
	double	ypoints_2f[] = { 1.0/64.0, 1.0/32.0, 1.0/16.0, 1.0/8.0, 1.0/4.0, 1.0/2.0,
							 1.0,      2.0,      3.0,      5.0,     10.0,    25.0,
							 50.0,     100.0,    250.0,    500.0,   1000.0,  2500,    5000.0,
							 10000.0,  25000.0,  50000.0,  100000.0,500000.0,1000000, 5000000,
							 10000000, 50000000 },
			ypoints_dec[NUM_YPOINTS] = 						/* "pretty" points for dec (non-fraction) */
						   { 0.005,    0.01,     0.025,    0.05,     0.1,     0.2,     0.25,    0.5,
						     1.0,      2.0,      3.0,      5.0,     10.0,    25.0,
						     50.0,     100.0,    250.0,    500.0,   1000.0,  2500,    5000.0,
						     10000.0,  25000.0,  50000.0,  100000.0,500000.0,1000000, 5000000 },
			*ypoints = do_ylbl_fractions? ypoints_2f: ypoints_dec;
	int		max_num_ylbls;
	int		longest_ylblen = 0;
															/* maximum y lables that'll fit... */
	max_num_ylbls = grapheight / (3+GDC_fontc[GDC_yaxisfont_size==GDC_TINY? GDC_yaxisfont_size+1:
																			GDC_yaxisfont_size].h);
	if( max_num_ylbls < 3 )
		{
		/* gdImageDestroy(im);		haven't yet created it */
		out_err( IMGWIDTH, IMGHEIGHT,
				 img_fptr,
				 GDC_BGColor, GDC_LineColor,
				 "Insificient Height" );
		FREE_ARRAY1( uvals );
		FREE_ARRAY1( ExtVolColor );
		FREE_ARRAY2( ExtColor );
		FREE_ARRAY2( ExtColorShd );
		return 2;
		}

	{													/* one "space" interval above + below */
	double	ylbl_density_space_intvl = ((double)max_num_ylbls-(1.0+1.0)) * (double)GDC_ylabel_density/100.0;
	for( i=1; i<NUM_YPOINTS; ++i )
		/* if( ypoints[i] > ylbl_interval ) */
		/*	break; */
		if( (highest-lowest)/ypoints[i] < ylbl_density_space_intvl )
			break;
	/* gotta go through the above loop to catch the 'tweeners :-| */
	}

	ylbl_interval = GDC_requested_yinterval != GDC_NOVALUE &&
					GDC_requested_yinterval > ypoints[i-1]?	  GDC_requested_yinterval:
															  ypoints[i-1];

														/* perform doubleing point remainders */
														/* gonculate largest interval-point < lowest */
	if( lowest != 0.0 &&
		lowest != GDC_requested_ymin )
		{
		if( lowest < 0.0 )
			lowest -= ylbl_interval;
		/* lowest = (lowest-ypoints[0]) - */
		/* 			( ( ((lowest-ypoints[0])/ylbl_interval)*ylbl_interval ) - */
		/* 			   ( (double)((int)((lowest-ypoints[0])/ylbl_interval))*ylbl_interval ) ); */
		lowest = ylbl_interval * (double)(int)((lowest-ypoints[0])/ylbl_interval);
		}
														/* find smallest interval-point > highest */
	tmp_highest = lowest;
	do	/* while( (tmp_highest += ylbl_interval) <= highest ) */
		{
		int		nmrtr, dmntr, whole;
		int		lbl_len;
		char	foo[32];

		if( GDC_yaxis )
			{											/* XPG2 compatibility */
			sprintf( foo, do_ylbl_fractions? "%.0f": GDC_ylabel_fmt, tmp_highest );
			lbl_len = ylbl_interval<1.0? strlen( price_to_str(tmp_highest,
															  &nmrtr,
															  &dmntr,
															  &whole,
															  do_ylbl_fractions? NULL: GDC_ylabel_fmt) ):
										 strlen( foo );
			longest_ylblen = MAX( longest_ylblen, lbl_len );
			}
		} while( (tmp_highest += ylbl_interval) <= highest );
	ylabel_wth = longest_ylblen * GDC_fontc[GDC_yaxisfont_size].w;
	highest = GDC_requested_ymax==GDC_NOVALUE? tmp_highest:
											   MAX( GDC_requested_ymax, highest );

	if( do_vol )
		{
		double	num_yintrvls = (highest-lowest) / ylbl_interval;
															/* no skyscrapers */
		if( vhighest != 0.0 )
			vhighest += (vhighest-vlowest) / (num_yintrvls*2.0);
		if( vlowest != 0.0 )
			vlowest -= (vhighest-vlowest) / (num_yintrvls*2.0);

		if( GDC_yaxis2 )
			{
			char	svlongest[32];
			int		lbl_len_low  = sprintf( svlongest, GDC_ylabel2_fmt? GDC_ylabel2_fmt: "%.0f", vlowest );
			int		lbl_len_high = sprintf( svlongest, GDC_ylabel2_fmt? GDC_ylabel2_fmt: "%.0f", vhighest );
			vlabel_wth = 1
						 + MAX( lbl_len_low,lbl_len_high ) * GDC_fontc[GDC_yaxisfont_size].w;
			}
		}
	}

	graphwidth = IMGWIDTH - ( ( (GDC_hard_size && GDC_hard_xorig)? GDC_hard_xorig:
																   ( ytitle_hgt +
																     ylabel_wth +
																     ytics ) )
							  + vtics
							  + vtitle_hgt
							  + vlabel_wth
							  + xdepth_3Dtotal );
	if( GDC_hard_size && GDC_hard_graphwidth )				/* user wants to use his */
		graphwidth = GDC_hard_graphwidth;
	GDC_hard_graphwidth = graphwidth;

	/* ----- scale to img size ----- */
	/* offset to 0 at lower left (where it should be) */
	xscl = (double)(graphwidth-xdepth_3Dtotal) / (double)(num_points + (do_bar?2:0));
	yscl = -((double)grapheight) / (double)(highest-lowest);
	if( do_vol )
		{
		double	hilow_diff = vhighest-vlowest==0.0? 1.0: vhighest-vlowest;

		vyscl = -((double)grapheight) / hilow_diff;
		vyorig = (double)grapheight
				 + ABS(vyscl) * MIN(vlowest,vhighest)
				 + ydepth_3Dtotal
				 + title_hgt
				 + annote_hgt;
		}
	xorig = (double)( IMGWIDTH - ( graphwidth +
								  vtitle_hgt +
								  vtics      +
								  vlabel_wth ) );
	if( GDC_hard_size && GDC_hard_xorig )
		xorig = GDC_hard_xorig;
	GDC_hard_xorig = (int) xorig;
/*	yorig = (double)grapheight + ABS(yscl * lowest) + ydepth_3Dtotal + title_hgt; */
	yorig = (double)grapheight
				+ ABS(yscl) * MIN(lowest,highest)
				+ ydepth_3Dtotal
				+ title_hgt
				+ annote_hgt;
/*????	if( GDC_hard_size && GDC_hard_yorig )					/* vyorig too? */
/*????		yorig = GDC_hard_yorig;	FRED - check email */
	GDC_hard_yorig = (int) yorig;

	hlf_barwdth     = (int)( (double)(PX(2)-PX(1)) * (((double)GDC_bar_width/100.0)/2.0) );	/* used only for bars */
	hlf_hlccapwdth  = (int)( (double)(PX(2)-PX(1)) * (((double)GDC_HLC_cap_width/100.0)/2.0) );
	}
	/* scaled, sized, ready */


	/* ----- OK start the graphic ----- */
	BGColor        = colorAllocate( l2gdcal(GDC_BGColor) );
	LineColor      = clrallocate( GDC_LineColor );
	PlotColor      = clrallocate( GDC_PlotColor );
	GridColor      = clrallocate( GDC_GridColor );
	if( do_vol )
	  {
	  VolColor     = clrallocate( GDC_VolColor );
	  for( i=0; i<num_points; ++i )
		if( GDC_ExtVolColor )
		  ExtVolColor[i] = clrallocate( GDC_ExtVolColor[i] );
		else
		  ExtVolColor[i] = VolColor;
	  }
/*	ArrowDColor    = colorAllocate( 0xFF,    0, 0 ); */
/*	ArrowUColor    = colorAllocate(    0, 0xFF, 0 ); */
	if( GDC_annotation )
		AnnoteColor = clrallocate( GDC_annotation->color );

	/* attempt to import optional background image */
	if( GDC_BGImage )
		{
		FILE	*in = fopen(GDC_BGImage, "rb");
		if( !in )
			{
			; /* Cant load background image, drop it */
			}
		else
			{
			/* assume GIF */
			/* should determine type by file extension, option, ... */
        bg_img.createFromGif(in);
				{
          int	bgxpos = bg_img.sx()<IMGWIDTH?  IMGWIDTH/2 - bg_img.sx()/2:  0,
					bgypos = bg_img.sy()<IMGHEIGHT? IMGHEIGHT/2 - bg_img.sy()/2: 0;


				if( bg_img.sx() > IMGWIDTH ||				/* resize only if too big */
					bg_img.sy() > IMGHEIGHT )				/*  [and center] */
					{
					copyResized(bg_img,				/* dst, src */
										bgxpos, bgypos,			/* dstX, dstY */
										0, 0,					/* srcX, srcY */
										IMGWIDTH, IMGHEIGHT,	/* dstW, dstH */
										IMGWIDTH, IMGHEIGHT );	/* srcW, srcH */
					}
				else											/* just center */
					copy(bg_img,					/* dst, src */
								 bgxpos, bgypos,				/* dstX, dstY */
								 0, 0,							/* srcX, srcY */
								 IMGWIDTH, IMGHEIGHT );			/* W, H */
				}
			  fclose(in);
			}
		}

	for( j=0; j<num_sets; ++j )
		for( i=0; i<num_points; ++i )
			if( GDC_ExtColor )
				{
				unsigned long	ext_clr = *(GDC_ExtColor+num_points*j+i);

				ExtColor[j][i]            = clrallocate( ext_clr );
				if( threeD )
					ExtColorShd[j][i]     = clrshdallocate( ext_clr );
				}
			else if( GDC_SetColor )
				{
				int	set_clr = GDC_SetColor[j];
				ExtColor[j][i]     = clrallocate( set_clr );
				if( threeD )
				 ExtColorShd[j][i] = clrshdallocate( set_clr );
				}
			else
				{
				ExtColor[j][i]     = PlotColor;
				if( threeD )
				 ExtColorShd[j][i] = clrshdallocate( GDC_PlotColor );
				}
			

	if( GDC_transparent_bg )
		colorTransparent( BGColor );

	if( GDC_title )
		{
		struct fnt_sz_t	tftsz      = GDCfnt_sz( GDC_title, GDC_title_size, gdc_title_font, gdc_title_ptsize, 0.0, NULL );
		int				titlecolor = clrallocate( GDC_TitleColor );

		GDCImageStringNL(
						  &GDC_fontc[GDC_title_size],
						  gdc_title_font, gdc_title_ptsize,
						  0.0,
						  IMGWIDTH/2 - tftsz.w/2,
						  1,
						  GDC_title,
						  titlecolor,
						  GDC_JUSTIFY_CENTER,
						  NULL );
		}
	if( GDC_xtitle )
		{
		struct fnt_sz_t	xtftsz     = GDCfnt_sz( GDC_xtitle, GDC_xtitle_size, gdc_xtitle_font, gdc_xtitle_ptsize, 0.0, NULL );
		int				titlecolor = GDC_XTitleColor==GDC_DFLTCOLOR? PlotColor:
																	 clrallocate( GDC_XTitleColor );
		GDCImageStringNL(
						  &GDC_fontc[GDC_xtitle_size],
						  gdc_xtitle_font, gdc_xtitle_ptsize,
						  0.0,
						  IMGWIDTH/2 - xtftsz.w/2,
						  IMGHEIGHT-1-xtftsz.h-1,
						  GDC_xtitle,
						  titlecolor,
						  GDC_JUSTIFY_CENTER,
						  NULL );
		}


	/* ----- start drawing ----- */
	/* ----- backmost first - border, grid & labels ----- */
	/* if no grid, on 3D, border needs to handle it */
	if( !GDC_grid && threeD &&
		((GDC_border == GDC_BORDER_ALL) || (GDC_border & GDC_BORDER_X) || (GDC_border & GDC_BORDER_Y)) )
		{
		int	x1, x2,
			y1, y2;

		x1 = PX(0);
		y1 = PY(lowest);

		setno = set_depth;
		x2 = PX(0);
		y2 = PY(lowest);

		line( x1, y1, x2, y2, LineColor );			/* depth at origin */
		if( (GDC_border == GDC_BORDER_ALL) || (GDC_border & GDC_BORDER_X) )
			line( x2, y2, PX(num_points-1+(do_bar?2:0)), y2, LineColor );
		if( (GDC_border == GDC_BORDER_ALL) || (GDC_border & GDC_BORDER_Y) )
			line( x2, PY(highest), x2, y2, LineColor );
		setno = 0;
		}
	if( GDC_grid || GDC_ticks || GDC_yaxis )
		{	/* grid lines & y label(s) */
		double	tmp_y = lowest;
		int		labelcolor = GDC_YLabelColor==GDC_DFLTCOLOR? 
							 LineColor: clrallocate( GDC_YLabelColor );
		int		label2color = GDC_YLabel2Color==GDC_DFLTCOLOR? 
							  VolColor: clrallocate( GDC_YLabel2Color );

		/* step from lowest to highest puting in labels and grid at interval points */
		/* since now "odd" intervals may be requested, try to step starting at 0,   */
		/* if lowest < 0 < highest                                                  */
		for( i=-1; i<=1; i+=2 )									/* -1, 1 */
			{
			if( i == -1 )	if( lowest >= 0.0 )					/*	all pos plotting */
								continue;
							else
								tmp_y = MIN( 0, highest );		/*	step down to lowest */

			if( i == 1 )	if( highest <= 0.0 )				/*	all neg plotting */
								continue;
							else
								tmp_y = MAX( 0, lowest );		/*	step up to highest */


/*			if( !(highest > 0 && lowest < 0) )					// doesn't straddle 0 */
/*				{ */
/*				if( i == -1 )									// only do once: normal */
/*					continue; */
/*				} */
/*			else */
/*				tmp_y = 0; */

			do	/* while( (tmp_y (+-)= ylbl_interval) < [highest,lowest] ) */
				{
				int		n, d, w;
				char	nmrtr[3+1], dmntr[3+1], whole[8];
				char	all_whole = ylbl_interval<1.0? FALSE: TRUE;

				char	*ylbl_str = price_to_str( tmp_y,&n,&d,&w,
												  do_ylbl_fractions? NULL: GDC_ylabel_fmt );
				if( do_ylbl_fractions )
					{
					sprintf( nmrtr, "%d", n );
					sprintf( dmntr, "%d", d );
					sprintf( whole, "%d", w );
					}

				if( GDC_grid || GDC_ticks )
					{
					int	x1, x2, y1, y2;
					/* int	gridline_clr = tmp_y == 0.0? LineColor: GridColor; */
																			/* tics */
					x1 = PX(0);		y1 = PY(tmp_y);
					if( GDC_ticks )
						line( x1-2, y1, x1, y1, GridColor );
					if( GDC_grid )
						{
						setno = set_depth;
						x2 = PX(0);		y2 = PY(tmp_y);						/* w/ new setno */
						line( x1, y1, x2, y2, GridColor );		/* depth for 3Ds */
						line( x2, y2, PX(num_points-1+(do_bar?2:0)), y2, GridColor );
						setno = 0;											/* set back to foremost */
						}
					}
				if( GDC_yaxis )
					if( do_ylbl_fractions )
						{
						if( w || (!w && !n && !d) )
							{
							string(
										   GDC_fontc[GDC_yaxisfont_size].f,
										   PX(0)-2-strlen(whole)*GDC_fontc[GDC_yaxisfont_size].w
												  - ( (!all_whole)?
														(strlen(nmrtr)*GDC_fontc[GDC_yaxisfont_size-1].w +
														 GDC_fontc[GDC_yaxisfont_size].w                 +
														 strlen(nmrtr)*GDC_fontc[GDC_yaxisfont_size-1].w) :
														1 ),
										   PY(tmp_y)-GDC_fontc[GDC_yaxisfont_size].h/2,
										   whole,
										   labelcolor );
							}
						if( n )
							{
							string(
										   GDC_fontc[GDC_yaxisfont_size-1].f,
										   PX(0)-2-strlen(nmrtr)*GDC_fontc[GDC_yaxisfont_size-1].w
												  -GDC_fontc[GDC_yaxisfont_size].w
												  -strlen(nmrtr)*GDC_fontc[GDC_yaxisfont_size-1].w + 1,
										   PY(tmp_y)-GDC_fontc[GDC_yaxisfont_size].h/2 + 1,
										   nmrtr,
										   labelcolor );
							string(
										   GDC_fontc[GDC_yaxisfont_size].f,
										   PX(0)-2-GDC_fontc[GDC_yaxisfont_size].w
												  -strlen(nmrtr)*GDC_fontc[GDC_yaxisfont_size-1].w,
										   PY(tmp_y)-GDC_fontc[GDC_yaxisfont_size].h/2,
										   "/",
										   labelcolor );
							string(
										   GDC_fontc[GDC_yaxisfont_size-1].f,
										   PX(0)-2-strlen(nmrtr)*GDC_fontc[GDC_yaxisfont_size-1].w - 2,
										   PY(tmp_y)-GDC_fontc[GDC_yaxisfont_size].h/2 + 3,
										   dmntr,
										   labelcolor );
							}
						}
					else
						string(
									   GDC_fontc[GDC_yaxisfont_size].f,
									   PX(0)-2-strlen(ylbl_str)*GDC_fontc[GDC_yaxisfont_size].w,
									   PY(tmp_y)-GDC_fontc[GDC_yaxisfont_size].h/2,
									   ylbl_str,
									   labelcolor );


				if( do_vol && GDC_yaxis2 )
					{
					char	vylbl[16];
																				/* opposite of PV(y) */
					sprintf( vylbl,
							 GDC_ylabel2_fmt? GDC_ylabel2_fmt: "%.0f",
							 ((double)(PY(tmp_y)+(setno*ydepth_3D)-vyorig))/vyscl );

					setno = set_depth;
					if( GDC_ticks )
						line( PX(num_points-1+(do_bar?2:0)), PY(tmp_y),
										 PX(num_points-1+(do_bar?2:0))+3, PY(tmp_y), GridColor );
					if( atof(vylbl) == 0.0 )									/* rounding can cause -0 */
						strcpy( vylbl, "0" );
					string(
								   GDC_fontc[GDC_yaxisfont_size].f,
								   PX(num_points-1+(do_bar?2:0))+6,
								   PY(tmp_y)-GDC_fontc[GDC_yaxisfont_size].h/2,
								   vylbl,
								   label2color );
					setno = 0;
					}
				}
			while( ((i>0) && ((tmp_y += ylbl_interval) < highest)) ||
				   ((i<0) && ((tmp_y -= ylbl_interval) > lowest)) );
			}

		/* catch last (bottom) grid line - specific to an "off" requested interval */
		if( GDC_grid && threeD )
			{
			setno = set_depth;
			line( PX(0), PY(lowest), PX(num_points-1+(do_bar?2:0)), PY(lowest), GridColor );
			setno = 0;											/* set back to foremost */
			}

		/* vy axis title */
		if( do_vol && GDC_ytitle2 )
			{
			struct fnt_sz_t	ytftsz     = GDCfnt_sz( GDC_ytitle2, GDC_ytitle_size, gdc_ytitle_font, gdc_ytitle_ptsize, 0.0, NULL );
			int				titlecolor = GDC_YTitle2Color==GDC_DFLTCOLOR? VolColor:
																		  clrallocate( GDC_YTitle2Color );
			GDCImageStringNL(
							  &GDC_fontc[GDC_ytitle_size],
							  gdc_ytitle_font, gdc_ytitle_ptsize,
							  M_PI/2.0,
							  IMGWIDTH-(1+ytftsz.h),
							  int(yorig/2+ytftsz.w/2),
							  GDC_ytitle2,
							  titlecolor,
							  GDC_JUSTIFY_CENTER,
							  NULL );
			}

		/* y axis title */
		if( GDC_yaxis && GDC_ytitle )
			{
			struct fnt_sz_t	ytftsz     = GDCfnt_sz( GDC_ytitle, GDC_ytitle_size, gdc_ytitle_font, gdc_ytitle_ptsize, 0.0, NULL );
			int				titlecolor = GDC_YTitleColor==GDC_DFLTCOLOR? PlotColor:
																		 clrallocate( GDC_YTitleColor );
			GDCImageStringNL(
							  &GDC_fontc[GDC_ytitle_size],
							  gdc_ytitle_font, gdc_ytitle_ptsize,
							  M_PI/2.0,
							  1,
							  int(yorig/2+ytftsz.w/2),
							  GDC_ytitle,
							  titlecolor,
							  GDC_JUSTIFY_CENTER,
							  NULL );
			}
		}

	/* interviening set grids */
	/*  0 < setno < num_sets   non-inclusive, they've already been covered */
	if( GDC_grid && threeD )
		{
		for( setno=set_depth - 1;
			 setno > 0;
			 --setno )
			{
			line( PX(0), PY(lowest), PX(0), PY(highest), GridColor );
			line( PX(0), PY(lowest), PX(num_points-1+(do_bar?2:0)), PY(lowest), GridColor );
			}
		setno = 0;
		}

	if( ( GDC_grid || GDC_0Shelf ) &&							/* line color grid at 0 */
		( (lowest < 0.0 && highest > 0.0) ||
		  ( (lowest == 0.0 || highest == 0.0) && !(GDC_border&GDC_BORDER_X) ) ) )
		{
		int	x1, x2, y1, y2;
																/* tics */
		x1 = PX(0);		y1 = PY(0);
		if( GDC_ticks )
			line( x1-2, y1, x1, y1, LineColor );
		setno = set_depth;
		x2 = PX(0);		y2 = PY(0);								/* w/ new setno */
		line( x1, y1, x2, y2, LineColor );			/* depth for 3Ds */
		line( x2, y2, PX(num_points-1+(do_bar?2:0)), y2, LineColor );
		setno = 0;												/* set back to foremost */
		}

	/* x ticks and xlables */
	if( GDC_grid || GDC_xaxis )
		{
		int		num_xlbls  = graphwidth / 						/* maximum x lables that'll fit */
								( (GDC_xlabel_spacing==SHRT_MAX?0:GDC_xlabel_spacing) + xlbl_hgt );
		int		labelcolor = GDC_XLabelColor==GDC_DFLTCOLOR? LineColor:
															 clrallocate( GDC_XLabelColor );
		for( i=0; i<num_points+(do_bar?2:0); ++i )
			{
			int	xi = do_bar? i-1: i;
			int	x1, x2, y1, y2, yh;									/* ticks & grids */
			#define DO_TICK(x,y)			if( GDC_ticks )													\
												line( x, y, x,  y+2, GridColor );				\
											else
			#define DO_GRID(x1,y1,x2,y2)	if( GDC_grid )													\
												{															\
												line( x1, y1, x2,  y2, GridColor ); /* depth */	\
												line( x2, y2, x2,  yh, GridColor );				\
												}															\
											else

			x1 = PX(i);		y1 = PY(lowest);
			setno = set_depth;
			x2 = PX(i);		y2 = PY(lowest);	yh = PY(highest);
			setno = 0;												/* reset to foremost */

			if( i == 0 )											/* catch 3D Y back corner */
				DO_GRID(x1,y1,x2,y2);

			/* labeled points */
			if( (!GDC_xlabel_ctl && ( (i%(1+num_points/num_xlbls) == 0) ||	/* # x labels are regulated */
										  num_xlbls >= num_points       ||
										  GDC_xlabel_spacing == SHRT_MAX ))
				||
				(GDC_xlabel_ctl && xi>=0 && *(GDC_xlabel_ctl+xi)) )
				{
				DO_TICK(x1,y1);										/* labeled points tick & grid */
				DO_GRID(x1,y1,x2,y2);

				if( !do_bar || (i>0 && xi<num_points) )
					if( GDC_xaxis && xlbl && xlbl[xi] && *(xlbl[xi]) )
						{
						char			*sts;
						struct fnt_sz_t	lftsz = GDCfnt_sz( xlbl[xi], GDC_xaxisfont_size,
														   gdc_xaxis_font, gdc_xaxis_ptsize,
														   gdc_xaxis_rad,
														   &sts );
						if( gdc_xaxis_rad == M_PI/2.0 ||
							(sts && *sts) )							/* FT fail status, used default gdfont */
							{
							#ifdef DEBUG
							fprintf( stderr, "TTF/FT failure: %s\n", sts );
							#endif
							GDCImageStringNL(
											  &GDC_fontc[GDC_xaxisfont_size],
											  gdc_xaxis_font, gdc_xaxis_ptsize,
											  M_PI/2.0,
											  PX(i)-1 - (lftsz.h/2),
											  PY(lowest) + 2 + 1 + lftsz.w,
											  xlbl[xi],
											  labelcolor,
											  GDC_JUSTIFY_RIGHT,
											  NULL );
							}
						else
						if( gdc_xaxis_rad == 0.0 )
							GDCImageStringNL(
											  &GDC_fontc[GDC_xaxisfont_size],
											  gdc_xaxis_font, gdc_xaxis_ptsize,
											  0.0,
											  PX(i)-1 - (lftsz.w/2),
											  PY(lowest) + 2 + 1,
											  xlbl[xi],
											  labelcolor,
											  GDC_JUSTIFY_CENTER,
											  NULL );
						else
							GDCImageStringNL(
											  &GDC_fontc[GDC_xaxisfont_size],
											  gdc_xaxis_font, gdc_xaxis_ptsize,
											  gdc_xaxis_rad,
											  PX(i)-1 - (int)((double)lftsz.w*cos_xangle
													        + (double)lftsz.h*gdc_xaxis_rad/(M_PI/2.0)/2.0),
											  PY(lowest) + 2 + 1 + (int)((double)lftsz.w*sin_xangle),
											  xlbl[xi],
											  labelcolor,
											  GDC_JUSTIFY_RIGHT,
											  NULL );
						}
				}
			/* every point, on-point */
				if( i>0 )
					{
					if( GDC_grid == GDC_TICK_POINTS )				/* --- GRID --- */
						DO_GRID( x1, y1, x2, y2 );
					else if( GDC_grid > GDC_TICK_NONE )
						{
						int k;
						int	xt;
						int	prevx      = PX(i-1);
						int	intrv_dist = (x1-prevx)/(GDC_grid+1);
						DO_GRID( x1, y1, x2, y2 );
						for( k=0,          xt=prevx + intrv_dist;
							 k<GDC_grid && xt<x1;
							 ++k,          xt += intrv_dist )
							DO_GRID( xt, y1, xt+xdepth_3Dtotal, y2 );
						}

					if( GDC_ticks == GDC_TICK_POINTS )				/* --- TICKS --- */
						DO_TICK(x1,y1);
					else if( GDC_ticks > GDC_TICK_NONE )
						{
						int k;
						int	xt;
						int	prevx=PX(i-1);
						int	intrv_dist = (x1-prevx)/(GDC_ticks+1);
						DO_TICK( x1, y1 );
						for( k=0,           xt=prevx + intrv_dist;
							 k<GDC_ticks && xt<x1;
							 ++k,           xt += intrv_dist )
							DO_TICK( xt, y1 );
						}
					}
			}
		}

	/* ----- secondard data plotting (volume) ----- */
	/*  so that grid lines appear under vol */
	if( do_vol )
		{
		setno = set_depth;
		if( type == GDC_COMBO_HLC_BAR    ||
			type == GDC_COMBO_LINE_BAR   ||
			type == GDC_3DCOMBO_LINE_BAR ||
			type == GDC_3DCOMBO_HLC_BAR )
			{
			if( uvol[0] != GDC_NOVALUE )
				draw_3d_bar( PX(0), PX(0)+hlf_barwdth,
								 PV(0), PV(uvol[0]),
								 0, 0,
								 ExtVolColor[0],
								 ExtVolColor[0] );
			for( i=1; i<num_points-1; ++i )
				if( uvol[i] != GDC_NOVALUE )
					draw_3d_bar( PX(i)-hlf_barwdth, PX(i)+hlf_barwdth,
									 PV(0), PV(uvol[i]),
									 0, 0,
									 ExtVolColor[i],
									 ExtVolColor[i] );
			if( uvol[i] != GDC_NOVALUE )
				draw_3d_bar( PX(i)-hlf_barwdth, PX(i),
								 PV(0), PV(uvol[i]),
								 0, 0,
								 ExtVolColor[i],
								 ExtVolColor[i] );
			}
		else
		if( type == GDC_COMBO_HLC_AREA   ||
			type == GDC_COMBO_LINE_AREA  ||
			type == GDC_3DCOMBO_LINE_AREA||
			type == GDC_3DCOMBO_HLC_AREA )
			{
			for( i=1; i<num_points; ++i )
				if( uvol[i-1] != GDC_NOVALUE && uvol[i] != GDC_NOVALUE )
					draw_3d_area( PX(i-1), PX(i),
									 PV(0), PV(uvol[i-1]), PV(uvol[i]),
									 0, 0,
									 ExtVolColor[i],
									 ExtVolColor[i] );
			}
		else
		if( type == GDC_COMBO_LINE_LINE ||
			type == GDC_3DCOMBO_LINE_LINE )
			{
			for( i=1; i<num_points; ++i )
				if( uvol[i-1] != GDC_NOVALUE && uvol[i] != GDC_NOVALUE )
					line( PX(i-1), PV(uvol[i-1]),
									 PX(i),   PV(uvol[i]),
									 ExtVolColor[i] );
			}
		setno = 0;
		}		/* volume polys done */

	if( GDC_annotation && threeD )		/* back half of annotation line */
		{
		int	x1 = PX(GDC_annotation->point+(do_bar?1:0)),
			y1 = PY(lowest);
		setno = set_depth;
		line( x1, y1, PX(GDC_annotation->point+(do_bar?1:0)), PY(lowest), AnnoteColor );
		line( PX(GDC_annotation->point+(do_bar?1:0)), PY(lowest),
						 PX(GDC_annotation->point+(do_bar?1:0)), PY(highest)-2, AnnoteColor );
		setno = 0;
		}

	/* ---------- start plotting the data ---------- */
	switch( type )
		{
		case GDC_3DBAR:					/* depth, width, y interval need to allow for whitespace between bars */
		case GDC_BAR:
		/* --------- */
		switch( GDC_stack_type )
			{
			case GDC_STACK_DEPTH:
			for( setno=num_sets-1; setno>=0; --setno )		/* back sets first   PX, PY depth */
				for( i=0; i<num_points; ++i )
					if( uvals[setno][i] != GDC_NOVALUE )
						draw_3d_bar( PX(i+(do_bar?1:0))-hlf_barwdth, PX(i+(do_bar?1:0))+hlf_barwdth,
										 PY(0), PY(uvals[setno][i]),
										 xdepth_3D, ydepth_3D,
										 ExtColor[setno][i],
										 threeD? ExtColorShd[setno][i]: ExtColor[setno][i] );
			setno = 0;
			break;

			case GDC_STACK_LAYER:
				{
				int			j = 0;
				CREATE_ARRAY1( barset, struct BS, num_sets );

/*				double	lasty[ num_points ]; */
/*				for( i=0; i<num_points; ++i ) */
/*					if( uvals[j][i] != GDC_NOVALUE ) */
/*						{ */
/*						lasty[i] = uvals[j][i]; */
/*						draw_3d_bar( PX(i+(do_bar?1:0))-hlf_barwdth, PX(i+(do_bar?1:0))+hlf_barwdth, */
/*										 PY(0), PY(uvals[j][i]), */
/*										 xdepth_3D, ydepth_3D, */
/*										 ExtColor[j][i], */
/*										 threeD? ExtColorShd[j][i]: ExtColor[j][i] ); */
/*						} */
				for( i=0; i<num_points; ++i )
					{
					double		lasty_pos = 0.0;
					double		lasty_neg = 0.0;
					int			k;

					for( j=0, k=0; j<num_sets; ++j )
						{
						if( uvals[j][i] != GDC_NOVALUE )
							{
							if( uvals[j][i] < 0.0 )
								{
								barset[k].y1 = lasty_neg;
								barset[k].y2 = uvals[j][i] + lasty_neg;
								lasty_neg    = barset[k].y2;
								}
							else
								{
								barset[k].y1 = lasty_pos;
								barset[k].y2 = uvals[j][i] + lasty_pos;
								lasty_pos    = barset[k].y2;
								}
							barset[k].clr   = ExtColor[j][i];
							barset[k].shclr = threeD? ExtColorShd[j][i]: ExtColor[j][i];
							++k;
							}
						}
					qsort( barset, k, sizeof(struct BS), barcmpr );

					for( j=0; j<k; ++j )
						{
						draw_3d_bar(
									 PX(i+(do_bar?1:0))-hlf_barwdth, PX(i+(do_bar?1:0))+hlf_barwdth,
									 PY(barset[j].y1), PY(barset[j].y2),
									 xdepth_3D, ydepth_3D,
									 barset[j].clr,
									 barset[j].shclr );
						}
					}
				FREE_ARRAY1( barset );
				}
				break;

			case GDC_STACK_BESIDE:
				{												/* h/.5, h/1, h/1.5, h/2, ... */
				int	new_barwdth = (int)( (double)hlf_barwdth / ((double)num_sets/2.0) );
				for( i=0; i<num_points; ++i )
					for( j=0; j<num_sets; ++j )
						if( uvals[j][i] != GDC_NOVALUE )
							draw_3d_bar( PX(i+(do_bar?1:0))-hlf_barwdth+new_barwdth*j+1,
											 PX(i+(do_bar?1:0))-hlf_barwdth+new_barwdth*(j+1),
											 PY(0), PY(uvals[j][i]),
											 xdepth_3D, ydepth_3D,
											 ExtColor[j][i],
											 threeD? ExtColorShd[j][i]: ExtColor[j][i] );
					}
				break;
			}
			break;

		case GDC_3DFLOATINGBAR:
		case GDC_FLOATINGBAR:
		  /* --------- */
		  switch( GDC_stack_type )
			{
			case GDC_STACK_DEPTH:
				for( setno=num_groups-1; setno>=0; --setno )	/* back sets first   PX, PY depth */
					for( i=0; i<num_points; ++i )
						if( uvals[0+setno*2][i] != GDC_NOVALUE &&
							uvals[1+setno*2][i] != GDC_NOVALUE &&
							uvals[1+setno*2][i] > uvals[0+setno*2][i] )
							draw_3d_bar( PX(i+(do_bar?1:0))-hlf_barwdth, PX(i+(do_bar?1:0))+hlf_barwdth,
										 PY(uvals[0+setno*2][i]), PY(uvals[1+setno*2][i]),
										 xdepth_3D, ydepth_3D,
										 ExtColor[setno][i],
										 threeD? ExtColorShd[setno][i]: ExtColor[setno][i] );
				setno = 0;
				break;

			case GDC_STACK_BESIDE:
				{												/* h/.5, h/1, h/1.5, h/2, ... */
				int	new_barwdth = (int)( (double)hlf_barwdth / ((double)num_groups/2.0) );
				for( i=0; i<num_points; ++i )
					for( j=0; j<num_groups; ++j )
						if( uvals[0+j*2][i] != GDC_NOVALUE &&
							uvals[1+j*2][i] != GDC_NOVALUE &&
							uvals[1+j*2][i] > uvals[0+j*2][i] )
							draw_3d_bar( PX(i+(do_bar?1:0))-hlf_barwdth+new_barwdth*j+1,
											 PX(i+(do_bar?1:0))-hlf_barwdth+new_barwdth*(j+1),
											 PY(uvals[0+j*2][i]), PY(uvals[1+j*2][i]),
											 xdepth_3D, ydepth_3D,
											 ExtColor[j][i],
											 threeD? ExtColorShd[j][i]: ExtColor[j][i] );
				}
				break;
			}
			break;

		case GDC_LINE:
		case GDC_COMBO_LINE_BAR:
		case GDC_COMBO_LINE_AREA:
		case GDC_COMBO_LINE_LINE:
			for( j=num_sets-1; j>=0; --j )
				for( i=1; i<num_points; ++i )
					if( uvals[j][i-1] != GDC_NOVALUE && uvals[j][i] != GDC_NOVALUE )
						{
						line( PX(i-1), PY(uvals[j][i-1]), PX(i), PY(uvals[j][i]), ExtColor[j][i] );
						line( PX(i-1), PY(uvals[j][i-1])+1, PX(i), PY(uvals[j][i])+1, ExtColor[j][i] );
						}
					else
						{
						if( uvals[j][i-1] != GDC_NOVALUE )
							setPixel( PX(i-1), PY(uvals[j][i-1]), ExtColor[j][i] );
						if( uvals[j][i] != GDC_NOVALUE )
							setPixel( PX(i), PY(uvals[j][i]), ExtColor[j][i] );
						}
			break;

		case GDC_3DLINE:
		case GDC_3DCOMBO_LINE_BAR:
		case GDC_3DCOMBO_LINE_AREA:
		case GDC_3DCOMBO_LINE_LINE:
			{
			CREATE_ARRAY1( y1, int, num_sets );
			CREATE_ARRAY1( y2, int, num_sets );
			CREATE_ARRAY1( clr,    int, num_sets );
			CREATE_ARRAY1( clrshd, int, num_sets );

			for( i=1; i<num_points; ++i )
				{
				if( GDC_stack_type == GDC_STACK_DEPTH )
					{
					for( j=num_sets-1; j>=0; --j )
						if( uvals[j][i-1] != GDC_NOVALUE &&
							uvals[j][i]   != GDC_NOVALUE )
							{
							setno = j;
							y1[j] = PY(uvals[j][i-1]);
							y2[j] = PY(uvals[j][i]);

							draw_3d_line(
										  PY(0),
										  PX(i-1), PX(i), 
										  &(y1[j]), &(y2[j]),
										  xdepth_3D, ydepth_3D,
										  1,
										  &(ExtColor[j][i]),
										  &(ExtColorShd[j][i]) );
							setno = 0;
							}
					}
				else
				if( GDC_stack_type == GDC_STACK_BESIDE ||
					GDC_stack_type == GDC_STACK_SUM )			/* all same plane */
					{
					int		set;
					double	usey1 = 0.0,
							usey2 = 0.0;
					for( j=0,set=0; j<num_sets; ++j )
						if( uvals[j][i-1] != GDC_NOVALUE &&
							uvals[j][i]   != GDC_NOVALUE )
							{
							if( GDC_stack_type == GDC_STACK_SUM )
								{
								usey1 += uvals[j][i-1];
								usey2 += uvals[j][i];
								}
							else
								{
								usey1 = uvals[j][i-1];
								usey2 = uvals[j][i];
								}
							y1[set]     = PY(usey1);
							y2[set]     = PY(usey2);
							clr[set]    = ExtColor[j][i];
							clrshd[set] = ExtColorShd[j][i];	/* fred */
							++set;
							}
					draw_3d_line(
						  PY(0),
						  PX(i-1), PX(i), 
						  y1, y2,
						  xdepth_3D, ydepth_3D,
						  set,
						  clr,
						  clrshd );
					}
				}
			FREE_ARRAY1( clr );
			FREE_ARRAY1( clrshd );
			FREE_ARRAY1( y1 );
			FREE_ARRAY1( y2 );
			}
			break;

		case GDC_AREA:
		case GDC_3DAREA:
		  switch( GDC_stack_type )
			{
			case GDC_STACK_SUM:
				{
				CREATE_ARRAY1( lasty, double, num_points );
				int		j = 0;
				for( i=1; i<num_points; ++i )
					if( uvals[j][i] != GDC_NOVALUE )
						{
						lasty[i] = uvals[j][i];
						if( uvals[j][i-1] != GDC_NOVALUE )
							draw_3d_area( PX(i-1), PX(i),
											 PY(0), PY(uvals[j][i-1]), PY(uvals[j][i]),
											 xdepth_3D, ydepth_3D,
											 ExtColor[j][i],
											 threeD? ExtColorShd[j][i]: ExtColor[j][i] );
						}
				for( j=1; j<num_sets; ++j )
					for( i=1; i<num_points; ++i )
						if( uvals[j][i] != GDC_NOVALUE && uvals[j][i-1] != GDC_NOVALUE )
							{
							draw_3d_area( PX(i-1), PX(i),
											 PY(lasty[i]), PY(lasty[i-1]+uvals[j][i-1]), PY(lasty[i]+uvals[j][i]),
											 xdepth_3D, ydepth_3D,
											 ExtColor[j][i],
                                             threeD? ExtColorShd[j][i]: ExtColor[j][i] );
							lasty[i] += uvals[j][i];
							}
				FREE_ARRAY1( lasty );
				}
				break;

			case GDC_STACK_BESIDE:								/* behind w/o depth */
				for( j=num_sets-1; j>=0; --j )					/* back sets 1st  (setno = 0) */
					for( i=1; i<num_points; ++i )
						if( uvals[j][i-1] != GDC_NOVALUE && uvals[j][i] != GDC_NOVALUE )
							draw_3d_area( PX(i-1), PX(i),
											 PY(0), PY(uvals[j][i-1]), PY(uvals[j][i]),
											 xdepth_3D, ydepth_3D,
											 ExtColor[j][i],
                                             threeD? ExtColorShd[j][i]: ExtColor[j][i] );
				break;

			case GDC_STACK_DEPTH:
			default:
				for( setno=num_sets-1; setno>=0; --setno )		/* back sets first   PX, PY depth */
					for( i=1; i<num_points; ++i )
						if( uvals[setno][i-1] != GDC_NOVALUE && uvals[setno][i] != GDC_NOVALUE )
							draw_3d_area( PX(i-1), PX(i),
											 PY(0), PY(uvals[setno][i-1]), PY(uvals[setno][i]),
											 xdepth_3D, ydepth_3D,
											 ExtColor[setno][i],
                                             threeD? ExtColorShd[setno][i]: ExtColor[setno][i] );
				setno = 0;
			}
			break;

		case GDC_3DHILOCLOSE:
		case GDC_3DCOMBO_HLC_BAR:
		case GDC_3DCOMBO_HLC_AREA:
			{
			gdPoint     poly[4];
			for( j=num_groups-1; j>=0; --j )
			 {
			 for( i=1; i<num_points+1; ++i )
				 if( uvals[CLOSESET+j*3][i-1] != GDC_NOVALUE )
					 {
					 if( (GDC_HLC_style & GDC_HLC_I_CAP) &&			/* bottom half of 'I' */
						 uvals[LOWSET+j*3][i-1] != GDC_NOVALUE )
						 {
						 SET_3D_POLY( poly, PX(i-1)-hlf_hlccapwdth, PX(i-1)+hlf_hlccapwdth,
											PY(uvals[LOWSET+j*3][i-1]), PY(uvals[LOWSET+j*3][i-1]),
											xdepth_3D, ydepth_3D );
						 filledPolygon( poly, 4, ExtColor[LOWSET+j*3][i-1] );
						 polygon( poly, 4, ExtColorShd[LOWSET+j*3][i-1] );
						 }
																	 /* all HLC have vert line */
					 if( uvals[LOWSET+j*3][i-1] != GDC_NOVALUE )
						 {											/* bottom 'half' */
						 SET_3D_POLY( poly, PX(i-1), PX(i-1),
											PY(uvals[LOWSET+j*3][i-1]), PY(uvals[CLOSESET+j*3][i-1]),
											xdepth_3D, ydepth_3D );
						 filledPolygon( poly, 4, ExtColor[LOWSET+j*3][i-1] );
						 polygon( poly, 4, ExtColorShd[LOWSET+j*3][i-1] );
						 }
					 if( uvals[HIGHSET+j*3][i-1] != GDC_NOVALUE )
						 {											/* top 'half' */
						 SET_3D_POLY( poly, PX(i-1), PX(i-1),
											PY(uvals[CLOSESET+j*3][i-1]), PY(uvals[HIGHSET+j*3][i-1]),
											xdepth_3D, ydepth_3D );
						 filledPolygon( poly, 4, ExtColor[HIGHSET+j*3][i-1] );
						 polygon( poly, 4, ExtColorShd[HIGHSET+j*3][i-1] );
						 }
																	/* line at close */
					 line( PX(i-1),           PY(uvals[CLOSESET+j*3][i-1]),
									  PX(i-1)+xdepth_3D, PY(uvals[CLOSESET+j*3][i-1])-ydepth_3D,
									  ExtColorShd[CLOSESET+j*3][i-1] );
																 /* top half 'I' */
					 if( !( (GDC_HLC_style & GDC_HLC_DIAMOND) &&
							(PY(uvals[HIGHSET+j*3][i-1]) > PY(uvals[CLOSESET+j*3][i-1])-hlf_hlccapwdth) ) &&
						 uvals[HIGHSET+j*3][i-1] != GDC_NOVALUE )
						 if( GDC_HLC_style & GDC_HLC_I_CAP )
							 {
							 SET_3D_POLY( poly, PX(i-1)-hlf_hlccapwdth, PX(i-1)+hlf_hlccapwdth,
												PY(uvals[HIGHSET+j*3][i-1]), PY(uvals[HIGHSET+j*3][i-1]),
												xdepth_3D, ydepth_3D );
							 filledPolygon( poly, 4, ExtColor[HIGHSET+j*3][i-1] );
							 polygon( poly, 4, ExtColorShd[HIGHSET+j*3][i-1] );
							 }

					 if( i < num_points &&
						 uvals[CLOSESET+j*3][i] != GDC_NOVALUE )
						 {
						 if( GDC_HLC_style & GDC_HLC_CLOSE_CONNECTED )	/* line from prev close */
							 {
							 SET_3D_POLY( poly, PX(i-1), PX(i),
												PY(uvals[CLOSESET+j*3][i-1]), PY(uvals[CLOSESET+j*3][i-1]),
												xdepth_3D, ydepth_3D );
							 filledPolygon( poly, 4, ExtColor[CLOSESET+j*3][i] );
							 polygon( poly, 4, ExtColorShd[CLOSESET+j*3][i] );
							 }
						 else	/* CLOSE_CONNECTED and CONNECTING are mutually exclusive */
						 if( GDC_HLC_style & GDC_HLC_CONNECTING )	/* thin connecting line */
							 {
							 int	y1 = PY(uvals[CLOSESET+j*3][i-1]),
								 y2 = PY(uvals[CLOSESET+j*3][i]);
							 draw_3d_line(
										   PY(0),
										   PX(i-1), PX(i),
										   &y1, &y2,					/* rem only 1 set */
										   xdepth_3D, ydepth_3D,
										   1,
										   &(ExtColor[CLOSESET+j*3][i]),
										   &(ExtColorShd[CLOSESET+j*3][i]) );
																	 /* edge font of it */
							 line( PX(i-1), PY(uvals[CLOSESET+j*3][i-1]),
											  PX(i), PY(uvals[CLOSESET+j*3][i]),
											  ExtColorShd[CLOSESET+j*3][i] );
							 }
																	 /* top half 'I' again */
						 if( PY(uvals[CLOSESET+j*3][i-1]) <= PY(uvals[CLOSESET+j*3][i]) &&
							 uvals[HIGHSET+j*3][i-1] != GDC_NOVALUE  )
							 if( GDC_HLC_style & GDC_HLC_I_CAP )
								 {
								 SET_3D_POLY( poly, PX(i-1)-hlf_hlccapwdth, PX(i-1)+hlf_hlccapwdth,
													PY(uvals[HIGHSET+j*3][i-1]), PY(uvals[HIGHSET+j*3][i-1]),
													xdepth_3D, ydepth_3D );
								 filledPolygon( poly, 4, ExtColor[HIGHSET+j*3][i-1] );
								 polygon( poly, 4, ExtColorShd[HIGHSET+j*3][i-1] );
								 }
						 }
					 if( GDC_HLC_style & GDC_HLC_DIAMOND )
						 {									/* front */
						 poly[0].x = PX(i-1)-hlf_hlccapwdth;
						  poly[0].y = PY(uvals[CLOSESET+j*3][i-1]);
						 poly[1].x = PX(i-1);
						  poly[1].y = PY(uvals[CLOSESET+j*3][i-1])+hlf_hlccapwdth;
						 poly[2].x = PX(i-1)+hlf_hlccapwdth;
						  poly[2].y = PY(uvals[CLOSESET+j*3][i-1]);
						 poly[3].x = PX(i-1);
						  poly[3].y = PY(uvals[CLOSESET+j*3][i-1])-hlf_hlccapwdth;
						 filledPolygon( poly, 4, ExtColor[CLOSESET+j*3][i-1] );
						 polygon( poly, 4, ExtColorShd[CLOSESET+j*3][i-1] );
															 /* bottom side */
						 SET_3D_POLY( poly, PX(i-1), PX(i-1)+hlf_hlccapwdth,
											PY(uvals[CLOSESET+j*3][i-1])+hlf_hlccapwdth,
													 PY(uvals[CLOSESET+j*3][i-1]),
											xdepth_3D, ydepth_3D );
						 filledPolygon( poly, 4, ExtColorShd[CLOSESET+j*3][i-1] );
						 /* polygon( poly, 4, ExtColor[CLOSESET+j*3][i-1] ); */
															 /* top side */
						 SET_3D_POLY( poly, PX(i-1), PX(i-1)+hlf_hlccapwdth,
											PY(uvals[CLOSESET+j*3][i-1])-hlf_hlccapwdth,
													 PY(uvals[CLOSESET+j*3][i-1]),
											xdepth_3D, ydepth_3D );
						 filledPolygon( poly, 4, ExtColor[CLOSESET+j*3][i-1] );
						 polygon( poly, 4, ExtColorShd[CLOSESET+j*3][i-1] );
						 }
					 }
			 }
			}
			break;

		case GDC_HILOCLOSE:
		case GDC_COMBO_HLC_BAR:
		case GDC_COMBO_HLC_AREA:
			for( j=num_groups-1; j>=0; --j )
				{
				for( i=0; i<num_points; ++i )
					if( uvals[CLOSESET+j*3][i] != GDC_NOVALUE )
						{											/* all HLC have vert line */
						if( uvals[LOWSET+j*3][i] != GDC_NOVALUE )
							line( PX(i), PY(uvals[CLOSESET+j*3][i]),
											 PX(i), PY(uvals[LOWSET+j*3][i]),
											 ExtColor[LOWSET+(j*3)][i] );
						if( uvals[HIGHSET+j*3][i] != GDC_NOVALUE )
							line( PX(i), PY(uvals[HIGHSET+j*3][i]),
											 PX(i), PY(uvals[CLOSESET+j*3][i]),
											 ExtColor[HIGHSET+j*3][i] );

						if( GDC_HLC_style & GDC_HLC_I_CAP )
							{
							if( uvals[LOWSET+j*3][i] != GDC_NOVALUE )
								line( PX(i)-hlf_hlccapwdth, PY(uvals[LOWSET+j*3][i]),
												 PX(i)+hlf_hlccapwdth, PY(uvals[LOWSET+j*3][i]),
												 ExtColor[LOWSET+j*3][i] );
							if( uvals[HIGHSET+j*3][i] != GDC_NOVALUE )
								line( PX(i)-hlf_hlccapwdth, PY(uvals[HIGHSET+j*3][i]),
												 PX(i)+hlf_hlccapwdth, PY(uvals[HIGHSET+j*3][i]),
												 ExtColor[HIGHSET+j*3][i] );
							}
						if( GDC_HLC_style & GDC_HLC_DIAMOND )
							{
							gdPoint         cd[4];

							cd[0].x = PX(i)-hlf_hlccapwdth;	cd[0].y = PY(uvals[CLOSESET+j*3][i]);
							cd[1].x = PX(i);	cd[1].y = PY(uvals[CLOSESET+j*3][i])+hlf_hlccapwdth;
							cd[2].x = PX(i)+hlf_hlccapwdth;	cd[2].y = PY(uvals[CLOSESET+j*3][i]);
							cd[3].x = PX(i);	cd[3].y = PY(uvals[CLOSESET+j*3][i])-hlf_hlccapwdth;
							filledPolygon( cd, 4, ExtColor[CLOSESET+j*3][i] );
							}
						}
				for( i=1; i<num_points; ++i )
					if( uvals[CLOSESET+j*3][i-1] != GDC_NOVALUE && uvals[CLOSESET+j*3][i] != GDC_NOVALUE )
						{
						if( GDC_HLC_style & GDC_HLC_CLOSE_CONNECTED )	/* line from prev close */
								line( PX(i-1), PY(uvals[CLOSESET+j*3][i-1]),
												 PX(i), PY(uvals[CLOSESET+j*3][i-1]),
												 ExtColor[CLOSESET+j*3][i] );
						else	/* CLOSE_CONNECTED and CONNECTING are mutually exclusive */
						if( GDC_HLC_style & GDC_HLC_CONNECTING )		/* thin connecting line */
							line( PX(i-1), PY(uvals[CLOSESET+j*3][i-1]),
											 PX(i), PY(uvals[CLOSESET+j*3][i]),
											 ExtColor[CLOSESET+j*3][i] );
						}
				}
			break;
		}
		setno = 0;

	/* ---------- scatter points  over all other plots ---------- */
	/* scatters, by their very nature, don't lend themselves to standard array of points */
	/* also, this affords the opportunity to include scatter points onto any type of chart */
	/* drawing of the scatter point should be an exposed function, so the user can */
	/*  use it to draw a legend, and/or add their own */
	if( GDC_scatter )
		{
		CREATE_ARRAY1( scatter_clr, int, GDC_num_scatter_pts );
		gdPoint	ct[3];

		for( i=0; i<GDC_num_scatter_pts; ++i )
			{
			int		hlf_scatterwdth = (int)( (double)(PX(2)-PX(1))
											 * (((double)((GDC_scatter+i)->width)/100.0)/2.0) );
			int	scat_x = PX( (GDC_scatter+i)->point + (do_bar?1:0) ),
				scat_y = PY( (GDC_scatter+i)->val );

			if( (GDC_scatter+i)->point >= num_points ||				/* invalid point */
				(GDC_scatter+i)->point <  0 )
				continue;
			scatter_clr[i] = clrallocate( (GDC_scatter+i)->color );

			switch( (GDC_scatter+i)->ind )
				{
                case GDC_SCATTER_CIRCLE:
					{
					long	uniq_clr = get_uniq_color();
					int		s        = 0,
							e        = 360,
							fo       = 0;

					if( !do_bar )
						if( (GDC_scatter+i)->point == 0 )
							{ s = 270; e = 270+180; fo = 1; }
						else
						if( (GDC_scatter+i)->point == num_points-1 )
							{ s = 90; e = 90+180; fo = -1; }
					if( uniq_clr != -1L )							/* the safe way */
						{
						int	uc = colorAllocate( l2gdcal(uniq_clr) );
						arc( scat_x, scat_y,
										hlf_scatterwdth*2, hlf_scatterwdth*2,
										s, e,
										uc );
						if( fo )									/* close off  semi-circle case */
							line( scat_x, scat_y+hlf_scatterwdth,
											 scat_x, scat_y-hlf_scatterwdth,
											 uc );
						fillToBorder( scat_x+fo, scat_y, uc, scatter_clr[i] );
						arc( scat_x, scat_y,
										hlf_scatterwdth*2, hlf_scatterwdth*2,
										s, e,
										scatter_clr[i] );
						if( fo )
							line( scat_x, scat_y+hlf_scatterwdth,
											 scat_x, scat_y-hlf_scatterwdth,
											 scatter_clr[i] );
						colorDeallocate( uc );
						}
					else											/* chance it */
						{
						arc( scat_x, scat_y,
										hlf_scatterwdth*2, hlf_scatterwdth*2,
										s, e,
										scatter_clr[i] );
						if( fo )
							line( scat_x, scat_y+hlf_scatterwdth,
											 scat_x, scat_y-hlf_scatterwdth,
											 scatter_clr[i] );
						fillToBorder( scat_x+fo, scat_y,
												 scatter_clr[i], scatter_clr[i] );
						}
					}
                    break;
				case GDC_SCATTER_TRIANGLE_UP:
					ct[0].x = scat_x;
					ct[0].y = scat_y;
					ct[1].x = scat_x - hlf_scatterwdth;
					ct[1].y = scat_y + hlf_scatterwdth;;
					ct[2].x = scat_x + hlf_scatterwdth;
					ct[2].y = scat_y + hlf_scatterwdth;
					if( !do_bar )
						if( (GDC_scatter+i)->point == 0 )
							ct[1].x = scat_x;
						else
						if( (GDC_scatter+i)->point == num_points-1 )
							ct[2].x = scat_x;
					filledPolygon( ct, 3, scatter_clr[i] );
					break;
				case GDC_SCATTER_TRIANGLE_DOWN:
					ct[0].x = scat_x;
					ct[0].y = scat_y;
					ct[1].x = scat_x - hlf_scatterwdth;
					ct[1].y = scat_y - hlf_scatterwdth;;
					ct[2].x = scat_x + hlf_scatterwdth;
					ct[2].y = scat_y - hlf_scatterwdth;
					if( !do_bar )
						if( (GDC_scatter+i)->point == 0 )
							ct[1].x = scat_x;
						else
						if( (GDC_scatter+i)->point == num_points-1 )
							ct[2].x = scat_x;
					filledPolygon( ct, 3, scatter_clr[i] );
					break;
				}
			}
		FREE_ARRAY1( scatter_clr );
		}

	/* box it off */
	/*  after plotting so the outline covers any plot lines */
	{
	if( GDC_border == GDC_BORDER_ALL || (GDC_border & GDC_BORDER_X) )
		line(          PX(0),   PY(lowest), PX(num_points-1+(do_bar?2:0)),  PY(lowest), LineColor );

	if( GDC_border == GDC_BORDER_ALL || (GDC_border & GDC_BORDER_TOP) )
		{
		setno = set_depth;
		line(          PX(0),   PY(highest), PX(num_points-1+(do_bar?2:0)),  PY(highest), LineColor );
		setno = 0;
		}
	}
	if( GDC_border )
		{
		int	x1, y1, x2, y2;

		x1 = PX(0);
		y1 = PY(highest);
		x2 = PX(num_points-1+(do_bar?2:0));
		y2 = PY(lowest);
		if( GDC_border == GDC_BORDER_ALL || (GDC_border & GDC_BORDER_Y) )
			line( x1, PY(lowest), x1, y1, LineColor );

		setno = set_depth;
		if( GDC_border == GDC_BORDER_ALL || (GDC_border & GDC_BORDER_Y) || (GDC_border & GDC_BORDER_TOP) )
			line( x1, y1, PX(0), PY(highest), LineColor );
		/* if( !GDC_grid || do_vol || GDC_thumbnail )					// grid leaves right side Y open */
			{
			if( GDC_border == GDC_BORDER_ALL || (GDC_border & GDC_BORDER_X) || (GDC_border & GDC_BORDER_Y2) )
				line( x2, y2, PX(num_points-1+(do_bar?2:0)), PY(lowest), LineColor );
			if( GDC_border == GDC_BORDER_ALL || (GDC_border & GDC_BORDER_Y2) )
				line( PX(num_points-1+(do_bar?2:0)), PY(lowest),
								 PX(num_points-1+(do_bar?2:0)), PY(highest), LineColor );
			}
		setno = 0;
		}

	if( GDC_0Shelf && threeD &&								/* front of 0 shelf */
		( (lowest < 0.0 && highest > 0.0) ||
		  ( (lowest == 0.0 || highest == 0.0) && !(GDC_border&GDC_BORDER_X) ) ) )
		{
		int	x2 = PX( num_points-1+(do_bar?2:0) ),
			y2 = PY( 0 );

		line( PX(0), PY(0), x2, y2, LineColor );		/* front line */
		setno = set_depth;
																/* depth for 3Ds */
		line( x2, y2, PX(num_points-1+(do_bar?2:0)), PY(0), LineColor );
		setno = 0;												/* set back to foremost */
		}

	if( GDC_annotation )			/* front half of annotation line */
		{
		int		x1 = PX(GDC_annotation->point+(do_bar?1:0)),
				y1 = PY(highest);
		int		x2;
															/* front line */
		line( x1, PY(lowest)+1, x1, y1, AnnoteColor );
		if( threeD )
			{												/* on back plane */
			setno = set_depth;
			x2 = PX(GDC_annotation->point+(do_bar?1:0));
															/* prspective line */
			line( x1, y1, x2, PY(highest), AnnoteColor );
			}
		else												/* for 3D done with back line */
			{
			x2 = PX(GDC_annotation->point+(do_bar?1:0));
			line( x1, y1, x1, y1-2, AnnoteColor );
			}
		/* line-to and note */
		if( *(GDC_annotation->note) )						/* any note? */
			{
			if( GDC_annotation->point >= (num_points/2) )		/* note to the left */
				{
				line( x2,              PY(highest)-2,
								 x2-annote_hgt/2, PY(highest)-2-annote_hgt/2,
								 AnnoteColor );
				GDCImageStringNL(
								  &GDC_fontc[GDC_annotation_font_size],
								  gdc_annotation_font, gdc_annotation_ptsize,
								  0.0,
								  x2-annote_hgt/2-1-annote_len - 1,
								  PY(highest)-annote_hgt+1,
								  GDC_annotation->note,
								  AnnoteColor,
								  GDC_JUSTIFY_RIGHT,
								  NULL );
				}
			else												/* note to right */
				{
				line( x2,              PY(highest)-2,
								 x2+annote_hgt/2, PY(highest)-2-annote_hgt/2,
								 AnnoteColor );
				GDCImageStringNL(
								  &GDC_fontc[GDC_annotation_font_size],
								  gdc_annotation_font, gdc_annotation_ptsize,
								  0.0,
								  x2+annote_hgt/2+1 + 1,
								  PY(highest)-annote_hgt+1,
								  GDC_annotation->note,
								  AnnoteColor,
								  GDC_JUSTIFY_LEFT,
								  NULL );
				}
			}
		setno = 0;
		}


	/* usually GDC_generate_img is used in conjunction with hard or hold options */
	if( GDC_generate_img )
		{
		fflush(img_fptr);			/* clear anything buffered  */
		switch( GDC_image_type )
			{
#ifdef HAVE_JPEG
			case GDC_JPEG:	gdImageJpeg( im, img_fptr, GDC_jpeg_quality );	break;
#endif
			case GDC_WBMP:	wbmp(PlotColor, img_fptr );			break;
			case GDC_GIF:	gif(img_fptr);						break;
			case GDC_PNG:
			default:		png(img_fptr );
			}
		}

	FREE_ARRAY1( uvals );
	FREE_ARRAY1( ExtVolColor );
	FREE_ARRAY2( ExtColor );
	FREE_ARRAY2( ExtColorShd );
	return 0;
}
//------------------------------------------------------------------------------
GDChart::GDC_FONT_T GDChart::GDC_fontc[GDChart::GDC_numfonts] =
{
  { NULL, 8,  5 },
  { NULL, 8,  5 },
  { NULL, 12, 6 },
  { NULL, 13, 7 },
  { NULL, 16, 8 },
  { NULL, 15, 9 }
};
//------------------------------------------------------------------------------
GDChart::~GDChart()
{
}
//------------------------------------------------------------------------------
GDChart::GDChart() :
  GDC_interpolations(FALSE),
  GDC_stack_type(GDC_STACK_DEPTH),
  GDC_ylabel_fmt(NULL),
  GDC_xaxis_angle(90.0),
  GDC_generate_img(FALSE),
  GDC_BGColor(0x000000L),
  GDC_LineColor(GDC_DFLTCOLOR),
  GDC_hold_img(GDC_DESTROY_IMAGE),
  GDC_image_type(GDC_PNG),
  GDC_thumbnail(FALSE),
  GDC_grid(GDC_TICK_LABELS),
  GDC_ticks(GDC_TICK_LABELS),
  GDC_xaxis(TRUE),
  GDC_yaxis(TRUE),
  GDC_yaxis2(TRUE),
  GDC_yval_style(TRUE),
  GDC_3d_depth(5.0),
  GDC_scatter((GDC_SCATTER_T*) NULL),
  GDC_num_scatter_pts(0),
  GDC_requested_ymin(GDC_NOVALUE),
  GDC_requested_ymax(GDC_NOVALUE),
  GDC_requested_yinterval(GDC_NOVALUE),
  GDC_title(NULL),
  GDC_title_size(GDC_MEDBOLD),
  GDC_ytitle_size(GDC_MEDBOLD),
  GDC_xtitle_size(GDC_MEDBOLD),
  GDC_yaxisfont_size(GDC_SMALL),
  GDC_xaxisfont_size(GDC_SMALL),
  GDC_ytitle(NULL),
  GDC_xtitle(NULL),
  GDC_ytitle2(NULL),
  GDC_3d_angle(45),
  GDC_annotation((GDC_ANNOTATION_T*) NULL),
  GDC_annotation_font_size(GDC_SMALL),
  GDC_xlabel_ctl(NULL),
  GDC_hard_size(FALSE),
  GDC_hard_xorig(0),
  GDC_hard_graphwidth(0),
  GDC_hard_yorig(0),
  GDC_hard_grapheight(0),
  GDC_ylabel2_fmt(NULL),
  GDC_bar_width(75),
  GDC_HLC_style(GDC_HLC_CLOSE_CONNECTED),
  GDC_HLC_cap_width(25),
  GDC_GridColor(0xA0A0A0L),
  GDC_PlotColor(GDC_DFLTCOLOR),
  GDC_VolColor(0xA0A0FFL),
  GDC_ExtVolColor(NULL),
  GDC_SetColor(NULL),
  GDC_ExtColor(NULL),
  GDC_BGImage(NULL),
  GDC_transparent_bg(FALSE),
  GDC_TitleColor(GDC_DFLTCOLOR),
  GDC_XTitleColor(GDC_DFLTCOLOR),
  GDC_YTitleColor(GDC_DFLTCOLOR),
  GDC_YTitle2Color(GDC_DFLTCOLOR),
  GDC_XLabelColor(GDC_DFLTCOLOR),
  GDC_YLabelColor(GDC_DFLTCOLOR),
  GDC_YLabel2Color(GDC_DFLTCOLOR),
  GDC_border(GDC_BORDER_ALL),
  GDC_0Shelf(TRUE),
  GDC_xlabel_spacing(5),
  GDC_ylabel_density(80)

{
}
//------------------------------------------------------------------------------
GDChart & GDChart::createChart(uintptr_t sx,uintptr_t sy)
{
  data_.resize(6);
  data_[0] = 1;
  data_[1] = 2;
  data_[2] = 3;
  data_[3] = 4;
  data_[4] = 5;
  data_[5] = 6;
  GDC_image_type = GDC_PNG;
  char    *t[6] = { "Chicago", "New York", "L.A.", "Atlanta", "Paris, MD\n(USA) ", "London" };
  /* ----- data set colors (RGB) ----- */
  //unsigned long   sc[2]    = { 0xFF8080, 0x8080FF };

  //GDC_BGColor   = 0xFFFFFFL;                  /* backgound color (white) */
  //GDC_LineColor = 0x000000L;                  /* line color      (black) */
  //GDC_SetColor  = &(sc[0]);                   /* assign set colors */

  GDC_out_graph(
    sx & (((unsigned int) ~int(0)) >> 1),
    sy & (((unsigned int) ~int(0)) >> 1),
    NULL,
    GDC_LINE,     /* GDC_CHART_T chart type */
    6,             /* int         number of points per data set */
    t,             /* char*[]     array of X labels */
    1,             /* int         number of data sets */
    data_,
    NULL
  );             /* double[]     data set 1 */
  gdFree(png_);
  png_ = pngPtrEx(&pngSize_,9);
  return *this;
}
//------------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------
