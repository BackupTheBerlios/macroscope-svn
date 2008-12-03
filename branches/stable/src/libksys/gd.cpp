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
    gdFontPtr PROTOF(gdFontGetTiny)(void);
    gdFontPtr PROTOF(gdFontGetSmall)(void);
    gdFontPtr PROTOF(gdFontGetMediumBold)(void);
    gdFontPtr PROTOF(gdFontGetLarge)(void);
    gdFontPtr PROTOF(gdFontGetGiant)(void);
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
    uint8_t mutex_[sizeof(WriteLock)];
    WriteLock & mutex(){ return *reinterpret_cast<WriteLock *>(mutex_); }
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
    "gdFontGetTiny",
    "gdFontGetSmall",
    "gdFontGetMediumBold",
    "gdFontGetLarge",
    "gdFontGetGiant",
    "gdImagePngPtrEx",
    "gdFree",
    "gdImageFill",
    "gdImageFilledEllipse",
    "gdImageFilledRectangle"
  };
  AutoLock<WriteLock> lock(api.mutex());
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
        for( uintptr_t j = 0; j < 128; j += 4 ){
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
  AutoLock<WriteLock> lock(mutex());
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
  new (api.mutex_) WriteLock;
  api.count_ = 0;
}
//------------------------------------------------------------------------------
void GD::cleanup()
{
  api.mutex().~WriteLock();
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
  return api.gdFontGetTiny();
}
//------------------------------------------------------------------------------
void * GD::fontSmall() const
{
  return api.gdFontGetSmall();
}
//------------------------------------------------------------------------------
void * GD::fontMediumBold() const
{
  return api.gdFontGetMediumBold();
}
//------------------------------------------------------------------------------
void * GD::fontLarge() const
{
  return api.gdFontGetLarge();
}
//------------------------------------------------------------------------------
void * GD::fontGiant() const
{
  return api.gdFontGetGiant();
}
//------------------------------------------------------------------------------
void * GD::pngPtrEx(intptr_t * size, intptr_t level)
{
  int size0;
  void * p = api.gdImagePngPtrEx((gdImagePtr) image_,&size0,int(level));
  *size = size0;
  return p;
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
void * GD::font(Font f) const
{
  gdFontPtr font = NULL;
  switch( f ){
    case ftTiny       : font = api.gdFontGetTiny(); break;
    case ftSmall      : font = api.gdFontGetSmall(); break;
    case ftMediumBold : font = api.gdFontGetMediumBold(); break;
    case ftLarge      : font = api.gdFontGetLarge(); break;
    case ftGiant      : font = api.gdFontGetGiant(); break;
  }
  return font;
}
//------------------------------------------------------------------------------
uintptr_t GD::fontWidth(Font f) const
{
  return ((gdFontPtr) font(f))->w;
}
//------------------------------------------------------------------------------
uintptr_t GD::fontHeight(Font f) const
{
  return ((gdFontPtr) font(f))->h;
}
//------------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------
