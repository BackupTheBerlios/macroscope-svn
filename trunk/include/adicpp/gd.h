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

    enum Font { ftGiant, ftLarge, ftMediumBold, ftSmall, ftTiny };

    void * font(Font f) const;
    uintptr_t fontWidth(Font f) const;
    uintptr_t fontHeight(Font f) const;
  protected:
  public:

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
    GDChart(const GDChart & v) { operator = (v); }
    virtual GDChart & operator = (const GDChart & v);

    GDChart & createChart();

    Vector<Array<ldouble> > & data() const { return data_; }

    const uintptr_t & width() const { return width_; }
    GDChart & width(uintptr_t a) { width_ = a; return *this; }
    const uintptr_t & height() const { return height_; }
    GDChart & height(uintptr_t a) { height_ = a; return *this; }
    const uintptr_t & leftBorder() const { return leftBorder_; }
    GDChart & leftBorder(uintptr_t a) { leftBorder_ = a; return *this; }
    const uintptr_t & rightBorder() const { return rightBorder_; }
    GDChart & rightBorder(uintptr_t a) { rightBorder_ = a; return *this; }
    const uintptr_t & topBorder() const { return topBorder_; }
    GDChart & topBorder(uintptr_t a) { topBorder_ = a; return *this; }
    const uintptr_t & bottomBorder() const { return bottomBorder_; }
    GDChart & bottomBorder(uintptr_t a) { bottomBorder_ = a; return *this; }
    const ldouble & xlvs() const { return xlvs_; }
    GDChart & xlvs(ldouble a) { xlvs_ = a; return *this; }
    const Font & font() const { return font_; }
    GDChart & font(Font a) { font_ = a; return *this; }
  protected:
    mutable Vector<Array<ldouble> > data_;
    uintptr_t width_;
    uintptr_t height_;
    uintptr_t leftBorder_;
    uintptr_t rightBorder_;
    uintptr_t topBorder_;
    uintptr_t bottomBorder_;
    ldouble xlvs_;
    Font font_;
  private:
};
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
