# BVGThumb
Плагин для Проводника Windows, который показывает превью для файлов с расширением `.bvg`. Плагин читает чанк `THMB` из файла, декодирует PNG и отдаёт его Windows через COM-интерфейс `IThumbnailProvider`.

## Структура проекта
```text
bvg-thumb/
├── CMakeLists.txt
├── BvgThumbProvider.cpp
├── BvgThumbProvider.h
├── BvgThumb.cpp
├── bvg/
│   ├── include/
│   ├── lib/
│   └── bin/
└── build/
```

## Сборка
```cmd
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config Release
```

После сборки появится файл:
```text
build\Release\BvgThumb.dll
```

## Регистрация
Открой командную строку от имени администратора и перейди в папку проекта.

```cmd
cd путь\к\bvg-thumb
regsvr32 "build\Release\BvgThumb.dll"
```

Отмена регистрации:
```cmd
regsvr32 /u "build\Release\BvgThumb.dll"
```

## Проверка
1. Открой папку в Проводнике и переключи вид на «Крупные значки».
2. В колонке «Тип» должно быть написано «BVG vector image», а на файле — превью.

## Проверка реестра
```cmd
reg query "HKCR\CLSID\{3F2504E0-4F89-11D3-9A0C-0305E82C3301}\InprocServer32"
reg query "HKCR\.bvg\ShellEx\{e357fccd-a995-4576-b01f-234630154e96}"
```

## Требования к файлу
Плагин читает только чанк `THMB`. Если его нет — превью не будет.

## Лицензия

MIT