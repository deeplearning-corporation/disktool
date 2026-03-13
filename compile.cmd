@echo off
echo Compiling DiskTool...

REM 创建build目录
if not exist build mkdir build

REM 编译所有源文件
gcc -c src/disk_info.c -Iinclude -o build/disk_info.o
if %errorlevel% neq 0 goto error

gcc -c src/disk_utils.c -Iinclude -o build/disk_utils.o
if %errorlevel% neq 0 goto error

gcc -c src/gui.c -Iinclude -o build/gui.o -mwindows
if %errorlevel% neq 0 goto error

gcc -c src/main.c -Iinclude -o build/main.o -mwindows
if %errorlevel% neq 0 goto error

REM 链接所有对象文件
gcc build/disk_info.o build/disk_utils.o build/gui.o build/main.o -o build/disktool.exe -lcomctl32 -lcomdlg32 -lgdi32 -lole32 -luuid -mwindows
if %errorlevel% neq 0 goto error

echo.
echo Compilation successful!
echo Output: build\disktool.exe
echo.
goto end

:error
echo.
echo Compilation failed!
pause

:end