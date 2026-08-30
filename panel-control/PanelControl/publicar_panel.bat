@echo off
setlocal

set "ORIGEN=%~dp0"
set "SALIDA_PUBLICACION=%ORIGEN%publicado"
set "DESTINO=%ORIGEN%..\..\servicio-inferencia\out\build\x64-Debug"

echo Publicando PanelControl (esto puede tardar un rato la primera vez)...
dotnet publish "%ORIGEN%PanelControl.csproj" -c Debug -p:Platform=x64 -r win-x64 --self-contained true -p:WindowsPackageType=None -o "%SALIDA_PUBLICACION%"

if errorlevel 1 (
    echo.
    echo La publicacion fallo. Revisa el mensaje de arriba y pegamelo.
    pause
    exit /b 1
)

if not exist "%SALIDA_PUBLICACION%\PanelControl.exe" (
    echo.
    echo dotnet publish termino pero no encuentro PanelControl.exe en:
    echo %SALIDA_PUBLICACION%
    echo Revisa esa carpeta a mano.
    pause
    exit /b 1
)

echo.
echo Copiando PanelControl.exe y sus dependencias a:
echo %DESTINO%
robocopy "%SALIDA_PUBLICACION%" "%DESTINO%" /E

echo.
echo Listo. Ahora ejecuta servicio_inferencia.exe como siempre.
pause
