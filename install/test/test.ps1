#!/usr/bin/env pwsh
#
# Windows PowerShell Test Script for ASTERIX Decoder
# Equivalent to test.sh for Unix platforms
#
# Usage:
#   .\test.ps1
#   .\test.ps1 -Verbose
#   .\test.ps1 -Configuration Debug
#

param(
    [ValidateSet('Release', 'Debug')]
    [string]$Configuration = 'Release',

    [switch]$Verbose
)

# Script configuration
$ErrorActionPreference = 'Continue'
$failedTests = 0
$totalTests = 0

# Paths (adjust for Windows)
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$installDir = Split-Path -Parent $scriptDir

if ($Configuration -eq 'Release') {
    $exe = Join-Path $installDir "bin\asterix.exe"
} else {
    $exe = Join-Path $installDir "bin\asterix.exe"  # Debug build path (adjust if different)
}

$config = Join-Path $installDir "config\asterix.ini"
$sampleDataDir = Join-Path $installDir "sample_data"
$sampleOutputDir = Join-Path $installDir "sample_output"

# Color output helpers
function Write-TestHeader {
    param([string]$Message)
    Write-Host "`n========================================" -ForegroundColor Cyan
    Write-Host "  $Message" -ForegroundColor Cyan
    Write-Host "========================================`n" -ForegroundColor Cyan
}

function Write-TestResult {
    param(
        [string]$TestName,
        [bool]$Success
    )

    $global:totalTests++

    if ($Success) {
        Write-Host "[OK]      " -ForegroundColor Green -NoNewline
        Write-Host $TestName
    } else {
        Write-Host "[FAILED]  " -ForegroundColor Red -NoNewline
        Write-Host $TestName
        $global:failedTests++
    }
}

# Test function: Compare command output with expected file
function Test-Compare {
    param(
        [string]$TestName,
        [string]$Command,
        [string]$ExpectedFile
    )

    try {
        # Execute command and capture output
        $result = Invoke-Expression $Command 2>&1 | Out-String

        # Read expected output
        if (Test-Path $ExpectedFile) {
            $expected = Get-Content $ExpectedFile -Raw

            # Normalize line endings (Windows vs Unix)
            $result = $result -replace "`r`n", "`n"
            $expected = $expected -replace "`r`n", "`n"

            # Compare
            $success = ($result.Trim() -eq $expected.Trim())
            Write-TestResult -TestName $TestName -Success $success

            if ($Verbose -and -not $success) {
                Write-Host "  Expected length: $($expected.Length) bytes" -ForegroundColor Yellow
                Write-Host "  Actual length:   $($result.Length) bytes" -ForegroundColor Yellow
            }
        } else {
            Write-TestResult -TestName "$TestName (Expected file not found: $ExpectedFile)" -Success $false
        }
    } catch {
        Write-TestResult -TestName "$TestName (Exception: $_)" -Success $false
    }
}

# Test function: Check command exit code
function Test-Output {
    param(
        [string]$TestName,
        [string]$Command,
        [int]$ExpectedExitCode = 0
    )

    try {
        # Execute command
        Invoke-Expression $Command > $null 2>&1
        $exitCode = $LASTEXITCODE

        $success = ($exitCode -eq $ExpectedExitCode)
        Write-TestResult -TestName $TestName -Success $success

        if ($Verbose -and -not $success) {
            Write-Host "  Expected exit code: $ExpectedExitCode" -ForegroundColor Yellow
            Write-Host "  Actual exit code:   $exitCode" -ForegroundColor Yellow
        }
    } catch {
        Write-TestResult -TestName "$TestName (Exception: $_)" -Success $false
    }
}

# ============================================================
# MAIN TEST EXECUTION
# ============================================================

Write-TestHeader "ASTERIX Decoder Test Suite (Windows)"

# Verify executable exists
if (-not (Test-Path $exe)) {
    Write-Host "ERROR: Executable not found at: $exe" -ForegroundColor Red
    Write-Host "Please build the project first." -ForegroundColor Red
    exit 1
}

