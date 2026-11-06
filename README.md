\# BVGThumb — Thumbnail Provider for BVG files



Плагин для Проводника Windows, который показывает превью (thumbnail) для файлов `.bvg` (Binary Vector Graphics). Плагин читает чанк `THMB` из BVG-файла, декодирует PNG и отдаёт его Windows через COM-интерфейс `IThumbnailProvider`.



\## Структура проекта



```text

bvg-thumb/

├── CMakeLists.txt

├── BvgThumbProvider.cpp

├── BvgThumbProvider.h

├── BvgThumb.cpp

├── bvg/

│   ├── include/

│   ├── lib/

│   └── bin/                # zstd.dll, z.dll, lz4.dll

└── build/                  # папка сборки (создаётся)

```



\## Сборка



Открой `cmd` (не обязательно от админа) и выполни:



```cmd

cd C:\\Users\\Tima2\\Documents\\bvg-thumb

mkdir build

cd build

cmake -G "Visual Studio 17 2022" -A x64 ..

cmake --build . --config Release

```



После сборки появится файл:



```text

C:\\Users\\Tima2\\Documents\\bvg-thumb\\build\\Release\\BvgThumb.dll

```



Рядом с ним CMake автоматически скопирует `zstd.dll`, `z.dll`, `lz4.dll` из `bvg/bin`.



\## Регистрация



Открой \*\*новую\*\* командную строку \*\*от имени администратора\*\*.



```cmd

regsvr32 "C:\\Users\\Tima2\\Documents\\bvg-thumb\\build\\Release\\BvgThumb.dll"

```



Должно появиться окно `DllRegisterServer in ... succeeded`.



\### Отмена регистрации



```cmd

regsvr32 /u "C:\\Users\\Tima2\\Documents\\bvg-thumb\\build\\Release\\BvgThumb.dll"

```



\## Очистка кэша Проводника



Сохрани этот код как `apply.cmd` и запускай \*\*от имени администратора\*\*:



```cmd

@echo off

setlocal



net session >nul 2>\&1

if %errorLevel% neq 0 (

&#x20;   echo \[ERROR] Run as Administrator!

&#x20;   pause

&#x20;   exit /b 1

)



taskkill /f /im explorer.exe >nul 2>\&1

del /f /s /q /a "%LocalAppData%\\Microsoft\\Windows\\Explorer\\thumbcache\_\*.db" >nul 2>\&1

del /f /s /q /a "%LocalAppData%\\Microsoft\\Windows\\Explorer\\iconcache\_\*.db" >nul 2>\&1

start explorer.exe



echo DONE!

pause

```



\## Проверка



1\. Запусти `apply.cmd` от админа.

2\. Скопируй любой `.bvg` файл в файл с новым именем (например, `test\_final\_999.bvg`).

3\. Открой папку в Проводнике и переключи вид на «Крупные значки».

4\. В колонке «Тип» должно быть написано «BVG vector image», а на файле — превью из чанка `THMB`.



\## Проверка реестра



```cmd

reg query "HKCR\\CLSID\\{3F2504E0-4F89-11D3-9A0C-0305E82C3301}\\InprocServer32"

reg query "HKCR\\.bvg\\ShellEx\\{e357fccd-a995-4576-b01f-234630154e96}"

```



\## Требования к BVG-файлу



Плагин читает только чанк `THMB` (сырые PNG-байты). Если его нет — превью не будет.



При сохранении через Python-библиотеку `bvg` используй `default\_codec=None` в `write()`, чтобы не перезаписать настройки `THMB`.



\## Лицензия



MIT

