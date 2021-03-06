/*-
 * Copyright 2005-2008 Guram Dukashvili
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
#ifndef _table_H
#define _table_H
//-----------------------------------------------------------------------------
namespace ksys {
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
template <typename T,class RT = Array<T> > class Table {
  public:
    virtual ~Table();
    Table();
    Table(const Table<T,RT> & table){ operator = (table); }

    Table<T,RT> & operator = (const Table<T,RT> & table);

    Table<T,RT> & clear();
    Table<T,RT> & resize(uintptr_t rows, uintptr_t columns);

    Table<T,RT> & addRow();
    Table<T,RT> & insertRow(uintptr_t row);
    Table<T,RT> & removeRow(uintptr_t row);
    Table<T,RT> & removeRows(uintptr_t bRow,uintptr_t eRow);
    Table<T,RT> & addColumn(const utf8::String & columnName = utf8::String());
    Table<T,RT> & insertColumn(uintptr_t column, const utf8::String & columnName = utf8::String());
    Table<T,RT> & removeColumn(uintptr_t column);

    const utf8::String & columnName(uintptr_t column) const;
    Table<T,RT> & columnName(uintptr_t column, const utf8::String & columnName);

    intptr_t columnIndex(const utf8::String & columnName) const;

    uintptr_t rowCount() const;
    uintptr_t columnCount() const;

    T & cell(uintptr_t row,uintptr_t column);
    Table<T,RT> & cell(uintptr_t row,uintptr_t column,const T & value);
    T & cell(uintptr_t row,const utf8::String & columnName);
    Table<T,RT> & cell(uintptr_t row,const utf8::String & columnName,const T & value);
    const T & cell(uintptr_t row,uintptr_t column) const;
    const T & cell(uintptr_t row,const utf8::String & columnName) const;

    T & operator ()(uintptr_t row,uintptr_t column);
    Table<T,RT> & operator ()(uintptr_t row,uintptr_t column,const T & value);
    T & operator ()(uintptr_t row,const utf8::String & columnName);
    Table<T,RT> & operator ()(uintptr_t row,const utf8::String & columnName,const T & value);
    const T & operator ()(uintptr_t row,uintptr_t column) const;
    const T & operator ()(uintptr_t row,const utf8::String & columnName) const;
    
    Mutant sum(uintptr_t column) const;
    Mutant sum(uintptr_t column,uintptr_t bRow,uintptr_t eRow) const;
    Mutant sum(const utf8::String & columnName) const;
    Mutant sum(const utf8::String & columnName,uintptr_t bRow,uintptr_t eRow) const;
  protected:
    template <typename TT,typename P> class SortParam {
      public:
        const TT * table_;
        const P  * param_;
        intptr_t (* const f_)(uintptr_t,uintptr_t,const P &);

        SortParam(const TT & table, const P & param, intptr_t(*const f)(uintptr_t, uintptr_t, const P &))
          : table_(&table), param_(&param), f_(f) {}
      private:
        SortParam(const SortParam<TT,P> &){}
        void operator = (const SortParam<TT,P> &){}
    };
    template <typename RRT,typename TT,typename P> static intptr_t sortHelper(RRT * const & p1, RRT * const & p2, const SortParam< TT,P> & param)
    {
      return param.f_(&p1 - param.table_->rows_.ptr(), &p2 - param.table_->rows_.ptr(), *param.param_);
    }
  public:
    template <typename P> Table<T,RT> & sort(intptr_t(*const f)(uintptr_t row1, uintptr_t row2, const P & param),const P & p)
    {
      SortParam<Table<T,RT>,P> param(*this,p,f);
      intptr_t (*pSortHelper)(RT * const & p1,RT * const & p2,const SortParam< Table<T,RT>,P> & param);
      qSort(rows_.ptr(), 0, rows_.count() - 1, pSortHelper = sortHelper< RT,Table<T,RT>,P>, param);
      return *this;
    }
  protected:
    struct DSP {
      Table<T,RT> * table_;
      uintptr_t column_;
      bool descending_;
    };
    template <typename P> static intptr_t defaultSortFunction(uintptr_t row1,uintptr_t row2,const P & param){
      intptr_t c = 0;
      for( uintptr_t i = 0; i < param.count(); i++ ){
        c = param[i].table_->cell(row1,param[i].column_) < param[i].table_->cell(row2,param[i].column_) ? -1 :
	    param[i].table_->cell(row1,param[i].column_) > param[i].table_->cell(row2,param[i].column_) ? 1 : 0;
	if( c != 0 ){
	  c = param[i].descending_ ? -c : c;
	  break;
	}
      }
      return c;
    }
  public:
    Table<T,RT> & sort(const utf8::String & sortOrder){
      Array<DSP> param;
      param.resize(enumStringParts(sortOrder));
      for( uintptr_t i = 0; i < param.count(); i++ ){
        utf8::String order(stringPartByNo(sortOrder,i));
	      utf8::String::Iterator it(order);
        param[i].descending_ = false;
	      if( it.getChar() == '+' ){ it++; }
	      else
	      if( it.getChar() == '-' ){ it++; param[i].descending_ = true; }
	      param[i].column_ = columnIndex(it);
	      param[i].table_ = this;
      }
      return sort(defaultSortFunction<Array<DSP> >,param);
    }
  protected:
    class Name2Index {
      public:
        utf8::String name_;
        uintptr_t index_;

        virtual ~Name2Index() {}
        Name2Index(){}
        Name2Index(const utf8::String & name, uintptr_t index) : name_(name), index_(index) {}
      protected:
      private:
    };
    Vector<Name2Index>   name2Index_;
    Array<Name2Index *>  index2Name_;
    Vector<RT>           rows_;
  private:
    utf8::String genColumnName(const utf8::String & columnNameTemplate,uintptr_t & i,intptr_t & c) const;
    intptr_t name2Index(const utf8::String & columnName) const;
    uintptr_t name2Index(const utf8::String & columnName,intptr_t & c) const;
};
//-----------------------------------------------------------------------------
template <typename T,class RT> inline Table<T,RT>::~Table()
{
}
//-----------------------------------------------------------------------------
template <typename T,class RT> inline Table<T,RT>::Table()
{
}
//-----------------------------------------------------------------------------
template <typename T,class RT> inline
Table<T,RT> & Table<T,RT>::clear()
{
  rows_.clear();
  name2Index_.clear();
  index2Name_.clear();
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T,class RT>
#ifndef __BCPLUSPLUS__
inline
#endif
Table<T,RT> & Table<T,RT>::resize(uintptr_t rows, uintptr_t columns)
{
  assert(rows >= 0 && columns >= 0);
  while( rows > rows_.count() ) addRow();
  if( rows < rows_.count() ) rows_.resize(rows);
  while( columns < name2Index_.count() ) removeColumn(name2Index_.count() - 1);
  while( columns > name2Index_.count() ) addColumn();
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T,class RT> inline
Table<T,RT> & Table<T,RT>::addRow()
{
  rows_.add().resize(name2Index_.count());
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T,class RT> inline
Table<T,RT> & Table<T,RT>::insertRow(uintptr_t row)
{
  rows_.insert(row).resize(name2Index_.count());
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T,class RT> inline
Table<T,RT> & Table<T,RT>::removeRow(uintptr_t row)
{
  rows_.remove(row);
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T,class RT> inline
Table<T,RT> & Table<T,RT>::removeRows(uintptr_t bRow,uintptr_t eRow)
{
  if( bRow > eRow ) ksys::xchg(bRow,eRow);
  while( bRow <= eRow ){
    rows_.remove(bRow);
    if( bRow == eRow ) break;
    eRow--;
  }
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T,class RT>
#ifndef __BCPLUSPLUS__
inline
#endif
utf8::String Table<T,RT>::genColumnName(const utf8::String & columnNameTemplate, uintptr_t & i, intptr_t & c) const
{
  utf8::String cn(columnNameTemplate);
  i = name2Index(cn, c);
  if( c == 0 ){
    if( cn.length() == 0 && name2Index_.count() > 0 )
      cn = index2Name_[name2Index_.count() - 1]->name_;
    if( cn.isNull() == 0 || cn.ncompare("COLUMN_", 7) == 0 ) cn = "COLUMN";
    for( i = name2Index_.count(); i < ~uintptr_t(0); i++ ){
      utf8::String s(cn + "_" + utf8::int2Str((uintmax_t) i));
      i = name2Index(s, c);
      if( c != 0 ){
        cn = s;
        break;
      }
    }
  }
  return cn;
}
//-----------------------------------------------------------------------------
template <typename T,class RT>
#ifndef __BCPLUSPLUS__
inline
#endif
Table<T,RT> & Table<T,RT>::addColumn(const utf8::String & columnName)
{
  uintptr_t u;
  intptr_t c;
  utf8::String s(genColumnName(columnName,u,c));
  name2Index_.insert(u += (c > 0),Name2Index(s,name2Index_.count()));
  index2Name_.add(&name2Index_[u]);
  for( intptr_t i = rows_.count() - 1; i >= 0; i-- )
    rows_[i].resize(name2Index_.count());
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T,class RT>
#ifndef __BCPLUSPLUS__
inline
#endif
Table<T,RT> & Table<T,RT>::insertColumn(uintptr_t column, const utf8::String & columnName)
{
  intptr_t      i, c;
  utf8::String  s (genColumnName(columnName, i, c));
  name2Index_.insert(i += (c > 0), Name2Index(s, column));
  index2Name_.insert(column, &name2Index_[i]);
  for( i = rows_.count() - 1; i >= 0; i-- )
    rows_[i].insert(column, T());
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T,class RT>
#ifndef __BCPLUSPLUS__
inline
#endif
Table<T,RT> & Table<T,RT>::removeColumn(uintptr_t column)
{
  intptr_t i;
  for( i = rows_.count() - 1; i >= 0; i-- ) rows_[i].remove(column);
  name2Index_.remove(name2Index(index2Name_[column]->name_));
  index2Name_.remove(column);
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T,class RT> inline
const utf8::String & Table<T,RT>::columnName(uintptr_t column) const
{
  assert((uintptr_t) column < (uintptr_t) name2Index_.count());
  return index2Name_[column]->name_;
}
//-----------------------------------------------------------------------------
template <typename T,class RT>
#ifndef __BCPLUSPLUS__
inline
#endif
Table<T,RT> & Table<T,RT>::columnName(uintptr_t column, const utf8::String & columnName)
{
  assert((uintptr_t) column < (uintptr_t) name2Index_.count());
  intptr_t  c, i  = name2Index(columnName, c);
  if( c != 0 ){
    name2Index_.remove(index2Name_[column]);
    i = name2Index(columnName, c);
    name2Index_.insert(i += (c > 0), Name2Index(columnName, column));
    index2Name_[column] = &name2Index_[i];
  }
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T,class RT>
#ifndef __BCPLUSPLUS__
inline
#endif
intptr_t Table<T,RT>::name2Index(const utf8::String & columnName) const
{
  intptr_t  c, low  = 0, high = name2Index_.count() - 1, pos ;

  while( low <= high ){
    pos = (low + high) / 2;
    c = columnName.casecompare(name2Index_[pos].name_);
    if( c > 0 ){
      low = pos + 1;
    }
    else if( c < 0 ){
      high = pos - 1;
    }
    else
      return pos;
  }
  return -1;
}
//-----------------------------------------------------------------------------
template <typename T,class RT>
#ifndef __BCPLUSPLUS__
inline
#endif
uintptr_t Table<T,RT>::name2Index(const utf8::String & columnName, intptr_t & c) const
{
  intptr_t  low = 0, high = name2Index_.count() - 1, pos = -1;

  c = 1;
  while( low <= high ){
    pos = (low + high) / 2;
    c = columnName.casecompare(name2Index_[pos].name_);
    if( c > 0 ){
      low = pos + 1;
    }
    else if( c < 0 ){
      high = pos - 1;
    }
    else
      break;
  }
  return pos;
}
//-----------------------------------------------------------------------------
template <typename T,class RT> inline
intptr_t Table<T,RT>::columnIndex(const utf8::String & columnName) const
{
  intptr_t i = name2Index(columnName);
  if( i >= 0 ) i = name2Index_[i].index_;
  return i;
}
//-----------------------------------------------------------------------------
template <typename T,class RT> inline
uintptr_t Table<T,RT>::rowCount() const
{
  return rows_.count();
}
//-----------------------------------------------------------------------------
template <typename T,class RT> inline
uintptr_t Table<T,RT>::columnCount() const
{
  return name2Index_.count();
}
//-----------------------------------------------------------------------------
template <typename T,class RT> inline
T & Table<T,RT>::cell(uintptr_t row, uintptr_t column)
{
  assert(row < rows_.count() && column < name2Index_.count());
  return rows_[row][column];
}
//-----------------------------------------------------------------------------
template <typename T,class RT> inline
Table<T,RT> & Table<T,RT>::cell(uintptr_t row, uintptr_t column,const T & value)
{
  assert(row < rows_.count() && column < name2Index_.count());
  rows_[row][column] = value;
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T,class RT> inline
T & Table<T,RT>::cell(uintptr_t row, const utf8::String & columnName)
{
  uintptr_t column  = columnIndex(columnName);
  assert(row < rows_.count() && column < name2Index_.count());
  return rows_[row][column];
}
//-----------------------------------------------------------------------------
template <typename T,class RT> inline
Table<T,RT> & Table<T,RT>::cell(uintptr_t row, const utf8::String & columnName,const T & value)
{
  uintptr_t column  = columnIndex(columnName);
  assert(row < rows_.count() && column < name2Index_.count());
  rows_[row][column] = value;
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T,class RT> inline
const T & Table<T,RT>::cell(uintptr_t row, uintptr_t column) const
{
  assert(row < rows_.count() && column < name2Index_.count());
  return rows_[row][column];
}
//-----------------------------------------------------------------------------
template <typename T,class RT> inline
const T & Table<T,RT>::cell(uintptr_t row, const utf8::String & columnName) const
{
  uintptr_t column  = columnIndex(columnName);
  assert(row < rows_.count() && column < name2Index_.count());
  return rows_[row][column];
}
//-----------------------------------------------------------------------------
template <typename T,class RT> inline
T & Table<T,RT>::operator ()(uintptr_t row, uintptr_t column)
{
  return cell(row, column);
}
//-----------------------------------------------------------------------------
template <typename T,class RT> inline
Table<T,RT> & Table<T,RT>::operator ()(uintptr_t row, uintptr_t column,const T & value)
{
  return cell(row,column,value);
}
//-----------------------------------------------------------------------------
template <typename T,class RT> inline
T & Table<T,RT>::operator ()(uintptr_t row, const utf8::String & columnName)
{
  return cell(row, columnName);
}
//-----------------------------------------------------------------------------
template <typename T,class RT> inline
Table<T,RT> & Table<T,RT>::operator ()(uintptr_t row, const utf8::String & columnName,const T & value)
{
  return cell(row,columnName,value);
}
//-----------------------------------------------------------------------------
template <typename T,class RT> inline
const T & Table<T,RT>::operator ()(uintptr_t row, uintptr_t column) const
{
  return cell(row, column);
}
//-----------------------------------------------------------------------------
template <typename T,class RT> inline
const T & Table<T,RT>::operator ()(uintptr_t row, const utf8::String & columnName) const
{
  return cell(row,columnName);
}
//-----------------------------------------------------------------------------
template <typename T,class RT> inline
Mutant Table<T,RT>::sum(uintptr_t column) const
{
  return sum(column,0,rows_.count() - 1);
}
//-----------------------------------------------------------------------------
template <typename T,class RT> inline
Mutant Table<T,RT>::sum(uintptr_t column,uintptr_t bRow,uintptr_t eRow) const
{
  assert( column < name2Index_.count() );
  if( bRow > eRow ) ksys::xchg(bRow,eRow);
  if( bRow >= rows_.count() ) bRow = rows_.count() - 1;
  if( eRow >= rows_.count() ) bRow = rows_.count() - 1;
  Mutant summa = 0;
  if( rows_.count() > 0 ){
    while( bRow <= eRow ){
      Mutant a = rows_[bRow][column];
      if( summa.type() == mtFloat || a.type() == mtFloat ) 
        summa = (ldouble) summa + (ldouble) a;
      else
        summa = intmax_t(summa) + intmax_t(a);
      if( bRow == eRow ) break;
      bRow++;
    }
  }
  return summa;
}
//-----------------------------------------------------------------------------
template <typename T,class RT> inline
Mutant Table<T,RT>::sum(const utf8::String & columnName) const
{
  return sum(columnIndex(columnName));
}
//-----------------------------------------------------------------------------
template <typename T,class RT> inline
Mutant Table<T,RT>::sum(const utf8::String & columnName,uintptr_t bRow,uintptr_t eRow) const
{
  return sum(columnIndex(columnName),bRow,eRow);
}
//-----------------------------------------------------------------------------
template <typename T,class RT> inline
Table<T,RT> & Table<T,RT>::operator = (const Table<T,RT> & table)
{
  clear();
  for( uintptr_t i = 0; i < table.columnCount(); i++ )
    addColumn(table.columnName(i));
  for( uintptr_t i = 0; i < table.rowCount(); i++ ){
    addRow();
    for( uintptr_t j = 0; j < table.columnCount(); j++ )
      cell(i,j) = table.cell(i,j);
  }
  return *this;
}
//-----------------------------------------------------------------------------
} // namespace ksys
//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------
