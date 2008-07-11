/*-
 * Copyright 2007-2008 Guram Dukashvili
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
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class GD_API {
  friend class GD;
  public:
    ~GD_API() {}
    GD_API() {}

#if defined(__WIN32__) || defined(__WIN64__)
#define PROTOF(x) (__stdcall * x)
#else
#define PROTOF(x) (* x)
#endif
    gdImagePtr PROTOF(gdImageCreate)(int sx, int sy);
    gdImagePtr PROTOF(gdImageCreateTrueColor)(int sx, int sy);
    gdImagePtr PROTOF(gdImageCreateFromGif )(FILE * fd);
    void PROTOF(gdImageDestroy)(gdImagePtr im);
    int PROTOF(gdImageColorAllocate)(gdImagePtr im, int r, int g, int b);
    void PROTOF(gdImageString)(gdImagePtr im, gdFontPtr f, int x, int y, const char *s, int color);
    int PROTOF(gdImageColorExact)(gdImagePtr im, int r, int g, int b);
    int PROTOF(gdImageColorAllocateAlpha)(gdImagePtr im, int r, int g, int b, int a);
    int PROTOF(gdImageColorClosest)(gdImagePtr im, int r, int g, int b);
    void PROTOF(gdImageCopyResized)(gdImagePtr dst, gdImagePtr src, int dstX, int dstY, int srcX, int srcY, int dstW, int dstH, int srcW, int srcH);
    void PROTOF(gdImageColorTransparent)(gdImagePtr im, int color);
    void PROTOF(gdImageCopy)(gdImagePtr dst, gdImagePtr src, int dstX, int dstY, int srcX, int srcY, int w, int h);
    void PROTOF(gdImageStringUp)(gdImagePtr im, gdFontPtr f, int x, int y, const char *s, int color);
    void PROTOF(gdImageLine)(gdImagePtr im, int x1, int y1, int x2, int y2, int color);
    void PROTOF(gdImagePolygon)(gdImagePtr im, gdPointPtr p, int n, int c);
    void PROTOF(gdImageOpenPolygon)(gdImagePtr im, gdPointPtr p, int n, int c);
    void PROTOF(gdImageFilledPolygon)(gdImagePtr im, gdPointPtr p, int n, int c);
    void PROTOF(gdImageSetPixel)(gdImagePtr im, int x, int y, int color);
    void PROTOF(gdImageArc)(gdImagePtr im, int cx, int cy, int w, int h, int s, int e, int color);
    void PROTOF(gdImageFillToBorder)(gdImagePtr im, int x, int y, int border, int color);
    void PROTOF(gdImageColorDeallocate)(gdImagePtr im, int color);
    void PROTOF(gdImageWBMP)(gdImagePtr image, int fg, FILE * out);
    void PROTOF(gdImageGif)(gdImagePtr image, FILE * out);
    void PROTOF(gdImagePng)(gdImagePtr image, FILE * out);
    gdFontPtr gdFontTiny;
    gdFontPtr gdFontSmall;
    gdFontPtr gdFontMediumBold;
    gdFontPtr gdFontLarge;
    gdFontPtr gdFontGiant;
    void * PROTOF(gdImagePngPtrEx)(gdImagePtr im, int *size, int level);
    void PROTOF(gdFree)(void *m);
    void PROTOF(gdImageFill)(gdImagePtr im, int x, int y, int color);
    void PROTOF(gdImageEllipse)(gdImagePtr im, int cx, int cy, int w, int h, int color);
    void PROTOF(gdImageFilledRectangle)(gdImagePtr im, int x1, int y1, int x2, int y2, int color);

    GD_API & open();
    GD_API & close();
  protected:
#if defined(__WIN32__) || defined(__WIN64__)
    HINSTANCE handle_;
#else
    void * handle_;
#endif
    intptr_t count_;
    uint8_t mutex_[sizeof(InterlockedMutex)];
    InterlockedMutex & mutex(){ return *reinterpret_cast<InterlockedMutex *>(mutex_); }
  private:
    GD_API(const GD_API &);
    void operator = (const GD_API &);
};
//------------------------------------------------------------------------------
static GD_API api;
//------------------------------------------------------------------------------
GD_API & GD_API::open()
{
  static const char * const symbols[] = {
    "gdImageCreate",
    "gdImageCreateTrueColor",
    "gdImageCreateFromGif",
    "gdImageDestroy",
    "gdImageColorAllocate",
    "gdImageString",
    "gdImageColorExact",
    "gdImageColorAllocateAlpha",
    "gdImageColorClosest",
    "gdImageCopyResized",
    "gdImageColorTransparent",
    "gdImageCopy",
    "gdImageStringUp",
    "gdImageLine",
    "gdImagePolygon",
    "gdImageOpenPolygon",
    "gdImageFilledPolygon",
    "gdImageSetPixel",
    "gdImageArc",
    "gdImageFillToBorder",
    "gdImageColorDeallocate",
    "gdImageWBMP",
    "gdImageGif",
    "gdImagePng",
    "gdFontTiny",
    "gdFontSmall",
    "gdFontMediumBold",
    "gdFontLarge",
    "gdFontGiant",
    "gdImagePngPtrEx",
    "gdFree",
    "gdImageFill",
    "gdImageFilledEllipse",
    "gdImageFilledRectangle"
  };
  AutoLock<InterlockedMutex> lock(api.mutex());
#if defined(__WIN32__) || defined(__WIN64__)
#define LIB_NAME "bgd.dll"
    api.handle_ = LoadLibraryExA(LIB_NAME,NULL,LOAD_WITH_ALTERED_SEARCH_PATH);
#elif HAVE_DLFCN_H
#define LIB_NAME "libgd.so"
    api.handle_ = dlopen(LIB_NAME,
#ifdef __linux__
      RTLD_GLOBAL | RTLD_NOW
#else
      0
#endif
    );
#endif
  if( handle_ == NULL ){
    int32_t err = oserror() + errorOffset;
    stdErr.debug(
      9,
      utf8::String::Stream() << "Load " LIB_NAME " failed\n"
    );
    newObjectV1C2<Exception>(err, __PRETTY_FUNCTION__)->throwSP();
  }
#undef LIB_NAME
  for( uintptr_t i = 0; i < sizeof(symbols) / sizeof(symbols[0]); i++ ){
#if defined(__WIN32__) || defined(__WIN64__)
#define FUNC_NAME "GetProcAddress"
    ((void **) &api.gdImageCreate)[i] = GetProcAddress(handle_,symbols[i]);
    char sym[128];
    if( ((void **) &api.gdImageCreate)[i] == NULL ){
      strcpy(sym,"_");
      strcat(sym,symbols[i]);
      ((void **) &api.gdImageCreate)[i] = GetProcAddress(handle_,sym);
      if( ((void **) &api.gdImageCreate)[i] == NULL ){
        for( uintptr_t j = 4; j < 128; j += 4 ){
          strcpy(sym,"_");
          strcat(sym,symbols[i]);
          char b[5];
          sprintf(b,"@%"PRIuPTR,j);
          strcat(sym,b);
          ((void **) &api.gdImageCreate)[i] = GetProcAddress(handle_,sym);
          if( ((void **) &api.gdImageCreate)[i] != NULL ) break;
        }
      }
    }
#elif HAVE_DLFCN_H
#define FUNC_NAME "dlsym"
    ((void **) &api.gdImageCreate)[i] = dlsym(handle_,symbols[i]);
#endif
    if( ((void **) &api.gdImageCreate)[i] == NULL ){
      int32_t err = oserror() + errorOffset;
#if defined(__WIN32__) || defined(__WIN64__)
      FreeLibrary(handle_);
#elif HAVE_DLFCN_H
      dlclose(handle_);
#endif
      handle_ = NULL;
      stdErr.debug(
        9,
        utf8::String::Stream() << FUNC_NAME "(\"" << symbols[i] << "\") falied\n"
      );
      newObjectV1C2<Exception>(err, __PRETTY_FUNCTION__)->throwSP();
    }
#undef FUNC_NAME
  }
  api.count_++;
  return *this;
}
//------------------------------------------------------------------------------
GD_API & GD_API::close()
{
  AutoLock<InterlockedMutex> lock(mutex());
  assert( count_ > 0 );
  if( count_ == 1 ){
#if defined(__WIN32__) || defined(__WIN64__)
    FreeLibrary(handle_);
#elif HAVE_DLFCN_H
    dlclose(handle_);
#endif
    handle_ = NULL;
  }
  count_--;
  return *this;
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
void GD::initialize()
{
  new (api.mutex_) InterlockedMutex;
  api.count_ = 0;
}
//------------------------------------------------------------------------------
void GD::cleanup()
{
  api.mutex().~InterlockedMutex();
}
//------------------------------------------------------------------------------
GD::~GD()
{
  clear();
}
//------------------------------------------------------------------------------
GD::GD() : image_(NULL), png_(NULL), pngSize_(0)
{
}
//------------------------------------------------------------------------------
GD & GD::clear()
{
  if( png_ != NULL ){
    png_ = NULL;
    pngSize_ = 0;
    api.gdFree(png_);
  }
  if( image_ != NULL ){
    api.gdImageDestroy((gdImagePtr) image_);
    api.close();
    image_ = NULL;
  }
  return *this;
}
//------------------------------------------------------------------------------
GD & GD::create(uintptr_t sx,uintptr_t sy)
{
  clear();
  api.open();
  image_ = api.gdImageCreate(sx & (((unsigned int) ~int(0)) >> 1),sy & (((unsigned int) ~int(0)) >> 1));
  return *this;
}
//------------------------------------------------------------------------------
GD & GD::createTrueColor(uintptr_t sx,uintptr_t sy)
{
  clear();
  api.open();
  image_ = api.gdImageCreateTrueColor(sx & (((unsigned int) ~int(0)) >> 1),sy & (((unsigned int) ~int(0)) >> 1));
  return *this;
}
//------------------------------------------------------------------------------
GD & GD::createFromGif(FILE * fd)
{
  clear();
  api.open();
  image_ = api.gdImageCreateFromGif(fd);
  return *this;
}
//------------------------------------------------------------------------------
intptr_t GD::colorAllocate(intptr_t r,intptr_t g,intptr_t b)
{
  return api.gdImageColorAllocate((gdImagePtr) image_,(int) r,(int) b,(int) g);
}
//------------------------------------------------------------------------------
GD & GD::string(void * font, intptr_t x, intptr_t y, const char * s, intptr_t color)
{
  api.gdImageString((gdImagePtr) image_,(gdFontPtr) font,(int) x,(int) y,s,(int) color);
  return *this;
}
//------------------------------------------------------------------------------
intptr_t GD::colorExact(intptr_t r,intptr_t g,intptr_t b)
{
  return api.gdImageColorExact((gdImagePtr) image_,(int) r,(int) b,(int) g);
}
//------------------------------------------------------------------------------
intptr_t GD::colorsTotal() const
{
  return ((gdImagePtr) image_)->colorsTotal;
}
//------------------------------------------------------------------------------
intptr_t GD::sx() const
{
  return ((gdImagePtr) image_)->sx;
}
//------------------------------------------------------------------------------
intptr_t GD::sy() const
{
  return ((gdImagePtr) image_)->sy;
}
//------------------------------------------------------------------------------
intptr_t GD::colorAllocateAlpha(intptr_t r,intptr_t g,intptr_t b,intptr_t a)
{
  return api.gdImageColorAllocateAlpha((gdImagePtr) image_,(int) r,(int) b,(int) g,(int) a);
}
//------------------------------------------------------------------------------
intptr_t GD::colorClosest(intptr_t r,intptr_t g,intptr_t b)
{
  return api.gdImageColorClosest((gdImagePtr) image_,(int) r,(int) b,(int) g);
}
//------------------------------------------------------------------------------
GD & GD::copyResized(const GD & src, intptr_t dstX, intptr_t dstY, intptr_t srcX, intptr_t srcY, intptr_t dstW, intptr_t dstH, intptr_t srcW, intptr_t srcH)
{
  api.gdImageCopyResized((gdImagePtr) image_,(gdImagePtr) src.image_,(int) dstX, (int) dstY, (int) srcX, (int) srcY, (int) dstW, (int) dstH, (int) srcW, (int) srcH);
  return *this;
}
//------------------------------------------------------------------------------
GD & GD::colorTransparent(intptr_t color)
{
  api.gdImageColorTransparent((gdImagePtr) image_,(int) color);
  return *this;
}
//------------------------------------------------------------------------------
GD & GD::copy(const GD & src, intptr_t dstX, intptr_t dstY, intptr_t srcX, intptr_t srcY, intptr_t w, intptr_t h)
{
  api.gdImageCopy((gdImagePtr) image_, (gdImagePtr) src.image_, (int) dstX, (int) dstY, (int) srcX, (int) srcY, (int) w, (int) h);
  return *this;
}
//------------------------------------------------------------------------------
GD & GD::stringUp(void * font, intptr_t x, intptr_t y, const char *s, intptr_t color)
{
  api.gdImageStringUp((gdImagePtr) image_,(gdFontPtr) font,(int) x,(int) y,s,(int) color);
  return *this;
}
//------------------------------------------------------------------------------
GD & GD::line(intptr_t x1, intptr_t y1, intptr_t x2, intptr_t y2, intptr_t color)
{
  api.gdImageLine((gdImagePtr) image_,(int) x1, (int) y1, (int) x2, (int) y2, (int) color);
  return *this;
}
//------------------------------------------------------------------------------
GD & GD::polygon(void * points, intptr_t n, intptr_t c)
{
  api.gdImagePolygon((gdImagePtr) image_,(gdPointPtr) points,(int) n, (int) c);
  return *this;
}
//------------------------------------------------------------------------------
GD & GD::openPolygon(void * points, intptr_t n, intptr_t c)
{
  api.gdImageOpenPolygon((gdImagePtr) image_,(gdPointPtr) points,(int) n, (int) c);
  return *this;
}
//------------------------------------------------------------------------------
GD & GD::filledPolygon(void * points, intptr_t n, intptr_t c)
{
  api.gdImageFilledPolygon((gdImagePtr) image_,(gdPointPtr) points,(int) n, (int) c);
  return *this;
}
//------------------------------------------------------------------------------
GD & GD::setPixel(intptr_t x, intptr_t y, intptr_t color)
{
  api.gdImageSetPixel((gdImagePtr) image_,(int) x, (int) y, (int) color);
  return *this;
}
//------------------------------------------------------------------------------
GD & GD::arc(intptr_t cx, intptr_t cy, intptr_t w, intptr_t h, intptr_t s, intptr_t e, intptr_t color)
{
  api.gdImageArc((gdImagePtr) image_,(int) cx, (int) cy, (int) w, (int) h, (int) s, (int) e, (int) color);
  return *this;
}
//------------------------------------------------------------------------------
GD & GD::fillToBorder(intptr_t x, intptr_t y, intptr_t border, intptr_t color)
{
  api.gdImageFillToBorder((gdImagePtr) image_,(int) x, (int) y, (int) border, (int) color);
  return *this;
}
//------------------------------------------------------------------------------
GD & GD::colorDeallocate(intptr_t color)
{
  api.gdImageColorDeallocate((gdImagePtr) image_,(int) color);
  return *this;
}
//------------------------------------------------------------------------------
GD & GD::wbmp(int fg, FILE * out)
{
  api.gdImageWBMP((gdImagePtr) image_,fg,out);
  return *this;
}
//------------------------------------------------------------------------------
GD & GD::gif(FILE * out)
{
  api.gdImageGif((gdImagePtr) image_,out);
  return *this;
}
//------------------------------------------------------------------------------
GD & GD::png(FILE * out)
{
  api.gdImagePng((gdImagePtr) image_,out);
  return *this;
}
//------------------------------------------------------------------------------
void * GD::fontTiny() const
{
  return api.gdFontTiny;
}
//------------------------------------------------------------------------------
void * GD::fontSmall() const
{
  return api.gdFontSmall;
}
//------------------------------------------------------------------------------
void * GD::fontMediumBold() const
{
  return api.gdFontMediumBold;
}
//------------------------------------------------------------------------------
void * GD::fontLarge() const
{
  return api.gdFontLarge;
}
//------------------------------------------------------------------------------
void * GD::fontGiant() const
{
  return api.gdFontGiant;
}
//------------------------------------------------------------------------------
void * GD::pngPtrEx(intptr_t * size, intptr_t level)
{
  return api.gdImagePngPtrEx((gdImagePtr) image_,size,level);
}
//------------------------------------------------------------------------------
GD & GD::gdFree(void *m)
{
  api.gdFree(m);
  return *this;
}
//------------------------------------------------------------------------------
GD & GD::fill(intptr_t x, intptr_t y, intptr_t color)
{
  api.gdImageFill((gdImagePtr) image_,(int) x, (int) y, (int) color);
  return *this;
}
//------------------------------------------------------------------------------
GD & GD::ellipse(intptr_t cx, intptr_t cy, intptr_t w, intptr_t h, intptr_t color)
{
  api.gdImageEllipse((gdImagePtr) image_,(int) cx, (int) cy, (int) w, (int) h, (int) color);
  return *this;
}
//------------------------------------------------------------------------------
GD & GD::filledRectangle(intptr_t x1, intptr_t y1, intptr_t x2, intptr_t y2, intptr_t color)
{
  api.gdImageFilledRectangle((gdImagePtr) image_,(int) x1, (int) y1, (int) x2, (int) y2, (int) color);
  return *this;
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
GDChart & GDChart::createChart(uintptr_t sx,uintptr_t sy)
{
  //GDC_image_type = GDC_PNG;
  //char    *t[6] = { "Chicago", "New York", "L.A.", "Atlanta", "Paris, MD\n(USA) ", "London" };
  ///* ----- data set colors (RGB) ----- */
  ////unsigned long   sc[2]    = { 0xFF8080, 0x8080FF };

  ////GDC_BGColor   = 0xFFFFFFL;                  /* backgound color (white) */
  ////GDC_LineColor = 0x000000L;                  /* line color      (black) */
  ////GDC_SetColor  = &(sc[0]);                   /* assign set colors */

  //GDC_out_graph(
  //  sx & (((unsigned int) ~int(0)) >> 1),
  //  sy & (((unsigned int) ~int(0)) >> 1),
  //  NULL,
  //  GDC_LINE,     /* GDC_CHART_T chart type */
  //  6,             /* int         number of points per data set */
  //  t,             /* char*[]     array of X labels */
  //  1,             /* int         number of data sets */
  //  data_,
  //  NULL
  //);             /* double[]     data set 1 */

  sx = tmax(sx,100u);
  sy = tmax(sy,100u);
  data_.resize(1)[0].resize(10);
  data_[0][0] = -1.4;
  data_[0][1] = -2.7;
  data_[0][2] = 3.143;
  data_[0][3] = 4.45645;
  data_[0][4] = 4.963453;
  data_[0][5] = 6.3;
  data_[0][6] = 7.3453645;
  data_[0][7] = 7.78978;
  data_[0][8] = 8.4556;
  data_[0][9] = 10.3;

  create(sx,sy);

  intptr_t i, j, xCount = 0, x, y, x0, y0, borderSize = 8;
  // calc min max
  ldouble minValue = DBL_MAX, maxValue = -DBL_MAX;
  for( i = data_.count() - 1; i >= 0; i-- ){
    j = data_[i].count();
    xCount = tmax(xCount,j);
    const Array<ldouble> & data = data_[i];
    for( j = data.count() - 1; j >= 0; j-- ){
      minValue = tmin(minValue,data[j]);
      maxValue = tmax(maxValue,data[j]);
    }
  }
  ldouble yAxis = (maxValue - minValue) / (sy - borderSize * 2u);
  // draw lines
  fill(0,0,colorAllocate(255,255,255));
  for( i = 0; uintptr_t(i) < data_.count(); i++ ){
    const Array<ldouble> & data = data_[i];
    for( j = 0; uintptr_t(j) < data.count(); j++ ){
      x = (sx - borderSize * 2u) * j / (xCount - 1) + borderSize;
      y = intptr_t(sy - borderSize * 2u - 1 - (data[j] - minValue) / yAxis) + borderSize;
      if( j > 0 ) line(x0,y0,x,y,colorAllocate(0,0,0));
      x0 = x;
      y0 = y;
    }
  }
  // draw bars
  intptr_t xBarSize = 4, yBarSize = 4;
  for( i = 0; uintptr_t(i) < data_.count(); i++ ){
    const Array<ldouble> & data = data_[i];
    for( j = 0; uintptr_t(j) < data.count(); j++ ){
      x = (sx - borderSize * 2u) * j / (xCount - 1) + borderSize;
      y = intptr_t(sy - borderSize * 2u - 1 - (data[j] - minValue) / yAxis) + borderSize;
      filledRectangle(x - xBarSize,y - yBarSize,x + xBarSize,y + yBarSize,colorAllocate(255,0,0));
      x0 = x;
      y0 = y;
    }
  }
  
  gdFree(png_);
  png_ = pngPtrEx(&pngSize_,9);
  return *this;
}
//------------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------
