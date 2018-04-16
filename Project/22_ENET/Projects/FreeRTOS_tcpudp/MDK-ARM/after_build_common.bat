@echo off
set keil_compiler_include_floder=%1
set full_name_path=%2
set linker_output_file_name=%3
echo %keil_compiler_include_floder%
echo %full_name_path%
echo %linker_output_file_name%

%keil_compiler_include_floder%\..\bin\fromelf.exe --bin -o  "bin\%linker_output_file_name%.bin" "%full_name_path%"
%keil_compiler_include_floder%\..\bin\fromelf.exe -acd --interleave=source -o "bin\%linker_output_file_name%.disasm" "%full_name_path%"
del "bin\%linker_output_file_name%_MP.bin"