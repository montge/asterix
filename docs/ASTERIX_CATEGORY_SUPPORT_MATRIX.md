# ASTERIX Category Support Matrix

**Document Version:** 1.0
**Last Updated:** 2025-11-01
**Repository:** https://github.com/CroatiaControlLtd/asterix
**Upstream Specs:** https://zoranbosnjak.github.io/asterix-specs/

---

## Executive Summary

This document provides a comprehensive support matrix for all ASTERIX (All Purpose Structured EUROCONTROL Surveillance Information Exchange) categories, tracking implementation status, version levels, and gaps in our codebase.

**Current Status:**
- **Supported Categories:** 24 categories (out of 127 standard categories)
- **Coverage:** ~19% of standard civil/military categories (001-127)
- **Total with Specifications Available:** 23 categories with XML definitions

### Key Findings
- **Most Critical Gaps:** Categories 003, 005, 006, 007, 012-014, 017, 022, 024, 026-029, 033, 035-047, 049-055, 056-061, 064, 066-100, 101-127
- **Newest Supported Category:** CAT015 (recently added v1.2)
- **Latest Available Versions in Specs:** Several categories have newer versions available (CAT048 up to v1.30, CAT062 up to v1.19)

---

## Category Range Organization

ASTERIX categories are allocated as follows:

| Range | Purpose | Count |
|-------|---------|-------|
| **001-127** | Standard Civil and Military Applications | 127 |
| **128-240** | Special Civil and Military Applications (National Allocation) | 113 |
| **241-255** | User-Defined Non-Standard Categories | 15 |

Our current implementation focuses on the 001-127 range.

---

## Complete Category Support Matrix

### Supported Categories (24 total)

#### **CAT001: Transmission of Monoradar Data Target Reports**
| Attribute | Value |
|-----------|-------|
| **Category Number** | 001 |
| **Full Name** | Transmission of Monoradar Data Target Reports |
| **Purpose** | Primary and secondary radar surveillance data; target positions from single radar systems |
| **Surveillance Type** | Monoradar (single radar) target reports |
| **Current Version** | v1.2 ✓ |
| **Latest Available** | v1.4 |
| **Version Gap** | 2 minor versions behind |
| **Support Status** | SUPPORTED |
| **Priority** | LOW (core category, update available) |

#### **CAT002: Transmission of Monoradar Service Messages**
| Attribute | Value |
|-----------|-------|
| **Category Number** | 002 |
| **Full Name** | Transmission of Monoradar Service Messages |
| **Purpose** | Service messages from monoradar systems (north markers, sector crossing, blind zone filtering) |
| **Surveillance Type** | System control and service messages |
| **Current Version** | v1.0 ✓ |
| **Latest Available** | v1.1 |
| **Version Gap** | 1 minor version behind |
| **Support Status** | SUPPORTED |
| **Priority** | LOW |

#### **CAT004: Safety Net Messages**
| Attribute | Value |
|-----------|-------|
| **Category Number** | 004 |
| **Full Name** | Safety Net Messages |
| **Purpose** | Safety Net and runway incursion warnings; collision avoidance alerts |
| **Surveillance Type** | Safety services |
| **Current Version** | v1.12 ✓ |
| **Latest Available** | v1.12 |
| **Version Gap** | Current |
| **Support Status** | SUPPORTED |
| **Priority** | LOW |

#### **CAT008: Monoradar Derived Weather Information**
| Attribute | Value |
|-----------|-------|
| **Category Number** | 008 |
| **Full Name** | Monoradar Derived Weather Information |
| **Purpose** | Weather/precipitation data derived from radar systems |
| **Surveillance Type** | Meteorological data |
| **Current Version** | v1.0 ✓ |
| **Latest Available** | v1.3 |
| **Version Gap** | 3 minor versions behind |
| **Support Status** | SUPPORTED |
| **Priority** | MEDIUM (weather data, significant update) |

#### **CAT010: Transmission of Monosensor Surface Movement Data**
| Attribute | Value |
|-----------|-------|
| **Category Number** | 010 |
| **Full Name** | Transmission of Monosensor Surface Movement Data |
| **Purpose** | Ground/airport surface movement data (aircraft and vehicles on runway/taxiway) |
| **Surveillance Type** | Surface movement (airport operations) |
| **Current Version** | v1.1 ✓ |
| **Latest Available** | v1.1 |
| **Version Gap** | Current |
| **Support Status** | SUPPORTED |
| **Priority** | LOW |

