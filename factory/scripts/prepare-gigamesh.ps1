# factory/scripts/prepare-gigamesh.ps1
# GigaMesh(Qt5) 빌드에 필요하지만 .gitignore 로 저장소에 없는 생성 파일 17개를 만든다.
# - gui/src/analyticsConfig.h   : 빈 키 스텁 (분석 전송 비활성 의도)
# - gui/languages/*.qm          : lrelease 로 .ts 에서 생성
# - gui/src/shaders/*.png       : qrc 가 참조하는데 없는 PNG 는 1x1 자리표시자
# - gui/latex_report_template/*.tex : qrc 가 참조하는데 없는 LaTeX 템플릿은 최소 문서
# 이미 있는 파일은 절대 덮어쓰지 않는다. 멱등.
# 사용: pwsh factory/scripts/prepare-gigamesh.ps1 [-Root <GigaMesh 폴더>] [-QtBin <Qt5 bin>]
param(
    [string]$Root = (Join-Path (Split-Path -Parent $PSScriptRoot) '..\GigaMesh'),
    [string]$QtBin = ''
)
$ErrorActionPreference = 'Stop'
$Root = (Resolve-Path $Root).Path
if (-not $QtBin) {
    if ($env:QT_ROOT_DIR -and (Test-Path (Join-Path $env:QT_ROOT_DIR 'bin\lrelease.exe'))) { $QtBin = Join-Path $env:QT_ROOT_DIR 'bin' }
    elseif (Test-Path 'C:\Qt\5.15.2\msvc2019_64\bin\lrelease.exe') { $QtBin = 'C:\Qt\5.15.2\msvc2019_64\bin' }
}
$made = @(); $kept = @(); $missing = @()

function Ensure-File([string]$path, [scriptblock]$writer) {
    if (Test-Path $path) { $script:kept += $path; return }
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $path) | Out-Null
    & $writer $path
    $script:made += $path
}

# 1) analyticsConfig.h
Ensure-File (Join-Path $Root 'gui\src\analyticsConfig.h') {
    param($p)
    "// factory/prepare-gigamesh.ps1 가 만든 스텁: 분석 키 없음 (git-ignored)`n#pragma once`n#define KEY `"`"`n" | Set-Content -Path $p -Encoding ascii -NoNewline
}

# 2) *.qm (lrelease)
$tsFiles = Get-ChildItem (Join-Path $Root 'gui\languages') -Filter *.ts
foreach ($ts in $tsFiles) {
    $qm = [IO.Path]::ChangeExtension($ts.FullName, '.qm')
    if (Test-Path $qm) { $kept += $qm; continue }
    if ($QtBin -and (Test-Path (Join-Path $QtBin 'lrelease.exe'))) {
        & (Join-Path $QtBin 'lrelease.exe') $ts.FullName -qm $qm | Out-Null
        if (Test-Path $qm) { $made += $qm } else { $missing += $qm }
    } else {
        $missing += "$qm (lrelease.exe 없음: -QtBin 지정 필요)"
    }
}

# 3) qrc 가 참조하는 PNG / TEX 중 없는 것
$png1x1 = [Convert]::FromBase64String('iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNkYPhfDwAChwGA60e6kgAAAABJRU5ErkJggg==')
$qrcs = @('gui\src\shaders\shaders.qrc', 'gui\latex_report_template\latex_templates.qrc', 'gui\resources.qrc', 'gui\forms\gigamesh.qrc')
foreach ($rel in $qrcs) {
    $qrc = Join-Path $Root $rel
    if (-not (Test-Path $qrc)) { continue }
    $dir = Split-Path -Parent $qrc
    [xml]$x = Get-Content $qrc
    foreach ($f in $x.RCC.qresource.file) {
        $name = if ($f -is [string]) { $f } else { $f.'#text' }
        if (-not $name) { continue }
        $target = Join-Path $dir $name
        if (Test-Path $target) { continue }
        switch ([IO.Path]::GetExtension($name).ToLower()) {
            '.png' { Ensure-File $target { param($p) [IO.File]::WriteAllBytes($p, $png1x1) } }
            '.tex' { Ensure-File $target { param($p) "% factory/prepare-gigamesh.ps1 가 만든 최소 LaTeX 템플릿 스텁`n\documentclass{article}`n\begin{document}`nstub`n\end{document}`n" | Set-Content -Path $p -Encoding ascii -NoNewline } }
            '.qm'  { }   # 위에서 처리
            default { $missing += "$target (qrc 참조, 자동 생성 규칙 없음)" }
        }
    }
}

Write-Host "=== prepare-gigamesh: 생성 $($made.Count), 기존 유지 $($kept.Count), 미해결 $($missing.Count) ==="
$made | ForEach-Object { Write-Host "  + $_" }
$missing | ForEach-Object { Write-Host "  ! $_" }
if ($missing.Count -gt 0) { exit 2 } else { exit 0 }
