@echo off
setlocal

:: Đặt tên cho server và client
set /p SERVER_NAME="Enter server name (e.g., localhost): "
set /p CLIENT_NAME="Enter client name (e.g., localhost): "

:: Đặt đường dẫn để lưu file .pfx
set /p SERVER_PFX_PATH="Enter path for server .pfx file (e.g., server.pfx): "
set /p CLIENT_PFX_PATH="Enter path for client .pfx file (e.g., client.pfx): "

:: Đặt mật khẩu cho file .pfx
set /p SERVER_PASSWORD="Enter password for server .pfx (e.g., qwerty): "
set /p CLIENT_PASSWORD="Enter password for client .pfx (e.g., qwerty): "

:: Tạo chứng chỉ cho server
powershell -Command "$serverCert = New-SelfSignedCertificate -Subject 'CN=%SERVER_NAME%' -CertStoreLocation 'Cert:\CurrentUser\My' -KeyExportPolicy Exportable -KeySpec Signature -KeyLength 2048 -KeyAlgorithm RSA -HashAlgorithm SHA256; $serverPwd = ConvertTo-SecureString -String '%SERVER_PASSWORD%' -Force -AsPlainText; Export-PfxCertificate -Cert $serverCert -FilePath '%SERVER_PFX_PATH%' -Password $serverPwd;"

if %ERRORLEVEL% neq 0 (
    echo Failed to create server certificate.
    exit /b %ERRORLEVEL%
)

:: Tạo chứng chỉ cho client
powershell -Command "$clientCert = New-SelfSignedCertificate -Subject 'CN=%CLIENT_NAME%' -CertStoreLocation 'Cert:\CurrentUser\My' -KeyExportPolicy Exportable -KeySpec Signature -KeyLength 2048 -KeyAlgorithm RSA -HashAlgorithm SHA256; $clientPwd = ConvertTo-SecureString -String '%CLIENT_PASSWORD%' -Force -AsPlainText; Export-PfxCertificate -Cert $clientCert -FilePath '%CLIENT_PFX_PATH%' -Password $clientPwd;"

if %ERRORLEVEL% neq 0 (
    echo Failed to create client certificate.
    exit /b %ERRORLEVEL%
)

echo Certificates created successfully!
pause
endlocal


