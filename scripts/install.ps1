# DataPainter Windows Installer
# Usage: powershell -ExecutionPolicy Bypass -File install.ps1

param(
    [string]$InstallPath = "$env:ProgramFiles\DataPainter"
)

$ErrorActionPreference = "Stop"

Write-Host "DataPainter Installer" -ForegroundColor Cyan
Write-Host "=====================" -ForegroundColor Cyan
Write-Host ""

# Check if running as administrator
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if (-not $isAdmin) {
    Write-Host "This installer requires administrator privileges." -ForegroundColor Red
    Write-Host "Please run PowerShell as Administrator and try again." -ForegroundColor Red
    Write-Host ""
    Write-Host "Right-click PowerShell and select 'Run as Administrator', then run:" -ForegroundColor Yellow
    Write-Host "  powershell -ExecutionPolicy Bypass -File install.ps1" -ForegroundColor Yellow
    exit 1
}

# Get the directory where this script is located
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path

# Check if datapainter.exe exists in the script directory
$ExePath = Join-Path $ScriptDir "datapainter.exe"
if (-not (Test-Path $ExePath)) {
    Write-Host "Error: datapainter.exe not found in $ScriptDir" -ForegroundColor Red
    Write-Host "Please make sure datapainter.exe is in the same directory as this script." -ForegroundColor Red
    exit 1
}

Write-Host "Installing DataPainter to: $InstallPath" -ForegroundColor Green

# Create installation directory
if (-not (Test-Path $InstallPath)) {
    New-Item -ItemType Directory -Path $InstallPath -Force | Out-Null
    Write-Host "Created installation directory" -ForegroundColor Green
} else {
    Write-Host "Installation directory already exists" -ForegroundColor Yellow
}

# Copy binary
Write-Host "Copying datapainter.exe..." -ForegroundColor Green
Copy-Item -Path $ExePath -Destination (Join-Path $InstallPath "datapainter.exe") -Force

# Copy README and LICENSE if they exist
$ReadmePath = Join-Path $ScriptDir "README.md"
if (Test-Path $ReadmePath) {
    Copy-Item -Path $ReadmePath -Destination (Join-Path $InstallPath "README.md") -Force
    Write-Host "Copied README.md" -ForegroundColor Green
}

$LicensePath = Join-Path $ScriptDir "LICENSE"
if (Test-Path $LicensePath) {
    Copy-Item -Path $LicensePath -Destination (Join-Path $InstallPath "LICENSE") -Force
    Write-Host "Copied LICENSE" -ForegroundColor Green
}

# Add to system PATH
Write-Host "Adding to system PATH..." -ForegroundColor Green
$currentPath = [Environment]::GetEnvironmentVariable("Path", "Machine")

if ($currentPath -notlike "*$InstallPath*") {
    $newPath = $currentPath + ";" + $InstallPath
    [Environment]::SetEnvironmentVariable("Path", $newPath, "Machine")
    Write-Host "Added $InstallPath to system PATH" -ForegroundColor Green
    Write-Host "You may need to restart your terminal for PATH changes to take effect" -ForegroundColor Yellow
} else {
    Write-Host "$InstallPath is already in system PATH" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "Installation complete!" -ForegroundColor Green
Write-Host ""
Write-Host "DataPainter has been installed to:" -ForegroundColor Cyan
Write-Host "  $InstallPath" -ForegroundColor White
Write-Host ""
Write-Host "To run DataPainter, open a new terminal and type:" -ForegroundColor Cyan
Write-Host "  datapainter" -ForegroundColor White
Write-Host ""
Write-Host "To uninstall, run:" -ForegroundColor Cyan
Write-Host "  powershell -ExecutionPolicy Bypass -File uninstall.ps1" -ForegroundColor White
Write-Host ""
