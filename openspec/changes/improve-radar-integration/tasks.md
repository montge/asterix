## Phase 1: CI/CD Optimization (Priority: HIGH)

### 1.1 Fix DO-278 CI Timeout
- [x] 1.1.1 Analyze radar integration test execution time
- [x] 1.1.2 Add test parallelization or split into multiple jobs
- [x] 1.1.3 Implement test timeout configuration
- [x] 1.1.4 Verify DO-278 CI completes successfully

**Commits:**
- `f1c6142` - Add pytest-timeout and pytest-xdist for parallel test execution
- Set 10-minute timeout for radar integration tests
- Use -n auto for pytest parallelization with fallback

### 1.2 Fix SonarCloud CI
- [x] 1.2.1 Fix conflicting sonar.cfamily properties
- [x] 1.2.2 Verify SonarCloud Analysis passes

**Commits:**
- `f86feda` - Remove conflicting sonar.cfamily.build-wrapper-output property
- `f576694` - Remove hardcoded paths from examples and documentation

### 1.3 Optimize Test Suite
- [x] 1.3.1 Profile slowest tests in radar_integration/test/
- [x] 1.3.2 Reduce redundant test iterations (not needed - tests already fast)
- [x] 1.3.3 Add pytest markers for slow tests (skip in quick CI)
- [x] 1.3.4 Verify 351 tests complete in <5 minutes (523 tests in 3.36s)

**Test Optimization (Dec 19, 2025):**
- Slowest tests: benchmark_run_all (0.43s), matplotlib visualizations (0.30-0.33s)
- Added pytest markers in pyproject.toml: slow, visualization, benchmark
- Marked TestMatplotlibVisualization class with @pytest.mark.slow @pytest.mark.visualization
- Marked test_run_all, test_benchmark_mock_radar_generation with @pytest.mark.slow @pytest.mark.benchmark
- Quick run with `-m "not slow"`: 35 passed in 1.39s (vs 42 in 3.36s)

## Phase 2: Code Quality (Priority: MEDIUM)

### 2.1 Reduce Code Duplication
- [x] 2.1.1 Identify duplicated patterns in encoder modules
- [x] 2.1.2 Extract common FSPEC encoding logic
- [x] 2.1.3 Create shared data item encoding utilities
- [x] 2.1.4 Verify duplication remains <3%

**Commits:**
- `acb279d` - Extract common ASTERIX encoding functions to encoder/common.py
- Created encode_fspec(), encode_i010(), encode_time_of_day(), encode_wgs84_position()
- Added encode_aircraft_address(), encode_callsign(), encode_track_number()
- Removed ~143 lines of duplicate code across 4 encoder modules

### 2.2 Fix SonarCloud Issues
- [x] 2.2.1 Address code smells in radar_integration module
- [x] 2.2.2 Fix cognitive complexity in visualization.py
- [x] 2.2.3 Add missing type hints (all core modules have comprehensive type hints)
- [ ] 2.2.4 Improve docstrings

**Commits:**
- `ed5a454` - Fix code smells: unused variables, legacy random, duplicate strings
- `f576694` - Refactor visualization.py to reduce cognitive complexity

## Phase 3: RadarSimPy Integration (Priority: LOW)

### 3.1 Design Integration
- [ ] 3.1.1 Research RadarSimPy API
- [ ] 3.1.2 Design adapter pattern for radar simulation
- [ ] 3.1.3 Define optional dependency handling

### 3.2 Implementation
- [ ] 3.2.1 Create RadarSimPy adapter module
- [ ] 3.2.2 Implement realistic target generation
- [ ] 3.2.3 Add integration tests with RadarSimPy
- [ ] 3.2.4 Document RadarSimPy usage

## Phase 4: Test Coverage (Priority: MEDIUM)

### 4.1 Improve Coverage
- [ ] 4.1.1 Identify uncovered code paths
- [ ] 4.1.2 Add tests for edge cases
- [ ] 4.1.3 Add property-based tests with Hypothesis
- [ ] 4.1.4 Target 80% coverage for radar_integration

## Progress Tracking

| Phase | Tasks | Completed |
|-------|-------|-----------|
| CI/CD Optimization | 10 | 10 |
| Code Quality | 8 | 7 |
| RadarSimPy Integration | 7 | 0 |
| Test Coverage | 4 | 0 |
| **Total** | **29** | **17** |
