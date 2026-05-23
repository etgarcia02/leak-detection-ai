param(
    [string]$InputDir = "C:\Users\smnbn\OneDrive\Documents\New project\docs\raw_serial_logs",
    [string]$OutputDir = "C:\Users\smnbn\OneDrive\Documents\New project\docs\parsed_serial_logs",
    [double]$SaturationThreshold = 32000.0,
    [double]$LowSignalThreshold = 100.0,
    [double]$LowSignalProbThreshold = 0.99
)

$ErrorActionPreference = "Stop"

$serialFields = @(
    "SoundLevel",
    "PeakFreq",
    "SpectralCenter",
    "ZeroCross",
    "LeakProb",
    "AudioGate",
    "ModelGate",
    "VibrationGate",
    "CandidateLeak",
    "LeakCount",
    "FinalLeak",
    "LED"
)

function Get-RawLabel {
    param([string]$Stem)

    $name = $Stem.ToLowerInvariant()
    if ($name -like "*no_leak*") { return "no_leak" }
    if ($name -like "*small_leak*") { return "small_leak" }
    if ($name -like "*medium_leak*") { return "medium_leak" }
    if ($name -like "*large_leak*") { return "large_leak" }
    if ($name -like "*leak*") { return "leak" }
    return "unknown"
}

function Get-SessionGroup {
    param([string]$Stem)

    $match = [regex]::Match($Stem, "session(?<num>\d+)")
    if ($match.Success) {
        return "session_$($match.Groups['num'].Value)"
    }
    return "unknown_session"
}

function Get-InvalidReason {
    param(
        [double]$SoundLevel,
        [double]$PeakFreq,
        [double]$SpectralCenter,
        [double]$LeakProb,
        [double]$SaturationThreshold,
        [double]$LowSignalThreshold,
        [double]$LowSignalProbThreshold
    )

    $reasons = New-Object System.Collections.Generic.List[string]
    if ($SoundLevel -le 0) { $reasons.Add("sound_level_zero") }
    if ($PeakFreq -eq 0 -and $SpectralCenter -eq 0) { $reasons.Add("empty_spectrum") }
    if ($SoundLevel -ge $SaturationThreshold) { $reasons.Add("saturated") }
    if ($SoundLevel -lt $LowSignalThreshold -and $LeakProb -ge $LowSignalProbThreshold) {
        $reasons.Add("low_signal_high_prob")
    }
    return ($reasons -join ";")
}

function Get-MeanValue {
    param(
        [object[]]$Rows,
        [string]$Property
    )

    if (-not $Rows -or $Rows.Count -eq 0) { return 0.0 }
    return (($Rows | Measure-Object -Property $Property -Average).Average)
}

function Get-MaxValue {
    param(
        [object[]]$Rows,
        [string]$Property
    )

    if (-not $Rows -or $Rows.Count -eq 0) { return 0.0 }
    return (($Rows | Measure-Object -Property $Property -Maximum).Maximum)
}

New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null

$inputFiles = Get-ChildItem -Path $InputDir -File | Where-Object { $_.Extension -in @(".txt", ".log") }
if (-not $inputFiles) {
    throw "No .txt or .log files found in $InputDir"
}

$combinedRaw = New-Object System.Collections.Generic.List[object]
$combinedClean = New-Object System.Collections.Generic.List[object]
$summaryRows = New-Object System.Collections.Generic.List[object]
$rowsBySession = @{}
$cleanBySession = @{}

