#!/bin/sh

work_dir=$1

if [ ! -d "$work_dir" ]; then
    work_dir=.
    if test -f "Makefile"; then
	make distclean 1>/dev/null 2>/dev/null
    fi
fi

echo clean in $work_dir
rm -Rf $work_dir/*.o
rm -Rf $work_dir/*.gch
rm -Rf $work_dir/*.obj
rm -Rf $work_dir/*.lo*
rm -Rf $work_dir/*.htm*
rm -Rf $work_dir/.libs
rm -Rf $work_dir/.deps
rm -Rf $work_dir/Makefile
rm -Rf $work_dir/Makefile.in
rm -Rf $work_dir/tools
rm -Rf $work_dir/autom4te.cache
rm -Rf $work_dir/configure
rm -Rf $work_dir/aclocal.m4
rm -Rf $work_dir/libtool
rm -Rf $work_dir/config.h.in*
rm -Rf $work_dir/config.status
rm -Rf $work_dir/config.log
rm -Rf $work_dir/config.cache
rm -Rf "$work_dir/html report"

dirs=`ls -lL $work_dir | awk '{ if( index($1,"d") > 0 && $1 != "total" ) printf "\"%s\" ",substr($0,index($0,$8) + length($8) + 1,1000000) }' - | sort`
for dir in $dirs; do
  dir=`echo -n $dir | awk '{ printf "%s", substr($0,2,length($0) - 2) }'"`
  if test -d "$work_dir/$dir"; then
    if test ! "$dir" = "."; then
      if test ! "$dir" = ".."; then
        $0 "$work_dir/$dir"
      fi
    fi
  fi
done