#### **CAT011: Transmission of A-SMGCS Data**
| Attribute | Value |
|-----------|-------|
| **Category Number** | 011 |
| **Full Name** | Transmission of A-SMGCS Data |
| **Purpose** | Advanced Surface Movement Guidance and Control System data |
| **Surveillance Type** | Airport surface guidance and control |
| **Current Version** | v1.2 ✓ |
| **Latest Available** | v1.3 |
| **Version Gap** | 1 minor version behind |
| **Support Status** | SUPPORTED |
| **Priority** | LOW |

#### **CAT015: Independent Non-Cooperative Surveillance System Target Reports**
| Attribute | Value |
|-----------|-------|
| **Category Number** | 015 |
| **Full Name** | Independent Non-Cooperative Surveillance System Target Reports |
| **Purpose** | INCS target reports (multilateration, WAM, or other sensors without ADS-B cooperation) |
| **Surveillance Type** | Non-cooperative surveillance |
| **Current Version** | v1.2 ✓ (RECENTLY ADDED) |
| **Latest Available** | v1.1 |
| **Version Gap** | Current (v1.2 is available) |
| **Support Status** | ✅ SUPPORTED (NEW) |
| **Priority** | LOW |
| **Notes** | Newly added category, ahead of main specs repo version |

#### **CAT019: Multilateration System Status Messages**
| Attribute | Value |
|-----------|-------|
| **Category Number** | 019 |
| **Full Name** | Multilateration System Status Messages |
| **Purpose** | Status and operational status of multilateration (MLAT) systems |
| **Surveillance Type** | MLAT system metadata |
| **Current Version** | v1.3 ✓ |
| **Latest Available** | v1.3 |
| **Version Gap** | Current |
| **Support Status** | SUPPORTED |
| **Priority** | LOW |

#### **CAT020: Multilateration Target Reports**
| Attribute | Value |
|-----------|-------|
| **Category Number** | 020 |
| **Full Name** | Multilateration Target Reports |
| **Purpose** | Position reports from multilateration (MLAT) systems |
| **Surveillance Type** | Multilateration position reports |
| **Current Version** | v1.10 ✓ |
| **Latest Available** | v1.10 |
| **Version Gap** | Current |
| **Support Status** | SUPPORTED |
| **Priority** | LOW |

#### **CAT021: ADS-B Target Reports**
| Attribute | Value |
|-----------|-------|
| **Category Number** | 021 |
| **Full Name** | ADS-B Target Reports |
| **Purpose** | Automatic Dependent Surveillance - Broadcast data from aircraft |
| **Surveillance Type** | ADS-B position and flight information |
| **Current Version** | v2.6 ✓ |
| **Latest Available** | v2.6 |
| **Version Gap** | Current |
| **Support Status** | SUPPORTED |
| **Priority** | LOW |

#### **CAT023: CNS/ATM Ground Station and Service Status Reports**
| Attribute | Value |
|-----------|-------|
| **Category Number** | 023 |
| **Full Name** | CNS/ATM Ground Station and Service Status Reports |
| **Purpose** | Status of Communication/Navigation/Surveillance infrastructure |
| **Surveillance Type** | System status and service status |
| **Current Version** | v1.3 ✓ |
| **Latest Available** | v1.3 |
| **Version Gap** | Current |
| **Support Status** | SUPPORTED |
| **Priority** | LOW |

#### **CAT025: CNS/ATM Ground System Status Reports**
| Attribute | Value |
|-----------|-------|
| **Category Number** | 025 |
| **Full Name** | CNS/ATM Ground System Status Reports |
| **Purpose** | Ground-based CNS/ATM system status and operational messages |
| **Surveillance Type** | CNS/ATM system status |
| **Current Version** | v1.5 ✓ |
| **Latest Available** | v1.5 |
| **Version Gap** | Current |
| **Support Status** | SUPPORTED |
| **Priority** | LOW |

#### **CAT030: Transmission of Data on Secondary Radar Data Processing System to SDPS**
| Attribute | Value |
|-----------|-------|
| **Category Number** | 030 |
| **Full Name** | Transmission of Data on Secondary Radar Data Processing System to SDPS |
| **Purpose** | Legacy secondary radar data processing output to Surveillance Data Processing System |
| **Surveillance Type** | Secondary radar processing |
| **Current Version** | v6.2 ✓ |
| **Latest Available** | v6.2 |
| **Version Gap** | Current |
| **Support Status** | SUPPORTED |
| **Priority** | LOW (legacy but still in use) |

