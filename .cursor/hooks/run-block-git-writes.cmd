@echo off
where py >nul 2>&1 && py -3 "%~dp0block-git-writes.py" && exit /b %ERRORLEVEL%
where python3 >nul 2>&1 && python3 "%~dp0block-git-writes.py" && exit /b %ERRORLEVEL%
where python >nul 2>&1 && python "%~dp0block-git-writes.py" && exit /b %ERRORLEVEL%
echo block-git-writes: no Python interpreter found (tried py -3, python3, python) >&2
exit /b 1
