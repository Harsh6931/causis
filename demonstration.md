.\build\causis.exe --help

then

.\build\causis.exe tokenize examples\target.ls

.\build\causis.exe parse examples\target.ls

.\build\causis.exe semantic invalid.ls
.\build\causis.exe semantic examples\target.ls

.\build\causis.exe ir examples\target.ls

.\build\causis.exe optimize examples\target.ls

.\build\causis.exe disassemble examples\target.ls

.\build\causis.exe run examples\target.ls

.\build\causis.exe optimize examples\target.ls --stats

if visulaize

$env:PATH = "C:\msys64\ucrt64\bin;C:\msys64\usr\bin;" + $env:PATH
cmake --build build

then run prog -> .\build\causis.exe run examples\target.ls --ticks 20 --log build\target_log.json

then open index.html

and feed build\path_log_json file