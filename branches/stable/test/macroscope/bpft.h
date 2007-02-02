/*-
 * Copyright 2007 Guram Dukashvili
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
//------------------------------------------------------------------------------
#ifndef _bpftH_
#define _bpftH_
//------------------------------------------------------------------------------
namespace macroscope {
//------------------------------------------------------------------------------
struct BPFTEntry32 {
  struct in_addr srcIp_;
  struct in_addr dstIp_;  /* src ip addr and dst ip addr */
  u_char  ipProtocol_;    /* which protocol been used (/etc/protocols) */
  u_short srcPort_;       /* source port */
  u_short dstPort_;       /* destination port */
  uint32_t  dgramSize_;   /* how many bytes in ip datagrams passed */
  uint32_t  dataSize_;    /* how many data bytes passed */
};

struct BPFTEntry {
  struct in_addr srcIp_;
  struct in_addr dstIp_;  /* src ip addr and dst ip addr */
  u_char  ipProtocol_;    /* which protocol been used (/etc/protocols) */
  u_short srcPort_;       /* source port */
  u_short dstPort_;       /* destination port */
  uint64_t dgramSize_;     /* how many bytes in ip datagrams passed */
  uint64_t dataSize_;      /* how many data bytes passed */
};

struct BPFTHeader32 {
  int32_t eCount_;
  struct timeval32 start_;
  struct timeval32 stop_;
};

struct BPFTHeader {
  int32_t eCount_;
  struct timeval64 start_;
  struct timeval64 stop_;
};
//------------------------------------------------------------------------------
} // namespace macroscope
//------------------------------------------------------------------------------
#endif									    
//------------------------------------------------------------------------------
