# DataPainter Windows Uninstaller
# Usage: powershell -ExecutionPolicy Bypass -File uninstall.ps1

param(
    [string]$InstallPath = "$env:ProgramFiles\DataPainter"
)

$ErrorActionPreference = "Stop"

Write-Host "DataPainter Uninstaller" -ForegroundColor Cyan
Write-Host "=======================" -ForegroundColor Cyan
Write-Host ""

# Check if running as administrator
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if (-not $isAdmin) {
    Write-Host "This uninstaller requires administrator privileges." -ForegroundColor Red
    Write-Host "Please run PowerShell as Administrator and try again." -ForegroundColor Red
    Write-Host ""
    Write-Host "Right-click PowerShell and select 'Run as Administrator', then run:" -ForegroundColor Yellow
    Write-Host "  powershell -ExecutionPolicy Bypass -File uninstall.ps1" -ForegroundColor Yellow
    exit 1
}

# Check if installation exists
if (-not (Test-Path $InstallPath)) {
    Write-Host "DataPainter installation not found at: $InstallPath" -ForegroundColor Yellow
    Write-Host "Already uninstalled?" -ForegroundColor Yellow
    exit 0
}

Write-Host "Uninstalling DataPainter from: $InstallPath" -ForegroundColor Green

# Remove from system PATH
Write-Host "Removing from system PATH..." -ForegroundColor Green
$currentPath = [Environment]::GetEnvironmentVariable("Path", "Machine")

if ($currentPath -like "*$InstallPath*") {
    $pathArray = $currentPath -split ';' | Where-Object { $_ -ne $InstallPath }
    $newPath = $pathArray -join ';'
    [Environment]::SetEnvironmentVariable("Path", $newPath, "Machine")
    Write-Host "Removed $InstallPath from system PATH" -ForegroundColor Green
} else {
    Write-Host "$InstallPath was not in system PATH" -ForegroundColor Yellow
}

# Remove installation directory
Write-Host "Removing installation directory..." -ForegroundColor Green
Remove-Item -Path $InstallPath -Recurse -Force
Write-Host "Removed $InstallPath" -ForegroundColor Green

Write-Host ""
Write-Host "Uninstallation complete!" -ForegroundColor Green
Write-Host "DataPainter has been removed from your system." -ForegroundColor Green
Write-Host ""
