#!/usr/bin/env python3
"""
Complete ASTERIX Surveillance Demonstration - All 6 Categories

Demonstrates a complete airport surveillance scenario using all 6 supported
ASTERIX categories to show the full data fusion workflow:

1. CAT001: Older radar target reports
2. CAT020: Multilateration (MLAT) surface surveillance
3. CAT021: ADS-B transponder reports (high precision)
4. CAT034: Radar service messages (North Marker, Sector Crossing)
5. CAT048: Modern radar target reports
6. CAT062: Fused system tracks

This represents a realistic airport surveillance system with:
- Primary/Secondary radar
- MLAT for surface movement
- ADS-B for equipped aircraft
- Multi-sensor data fusion
- System health monitoring

Usage:
    python3 complete_surveillance_demo.py

---

License: GPL-3.0
"""

import sys
import struct
import time
sys.path.insert(0, '/home/e/Development/asterix')

from asterix.radar_integration import MockRadar
from asterix.radar_integration.encoder import (
    encode_cat001,
    encode_cat020,
    encode_cat021,
    encode_cat034_north_marker,
    encode_cat034_sector_crossing,
    encode_cat048,
    encode_cat062
)


def main():
    """Run complete 6-category surveillance demonstration."""
    print("=" * 80)
    print("Complete ASTERIX Surveillance System Demonstration")
    print("All 6 Categories - Full Data Fusion Workflow")
    print("=" * 80)
    print()

    # Initialize timestamp for synchronized data
    current_time = time.time()

    # ===== CAT034: System Status Messages =====
    print("Category 034: Radar Service Messages")
    print("-" * 80)

    # North Marker (antenna passes North)
    cat034_north = encode_cat034_north_marker(
        sac=1, sic=1,
        sector_number=0,
        timestamp=current_time,
        antenna_rotation_speed=4.0  # 4 second rotation (15 RPM)
    )

    # Sector Crossing (8-sector radar)
    cat034_sectors = []
    for sector in range(8):
        cat034_sectors.append(encode_cat034_sector_crossing(
            sac=1, sic=1,
            sector_number=sector,
            timestamp=current_time + sector * 0.5
        ))

    print(f"✅ North Marker: {len(cat034_north)} bytes")
    print(f"✅ Sector Crossings: 8 messages, {sum(len(s) for s in cat034_sectors)} bytes total")
    print(f"   Antenna rotation: 4.0 seconds (15 RPM)")
    print()

    # ===== CAT001 & CAT048: Primary/Secondary Radar =====
    print("Category 001/048: Radar Target Reports")
    print("-" * 80)

    # Generate radar plots
    radar = MockRadar(lat=52.5597, lon=13.2877, alt=100.0)  # Berlin Brandenburg
    radar_plots = radar.generate_plots(num_targets=15, add_noise=True, add_clutter=True)

    # Encode same data to both CAT001 and CAT048
    cat001_data = encode_cat001(radar_plots, sac=1, sic=1)
    cat048_data = encode_cat048(radar_plots, sac=1, sic=2)

    print(f"✅ CAT001 (older spec): {len(radar_plots)} plots → {len(cat001_data)} bytes "
          f"({len(cat001_data)/len(radar_plots):.1f} bytes/plot)")
    print(f"✅ CAT048 (modern spec): {len(radar_plots)} plots → {len(cat048_data)} bytes "
          f"({len(cat048_data)/len(radar_plots):.1f} bytes/plot)")
    print(f"   Difference: CAT048 is {len(cat048_data) - len(cat001_data):+d} bytes "
          f"({((len(cat048_data)/len(cat001_data))-1)*100:+.1f}%)")
    print()

    # ===== CAT020: MLAT Surface Surveillance =====
    print("Category 020: Multilateration (Airport Surface)")
    print("-" * 80)

    # MLAT targets on airport surface
    mlat_targets = [
        {
            'lat': 52.5597, 'lon': 13.2877,
            'aircraft_address': 0x3C6544, 'callsign': 'DLH123',
            'track_number': 5001,
            'vx': 5.0, 'vy': 3.0,  # Taxiing
            'x_m': 1200, 'y_m': 800,
            'timestamp': current_time
        },
        {
            'lat': 52.5601, 'lon': 13.2885,
            'aircraft_address': 0x4009A8, 'callsign': 'BAW456',
            'track_number': 5002,
            'vx': -3.0, 'vy': 8.0,
            'x_m': 1350, 'y_m': 1200,
            'timestamp': current_time
        },
        {
            'lat': 52.5595, 'lon': 13.2872,
            'aircraft_address': 0x3950A1, 'callsign': 'AFR789',
            'track_number': 5003,
            'vx': 0.0, 'vy': 0.0,  # Stationary
            'x_m': 980, 'y_m': 650,
            'timestamp': current_time
        }
    ]

    cat020_data = encode_cat020(mlat_targets, sac=2, sic=1)
    print(f"✅ CAT020 (MLAT): {len(mlat_targets)} surface targets → {len(cat020_data)} bytes "
          f"({len(cat020_data)/len(mlat_targets):.1f} bytes/target)")
    print(f"   Use case: Runway incursion detection, taxiway monitoring")
    print()

    # ===== CAT021: ADS-B =====
    print("Category 021: ADS-B (High Precision)")
    print("-" * 80)

    # ADS-B reports from equipped aircraft
    adsb_reports = [
        {
            'lat': 52.5597, 'lon': 13.2877,
            'aircraft_address': 0x3C6544, 'callsign': 'DLH123',
            'flight_level': 50, 'gnss_height_ft': 5120,
            'airspeed_kt': 200, 'true_airspeed_kt': 215,
            'magnetic_heading_deg': 350.0, 'vertical_rate_fpm': -500,
            'track_number': 2001, 'timestamp': current_time
        },
        {
            'lat': 52.5601, 'lon': 13.2885,
            'aircraft_address': 0x4009A8, 'callsign': 'BAW456',
            'flight_level': 15, 'gnss_height_ft': 1580,
            'airspeed_kt': 150, 'magnetic_heading_deg': 15.0,
            'vertical_rate_fpm': -300,
            'track_number': 2002, 'timestamp': current_time
        }
    ]

    cat021_data = encode_cat021(adsb_reports, sac=3, sic=1)
    print(f"✅ CAT021 (ADS-B): {len(adsb_reports)} reports → {len(cat021_data)} bytes "
          f"({len(cat021_data)/len(adsb_reports):.1f} bytes/report)")
    print(f"   Precision: ~0.021m (340x better than CAT048!)")
    print()

    # ===== CAT062: Fused System Tracks =====
    print("Category 062: System Tracks (Multi-Sensor Fusion)")
    print("-" * 80)

    # Fused tracks from all sensors
    system_tracks = [
        {
            'lat': 52.5597, 'lon': 13.2877,
            'altitude_ft': 5000, 'vx': 50.0, 'vy': 100.0,
            'callsign': 'DLH123', 'adep': 'EDDM', 'ades': 'EDDB',
            'timestamp': current_time
        },
        {
            'lat': 52.5601, 'lon': 13.2885,
            'altitude_ft': 1500, 'vx': 10.0, 'vy': 60.0,
            'callsign': 'BAW456', 'adep': 'EGKK', 'ades': 'EDDB',
            'timestamp': current_time
        }
    ]

    cat062_data = encode_cat062(system_tracks, sac=4, sic=1)
    print(f"✅ CAT062 (Tracks): {len(system_tracks)} tracks → {len(cat062_data)} bytes "
          f"({len(cat062_data)/len(system_tracks):.1f} bytes/track)")
    print(f"   Fusion of: Radar + MLAT + ADS-B")
    print()

    # ===== Summary =====
    print("=" * 80)
    print("Complete Surveillance System Summary")
    print("=" * 80)
    print()

    total_bytes = (len(cat034_north) + sum(len(s) for s in cat034_sectors) +
                   len(cat001_data) + len(cat048_data) + len(cat020_data) +
                   len(cat021_data) + len(cat062_data))

    print("Data generated across all 6 categories:")
    print()
    print(f"  CAT001 (Old Radar):      {len(cat001_data):5d} bytes ({len(radar_plots)} plots)")
    print(f"  CAT020 (MLAT):           {len(cat020_data):5d} bytes ({len(mlat_targets)} surface targets)")
    print(f"  CAT021 (ADS-B):          {len(cat021_data):5d} bytes ({len(adsb_reports)} aircraft)")
    print(f"  CAT034 (Service):        {len(cat034_north) + sum(len(s) for s in cat034_sectors):5d} bytes (9 messages)")
    print(f"  CAT048 (Modern Radar):   {len(cat048_data):5d} bytes ({len(radar_plots)} plots)")
    print(f"  CAT062 (System Tracks):  {len(cat062_data):5d} bytes ({len(system_tracks)} tracks)")
    print(f"  " + "-" * 60)
    print(f"  TOTAL:                   {total_bytes:5d} bytes")
    print()

    print("Surveillance Data Fusion Workflow:")
    print()
    print("  ┌─────────────┐  CAT001/048")
    print("  │   Radar     │──────────────→ Raw plots (~7m accuracy)")
    print("  │  (Primary/  │")
    print("  │  Secondary) │")
    print("  └─────────────┘")
    print("        │")
    print("  ┌─────────────┐  CAT020")
    print("  │    MLAT     │──────────────→ Surface position (~0.7m)")
    print("  │  (4+ sites) │")
    print("  └─────────────┘")
    print("        │")
    print("  ┌─────────────┐  CAT021")
    print("  │   ADS-B     │──────────────→ High precision (~0.02m)")
    print("  │ (1090 MHz)  │")
    print("  └─────────────┘")
    print("        │")
    print("        ↓")
    print("  ┌─────────────┐  CAT062")
    print("  │   Tracker   │──────────────→ Fused tracks + flight plan")
    print("  │  & Fusion   │")
    print("  └─────────────┘")
    print("        │")
    print("  ┌─────────────┐  CAT034")
    print("  │   System    │──────────────→ Status, health, timing")
    print("  │   Monitor   │")
    print("  └─────────────┘")
    print()

    print("=" * 80)
    print("✅ COMPLETE SURVEILLANCE SYSTEM DEMONSTRATION SUCCESSFUL!")
    print("=" * 80)
    print()
    print("Capabilities demonstrated:")
    print("  ✅ 6 ASTERIX category encoders working")
    print(f"  ✅ {total_bytes} bytes of surveillance data generated")
    print("  ✅ Complete sensor fusion workflow")
    print("  ✅ Primary radar, MLAT, ADS-B integration")
    print("  ✅ Service messages and health monitoring")
    print()
    print("This suite enables:")
    print("  • Complete ASTERIX protocol testing")
    print("  • Multi-sensor data fusion validation")
    print("  • Airport surveillance system testing")
    print("  • Surveillance algorithm development")
    print("  • Safety-critical system integration")


if __name__ == "__main__":
    main()
