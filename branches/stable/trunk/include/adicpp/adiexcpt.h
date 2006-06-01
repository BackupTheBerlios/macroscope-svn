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
#ifndef _adiexcpt_H_
#define _adiexcpt_H_
//---------------------------------------------------------------------------
namespace adicpp {

//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
/*class EADI : public ksys::Exception {
  public:
    EADI();
    EADI(const ksys::Exception & e);
    EADI(const fbcpp::EClientServer & e);
    EADI(const mycpp::EClientServer & e);

    bool isFatalError() const;
  protected:
  private:
    bool fatalError_;
};
//---------------------------------------------------------------------------
inline EADI::EADI(const ksys::Exception & e) : ksys::Exception(e), fatalError_(false)
{
}
//---------------------------------------------------------------------------
inline EADI::EADI(const fbcpp::EClientServer & e) : ksys::Exception(e), fatalError_(e.isFatalError())
{
}
//---------------------------------------------------------------------------
inline EADI::EADI(const mycpp::EClientServer & e) : ksys::Exception(e), fatalError_(e.isFatalError())
{
}
//---------------------------------------------------------------------------
inline const bool & EADI::isFatalError() const
{
  return fatalError_;
} */
//---------------------------------------------------------------------------
} // namespace adicpp
//---------------------------------------------------------------------------
#endif /* _adiexcpt_H_ */
