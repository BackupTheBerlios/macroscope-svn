#!/bin/sh

versions="libadicpp"

version_libadicpp="include/adicpp"

wd=`pwd`;

for i in $versions; do
  eval version=\${i}
  eval version_header=\$version_${i}
  cd $version_header
  if test ".$1" = ".s"; then
    shtool version -l c -p ${version}_ -n $version -s $2 version.h >/dev/null
  elif test ".$1" = ".v"; then
    shtool version -l c -p ${version}_ -n $version -i v version.h >/dev/null
  elif test ".$1" = ".r"; then
    shtool version -l c -p ${version}_ -n $version -i r version.h >/dev/null
  elif test ".$1" = ".l"; then
    shtool version -l c -p ${version}_ -n $version -i l version.h >/dev/null
  else
    shtool version -l c -p ${version}_ -n $version -d long version.h
  fi
  cd $wd
done