Write-Host "Executable:   $exe" -ForegroundColor Cyan
Write-Host "Config:       $config" -ForegroundColor Cyan
Write-Host "Configuration: $Configuration`n" -ForegroundColor Cyan

# Verify config file exists
if (-not (Test-Path $config)) {
    Write-Host "WARNING: Config file not found: $config" -ForegroundColor Yellow
    Write-Host "Some tests may fail.`n" -ForegroundColor Yellow
}

# ============================================================
# Test 1: Basic Execution Tests
# ============================================================

Write-TestHeader "Basic Execution Tests"

Test-Output -TestName "Test version output" `
    -Command "& `"$exe`" --version" `
    -ExpectedExitCode 0

Test-Output -TestName "Test help output" `
    -Command "& `"$exe`" --help" `
    -ExpectedExitCode 0

if (Test-Path $config) {
    Test-Output -TestName "Test list filters" `
        -Command "& `"$exe`" -d `"$config`" -L" `
        -ExpectedExitCode 0
}

# ============================================================
# Test 2: File Parsing Tests (if sample files exist)
# ============================================================

if (Test-Path $sampleDataDir) {
    Write-TestHeader "File Parsing Tests"

    # Test PCAP parsing
    $pcapFile = Join-Path $sampleDataDir "cat_062_065.pcap"
    if (Test-Path $pcapFile) {
        Test-Output -TestName "Test PCAP parsing (cat_062_065)" `
            -Command "& `"$exe`" -P -d `"$config`" -j -f `"$pcapFile`"" `
            -ExpectedExitCode 0
    }

    # Test raw format parsing
    $rawFile = Join-Path $sampleDataDir "cat048.raw"
    if (Test-Path $rawFile) {
        Test-Output -TestName "Test raw format parsing (cat048)" `
            -Command "& `"$exe`" -d `"$config`" -j -f `"$rawFile`"" `
            -ExpectedExitCode 0
    }

    # Test GPS parsing
    $gpsFile = Join-Path $sampleDataDir "parsegps.gps"
    if (Test-Path $gpsFile) {
        Test-Output -TestName "Test GPS parsing" `
            -Command "& `"$exe`" -G -d `"$config`" -f `"$gpsFile`"" `
            -ExpectedExitCode 0
    }
} else {
    Write-Host "`nSkipping file parsing tests (sample data not found)" -ForegroundColor Yellow
}

# ============================================================
# Test 3: Output Format Tests (if expected outputs exist)
# ============================================================

