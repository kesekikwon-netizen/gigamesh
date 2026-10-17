# factory/scripts/verify.ps1 — 표준 검증 절차 (configure → build → ctest → 캡처)
# 사용:
#   pwsh factory/scripts/verify.ps1                       # Studio (기본)
#   pwsh factory/scripts/verify.ps1 -Target gigamesh      # GigaMesh Qt5 (prepare-gigamesh.ps1 선행, 약 7분)
#   pwsh factory/scripts/verify.ps1 -Job JOB-001          # 로그·캡처를 factory/jobs/JOB-001/artifacts/ 로 복사
# 종료 코드: 0 = 전부 통과, 그 외 = 처음 실패한 단계의 종료 코드. 요약은 <BuildRoot>/<Target>/verify-summary.md 에 남는다.
# 원칙: 빌드 폴더는 짧은 경로(MSBuild FTK1011 회피), ctest 는 빌드 폴더로 cd 한 뒤 실행, 저장소 안에는 산출물을 만들지 않는다.
param(
    [ValidateSet('studio', 'gigamesh')] [string]$Target = 'studio',
    [string]$Job = '',
    [string]$Root = '',
    [string]$RecordRoot = '',   # -Job 산출물을 복사할 저장소 루트(기본 = Root). 검증 대상 worktree 를 읽기 전용으로 두려면 기준 worktree 경로를 지정
    [string]$BuildRoot = 'D:\_dongarch_build\verify',
    [string]$Qt6Dir = '',
    [string]$Qt5Dir = '',
    [switch]$NoShots,
    [switch]$Clean
)
$ErrorActionPreference = 'Continue'
if (-not $Root) { $Root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path }
if (-not $Qt6Dir) { $Qt6Dir = if ($env:QT_ROOT_DIR) { $env:QT_ROOT_DIR } else { 'D:/OSGeo4W/apps/Qt6' } }
if (-not $Qt5Dir) { $Qt5Dir = if ($env:QT5_ROOT_DIR) { $env:QT5_ROOT_DIR } else { 'C:/Qt/5.15.2/msvc2019_64' } }
$vcvars = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat'
if (-not (Test-Path $vcvars)) { $vcvars = 'C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat' }
$ninja = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe'
if (-not (Test-Path $ninja)) { $c = Get-Command ninja -ErrorAction SilentlyContinue; if ($c) { $ninja = $c.Source } }