#### **CAT031: Transmission of Data on Radar Data Processing System to SDPS**
| Attribute | Value |
|-----------|-------|
| **Category Number** | 031 |
| **Full Name** | Transmission of Data on Radar Data Processing System to SDPS |
| **Purpose** | Radar data processing system output to Surveillance Data Processing System |
| **Surveillance Type** | Radar data processing |
| **Current Version** | v6.2 ✓ |
| **Latest Available** | v6.2 |
| **Version Gap** | Current |
| **Support Status** | SUPPORTED |
| **Priority** | LOW |

#### **CAT032: Miniplan Reports to an SDPS**
| Attribute | Value |
|-----------|-------|
| **Category Number** | 032 |
| **Full Name** | Miniplan Reports to an SDPS |
| **Purpose** | Flight plan transmission and coordination to Surveillance Data Processing System |
| **Surveillance Type** | Flight plan data |
| **Current Version** | v6.2 (per .ini: 7.0 expected) ✓ |
| **Latest Available** | v7.0 |
| **Version Gap** | Discrepancy noted - versions vary |
| **Support Status** | SUPPORTED |
| **Priority** | MEDIUM (flight plan data, version mismatch) |

#### **CAT034: Transmission of Monoradar Service Messages**
| Attribute | Value |
|-----------|-------|
| **Category Number** | 034 |
| **Full Name** | Transmission of Monoradar Service Messages (Enhanced) |
| **Purpose** | Enhanced monoradar service messaging (evolution of CAT002) |
| **Surveillance Type** | Service messages |
| **Current Version** | v1.27 ✓ |
| **Latest Available** | v1.29 |
| **Version Gap** | 2 minor versions behind |
| **Support Status** | SUPPORTED |
| **Priority** | MEDIUM (actively evolving) |

#### **CAT048: Monoradar Target Reports**
| Attribute | Value |
|-----------|-------|
| **Category Number** | 048 |
| **Full Name** | Monoradar Target Reports (Enhanced) |
| **Purpose** | Modern radar target reports (combines legacy CAT001 and CAT016) |
| **Surveillance Type** | Radar target reports (all types) |
| **Current Version** | v1.21 ✓ |
| **Latest Available** | v1.30 |
| **Version Gap** | 9 minor versions behind |
| **Support Status** | SUPPORTED |
| **Priority** | **CRITICAL** (most actively developed, major version gap) |

#### **CAT062: SDPS Track Messages**
| Attribute | Value |
|-----------|-------|
| **Category Number** | 062 |
| **Full Name** | SDPS Track Messages |
| **Purpose** | Flight track messages from Surveillance Data Processing Systems (processed/correlated data) |
| **Surveillance Type** | Processed track data |
| **Current Version** | v1.18 ✓ |
| **Latest Available** | v1.19 |
| **Version Gap** | 1 minor version behind |
| **Support Status** | SUPPORTED |
| **Priority** | MEDIUM (core processing category) |

#### **CAT063: Sensor Status Reports**
| Attribute | Value |
|-----------|-------|
| **Category Number** | 063 |
| **Full Name** | Sensor Status Reports |
| **Purpose** | Operational status and health reporting of surveillance sensors |
| **Surveillance Type** | System status |
| **Current Version** | v1.3 ✓ |
| **Latest Available** | v1.6 |
| **Version Gap** | 3 minor versions behind |
| **Support Status** | SUPPORTED |
| **Priority** | MEDIUM (actively evolving) |

#### **CAT065: SDPS Service Status Reports**
| Attribute | Value |
|-----------|-------|
| **Category Number** | 065 |
| **Full Name** | SDPS Service Status Reports |
| **Purpose** | Operational status and service health of Surveillance Data Processing Systems |
| **Surveillance Type** | System status |
| **Current Version** | v1.3 ✓ |
| **Latest Available** | v1.5 |
| **Version Gap** | 2 minor versions behind |
| **Support Status** | SUPPORTED |
| **Priority** | MEDIUM |

#### **CAT205: Radio Direction Finder Reports**
| Attribute | Value |
|-----------|-------|
| **Category Number** | 205 |
| **Full Name** | Radio Direction Finder Reports |
| **Purpose** | Position reports from radio direction finding systems (emergency locating) |
| **Surveillance Type** | Radio direction finding |
| **Current Version** | v1.0 ✓ |
| **Latest Available** | v1.0 |
| **Version Gap** | Current |
| **Support Status** | SUPPORTED |
| **Priority** | LOW (specialized) |

