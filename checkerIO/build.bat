@echo off

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat"

set CommonCompilerFlags=-MTd -nologo -fp:fast -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4456 -wd4201 -wd4100 -wd4189 -wd4505 -wd4127 -DCHECKERIO_INTERNAL=1 -DCHECKERIO_SLOW=1 -DCHECKERIO_WIN32=1 -FC -Z7 -EHsc
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib

REM TODO - can we just build both with one exe?

IF NOT EXIST ..\bin mkdir ..\bin
pushd ..\bin

REM 32-bit build
REM cl %CommonCompilerFlags% ..\handmade\code\source.cpp /link -subsystem:windows,5.1 %CommonLinkerFlags%

REM 64-bit build
del *.pdb > NUL 2> NUL
REM Optimization switches /O2
echo WAITING FOR PDB > lock.tmp
cl %CommonCompilerFlags% ..\checkerIO\checkerio.cpp -Fmcheckerio.map -LD /link -incremental:no -opt:ref -PDB:handmade_%random%.pdb -EXPORT:GameUpdateAndRender
del lock.tmp
REM cl %CommonCompilerFlags% ..\code\win_checkerio.cpp -Fmwin_checkerio.map /link %CommonLinkerFlags%
popd
