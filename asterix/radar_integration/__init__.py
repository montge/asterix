"""
ASTERIX Radar Integration Package

This package provides tools for integrating radar simulators with ASTERIX encoding/decoding:
- Mock radar data generation for testing
- ASTERIX CAT048/CAT062 encoding (radar target reports and system tracks)
- Visualization utilities (ASCII and matplotlib)
- Statistics and analysis tools
- Example scenarios and validation utilities

Supported ASTERIX categories:
- CAT048: Transmission of Monoradar Target Reports
- CAT062: Transmission of System Track Data (fused radar tracks)

Usage:
    from asterix.radar_integration import MockRadar, generate_aircraft_scenario
    from asterix.radar_integration.encoder import encode_cat048, encode_cat062
    from asterix.radar_integration.visualization import plot_radar_ascii
    from asterix.radar_integration.statistics import compute_scenario_stats

    # Generate synthetic radar data
    radar = MockRadar(lat=52.5, lon=13.4, alt=100.0)
    plots = radar.generate_plots(num_targets=10)

    # Encode to ASTERIX CAT048
    asterix_data = encode_cat048(plots, sac=0, sic=1)

    # Visualize radar data
    print(plot_radar_ascii(plots, radar_pos=(52.5, 13.4)))

    # Compute statistics
    stats = compute_scenario_stats(plots)

    # Decode with main ASTERIX parser
    import asterix
    decoded = asterix.parse(asterix_data)

---

License: GPL-3.0
Author: ASTERIX Integration Team
Date: 2025-11-23
"""

from .mock_radar import (
    MockRadar,
    RadarPlot,
    RadarPosition,
    generate_aircraft_scenario,
    generate_approach_scenario
)

# Optional visualization and statistics imports (graceful degradation)
try:
    from .visualization import (
        plot_radar_ascii as plot_radar_ascii,
        check_matplotlib_available as check_matplotlib_available,
        print_visualization_info as print_visualization_info,
    )
    _HAS_VISUALIZATION = True
except ImportError:
    _HAS_VISUALIZATION = False

try:
    from .statistics import (
        compute_scenario_stats as compute_scenario_stats,
        compute_coverage_map as compute_coverage_map,
        analyze_detection_performance as analyze_detection_performance,
        analyze_track_statistics as analyze_track_statistics,
        compare_scenarios as compare_scenarios,
    )
    _HAS_STATISTICS = True
except ImportError:
    _HAS_STATISTICS = False

__all__ = [
    'MockRadar',
    'RadarPlot',
    'RadarPosition',
    'generate_aircraft_scenario',
    'generate_approach_scenario',
]

# Add visualization functions if available
if _HAS_VISUALIZATION:
    __all__.extend([
        'plot_radar_ascii',
        'check_matplotlib_available',
        'print_visualization_info',
    ])

# Add statistics functions if available
if _HAS_STATISTICS:
    __all__.extend([
        'compute_scenario_stats',
        'compute_coverage_map',
        'analyze_detection_performance',
        'analyze_track_statistics',
        'compare_scenarios',
    ])

__version__ = '0.2.0'
