param([ValidateSet(1,2,3,4)][int[]]$Profiles=@(1,2,3,4))
$ErrorActionPreference='Stop'
Set-Location (Split-Path $PSScriptRoot -Parent)
if (-not (Get-Command arduino-cli -ErrorAction SilentlyContinue)) { throw 'Install Arduino CLI, then run this script again.' }
function Invoke-Cli([string[]]$Arguments) {
    & arduino-cli @Arguments
    if ($LASTEXITCODE -ne 0) { throw "Arduino CLI failed: $($Arguments -join ' ')" }
}
$index='https://espressif.github.io/arduino-esp32/package_esp32_index.json'
Invoke-Cli -Arguments @('core','update-index','--additional-urls',$index)
Invoke-Cli -Arguments @('core','install','esp32:esp32@3.3.12','--additional-urls',$index)
Invoke-Cli -Arguments @('lib','update-index')
Invoke-Cli -Arguments @('lib','install','Adafruit GFX Library@1.12.6','Adafruit ST7735 and ST7789 Library@1.11.0','NimBLE-Arduino@2.5.1','RadioLib@7.7.1','U8g2@2.37.1')
$fqbn='esp32:esp32:esp32c3:CDCOnBoot=cdc,PartitionScheme=huge_app,FlashMode=dio,FlashSize=4M'
foreach ($profile in $Profiles) {
    New-Item -ItemType Directory -Force "build/profile-$profile" | Out-Null
    Invoke-Cli -Arguments @('compile','--fqbn',$fqbn,'--warnings','all','--build-property',"compiler.cpp.extra_flags=-DMURMUR_PROFILE=$profile",'--output-dir',"build/profile-$profile",'MURMUR_C3')
}
Get-ChildItem build -Recurse -Filter *.bin | Get-FileHash -Algorithm SHA256
