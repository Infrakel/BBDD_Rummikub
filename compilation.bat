@REM Author: Andreu Sánchez
@REM Compilation script
@cls
@echo ---------------------------------------------------
@echo  ESAT 2026-2027: Programacion Avanzada
@echo ---------------------------------------------------
@echo  Batch script initiated
@echo ---------------------------------------------------
@echo off

cl /nologo /Zi /GR- /EHs /MD ^
  %1 ^
  .\Database\DatabaseLite.cc .\Database\DatabaseMaria.cc ^
  .\NativeFileDialogExtended\nfd_win.cpp ^
  -I .\MariaDB\include ^
  -I .\SQLite ^
  -I .\NativeFileDialogExtended\include ^
  -I .\Lib_Graph\ESAT_rev248\include ^
  -I . ^
  .\Lib_Graph\ESAT_rev248\bin\ESAT.lib ^
  .\MariaDB\lib\libmariadb.lib ^
  opengl32.lib user32.lib gdi32.lib shell32.lib Ws2_32.lib comctl32.lib ole32.lib

@echo ---------------------------------------------------
@echo  Batch script finished
@echo ---------------------------------------------------