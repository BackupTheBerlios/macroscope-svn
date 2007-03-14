/*-
 * Copyright 2005-2007 Guram Dukashvili
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
#ifndef _endian_H_
#define _endian_H_
//---------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//---------------------------------------------------------------------------
#if !HAVE_BE16ENC && BYTE_ORDER == LITTLE_ENDIAN
//---------------------------------------------------------------------------
__forceinline void be16enc(void * pp,uint16_t u)
{
  unsigned char *p = (unsigned char *)pp;

  p[0] = (u >> 8) & 0xff;
  p[1] = u & 0xff;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !HAVE_BE32ENC && BYTE_ORDER == LITTLE_ENDIAN
//---------------------------------------------------------------------------
__forceinline void be32enc(void * pp,uint32_t u)
{
  uint8_t * p = (uint8_t *) pp;

  p[0] = (u >> 24) & 0xff;
  p[1] = (u >> 16) & 0xff;
  p[2] = (u >> 8) & 0xff;
  p[3] = u & 0xff;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !HAVE_BE64ENC && BYTE_ORDER == LITTLE_ENDIAN
//---------------------------------------------------------------------------
__forceinline void be64enc(void *pp, uint64_t u)
{
  unsigned char *p = (unsigned char *)pp;

  be32enc(p,(uint32_t) (u >> 32));
  be32enc(p + 4,(uint32_t) (u & 0xffffffff));
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !HAVE_HTOBE16 && BYTE_ORDER == LITTLE_ENDIAN
//---------------------------------------------------------------------------
__forceinline uint16_t htobe16(uint16_t u)
{
  return (u >> 8) | ((u & 0xff) << 8);
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !HAVE_HTOBE32 && BYTE_ORDER == LITTLE_ENDIAN
//---------------------------------------------------------------------------
__forceinline uint32_t htobe32(uint32_t u)
{
  return (u >> 24) | ((u >> 8) & 0xff00) | ((u << 8) & 0xff0000) | (u << 24);
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !HAVE_HTOBE64 && BYTE_ORDER == LITTLE_ENDIAN
//---------------------------------------------------------------------------
__forceinline uint64_t htobe64(uint64_t u)
{
  return ((uint64_t) htobe32((uint32_t) u) << 32) | htobe32((uint32_t) (u >> 32));
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !HAVE_LE16ENC && BYTE_ORDER == LITTLE_ENDIAN
//---------------------------------------------------------------------------
__forceinline void le16enc(void *pp, uint16_t u)
{
  *(uint16_t *) pp = u;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !HAVE_LE32ENC && BYTE_ORDER == LITTLE_ENDIAN
//---------------------------------------------------------------------------
__forceinline void le32enc(void *pp, uint32_t u)
{
  *(uint32_t *) pp = u;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !HAVE_LE64ENC && BYTE_ORDER == LITTLE_ENDIAN
//---------------------------------------------------------------------------
__forceinline void le64enc(void *pp, uint64_t u)
{
  *(uint64_t *) pp = u;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !HAVE_LE16TOH && BYTE_ORDER == LITTLE_ENDIAN
//---------------------------------------------------------------------------
__forceinline uint16_t le16toh(uint16_t u)
{
  return u;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !HAVE_LE32TOH && BYTE_ORDER == LITTLE_ENDIAN
//---------------------------------------------------------------------------
__forceinline uint32_t le32toh(uint32_t u)
{
  return u;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !HAVE_LE64TOH && BYTE_ORDER == LITTLE_ENDIAN
//---------------------------------------------------------------------------
__forceinline uint64_t le64toh(uint64_t u)
{
  return u;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !HAVE_BE16DEC && BYTE_ORDER == LITTLE_ENDIAN
//---------------------------------------------------------------------------
__forceinline uint16_t be16dec(const void * pp)
{
  unsigned char const *p = (unsigned char const *)pp;
 
  return ((p[0] << 8) | p[1]);
}

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !HAVE_BE32DEC && BYTE_ORDER == LITTLE_ENDIAN
//---------------------------------------------------------------------------
__forceinline uint32_t be32dec(const void * pp)
{
  uint8_t const * p = (uint8_t const *) pp;
  
  return ((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]);
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !HAVE_BE64DEC && BYTE_ORDER == LITTLE_ENDIAN
//---------------------------------------------------------------------------
__forceinline uint64_t be64dec(const void *pp)
{
  unsigned char const *p = (unsigned char const *)pp;

  return (((uint64_t)be32dec(p) << 32) | be32dec(p + 4));
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !HAVE_BE16TOH && BYTE_ORDER == LITTLE_ENDIAN
//---------------------------------------------------------------------------
__forceinline uint16_t be16toh(uint16_t u)
{
  return (u >> 8) | (u & 0xff) << 8;
}

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !HAVE_BE32TOH && BYTE_ORDER == LITTLE_ENDIAN
//---------------------------------------------------------------------------
__forceinline uint32_t be32toh(uint32_t u)
{
  return (u << 24) | ((u << 8) & 0xff0000) | ((u >> 8) & 0xff00) | (u >> 24);
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !HAVE_BE64TOH && BYTE_ORDER == LITTLE_ENDIAN
//---------------------------------------------------------------------------
__forceinline uint64_t be64toh(uint64_t u)
{
  return ((uint64_t) be32toh((uint32_t) u) << 32) | be32toh((uint32_t) (u >> 32));
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !HAVE_LE16DEC && BYTE_ORDER == LITTLE_ENDIAN
//---------------------------------------------------------------------------
__forceinline uint16_t le16dec(const void *pp)
{
  return *(uint16_t *) pp;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !HAVE_LE32DEC && BYTE_ORDER == LITTLE_ENDIAN
//---------------------------------------------------------------------------
__forceinline uint32_t le32dec(const void *pp)
{
  return *(uint32_t *) pp;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !HAVE_LE64DEC && BYTE_ORDER == LITTLE_ENDIAN
//---------------------------------------------------------------------------
__forceinline uint64_t le64dec(const void *pp)
{
  return *(uint64_t *) pp;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !HAVE_HTOLE16 && BYTE_ORDER == LITTLE_ENDIAN
//---------------------------------------------------------------------------
__forceinline uint16_t htole16(uint16_t u)
{
  return u;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !HAVE_HTOLE32 && BYTE_ORDER == LITTLE_ENDIAN
//---------------------------------------------------------------------------
__forceinline uint32_t htole32(uint32_t u)
{
  return u;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !HAVE_HTOLE64 && BYTE_ORDER == LITTLE_ENDIAN
//---------------------------------------------------------------------------
__forceinline uint64_t htole64(uint64_t u)
{
  return u;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#ifdef __cplusplus
};
#endif
//---------------------------------------------------------------------------
#endif /* _endian_H_ */
//---------------------------------------------------------------------------
