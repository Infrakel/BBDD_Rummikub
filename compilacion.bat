@REM Compilación y Enlace con biblioteca gráfica.
@cls
@echo ---------------------------------------------------
@echo  ESAT Curso 2025-2026 Asignatura PRG Primero
@echo ---------------------------------------------------
@echo  Proceso por lotes iniciado.
@echo ---------------------------------------------------
@echo off

cl /nologo /Zi /GR- /EHs /MD ^
  %2 ^
  .\Database\DatabaseLite.cc .\Database\DatabaseMaria.cc ^
  .\NativeFileDialog\nfd_common.c .\NativeFileDialog\nfd_win.cpp ^
  -I .\MariaDB\include ^
  -I .\SQLite ^
  -I .\NativeFileDialog\include ^
  -I %1\Desarrollo\Lib_Graph\ESAT_rev248\include ^
  -I . ^
  %1\Desarrollo\Lib_Graph\ESAT_rev248\bin\ESAT.lib ^
  .\MariaDB\lib\libmariadb.lib ^
  opengl32.lib user32.lib gdi32.lib shell32.lib Ws2_32.lib comctl32.lib ole32.lib

@REM We need to include this copy, for work with the x86 binary
copy /Y ".\MariaDB\lib\libmariadb.dll" "."

@echo ---------------------------------------------------
@echo  Proceso por lotes finalizado.
@echo ---------------------------------------------------