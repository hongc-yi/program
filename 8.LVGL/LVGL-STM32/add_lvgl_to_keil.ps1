# Restructure: replace flat "Middlewares/LVGL" group with per-folder groups
# mirroring the LVGL source tree (core/draw/extra/font/hal/misc/widgets + porting).
# Usage: right-click -> Run with PowerShell, or:  powershell -ExecutionPolicy Bypass -File add_lvgl_to_keil.ps1
$ErrorActionPreference = 'Stop'

$projFile = 'D:\learn\program\8.LVGL\LVGL-STM32\MDK-ARM\FreeRTOS.uvprojx'
$lvglRoot = 'D:\learn\program\8.LVGL\LVGL-STM32\Middlewares\LVGL'

# 1) timestamped backup
$bak = "$projFile.bak-" + (Get-Date -Format 'yyyyMMddHHmmss')
Copy-Item $projFile $bak -Force
Write-Host "backup -> $bak"

function Get-RelPath([string]$fromDir, [string]$toPath) {
    $from = [System.IO.Path]::GetFullPath($fromDir).TrimEnd('\') + '\'
    $to   = [System.IO.Path]::GetFullPath($toPath)
    if ($to.StartsWith($from, [System.StringComparison]::OrdinalIgnoreCase)) {
        return $to.Substring($from.Length)
    }
    $fromParts = $from.TrimEnd('\').Split('\')
    $toParts   = $to.Split('\')
    $i = 0
    while ($i -lt $fromParts.Count -and $i -lt $toParts.Count -and $fromParts[$i] -eq $toParts[$i]) { $i++ }
    $ups = $fromParts.Count - $i
    $rel = (@('..') * $ups) + $toParts[$i..($toParts.Count - 1)]
    return ($rel -join '\')
}

function Build-GroupBlock([string]$groupName, [System.IO.FileInfo[]]$files) {
    $sb = New-Object System.Text.StringBuilder
    [void]$sb.AppendLine('        <Group>')
    [void]$sb.AppendLine("          <GroupName>$groupName</GroupName>")
    [void]$sb.AppendLine('          <Files>')
    foreach ($f in $files) {
        $r = (Get-RelPath (Split-Path $projFile) $f.FullName).Replace('\', '/')
        [void]$sb.AppendLine('            <File>')
        [void]$sb.AppendLine("              <FileName>$($f.Name)</FileName>")
        [void]$sb.AppendLine('              <FileType>1</FileType>')
        [void]$sb.AppendLine("              <FilePath>$r</FilePath>")
        [void]$sb.AppendLine('            </File>')
    }
    [void]$sb.AppendLine('          </Files>')
    [void]$sb.AppendLine('        </Group>')
    return $sb.ToString()
}

# 2) read project lines
$lines = New-Object System.Collections.ArrayList
foreach ($l in [System.IO.File]::ReadAllLines($projFile)) { [void]$lines.Add($l) }

# 3) remove any existing groups whose GroupName starts with "Middlewares/LVGL"
$removeRanges = New-Object System.Collections.ArrayList
for ($i = 0; $i -lt $lines.Count; $i++) {
    if ($lines[$i].Trim() -eq '<Group>') {
        $name = $null
        for ($j = $i + 1; $j -lt $lines.Count; $j++) {
            $t = $lines[$j].Trim()
            if ($t -like '<GroupName>*') {
                $name = $t -replace '^<GroupName>(.*)</GroupName>$', '$1'
                break
            }
            if ($t -eq '<Group>') { break }   # nested group before name: skip outer
        }
        if ($name -and $name.StartsWith('Middlewares/LVGL')) {
            # find closing </Group> (flat groups: first one after start)
            for ($k = $i; $k -lt $lines.Count; $k++) {
                if ($k -gt $i -and $lines[$k].Trim() -eq '</Group>') { break }
            }
            [void]$removeRanges.Add(@($i, $k))
            $i = $k
        }
    }
}
# remove from bottom to top
$removed = 0
for ($r = $removeRanges.Count - 1; $r -ge 0; $r--) {
    $start = $removeRanges[$r][0]; $end = $removeRanges[$r][1]
    $lines.RemoveRange($start, $end - $start + 1)
    $removed += ($end - $start + 1)
}
Write-Host "removed $($removeRanges.Count) old group(s) ($removed lines)"

# 4) build new per-folder groups
$folders = @(
    @{ Name = 'porting'; Path = "$lvglRoot\porting" },
    @{ Name = 'core';    Path = "$lvglRoot\src\core" },
    @{ Name = 'draw';    Path = "$lvglRoot\src\draw" },
    @{ Name = 'extra';   Path = "$lvglRoot\src\extra" },
    @{ Name = 'font';    Path = "$lvglRoot\src\font" },
    @{ Name = 'hal';     Path = "$lvglRoot\src\hal" },
    @{ Name = 'misc';    Path = "$lvglRoot\src\misc" },
    @{ Name = 'widgets'; Path = "$lvglRoot\src\widgets" }
)
$blocks = New-Object System.Collections.ArrayList
$total = 0
foreach ($f in $folders) {
    $files = @(Get-ChildItem $f.Path -Recurse -Filter *.c -ErrorAction Stop | Sort-Object FullName)
    if ($files.Count -eq 0) { Write-Host "skip $($f.Name) (no .c files)"; continue }
    [void]$blocks.Add((Build-GroupBlock "Middlewares/LVGL/$($f.Name)" $files))
    $total += $files.Count
    Write-Host "group Middlewares/LVGL/$($f.Name): $($files.Count) files"
}
Write-Host "total $total files"

# 5) locate ::CMSIS group and insert before it
$idx = -1
for ($i = 0; $i -lt $lines.Count - 1; $i++) {
    if ($lines[$i].Trim() -eq '<GroupName>::CMSIS</GroupName>') {
        $j = $i
        while ($j -ge 0 -and $lines[$j].Trim() -ne '<Group>') { $j-- }
        $idx = $j
        break
    }
}
if ($idx -lt 0) { throw 'could not locate ::CMSIS group in project file' }

for ($b = $blocks.Count - 1; $b -ge 0; $b--) {
    $lines.Insert($idx, $blocks[$b])
}

# 6) save as UTF-8 without BOM
$enc = New-Object System.Text.UTF8Encoding($false)
[System.IO.File]::WriteAllLines($projFile, $lines, $enc)

Write-Host 'OK: project restructured.'
