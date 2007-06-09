/*-
 * Copyright 2005 Guram Dukashvili
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
// WARNING: MACHINE GENERATED, PLEASE DO NOT EDIT
//---------------------------------------------------------------------------
#include <adicpp/lconfig.h>
#include <adicpp/utf8embd.h>
//---------------------------------------------------------------------------
namespace utf8 {
//---------------------------------------------------------------------------
#if defined(EMBED_CP1258_SUPPORT) || defined(EMBED_ALL_CP)
extern const uint8_t cp1258s2utf8s[180] = {
  0xEA, 0x04, 0x00, 0x00, 0x11, 0x00, 0x7F, 0x00,
  0x00, 0x86, 0x87, 0x20, 0x20, 0x8D, 0x90, 0x8D,
  0x00, 0x91, 0x92, 0x18, 0x20, 0x93, 0x94, 0x1C,
  0x20, 0x96, 0x97, 0x13, 0x20, 0x9D, 0x9E, 0x9D,
  0x00, 0xA0, 0xC2, 0xA0, 0x00, 0xC4, 0xCB, 0xC4,
  0x00, 0xCD, 0xCF, 0xCD, 0x00, 0xD3, 0xD4, 0xD3,
  0x00, 0xD6, 0xDC, 0xD6, 0x00, 0xDF, 0xE2, 0xDF,
  0x00, 0xE4, 0xEB, 0xE4, 0x00, 0xED, 0xEF, 0xED,
  0x00, 0xF3, 0xF4, 0xF3, 0x00, 0xF6, 0xFC, 0xF6,
  0x00, 0x23, 0x80, 0xAC, 0x20, 0x81, 0x81, 0x00,
  0x82, 0x1A, 0x20, 0x83, 0x92, 0x01, 0x84, 0x1E,
  0x20, 0x85, 0x26, 0x20, 0x88, 0xC6, 0x02, 0x89,
  0x30, 0x20, 0x8A, 0x8A, 0x00, 0x8B, 0x39, 0x20,
  0x8C, 0x52, 0x01, 0x95, 0x22, 0x20, 0x98, 0xDC,
  0x02, 0x99, 0x22, 0x21, 0x9A, 0x9A, 0x00, 0x9B,
  0x3A, 0x20, 0x9C, 0x53, 0x01, 0x9F, 0x78, 0x01,
  0xC3, 0x02, 0x01, 0xCC, 0x00, 0x03, 0xD0, 0x10,
  0x01, 0xD1, 0xD1, 0x00, 0xD2, 0x09, 0x03, 0xD5,
  0xA0, 0x01, 0xDD, 0xAF, 0x01, 0xDE, 0x03, 0x03,
  0xE3, 0x03, 0x01, 0xEC, 0x01, 0x03, 0xF0, 0x11,
  0x01, 0xF1, 0xF1, 0x00, 0xF2, 0x23, 0x03, 0xF5,
  0xA1, 0x01, 0xFD, 0xB0, 0x01, 0xFE, 0xAB, 0x20,
  0xFF, 0xFF, 0x00, 0x00
};
//---------------------------------------------------------------------------
extern const uint8_t utf8s2cp1258s[212] = {
  0x01, 0x3F, 0x13, 0x01, 0x00, 0x00, 0x7F, 0x00,
  0x00, 0x8D, 0x00, 0x90, 0x00, 0x8D, 0x9D, 0x00,
  0x9E, 0x00, 0x9D, 0xA0, 0x00, 0xC2, 0x00, 0xA0,
  0xC4, 0x00, 0xCB, 0x00, 0xC4, 0xCD, 0x00, 0xCF,
  0x00, 0xCD, 0xD3, 0x00, 0xD4, 0x00, 0xD3, 0xD6,
  0x00, 0xDC, 0x00, 0xD6, 0xDF, 0x00, 0xE2, 0x00,
  0xDF, 0xE4, 0x00, 0xEB, 0x00, 0xE4, 0xED, 0x00,
  0xEF, 0x00, 0xED, 0xF3, 0x00, 0xF4, 0x00, 0xF3,
  0xF6, 0x00, 0xFC, 0x00, 0xF6, 0x13, 0x20, 0x14,
  0x20, 0x96, 0x18, 0x20, 0x19, 0x20, 0x91, 0x1C,
  0x20, 0x1D, 0x20, 0x93, 0x20, 0x20, 0x21, 0x20,
  0x86, 0x01, 0xFF, 0x1E, 0xFF, 0x21, 0x20, 0xFF,
  0x5E, 0xFF, 0x40, 0xFF, 0xFF, 0x23, 0x00, 0x01,
  0x81, 0x00, 0x81, 0x8A, 0x00, 0x8A, 0x9A, 0x00,
  0x9A, 0xD1, 0x00, 0xD1, 0xF1, 0x00, 0xF1, 0xFF,
  0x00, 0xFF, 0x02, 0x01, 0xC3, 0x03, 0x01, 0xE3,
  0x10, 0x01, 0xD0, 0x11, 0x01, 0xF0, 0x52, 0x01,
  0x8C, 0x53, 0x01, 0x9C, 0x78, 0x01, 0x9F, 0x92,
  0x01, 0x83, 0xA0, 0x01, 0xD5, 0xA1, 0x01, 0xF5,
  0xAF, 0x01, 0xDD, 0xB0, 0x01, 0xFD, 0xC6, 0x02,
  0x88, 0xDC, 0x02, 0x98, 0x00, 0x03, 0xCC, 0x01,
  0x03, 0xEC, 0x03, 0x03, 0xDE, 0x09, 0x03, 0xD2,
  0x23, 0x03, 0xF2, 0x1A, 0x20, 0x82, 0x1E, 0x20,
  0x84, 0x22, 0x20, 0x95, 0x26, 0x20, 0x85, 0x30,
  0x20, 0x89, 0x39, 0x20, 0x8B, 0x3A, 0x20, 0x9B,
  0xAB, 0x20, 0xFE, 0xAC, 0x20, 0x80, 0x22, 0x21,
  0x99, 0xFF, 0xFF, 0xFF
};
#endif
//---------------------------------------------------------------------------
} // namespace utf8
//---------------------------------------------------------------------------
