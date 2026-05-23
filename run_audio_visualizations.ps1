$candidates = @(
    "python",
    "py",
    "C:\Users\smnbn\AppData\Local\Programs\Python\Python312\python.exe",
    "C:\Users\smnbn\AppData\Local\Programs\Python\Python311\python.exe"
)

$python = $null
foreach ($candidate in $candidates) {
    try {
        if ($candidate -in @("python", "py")) {
            $command = Get-Command $candidate -ErrorAction Stop
            $python = $command.Source
            break
        }

        if (Test-Path $candidate) {
            $python = $candidate
            break
        }
    } catch {
    }
}

if (-not $python) {
    Write-Error "No usable Python interpreter was found."
    exit 1
}

& $python "C:\Users\smnbn\OneDrive\Documents\New project\ml\scripts\visualize_audio.py"