#### **CAT240: Radar Video Transmission**
| Attribute | Value |
|-----------|-------|
| **Category Number** | 240 |
| **Full Name** | Radar Video Transmission |
| **Purpose** | Raw radar video data transmission (radar sweep/video data) |
| **Surveillance Type** | Raw radar video |
| **Current Version** | v1.3 ✓ |
| **Latest Available** | v1.3 |
| **Version Gap** | Current |
| **Support Status** | SUPPORTED |
| **Priority** | LOW |

#### **CAT247: Version Number Exchange**
| Attribute | Value |
|-----------|-------|
| **Category Number** | 247 |
| **Full Name** | Version Number Exchange |
| **Purpose** | System and software version number exchange for compatibility checking |
| **Surveillance Type** | System metadata |
| **Current Version** | v1.2 (Per .ini: listed as 1.2) ✓ |
| **Latest Available** | v1.3 |
| **Version Gap** | 1 minor version behind |
| **Support Status** | SUPPORTED |
| **Priority** | LOW |

#### **CAT252: System and Session Control Messages**
| Attribute | Value |
|-----------|-------|
| **Category Number** | 252 |
| **Full Name** | System and Session Control Messages |
| **Purpose** | Session and system control, connection management between systems |
| **Surveillance Type** | System control |
| **Current Version** | v6.2 (Per .ini: listed as 7.0 in git status) |
| **Latest Available** | v7.0 |
| **Version Gap** | Needs verification |
| **Support Status** | SUPPORTED |
| **Priority** | MEDIUM (system critical) |

---

### Unsupported Categories - Priority Analysis

#### **TIER 1: HIGH PRIORITY - Standard Operational Categories**

| Category | Name | Purpose | Priority Reason | Notes |
|----------|------|---------|-----------------|-------|
| **CAT003** | Air Traffic Situation Picture | ATS coordination and ATSIC integration | Core ATC function | Retired/Proposed, needs clarification |
| **CAT016** | INCS Configuration Reports | Configuration for non-cooperative systems | Complementary to CAT015 | Specs available (v1.0) |
| **CAT018** | Mode S Datalink Messages | Mode S datalink services | Modern surveillance backbone | Specs available (v1.7) |

#### **TIER 2: MEDIUM PRIORITY - Specialized/Emerging Categories**

| Category | Name | Purpose | Priority Reason | Notes |
|----------|------|---------|-----------------|-------|
| **CAT009** | Composite Weather Reports | Combined multi-sensor weather | Weather integration | Specs available (v2.1) |

#### **TIER 3: LOW PRIORITY - Reserved/Future Categories**

| Category Range | Status | Notes |
|---|---|---|
| **005-007** | Not allocated/Reserved | Directed interrogation (CAT007) in limited use |
| **012-014, 017, 022, 024, 026-029, 033** | Reserved | Available for future allocation |
| **035-047** | Reserved | Large block reserved for future use |
| **049-055, 056-061, 064** | Reserved | Sparse allocation |
| **066-100** | Reserved | Legacy/future allocation |
| **101-127** | Reserved | Future civil/military applications |
| **128-240** | National allocation | Not standardized by EUROCONTROL |
| **241-255** | User-defined | Non-standard/proprietary |

---

## Gap Analysis

### Version Currency Issues

The following categories have significant version gaps (≥2 minor versions behind):

| Category | Current | Latest | Gap | Impact |
|----------|---------|--------|-----|--------|
| **CAT048** | v1.21 | v1.30 | **9 versions** | **CRITICAL** - Most actively developed category |
| **CAT008** | v1.0 | v1.3 | 3 versions | Weather data improvements |
| **CAT063** | v1.3 | v1.6 | 3 versions | Sensor status enhancements |
| **CAT034** | v1.27 | v1.29 | 2 versions | Service messaging evolution |
| **CAT065** | v1.3 | v1.5 | 2 versions | SDPS status updates |
| **CAT247** | v1.2 | v1.3 | 1 version | Minor updates |
| **CAT062** | v1.18 | v1.19 | 1 version | Minor updates |
| **CAT011** | v1.2 | v1.3 | 1 version | Minor updates |

### Missing Categories (23 categories with no support)

**Standard Categories with Specifications Available:**
- CAT009 (Composite Weather Reports)
- CAT016 (INCS Configuration Reports)
- CAT018 (Mode S Datalink Messages)

