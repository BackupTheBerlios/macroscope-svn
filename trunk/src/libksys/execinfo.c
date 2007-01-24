/*
 * Copyright (c) 2003 Maxim Sobolev <sobomax@FreeBSD.org>
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
 *
 * $Id: execinfo.c,v 1.3 2004/07/19 05:21:09 sobomax Exp $
 */

#include <adicpp/lconfig.h>
#include <adicpp/execinfo/execinfo.h>
#include <adicpp/execinfo/stacktraverse.h>

#define D10(x) ceil(log10(((x) == 0) ? 2 : ((x) + 1)))

inline static void * realloc_safe(void * ptr,size_t size)
{
  void * nptr = realloc(ptr,size);
  if( nptr == NULL ) free(ptr);
  return nptr;
}

intptr_t backtrace(void ** buffer,intptr_t size)
{
  intptr_t i;
  assert( buffer != NULL );
  for( i = 1; getframeaddr(i + 1) != NULL && i != size + 1; i++ ){
    buffer[i - 1] = getreturnaddr(i);
    fprintf(stderr,"%p\n",buffer[i - 1]);
    if( (i > 1 && buffer[i - 1] - buffer[i - 2] > 0x100000) || buffer[i - 1] == NULL ) break;
  }
  return i - 1;
}

char ** backtrace_symbols(void ** buffer,intptr_t size)
{
  intptr_t i, j, clen, alen, offset;
  char ** rval;
  char * cp;
  Dl_info info;

  assert( buffer != NULL );
  clen = size * sizeof(char *);
  rval = buffer;//malloc(clen);
  if( rval == NULL ) return NULL;
  for( i = 0; i < size; i++ ){
    cp = (char *) rval + clen;
    alen = 0;
    if( dladdr(buffer[i],&info) != 0 ){
      if( info.dli_sname == NULL ) info.dli_sname = "???";
      if( info.dli_saddr == NULL ) info.dli_saddr = buffer[i];
      offset = buffer[i] - info.dli_saddr;
// "0x01234567 <function+offset> at filename"
/*      alen = 2 +                      // "0x"
             (sizeof(void *) * 2) +   // "01234567"
             2 +                      // " <"
             strlen(info.dli_sname) + // "function"
             1 +                      // "+" */
             D10(offset) +            // "offset
             5 +                      // "> at "
             strlen(info.dli_fname) + // "filename"
             1;                       // "\0"*/
      for(;;){
        rval = realloc_safe(rval,clen + alen);
        if( rval == NULL ) return NULL;
        cp = (char *) rval + clen;
        j = snprintf(cp,alen,"%p <%s+%"PRIdPTR"> at %s",buffer[i],info.dli_sname,offset,info.dli_fname);
	if( j < alen ) break;
	alen = (alen << 1) + (alen == 0);
      }
      alen = j;
    }
    else {
      for(;;){
/*      alen = 2 +                      // "0x"
             (sizeof(void *) * 2) +   // "01234567"
             1;                       // "\0"*/
        rval = realloc_safe(rval,clen + alen);
        if( rval == NULL ) return NULL;
        cp = (char *) rval + clen;
        j = snprintf(cp,alen,"%p",buffer[i]);
	if( j < alen ) break;
	alen = (alen << 1) + (alen == 0);
      }
    }
    rval[i] = cp;
    clen += alen;
  }
  rval = realloc_safe(rval,clen);
  return rval;
}
