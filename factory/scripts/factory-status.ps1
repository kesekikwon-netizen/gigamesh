# factory/scripts/factory-status.ps1 — 작업 상태표 + 상태 전이 검사
# 사용: pwsh factory/scripts/factory-status.ps1 [-Root <저장소 루트>] [-Json]
# 종료 코드: 0 정상, 3 규칙 위반(허용되지 않은 전이 / 동시 build 2개 이상 / approved 인데 승인 기록 없음)
param([string]$Root = '', [switch]$Json)
$ErrorActionPreference = 'Stop'
if (-not $Root) { $Root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path }
$jobsDir = Join-Path $Root 'factory\jobs'
$allowed = @{
    ''            = @('queued')
    'queued'      = @('spec', 'dropped')
    'spec'        = @('build', 'queued', 'dropped')
    'build'       = @('verify', 'needs-human')
    'verify'      = @('review', 'rework', 'needs-human')
    'review'      = @('approved', 'rework', 'needs-human')
    'rework'      = @('build', 'needs-human', 'dropped')
    'needs-human' = @('rework', 'approved', 'dropped')
    'approved'    = @('merged')
    'merged'      = @()
    'dropped'     = @()
}
$rows = @(); $violations = @()
Get-ChildItem $jobsDir -Directory | Where-Object { $_.Name -match '^JOB-\d+$' } | Sort-Object Name | ForEach-Object {
    $p = Join-Path $_.FullName 'job.json'
    if (-not (Test-Path $p)) { $violations += "$($_.Name): job.json 없음"; return }
    $j = Get-Content $p -Raw -Encoding utf8 | ConvertFrom-Json
    $prev = ''
    foreach ($h in @($j.history)) {
        $from = if ($null -eq $h.from) { '' } else { [string]$h.from }
        $to = [string]$h.to
        if ($from -ne $prev) { $violations += "$($j.id): history 불연속 ($prev → $from)" }
        if (-not ($allowed.ContainsKey($from)) -or -not ($allowed[$from] -contains $to)) { $violations += "$($j.id): 허용되지 않은 전이 '$from' → '$to'" }
        $prev = $to
    }
    if ($prev -ne [string]$j.status) { $violations += "$($j.id): status '$($j.status)' 가 history 마지막 '$prev' 과 다름" }
    if (@('approved', 'merged') -contains $j.status -and @($j.approvals).Count -eq 0) { $violations += "$($j.id): approved/merged 인데 approvals 가 비어 있음 (사용자 승인 기록 필수)" }
    $wait = switch ($j.status) {
        'spec'        { '사용자: 명세 확인 후 /factory next' }
        'review'      { '사용자 승인 대기: /factory approve ' + $j.id }
        'needs-human' { '사용자 결정 대기: /factory rework|approve ' + $j.id }
        'build'       { 'builder 진행 중' }
        'verify'      { 'test-runner 진행 중' }
        'rework'      { '재작업 대기 (/factory next)' }
        'queued'      { '명세 대기 (/factory next)' }
        default       { '' }
    }
    $rows += [pscustomobject]@{ id = $j.id; title = $j.title; status = $j.status; branch = $j.branch; updated = $j.updated; approvals = @($j.approvals).Count; waiting = $wait }
}
$active = @($rows | Where-Object { @('build', 'verify', 'rework') -contains $_.status })   # review/needs-human 은 사용자 대기 상태라 동시 진행 수에 넣지 않음
if ($active.Count -gt 1) { $violations += "동시 진행 작업 $($active.Count)개 (허용 1): " + (($active | ForEach-Object { $_.id }) -join ', ') }

if ($Json) { [pscustomobject]@{ jobs = $rows; active = $active.Count; violations = $violations } | ConvertTo-Json -Depth 4; }
else {
    Write-Host "factory 작업 상태 ($(Get-Date -Format 'yyyy-MM-dd HH:mm'))  루트: $Root"
    if ($rows.Count -eq 0) { Write-Host "  (등록된 작업 없음)" } else { $rows | Format-Table id, status, title, approvals, waiting -AutoSize | Out-String | Write-Host }
    Write-Host "진행 중(build/verify/rework): $($active.Count) / 허용 1  (review·needs-human = 사용자 대기, 별도)"
    if ($violations.Count) { Write-Host "규칙 위반:"; $violations | ForEach-Object { Write-Host "  ! $_" } } else { Write-Host "규칙 위반 없음" }
}
if ($violations.Count) { exit 3 } else { exit 0 }
