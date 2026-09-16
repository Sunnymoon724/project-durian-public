@echo off
setlocal
chcp 65001 >nul

set "MESSAGE=%~1"
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

powershell.exe -NoProfile -Command "$message = $env:MESSAGE; $webhook = Get-Content -LiteralPath $env:ENV_FILE -Raw; $webhook = $webhook.Trim(); $body = ConvertTo-Json -InputObject @{ content = $message } -Compress; $bytes = [System.Text.Encoding]::UTF8.GetBytes($body); Invoke-RestMethod -Uri $webhook -Method Post -ContentType 'application/json; charset=utf-8' -Body $bytes"

if errorlevel 1 (
    echo 전송 실패.
    exit /b 1
)
echo 디스코드로 전송했습니다.
exit /b 0
