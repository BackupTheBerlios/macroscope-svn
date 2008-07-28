@echo off

SET DISR=libfbcpp-1.0.0-prealpha

cd ..\..
del *gz *bz2 *zip *rar
pkzip25 -add -attr=all -dir -nozip %DISR%.zip
tar -c --exclude=*zip --exclude=*tar . >%DISR%.tar
gzip --best -c %DISR%.tar >%DISR%.tar.gz
bzip2 --best -z %DISR%.tar
rem rar a -r -s -av- -mce -mc63:128t -ms -md4096 -m5 -x*.gz -x*.bz2 -x*.zip -y %DISR%.rar
