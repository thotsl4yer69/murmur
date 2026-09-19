param(
    [Parameter(Mandatory=$true)][ValidateNotNullOrEmpty()][string]$Port,
    [Parameter(Mandatory=$true)][ValidateSet(1,2,3,4)][int]$Profile
)
$ErrorActionPreference='Stop'
Set-Location (Split-Path $PSScriptRoot -Parent)
if (-not (Get-Command arduino-cli -ErrorAction SilentlyContinue)) { throw 'Arduino CLI is required.' }
$directory="build/profile-$Profile"
if (-not (Test-Path "$directory/MURMUR_C3.ino.bin")) { throw "No compiled profile $Profile application exists. Run tools/build.ps1 first." }
$fqbn='esp32:esp32:esp32c3:CDCOnBoot=cdc,PartitionScheme=huge_app,FlashMode=dio,FlashSize=4M'
& arduino-cli upload --fqbn $fqbn --port $Port --input-dir $directory
if ($LASTEXITCODE -ne 0) { throw 'Upload failed. Check the port, USB cable and BOOT/RESET sequence.' }
Write-Host "Upload completed for profile $Profile. Reset the board and run SYSTEM > Screen + button test."
