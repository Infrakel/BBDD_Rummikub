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
  .\Database\DatabaseLite.cc ^
  -I %1\Desarrollo\Lib_Graph\ESAT_rev248\include ^
  -I .\SQLite ^
  -I . ^
  %1\Desarrollo\Lib_Graph\ESAT_rev248\bin\ESAT.lib ^
  opengl32.lib user32.lib gdi32.lib shell32.lib Ws2_32.lib

@echo ---------------------------------------------------
@echo  Proceso por lotes finalizado.
@echo ---------------------------------------------------