if ((Test-Path $sampleDataDir) -and (Test-Path $sampleOutputDir)) {
    Write-TestHeader "Output Format Comparison Tests"

    # Test filter output
    $filterExpected = Join-Path $sampleOutputDir "filter.txt"
    if (Test-Path $filterExpected) {
        Test-Compare -TestName "Test filter output" `
            -Command "& `"$exe`" -d `"$config`" -L" `
            -ExpectedFile $filterExpected
    }

    # Test JSON output (cat_062_065)
    $pcapFile = Join-Path $sampleDataDir "cat_062_065.pcap"
    $jsonExpected = Join-Path $sampleOutputDir "cat_062_065_json.txt"
    if ((Test-Path $pcapFile) -and (Test-Path $jsonExpected)) {
        Test-Compare -TestName "Test JSON output (cat_062_065)" `
            -Command "& `"$exe`" -d `"$config`" -P -j -f `"$pcapFile`"" `
            -ExpectedFile $jsonExpected
    }

    # Test filtered output (cat_001_002)
    $pcapFile2 = Join-Path $sampleDataDir "cat_001_002.pcap"
    $filterFile = Join-Path $sampleOutputDir "filter.txt"
    $jsonExpected2 = Join-Path $sampleOutputDir "cat_001_002_json.txt"
    if ((Test-Path $pcapFile2) -and (Test-Path $filterFile) -and (Test-Path $jsonExpected2)) {
        Test-Compare -TestName "Test JSON filtered output (cat_001_002)" `
            -Command "& `"$exe`" -d `"$config`" -R -j -f `"$pcapFile2`" -LF `"$filterFile`"" `
            -ExpectedFile $jsonExpected2
    }

    # Test XML output
    $xmlExpected = Join-Path $sampleOutputDir "cat_001_002.xml"
    if ((Test-Path $pcapFile2) -and (Test-Path $filterFile) -and (Test-Path $xmlExpected)) {
        Test-Compare -TestName "Test XML output (cat_001_002)" `
            -Command "& `"$exe`" -d `"$config`" -R -x -f `"$pcapFile2`" -LF `"$filterFile`"" `
            -ExpectedFile $xmlExpected
    }

    # Test text output
    $txtExpected = Join-Path $sampleOutputDir "cat_001_002.txt"
    if ((Test-Path $pcapFile2) -and (Test-Path $filterFile) -and (Test-Path $txtExpected)) {
        Test-Compare -TestName "Test text output (cat_001_002)" `
            -Command "& `"$exe`" -d `"$config`" -R -f `"$pcapFile2`" -LF `"$filterFile`"" `
            -ExpectedFile $txtExpected
    }

    # Test line output
    $lineExpected = Join-Path $sampleOutputDir "cat_001_002_line.txt"
    if ((Test-Path $pcapFile2) -and (Test-Path $filterFile) -and (Test-Path $lineExpected)) {
        Test-Compare -TestName "Test line output (cat_001_002)" `
            -Command "& `"$exe`" -d `"$config`" -R -l -f `"$pcapFile2`" -LF `"$filterFile`"" `
            -ExpectedFile $lineExpected
    }

    # Test GPS parsing output
    $gpsFile = Join-Path $sampleDataDir "parsegps.gps"
    $gpsExpected = Join-Path $sampleOutputDir "parsegps.txt"
    if ((Test-Path $gpsFile) -and (Test-Path $gpsExpected)) {
        Test-Compare -TestName "Test GPS parsing output" `
            -Command "& `"$exe`" -d `"$config`" -G -f `"$gpsFile`"" `
            -ExpectedFile $gpsExpected
    }
} else {
    Write-Host "`nSkipping output format comparison tests (expected outputs not found)" -ForegroundColor Yellow
}

# ============================================================
# Test 4: Memory Leak Tests (Windows alternative to valgrind)
# ============================================================

# Note: Windows doesn't have valgrind. Alternatives:
# - Application Verifier (requires manual setup)
# - Dr. Memory (requires installation)
# - ASAN (Address Sanitizer) - requires recompilation with /fsanitize=address
#
# For CI/CD, these tests are skipped on Windows. Memory testing is done on Linux.

Write-Host "`n" -NoNewline
Write-Host "NOTE: " -ForegroundColor Yellow -NoNewline
Write-Host "Memory leak detection is not available on Windows (use Linux/valgrind for memory tests)" -ForegroundColor Gray

# ============================================================
# SUMMARY
# ============================================================

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  TEST SUMMARY" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

Write-Host "Total tests:  $totalTests" -ForegroundColor Cyan
Write-Host "Passed:       " -NoNewline -ForegroundColor Cyan
Write-Host ($totalTests - $failedTests) -ForegroundColor Green
Write-Host "Failed:       " -NoNewline -ForegroundColor Cyan
if ($failedTests -eq 0) {
    Write-Host $failedTests -ForegroundColor Green
} else {
    Write-Host $failedTests -ForegroundColor Red
}

Write-Host "`n" -NoNewline

if ($failedTests -eq 0) {
    Write-Host "✓ All tests PASSED!" -ForegroundColor Green
    exit 0
} else {
    Write-Host "✗ $failedTests test(s) FAILED" -ForegroundColor Red
    exit 1
}