foreach ($file in $inputFiles | Sort-Object Name) {
    $rawLabel = Get-RawLabel -Stem $file.BaseName
    $sessionGroup = Get-SessionGroup -Stem $file.BaseName
    $parsedRows = New-Object System.Collections.Generic.List[object]
    $cleanRows = New-Object System.Collections.Generic.List[object]
    $skippedNonDataLines = 0
    $rowIndex = 0
    $lineNumber = 0

    foreach ($line in Get-Content -Path $file.FullName) {
        $lineNumber += 1
        if ($line -notmatch "SoundLevel:") {
            $skippedNonDataLines += 1
            continue
        }

        $matches = [regex]::Matches($line, "([A-Za-z]+):([^\t\r\n ]+)")
        if ($matches.Count -eq 0) {
            $skippedNonDataLines += 1
            continue
        }

        $values = @{}
        foreach ($match in $matches) {
            $values[$match.Groups[1].Value] = $match.Groups[2].Value
        }

        $missingField = $false
        foreach ($field in $serialFields) {
            if (-not $values.ContainsKey($field)) {
                $missingField = $true
                break
            }
        }
        if ($missingField) {
            $skippedNonDataLines += 1
            continue
        }

        $rowIndex += 1
        $soundLevel = [double]$values["SoundLevel"]
        $peakFreq = [double]$values["PeakFreq"]
        $spectralCenter = [double]$values["SpectralCenter"]
        $zeroCross = [double]$values["ZeroCross"]
        $leakProb = [double]$values["LeakProb"]
        $audioGate = [int][double]$values["AudioGate"]
        $modelGate = [int][double]$values["ModelGate"]
        $vibrationGate = [int][double]$values["VibrationGate"]
        $candidateLeak = [int][double]$values["CandidateLeak"]
        $leakCount = [int][double]$values["LeakCount"]
        $finalLeak = [int][double]$values["FinalLeak"]
        $led = [int][double]$values["LED"]

        $invalidReason = Get-InvalidReason `
            -SoundLevel $soundLevel `
            -PeakFreq $peakFreq `
            -SpectralCenter $spectralCenter `
            -LeakProb $leakProb `
            -SaturationThreshold $SaturationThreshold `
            -LowSignalThreshold $LowSignalThreshold `
            -LowSignalProbThreshold $LowSignalProbThreshold

        $isValid = [int]([string]::IsNullOrEmpty($invalidReason))

        $rowObject = [pscustomobject]@{
            source_file = $file.Name
            session_id = $file.BaseName
            session_group = $sessionGroup
            raw_label = $rawLabel
            row_index = $rowIndex
            line_number = $lineNumber
            sound_level = $soundLevel
            peak_freq_hz = $peakFreq
            spectral_center_hz = $spectralCenter
            zero_crossing_rate = $zeroCross
            leak_prob = $leakProb
            audio_gate = $audioGate
            model_gate = $modelGate
            vibration_gate = $vibrationGate
            candidate_leak = $candidateLeak
            leak_count = $leakCount
            final_leak = $finalLeak
            led = $led
            is_valid = $isValid
            invalid_reason = $invalidReason
        }

        $parsedRows.Add($rowObject)
        $combinedRaw.Add($rowObject)

        if ($isValid -eq 1) {
            $cleanRows.Add($rowObject)
            $combinedClean.Add($rowObject)
        }
    }

    $rowsPath = Join-Path $OutputDir "$($file.BaseName)_rows.csv"
    $cleanPath = Join-Path $OutputDir "$($file.BaseName)_clean.csv"
    $parsedRows | Export-Csv -Path $rowsPath -NoTypeInformation
    $cleanRows | Export-Csv -Path $cleanPath -NoTypeInformation

    if (-not $rowsBySession.ContainsKey($sessionGroup)) {
        $rowsBySession[$sessionGroup] = New-Object System.Collections.Generic.List[object]
        $cleanBySession[$sessionGroup] = New-Object System.Collections.Generic.List[object]
    }
    foreach ($row in $parsedRows) { $rowsBySession[$sessionGroup].Add($row) }
    foreach ($row in $cleanRows) { $cleanBySession[$sessionGroup].Add($row) }

    $summaryRows.Add([pscustomobject]@{
        source_file = $file.Name
        session_id = $file.BaseName
        session_group = $sessionGroup
        raw_label = $rawLabel
        parsed_rows = $parsedRows.Count
        valid_rows = $cleanRows.Count
        invalid_rows = ($parsedRows.Count - $cleanRows.Count)
        skipped_non_data_lines = $skippedNonDataLines
        mean_sound_level = [math]::Round((Get-MeanValue -Rows $cleanRows -Property "sound_level"), 6)
        mean_peak_freq_hz = [math]::Round((Get-MeanValue -Rows $cleanRows -Property "peak_freq_hz"), 6)
        mean_spectral_center_hz = [math]::Round((Get-MeanValue -Rows $cleanRows -Property "spectral_center_hz"), 6)
        mean_zero_crossing_rate = [math]::Round((Get-MeanValue -Rows $cleanRows -Property "zero_crossing_rate"), 6)
        mean_leak_prob = [math]::Round((Get-MeanValue -Rows $cleanRows -Property "leak_prob"), 6)
        max_leak_prob = [math]::Round((Get-MaxValue -Rows $cleanRows -Property "leak_prob"), 6)
        model_gate_count = (($cleanRows | Where-Object { $_.model_gate -eq 1 }).Count)
        candidate_leak_count = (($cleanRows | Where-Object { $_.candidate_leak -eq 1 }).Count)
        max_leak_count = [int](Get-MaxValue -Rows $cleanRows -Property "leak_count")
        max_final_leak = [int](Get-MaxValue -Rows $cleanRows -Property "final_leak")
        high_prob_rows_ge_0_90 = (($cleanRows | Where-Object { $_.leak_prob -ge 0.90 }).Count)
    })
}

foreach ($sessionGroup in $rowsBySession.Keys | Sort-Object) {
    $sessionRowsPath = Join-Path $OutputDir "$sessionGroup`_organized.csv"
    $sessionCleanPath = Join-Path $OutputDir "$sessionGroup`_clean.csv"
    $rowsBySession[$sessionGroup] | Sort-Object source_file, row_index | Export-Csv -Path $sessionRowsPath -NoTypeInformation
    $cleanBySession[$sessionGroup] | Sort-Object source_file, row_index | Export-Csv -Path $sessionCleanPath -NoTypeInformation
}

$summaryPath = Join-Path $OutputDir "serial_log_summary.csv"
$combinedRowsPath = Join-Path $OutputDir "serial_log_rows_combined.csv"
$combinedCleanPath = Join-Path $OutputDir "serial_log_clean_combined.csv"

$summaryRows | Sort-Object session_group, raw_label | Export-Csv -Path $summaryPath -NoTypeInformation
$combinedRaw | Sort-Object session_group, source_file, row_index | Export-Csv -Path $combinedRowsPath -NoTypeInformation
$combinedClean | Sort-Object session_group, source_file, row_index | Export-Csv -Path $combinedCleanPath -NoTypeInformation

Write-Host "Parsed $($inputFiles.Count) file(s) from $InputDir"
Write-Host "Wrote outputs to $OutputDir"