$bld = Join-Path $BuildRoot $Target
$log = Join-Path $BuildRoot "$Target-logs"
if ($Clean -and (Test-Path $bld)) { Remove-Item -Recurse -Force $bld }
New-Item -ItemType Directory -Force -Path $bld, $log | Out-Null
$steps = @()
$overall = 0
function Step([string]$name, [string]$cmd, [string]$logFile) {
    $sw = [Diagnostics.Stopwatch]::StartNew()
    cmd /c "$cmd > `"$logFile`" 2>&1"
    $code = $LASTEXITCODE
    $sw.Stop()
    $script:steps += [pscustomobject]@{ step = $name; exit = $code; seconds = [int]$sw.Elapsed.TotalSeconds; log = $logFile; command = $cmd }
    Write-Host ("[{0}] exit={1} ({2}s) {3}" -f $name, $code, [int]$sw.Elapsed.TotalSeconds, $logFile)
    if ($code -ne 0 -and $script:overall -eq 0) { $script:overall = $code }
    return $code
}

if ($Target -eq 'studio') {
    $src = Join-Path $Root 'studio'
    $cfg = "`"$vcvars`" >nul && cmake -S `"$src`" -B `"$bld`" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_MAKE_PROGRAM=`"$ninja`" -DCMAKE_PREFIX_PATH=$Qt6Dir"
    if ((Step 'configure' $cfg (Join-Path $log '01_configure.log')) -eq 0) {
        if ((Step 'build' "`"$vcvars`" >nul && cmake --build `"$bld`" --parallel" (Join-Path $log '02_build.log')) -eq 0) {
            $env:PATH = "$($Qt6Dir -replace '/', '\')\bin;$env:PATH"
            $env:QT_PLUGIN_PATH = "$($Qt6Dir -replace '/', '\')\plugins"
            $env:QT_QPA_PLATFORM = 'offscreen'
            Step 'ctest' "cd /d `"$bld`" && ctest --output-on-failure" (Join-Path $log '03_ctest.log') | Out-Null
            if (-not $NoShots) {
                $exe = Join-Path $bld 'DongArch3DStudio.exe'
                $shots = Join-Path $bld 'shots'; New-Item -ItemType Directory -Force -Path $shots | Out-Null
                Step 'shots' "cd /d `"$bld`" && `"$exe`" --shot `"$shots\shot_home.png`" && `"$exe`" --sample --shot `"$shots\shot_main.png`" && `"$exe`" --sample --export --shot `"$shots\shot_export.png`"" (Join-Path $log '04_shots.log') | Out-Null
            }
        }
    }
} else {
    $src = Join-Path $Root 'GigaMesh'
    $psExe = if (Get-Command pwsh -ErrorAction SilentlyContinue) { 'pwsh' } else { 'powershell' }   # 이 PC 는 5.1 만 있음, CI 러너는 pwsh
    Step 'prepare' "$psExe -NoProfile -ExecutionPolicy Bypass -File `"$PSScriptRoot\prepare-gigamesh.ps1`" -Root `"$src`" -QtBin `"$($Qt5Dir -replace '/', '\')\bin`"" (Join-Path $log '00_prepare.log') | Out-Null
    $cfg = "cmake -S `"$src`" -B `"$bld`" -G `"Visual Studio 17 2022`" -A x64 -DCMAKE_PREFIX_PATH=$Qt5Dir -DCMAKE_CXX_FLAGS=`"/DWIN32 /D_WINDOWS /EHsc /utf-8`" -DENABLE_TESTS=ON"
    if ((Step 'configure' $cfg (Join-Path $log '01_configure.log')) -eq 0) {
        if ((Step 'build' "cmake --build `"$bld`" --config Release --parallel 16" (Join-Path $log '02_build.log')) -eq 0) {
            Step 'ctest' "cd /d `"$bld`" && ctest -C Release --output-on-failure" (Join-Path $log '03_ctest.log') | Out-Null
        }
    }
}

# 요약
$summary = @("# verify 요약 — $Target ($(Get-Date -Format 'yyyy-MM-dd HH:mm:ss'))", "", "| 단계 | 종료 코드 | 초 | 로그 |", "|---|---|---|---|")
foreach ($s in $steps) { $summary += "| $($s.step) | $($s.exit) | $($s.seconds) | $($s.log) |" }
$summary += ""; $summary += "소스: $Root"; $summary += "빌드 폴더: $bld"; $summary += "결과: " + $(if ($overall -eq 0) { 'PASS' } else { "FAIL (exit $overall)" })
$summaryPath = Join-Path $BuildRoot "$Target\verify-summary.md"
$summary -join "`n" | Set-Content -Path $summaryPath -Encoding utf8
Write-Host ($summary -join "`n")

if ($Job) {
    $rec = if ($RecordRoot) { $RecordRoot } else { $Root }
    $art = Join-Path $rec "factory\jobs\$Job\artifacts"
    New-Item -ItemType Directory -Force -Path $art | Out-Null
    Copy-Item (Join-Path $log '*') $art -Force
    Copy-Item $summaryPath (Join-Path $art "verify-summary-$Target.md") -Force
    if (Test-Path (Join-Path $bld 'shots')) { Copy-Item (Join-Path $bld 'shots\*.png') $art -Force -ErrorAction SilentlyContinue }
    Write-Host "산출물 복사: $art"
}
exit $overall
