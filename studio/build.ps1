# DongArch3D Studio 증분 빌드 (MSVC + Ninja + OSGeo4W Qt6)
#   powershell -File studio/build.ps1                    # 빌드
#   powershell -File studio/build.ps1 -Test              # 빌드 + CTest 11종 전체 스위트 실행
#   powershell -File studio/build.ps1 -ShotDir <폴더>    # 5종 검수 화면 캡처
#   powershell -File studio/build.ps1 -Run               # 빌드 + 실행
param([switch]$Test, [switch]$Run, [switch]$Configure, [string]$Target = "", [string]$ShotDir = "")
$ErrorActionPreference = "Stop"
$src = $PSScriptRoot
$bld = Join-Path $src "build-msvc"
$qt = if ($env:QT_ROOT_DIR) { $env:QT_ROOT_DIR } else { "D:/OSGeo4W/apps/Qt6" }
$vcvars = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
$cmake = "C:\Program Files\CMake\bin\cmake.exe"
$ctest = "C:\Program Files\CMake\bin\ctest.exe"
$ninja = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe"

$cfg = ""
if ($Configure -or -not (Test-Path (Join-Path $bld "build.ninja"))) {
  $cfg = "`"$cmake`" -S `"$src`" -B `"$bld`" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_MAKE_PROGRAM=`"$ninja`" -DCMAKE_PREFIX_PATH=$qt && "
}
$tgt = if ($Target) { "--target $Target" } else { "" }
cmd /c "`"$vcvars`" >nul && $cfg `"$cmake`" --build `"$bld`" --parallel $tgt"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$env:PATH = "$qt\bin;D:\OSGeo4W\bin;$env:PATH"
$env:QT_PLUGIN_PATH = "$qt\plugins"
$exe = Join-Path $bld "DongArch3DStudio.exe"
if ($Test) {
  & $ctest --test-dir $bld --output-on-failure --parallel 4
  if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}
if ($ShotDir) {
  New-Item -ItemType Directory -Force -Path $ShotDir | Out-Null
  & $exe --shot (Join-Path $ShotDir "shot_home.png") | Out-Null
  & $exe --sample --shot (Join-Path $ShotDir "shot_main.png") | Out-Null
  & $exe --blind --shot (Join-Path $ShotDir "shot_blind.png") | Out-Null
  & $exe --sample --max --shot (Join-Path $ShotDir "shot_max.png") | Out-Null
  & $exe --sample --export --shot (Join-Path $ShotDir "shot_export.png") | Out-Null
}
if ($Run) { Start-Process $exe }
