# Change: Improve Radar Integration Module

## Why
The radar_integration module is a new feature providing mock radar data generation and ASTERIX encoding capabilities. However, it currently has:
- High code duplication (contributing to overall 1.8% duplication)
- DO-278 CI pipeline timeouts when running 351+ radar integration tests
- Missing RadarSimPy integration for realistic radar simulation
- Code smells flagged by SonarCloud

## What Changes
- Optimize radar integration test suite to prevent CI timeouts
- Reduce code duplication in encoder modules (cat001, cat020, cat021, cat034, cat048, cat062)
- Fix SonarCloud code smells in radar_integration module
- Add RadarSimPy integration for physics-based radar simulation (optional dependency)
- Improve test coverage from 21.4% to target 80%

## Impact
- Affected specs: radar-integration (new capability spec)
- Affected code:
  - `asterix/radar_integration/` - All encoder and mock radar modules
  - `.github/workflows/ci-verification.yml` - DO-278 CI pipeline
  - `.github/workflows/radar-integration-tests.yml` - Radar CI pipeline
- CI/CD: Reduce test execution time to prevent timeouts