**Reserved/Unallocated Categories (20):**
- CAT003, CAT005-007, CAT012-014, CAT017, CAT022, CAT024, CAT026-029, CAT033, CAT035-047, CAT049-055, CAT056-061, CAT064, CAT066-100, CAT101-127

---

## Implementation Recommendations

### Phase 1: Critical Updates (Immediate - Next Release)

**Priority: MAXIMUM**

1. **Update CAT048 to v1.30**
   - Rationale: Most actively developed category with 9-version gap
   - Effort: Medium
   - Impact: High (radar data processing)
   - Estimated Time: 2-3 days

2. **Add CAT016 - INCS Configuration (v1.0)**
   - Rationale: Complementary to existing CAT015
   - Effort: Low-Medium
   - Impact: Medium (configuration data)
   - Estimated Time: 1-2 days

### Phase 2: Functional Completeness (Next Quarterly Release)

**Priority: HIGH**

1. **Update CAT062 to v1.19** (1 version gap)
   - Effort: Low
   - Time: 1 day

2. **Update CAT063 to v1.6** (3 version gap)
   - Effort: Low-Medium
   - Time: 1-2 days

3. **Update CAT034 to v1.29** (2 version gap)
   - Effort: Low
   - Time: 1 day

4. **Add CAT018 - Mode S Datalink (v1.7)**
   - Rationale: Important surveillance data source
   - Effort: Medium
   - Impact: Medium-High (Mode S services)
   - Time: 2-3 days

### Phase 3: Feature Expansion (Six-Month Roadmap)

**Priority: MEDIUM**

1. **Add CAT009 - Composite Weather Reports (v2.1)**
   - Rationale: Meteorological data integration
   - Effort: Low-Medium
   - Impact: Low-Medium (weather processing)
   - Time: 1-2 days

2. **Update CAT008 to v1.3** (3 version gap)
   - Effort: Low
   - Time: 1 day

3. **Update remaining categories with version gaps:**
   - CAT065 → v1.5 (2 versions)
   - CAT247 → v1.3 (1 version)
   - CAT011 → v1.3 (1 version)
   - Combined effort: 1-2 days

4. **Research CAT003 Status**
   - Determine if CAT003 (ATSP) is still relevant
   - Effort: 0.5 days
   - Decision: Implement or deprecate

### Phase 4: Long-Term Backlog (Next Year)

**Priority: LOW**

1. **Reserved categories investigation** (if needed by customers)
   - CAT005-007, CAT012-014, CAT017, CAT022, CAT024, CAT026-029, CAT033
   - CAT035-047, CAT049-055, CAT056-061, CAT064
   - Decision: Allocate only if customer requirements arise

2. **National/User-Defined Categories (128-255)**
   - Decision: Provide extensibility framework but not core implementation

---

## Implementation Roadmap

```
Timeline          Phase 1                Phase 2              Phase 3              Phase 4
(Weeks)           (1-2 weeks)           (2-4 weeks)          (4-12 weeks)         (12+ months)

Immediate:        - CAT048 v1.30        - CAT016 v1.0        - CAT018 v1.7        - Reserved cats
                  - CAT016 v1.0         - CAT062 v1.19       - CAT009 v2.1        - National cats
                                        - CAT063 v1.6        - CAT008 v1.3        - Framework
                                        - CAT034 v1.29       - CAT065 v1.5
                                                             - CAT247 v1.3
                                                             - CAT011 v1.3
```

---

## Version Synchronization Strategy

### Current Version Discrepancies

The file `/path/to/asterix/install/config/asterix.ini` shows:
- CAT032: Listed as v6.2 in one place, v7.0 in git status
- CAT252: Listed as v6.2 in one place, v7.0 in git status

**Action Required:** Verify actual installed versions vs. intended versions

### Automated Version Tracking

**Recommendation:** Implement version tracking in CI/CD pipeline
- Automatically detect version gaps from upstream specs
- Generate alerts when new versions become available
- Track version compliance in test suites

---

## Specification Source Tracking

### Available Specification Repositories

1. **Official EUROCONTROL**
   - URL: https://www.eurocontrol.int/asterix
   - Status: Official standard (slow update cycle)
   - Coverage: All allocated categories

2. **Community Specs Project**
   - URL: https://zoranbosnjak.github.io/asterix-specs/
   - Repository: https://github.com/zoranbosnjak/asterix-specs
   - Status: Comprehensive, faster updates
   - Coverage: 23+ categories with structured JSON/XML

