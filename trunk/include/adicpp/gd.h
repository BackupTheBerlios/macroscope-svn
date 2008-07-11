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
#ifndef gdH
#define gdH
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class GD  {
  public:
    virtual ~GD();
    GD();

    static void initialize();
    static void cleanup();

    GD & clear();

    GD & create(uintptr_t sx,uintptr_t sy);
    GD & createTrueColor(uintptr_t sx,uintptr_t sy);
    GD & createFromGif(FILE * fd);
    intptr_t colorAllocate(intptr_t r,intptr_t g,intptr_t b);
    GD & string(void * font, intptr_t x, intptr_t y, const char * s,intptr_t color);
    intptr_t colorExact(intptr_t r,intptr_t g,intptr_t b);
    intptr_t colorsTotal() const;
    intptr_t sx() const;
    intptr_t sy() const;
    intptr_t colorAllocateAlpha(intptr_t r,intptr_t g,intptr_t b,intptr_t a);
    intptr_t colorClosest(intptr_t r,intptr_t g,intptr_t b);
    GD & copyResized(const GD & src, intptr_t dstX, intptr_t dstY, intptr_t srcX, intptr_t srcY, intptr_t dstW, intptr_t dstH, intptr_t srcW, intptr_t srcH);
    GD & colorTransparent(intptr_t color);
    GD & copy(const GD & src, intptr_t dstX, intptr_t dstY, intptr_t srcX, intptr_t srcY, intptr_t w, intptr_t h);
    GD & stringUp(void * font, intptr_t x, intptr_t y, const char *s, intptr_t color);
    GD & line(intptr_t x1, intptr_t y1, intptr_t x2, intptr_t y2, intptr_t color);
    GD & polygon(void * points, intptr_t n, intptr_t c);
    GD & openPolygon(void * points, intptr_t n, intptr_t c);
    GD & filledPolygon(void * points, intptr_t n, intptr_t c);
    GD & setPixel(intptr_t x, intptr_t y, intptr_t color);
    GD & arc(intptr_t cx, intptr_t cy, intptr_t w, intptr_t h, intptr_t s, intptr_t e, intptr_t color);
    GD & fillToBorder(intptr_t x, intptr_t y, intptr_t border, intptr_t color);
    GD & colorDeallocate(intptr_t color);
    GD & fill(intptr_t x, intptr_t y, intptr_t color);
    GD & ellipse(intptr_t cx, intptr_t cy, intptr_t w, intptr_t h, intptr_t color);
    GD & filledRectangle(intptr_t x1, intptr_t y1, intptr_t x2, intptr_t y2, intptr_t color);

    const void * png() const { return png_; }
    uintptr_t pngSize() const { return pngSize_; }
  protected:
    void * image_;
    void * png_;
    intptr_t pngSize_;

    GD & wbmp(int fg, FILE * out);
    GD & gif(FILE * out);
    GD & png(FILE * out);
    void * fontTiny() const;
    void * fontSmall() const;
    void * fontMediumBold() const;
    void * fontLarge() const;
    void * fontGiant() const;
    void * pngPtrEx(intptr_t * size, intptr_t level);
    GD & gdFree(void * m);
  private:
    GD(const GD &);
    void operator = (const GD &);
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class GDChart : public GD {
  public:
    virtual ~GDChart();
    GDChart();

    GDChart & createChart(uintptr_t sx,uintptr_t sy);
  protected:
    typedef enum {
     GDC_LINE,
     GDC_AREA,
     GDC_BAR,
     GDC_FLOATINGBAR,
     GDC_HILOCLOSE,
     GDC_COMBO_LINE_BAR,			/* aka, VOL[ume] */
     GDC_COMBO_HLC_BAR,
     GDC_COMBO_LINE_AREA,
     GDC_COMBO_LINE_LINE,
     GDC_COMBO_HLC_AREA,
     GDC_3DHILOCLOSE,
     GDC_3DCOMBO_LINE_BAR,
     GDC_3DCOMBO_LINE_AREA,
     GDC_3DCOMBO_LINE_LINE,
     GDC_3DCOMBO_HLC_BAR,
     GDC_3DCOMBO_HLC_AREA,
     GDC_3DBAR,
     GDC_3DFLOATINGBAR,
     GDC_3DAREA,
     GDC_3DLINE
     } GDC_CHART_T;

    typedef enum {
     GDC_STACK_DEPTH,				/* "behind" (even non-3D) */
     GDC_STACK_SUM,
     GDC_STACK_BESIDE,
     GDC_STACK_LAYER
     } GDC_STACK_T;					/* applies only to num_lines > 1 */

    typedef enum {
     GDC_HLC_DIAMOND         = 1,
     GDC_HLC_CLOSE_CONNECTED = 2,	/* can't be used w/ CONNECTING */
     GDC_HLC_CONNECTING      = 4,	/* can't be used w/ CLOSE_CONNECTED */
     GDC_HLC_I_CAP           = 8
     } GDC_HLC_STYLE_T;				/* can be OR'd */

											    /* only 1 annotation allowed */
    typedef struct
    {
    double			point;			/* 0 <= point < num_points */
    unsigned long	color;
    char			note[19/*MAX_NOTE_LEN*/+1];	/* NLs ok here */
    } GDC_ANNOTATION_T;

    typedef enum {
     GDC_SCATTER_TRIANGLE_DOWN,
     GDC_SCATTER_TRIANGLE_UP,
     GDC_SCATTER_CIRCLE
     } GDC_SCATTER_IND_T;
    typedef struct
	    {
	    double				point;		/* 0 <= point < num_points */
	    double				val;
	    unsigned short		width;		/* % (1-100) */
	    unsigned long		color;
	    GDC_SCATTER_IND_T	ind;
	    } GDC_SCATTER_T;

    typedef enum {
     GDC_TICK_LABELS = -2,			/* only at labels */
     GDC_TICK_POINTS = -1,			/* each num_points */
     GDC_TICK_NONE	 = 0
     /* > GDC_TICK_NONE */			/* points & inter-point */
     } GDC_TICK_T;

    typedef enum {								/* backward compatible w/ FALSE/TRUE */
	     GDC_BORDER_NONE = 0,
	     GDC_BORDER_ALL  = 1,			/* should be GDC_BORDER_Y|Y2|X|TOP */
	     GDC_BORDER_X    = 2,
	     GDC_BORDER_Y    = 4,
	     GDC_BORDER_Y2   = 8,
	     GDC_BORDER_TOP  = 16
	     } GDC_BORDER_T;
    typedef enum {
		       GDC_GIF = 0,
		       GDC_JPEG = 1,
		       GDC_PNG = 2,
		       GDC_WBMP = 3					/* as of gd1.8.3 WBMP is black and white only. */
    } GDC_image_type_t;

    /* ordered by size */
    enum GDC_font_size { GDC_pad     = 0,
				       GDC_TINY    = 1,
				       GDC_SMALL   = 2,
				       GDC_MEDBOLD = 3,
				       GDC_LARGE   = 4,
				       GDC_GIANT   = 5,
				       GDC_numfonts= 6 };		/* GDC[PIE]_fontc depends on this */

    typedef enum {
		       GDC_DESTROY_IMAGE = 0,			/* default */
		       GDC_EXPOSE_IMAGE  = 1,			/* user must call GDC_destroy_image() */
		       GDC_REUSE_IMAGE   = 2			/* i.e., paint on top of */
		} GDC_HOLD_IMAGE_T;			/* EXPOSE & REUSE */

    struct	GDC_FONT_T	{
			void *  f;
			char		h;
			char		w;
    };

    typedef enum { GDC_JUSTIFY_RIGHT,
			   GDC_JUSTIFY_CENTER,
			   GDC_JUSTIFY_LEFT } GDC_justify_t;

    struct YS { int y1; int y2; double slope; int lnclr; int shclr; };
    struct BS { double y1; double y2; int clr; int shclr; };

    struct fnt_sz_t	{
				int	w;
				int	h;
		};

    char GDC_interpolations;		/* GDC_INTERP_VALUE in data */
    GDC_STACK_T GDC_stack_type;
    char * GDC_ylabel_fmt;
    double GDC_xaxis_angle;
    char GDC_generate_img;
    unsigned long	GDC_BGColor;
    unsigned long	GDC_LineColor;
    GDC_HOLD_IMAGE_T GDC_hold_img;
    static GDC_FONT_T GDC_fontc[];
    GDC_image_type_t GDC_image_type;
    char GDC_thumbnail;
    GDC_TICK_T GDC_grid;
    GDC_TICK_T GDC_ticks;
    char GDC_xaxis;
    char GDC_yaxis;
    char GDC_yaxis2;
    char GDC_yval_style;
    double GDC_3d_depth;
    GDC_SCATTER_T * GDC_scatter;
    int GDC_num_scatter_pts;
    double GDC_requested_ymin;
    double GDC_requested_ymax;
    double GDC_requested_yinterval;
    char * GDC_title;
    GDC_font_size GDC_title_size;
    GDC_font_size	GDC_ytitle_size;
    GDC_font_size	GDC_xtitle_size;
    GDC_font_size	GDC_yaxisfont_size;
    GDC_font_size	GDC_xaxisfont_size;
    char * GDC_ytitle;
    char * GDC_xtitle;
    char * GDC_ytitle2;
    unsigned char GDC_3d_angle;
    GDC_ANNOTATION_T * GDC_annotation;
    GDC_font_size	GDC_annotation_font_size;
    char * GDC_xlabel_ctl;
    char GDC_hard_size;
    int GDC_hard_xorig;
    int GDC_hard_graphwidth;
    int GDC_hard_yorig;
    int GDC_hard_grapheight;
    char * GDC_ylabel2_fmt;
    unsigned char GDC_bar_width;
    GDC_HLC_STYLE_T GDC_HLC_style;
    unsigned char GDC_HLC_cap_width;
    int _gdccfoo1;
    unsigned long _gdccfoo2;
    unsigned long GDC_GridColor;
    unsigned long GDC_PlotColor;
    unsigned long GDC_VolColor;
    unsigned long * GDC_ExtVolColor;
    unsigned long * GDC_SetColor;
    unsigned long * GDC_ExtColor;
    char * GDC_BGImage;
    char GDC_transparent_bg;
    unsigned long GDC_TitleColor;
    unsigned long GDC_XTitleColor;
    unsigned long	GDC_YTitleColor;
    unsigned long	GDC_YTitle2Color;
    unsigned long	GDC_XLabelColor;
    unsigned long	GDC_YLabelColor;
    unsigned long	GDC_YLabel2Color;
    GDC_BORDER_T GDC_border;
    char GDC_0Shelf;
    short GDC_xlabel_spacing;
    char GDC_ylabel_density;

    void out_err( int			IMGWIDTH,
		 int			IMGHEIGHT,
		 FILE			*fptr,
		 unsigned long	BGColor,
		 unsigned long	LineColor,
		 char			*err_str );

    static int qcmpr(const void * a,const void * b);
    static int barcmpr( const void *a, const void *b );
    void draw_3d_line(
			      int			y0,
			      int			x1,
			      int			x2,
			      int			y1[],
			      int			y2[],
			      int			xdepth,
			      int			ydepth,
			      int			num_sets,
			      int			clr[],
			      int			clrshd[] );
    void draw_3d_area(
			     int				x1,
			     int				x2,
			     int				y0,			/* drawn from 0 */
			     int				y1,
			     int				y2,
			     int				xdepth,
			     int				ydepth,
			     int				clr,
			     int				clrshd );
    void draw_3d_bar(
			     int				x1,
			     int				x2,
			     int				y0,
			     int				yhigh,
			     int				xdepth,
			     int				ydepth,
			     int				clr,
			     int				clrshd );
    void do_interpolations(int		num_points,
				       int		interp_point,
				       double	vals[] );
    int out_graph( int		IMGWIDTH,		/* no check for a image that's too small to fit */
		   int		IMGHEIGHT,		/* needed info (labels, etc), could core dump */
		   FILE			*img_fptr,		/* open file pointer (img out) */
		   int	    type1,
		   int			num_points,     /* points along x axis (even iterval) */
										/*	all arrays dependant on this */
		   char			*xlbl[],		/* array of xlabels */
		   int			num_sets,
						... );
    int GDC_out_graph( int		IMGWIDTH,		/* no check for a img that's too small to fit */
			       int		IMGHEIGHT,		/* needed info (labels, etc), could core dump */
			       FILE			*img_fptr,		/* open file pointer (img out) */
			       int    	type1,
			       int			num_points,     /* points along x axis (even iterval) */
											    /*	all arrays dependant on this */
			       char			*xlbl[],		/* array of xlabels */
			       int			num_sets,
			       double		*data,			/* (double*) cast on multi-dim array (num_sets > 1) */
			       double		*combo_data );	/* only used on COMBO chart types */
    void load_font_conversions();
    struct fnt_sz_t GDCfnt_sz( char					*s,
		       enum GDC_font_size	gdfontsz,
		       char					*ftfont,
		       double				ftfptsz,
		       double				rad,		/* w,h still relative to horiz. */
		       char					**sts );
    int GDCImageStringNL(
				      struct GDC_FONT_T	*f,
				      char				*ftfont,
				      double			ftptsz,
				      double			rad,
				      int				x,
				      int				y,
				      char				*str,
				      int				clr,
				      GDC_justify_t		justify,
				      char				**sts );
    long get_uniq_color();

    char rtn[64];

    char * price_to_str(
            double	price,
			      int	*numorator,
			      int	*demoninator,
			      int	*decimal,
			      char	*fltfmt );			/* printf fmt'ing str */
//-------------------------------------------------------------
    Vector<Array<ldouble> > data_;
  private:
    GDChart(const GDChart &);
    void operator = (const GDChart &);
};
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
