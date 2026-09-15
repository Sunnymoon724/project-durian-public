@echo off
setlocal

set "MESSAGE=%*"
if not defined MESSAGE set /p "MESSAGE=보낼 텍스트: "
if not defined MESSAGE (
    echo 보낼 텍스트가 없습니다.
    exit /b 1
)

set "ENV_FILE=%~dp0Webhook.env"
if not exist "%ENV_FILE%" (
    echo Webhook.env 파일을 찾을 수 없습니다.
    exit /b 1
)

set /p "WEBHOOK="<"%ENV_FILE%"
if not defined WEBHOOK (
    echo Webhook URL이 비어 있습니다.
    exit /b 1
)

curl.exe --fail-with-body --silent --show-error --connect-timeout 3 --max-time 8 ^
  -H "Content-Type: application/json" ^
  --data-raw "{\"content\":\"%MESSAGE%\"}" ^
  "%WEBHOOK%"

if errorlevel 1 (
    echo 전송 실패.
    exit /b 1
)
echo 디스코드로 전송했습니다.
exit /b 0