3. **Our Implementation**
   - Location: `asterix-specs-converter/specs/`
   - Source: Converted from community specs
   - Tool: `asterixjson2xml.py` converter

### Spec Update Process

Current workflow:
```
EUROCONTROL Specs → Community JSON → JSON→XML Converter → Our XML Config
```

**Recommendation:** Establish automated update pipeline to track community specs repo

---

## Technical Notes

### Category Status Legend

- ✅ **SUPPORTED** - Fully implemented and tested
- **OUTDATED** - Supported but with version gaps
- **NOT SUPPORTED** - No implementation
- **CURRENT** - Latest version implemented
- **CRITICAL** - Requires immediate attention

### Implementation Complexity Guide

| Effort Level | Category Examples | Typical Time |
|---|---|---|
| **Low** | Version updates (1-2 versions), simple adds | 1 day |
| **Medium** | New complex categories, multi-version updates | 2-3 days |
| **High** | Categories with new data formats, major redesigns | 4-7 days |

### Testing Requirements Per Category

All implementations should include:
1. Unit tests for each data item format
2. Integration tests with sample data
3. Regression tests for version compatibility
4. Performance benchmarks (especially for CAT048)

---

## References

### Official Standards

- EUROCONTROL ASTERIX Main Document (Part 1)
- EUROCONTROL Specification for Surveillance Data Exchange
- Individual category specifications (Part 2a, Part 2b, Part 3, etc.)

### Community Resources

- EUROCONTROL ASTERIX Library: https://www.eurocontrol.int/asterix
- Community Specifications: https://zoranbosnjak.github.io/asterix-specs/
- Wireshark ASTERIX Support: https://wiki.wireshark.org/ASTERIX

### Related Implementation

- CroatiaControl/asterix: https://github.com/CroatiaControlLtd/asterix (upstream)
- jASTERIX: https://github.com/hpuhr/jASTERIX (C++ alternative)
- Various language-specific implementations in other repos

---

## Appendix A: Category Allocation History

### Categories 001-010 Range (Original Monoradar & Surface)
- CAT001: Original monoradar category
- CAT002: Monoradar service messages
- CAT003-007: Reserved/special use
- CAT008: Weather information
- CAT009: Multi-sensor weather
- CAT010: Surface movement

### Categories 011-025 Range (MLAT, ADS-B, CNS/ATM)
- CAT011: A-SMGCS surface guidance
- CAT012-014: Reserved
- CAT015-016: Non-cooperative surveillance (INCS)
- CAT017: Reserved
- CAT018: Mode S datalink
- CAT019-020: Multilateration system (MLAT)
- CAT021: ADS-B
- CAT022-024: Reserved
- CAT025: CNS/ATM ground status

### Categories 030-034 Range (SDPS & Processing)
- CAT030-031: Legacy SDPS data
- CAT032: Miniplan (flight plans)
- CAT033: Reserved
- CAT034: Enhanced monoradar service messages

### Categories 048-065 Range (Modern Processing)
- CAT048: Modern monoradar target reports (replaces CAT001+CAT016)
- CAT049-061: Reserved
- CAT062: SDPS track messages
- CAT063: Sensor status
- CAT064: Reserved
- CAT065: SDPS service status

### Categories 205-252 Range (Special Use)
- CAT205: Radio direction finder
- CAT240: Radar video
- CAT247: Version exchange
- CAT252: System and session control

---

## Appendix B: Known Issues and Workarounds

### Version Tracking Issues
- Some categories show different versions in different files (CAT032, CAT252)
- Recommend establishing single source of truth

### Missing Upstream Specifications
- Categories 003-007, 012-014, 017, 022, 024, 026-029, 033, 035-047, 049-055, 056-061, 064-100, 101-127
- Many are reserved for future use; no official specs available

### Performance Considerations
- CAT048 processing is CPU-intensive (heavily optimized in recent commits)
- CAT240 (radar video) requires efficient buffer handling
- MLAT/ADS-B processing (CAT020, CAT021) scales with data volume

---

## Document Maintenance

This document should be updated:
- **Quarterly** - Review version gaps and update roadmap
- **Upon Release** - Update support status and versions
- **Upon Spec Changes** - Add new categories or revisions
- **Annually** - Comprehensive review and strategic planning

---

**Last Reviewed:** 2025-11-01
**Next Review Date:** 2026-02-01
**Owner:** Development Team
**Status:** ACTIVE
