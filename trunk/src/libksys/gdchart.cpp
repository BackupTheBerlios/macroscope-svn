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
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
GDChart::~GDChart()
{
}
//------------------------------------------------------------------------------
GDChart::GDChart() :
  width_(640),
  height_(480),
  leftBorder_(0),
  rightBorder_(0),
  topBorder_(0),
  bottomBorder_(0),
  xlvs_(0),
  font_(ftTiny)
{
}
//------------------------------------------------------------------------------
GDChart & GDChart::operator = (const GDChart & v)
{
  data_ = v.data_;
  width_ = v.width_;
  height_ = v.height_;
  leftBorder_ = v.leftBorder_;
  rightBorder_ = v.rightBorder_;
  topBorder_ = v.topBorder_;
  bottomBorder_ = v.bottomBorder_;
  xlvs_ = v.xlvs_;
  font_ = v.font_;
  return *this;
}
//------------------------------------------------------------------------------
intptr_t GDChart::makeColor(ldouble r,ldouble g,ldouble b)
{
  //static const ldouble pi = M_PI / 2;
  ldouble r0 = sin(r);
  ldouble g0 = cos(g);
  ldouble b0 = tan(b);
  return colorAllocate(intptr_t(r0 * 220),intptr_t(g0 * 220),intptr_t(b0 * 220));
}
//------------------------------------------------------------------------------
GDChart & GDChart::createChart()
{
  create(width_,height_);

  bool isIntergerOnlyValues = true;
  intptr_t i, j, xCount = 0, x, y, x0, y0;
  // calc min max
  ldouble minValue = DBL_MAX, maxValue = -DBL_MAX;
  for( i = data_.count() - 1; i >= 0; i-- ){
    j = data_[i].count();
    xCount = tmax(xCount,j);
    const Array<ldouble> & data = data_[i];
    for( j = data.count() - 1; j >= 0; j-- ){
      volatile intmax_t v = intmax_t(data[j]);
      volatile ldouble lv = ldouble(v);
      if( lv != data[j] ) isIntergerOnlyValues = false;
      minValue = tmin(minValue,data[j]);
      maxValue = tmax(maxValue,data[j]);
    }
  }
  ldouble yAxis = (maxValue - minValue) / (height_ - topBorder_ - bottomBorder_);
  intptr_t leftBorderDelta = 0, rightBorderDelta = 0, topBorderDelta = 0, bottomBorderDelta = 0;
  // clear image
  fill(0,0,colorAllocate(255,255,255));
  // draw lines
  intptr_t lineColor = colorAllocate(230,230,230);
  // draw vert grid lines
  for( j = 0; j < xCount; j++ ){
    x = (width_ - leftBorder_ - rightBorder_) * j / (xCount - 1) + leftBorder_;
    line(x,topBorder_,x,height_ - bottomBorder_,lineColor);
  }
  intptr_t yLabelColor = makeColor(j,j,j);
  for( y = topBorder_; uintptr_t(y) <= height_ - bottomBorder_; y += fontHeight(font_) * 2 ){
    ldouble v = maxValue - (y - topBorder_) * yAxis;
    // draw horiz grid line
    line(leftBorder_,y,width_ - rightBorder_,y,lineColor);
    // draw ylabel
    utf8::String label;
    if( isIntergerOnlyValues ){
      label = printTraffic(intmax_t(v),true);//utf8::String::print("%"PRIdPTR,intptr_t(v));
    }
    else {
      label = utf8::String::print("%.2"PRF_LDBL"f",v);
    }
    uintptr_t sz = label.size();
    x = leftBorder_ - sz * fontWidth(font_);
    string(GD::font(font_),x,y,label.c_str(),yLabelColor);
    if( x < 0 && -x > leftBorderDelta ) leftBorderDelta = -x;
  }
  // draw data lines
  for( i = 0; uintptr_t(i) < data_.count(); i++ ){
    intptr_t color = makeColor(i + 1,i + 1,i + 1);
    const Array<ldouble> & data = data_[i];
    for( j = 0; uintptr_t(j) < data.count(); j++ ){
      x = (width_ - leftBorder_ - rightBorder_) * j / (xCount - 1) + leftBorder_;
      y = intptr_t(height_ - topBorder_ - bottomBorder_ - (data[j] - minValue) / yAxis) + topBorder_;
      if( j > 0 ) line(x0,y0,x,y,color);
      x0 = x;
      y0 = y;
    }
  }
  intptr_t xBarSize = 2, yBarSize = 2;
  intptr_t barColor = colorAllocate(255,0,0);
  intptr_t xLabelColor = makeColor(i + 1,i + 1,i + 1);
  for( i = 0; uintptr_t(i) < data_.count(); i++ ){
    const Array<ldouble> & data = data_[i];
    for( j = 0; uintptr_t(j) < data.count(); j++ ){
      x = (width_ - leftBorder_ - rightBorder_) * j / (xCount - 1) + leftBorder_;
      y = intptr_t(height_ - topBorder_ - bottomBorder_ - (data[j] - minValue) / yAxis) + topBorder_;
      // draw bar
      filledRectangle(
        tmax(leftBorder_,uintptr_t(x - xBarSize)),
        tmax(topBorder_,uintptr_t(y - yBarSize)),
        tmin(width_ - rightBorder_,uintptr_t(x + xBarSize)),
        tmin(height_ - bottomBorder_,uintptr_t(y + yBarSize)),
        barColor
      );
      x0 = x;
      y0 = y;
      // draw xlabel
      y = height_ - bottomBorder_ + xBarSize;
      utf8::String label(utf8::String::print("%"PRIdPTR,intptr_t(j + xlvs_)));
      string(GD::font(font_),x + xBarSize,y,label.c_str(),xLabelColor);
      if( y + fontHeight(font_) >= height_ )
        bottomBorderDelta = y + fontHeight(font_) - height_ + 1;
      x = x + xBarSize + fontWidth(font_) * label.size();
      if( uintptr_t(x) >= width_ ) rightBorderDelta = x - width_ + 1;
    }
  }
  if( leftBorderDelta != 0 || rightBorderDelta != 0 || topBorderDelta != 0 || bottomBorderDelta != 0 ){
    GDChart chart(*this);
    chart.leftBorder_ += leftBorderDelta;
    chart.rightBorder_ += rightBorderDelta;
    chart.topBorder_ += topBorderDelta;
    chart.bottomBorder_ += bottomBorderDelta;
    chart.createChart();
    xchg(image_,chart.image_);
    xchg(png_,chart.png_);
    xchg(pngSize_,chart.pngSize_);
  }
  else {
    gdFree(png_);
    png_ = pngPtrEx(&pngSize_,9);
  }
  return *this;
}
//------------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------
