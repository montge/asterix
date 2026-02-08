//! CCSDS transport tests for improved coverage
//!
//! Tests for the CCSDS Space Packet Protocol integration with ASTERIX data.

#![cfg(feature = "ccsds")]

use asterix::transport::ccsds::{
    category_from_apid, parse_ccsds_header, CcsdsConfig, CcsdsError, CcsdsMode,
};

// =============================================================================
// CcsdsConfig Tests
// =============================================================================

#[test]
fn test_ccsds_config_default() {
    let config = CcsdsConfig::default();

    assert_eq!(config.base_apid, 0x300);
    assert!(matches!(config.mode, CcsdsMode::Telemetry));
    assert!(!config.enable_crc);
    assert!(config.multicast_addr.is_none());
    assert_eq!(config.udp_port, Some(7447));
    assert!(!config.use_secondary_header);
    assert_eq!(config.max_packet_length, 65536);
}

#[test]
fn test_ccsds_config_telemetry() {
    let config = CcsdsConfig::telemetry();

    assert!(matches!(config.mode, CcsdsMode::Telemetry));
    assert_eq!(config.base_apid, 0x300);
}

#[test]
fn test_ccsds_config_telecommand() {
    let config = CcsdsConfig::telecommand();

    assert!(matches!(config.mode, CcsdsMode::Telecommand));
    assert_eq!(config.base_apid, 0x300);
}

#[test]
fn test_ccsds_config_with_base_apid() {
    let config = CcsdsConfig::with_base_apid(0x400);

    assert_eq!(config.base_apid, 0x400);
}

#[test]
fn test_ccsds_config_with_multicast() {
    let config = CcsdsConfig::with_multicast("224.0.0.1", 10000);

    assert_eq!(config.multicast_addr, Some("224.0.0.1".to_string()));
    assert_eq!(config.udp_port, Some(10000));
}

#[test]
fn test_ccsds_config_custom_construction() {
    let config = CcsdsConfig {
        mode: CcsdsMode::Telemetry,
        base_apid: 0x350,
        enable_crc: true,
        multicast_addr: Some("224.1.2.3".to_string()),
        udp_port: Some(12345),
        use_secondary_header: false,
        max_packet_length: 65536,
    };

    assert!(matches!(config.mode, CcsdsMode::Telemetry));
    assert_eq!(config.base_apid, 0x350);
    assert!(config.enable_crc);
    assert_eq!(config.multicast_addr, Some("224.1.2.3".to_string()));
    assert_eq!(config.udp_port, Some(12345));
}

// =============================================================================
// CcsdsMode Tests
// =============================================================================

#[test]
fn test_ccsds_mode_telemetry() {
    let mode = CcsdsMode::Telemetry;
    assert!(matches!(mode, CcsdsMode::Telemetry));
}

#[test]
fn test_ccsds_mode_telecommand() {
    let mode = CcsdsMode::Telecommand;
    assert!(matches!(mode, CcsdsMode::Telecommand));
}

#[test]
fn test_ccsds_mode_clone() {
    let mode = CcsdsMode::Telemetry;
    let cloned = mode.clone();
    assert!(matches!(cloned, CcsdsMode::Telemetry));
}

#[test]
fn test_ccsds_mode_debug() {
    let mode = CcsdsMode::Telemetry;
    let debug_str = format!("{:?}", mode);
    assert!(debug_str.contains("Telemetry"));
}

// =============================================================================
// CcsdsError Tests
// =============================================================================

#[test]
fn test_ccsds_error_packet_error() {
    let err = CcsdsError::PacketError("packet too short".to_string());
    let msg = format!("{}", err);
    assert!(msg.contains("packet"));
}

#[test]
fn test_ccsds_error_decode_error() {
    let err = CcsdsError::DecodeError("bad version".to_string());
    let msg = format!("{}", err);
    assert!(msg.contains("bad version"));
}

#[test]
fn test_ccsds_error_encode_error() {
    let err = CcsdsError::EncodeError("encode failed".to_string());
    let msg = format!("{}", err);
    assert!(msg.contains("encode"));
}

#[test]
fn test_ccsds_error_invalid_apid() {
    let err = CcsdsError::InvalidApid("bad apid".to_string());
    let msg = format!("{}", err);
    assert!(msg.to_lowercase().contains("apid"));
}

#[test]
fn test_ccsds_error_serialization() {
    let err = CcsdsError::SerializationError("ser failed".to_string());
    let msg = format!("{}", err);
    assert!(msg.to_lowercase().contains("serialization"));
}

#[test]
fn test_ccsds_error_channel_closed() {
    let err = CcsdsError::ChannelClosed;
    let msg = format!("{}", err);
    assert!(msg.to_lowercase().contains("closed"));
}

#[test]
fn test_ccsds_error_debug() {
    let err = CcsdsError::PacketError("test".to_string());
    let debug_str = format!("{:?}", err);
    assert!(debug_str.contains("PacketError"));
}

#[test]
fn test_ccsds_error_to_asterix_error() {
    use asterix::AsterixError;
    let err = CcsdsError::EncodeError("encode failed".to_string());
    let asterix_err: AsterixError = err.into();
    let msg = asterix_err.to_string();
    assert!(msg.contains("encode"));
}

// =============================================================================
// parse_ccsds_header Tests
// =============================================================================

#[test]
fn test_parse_ccsds_header_valid_telemetry() {
    // Create a valid CCSDS telemetry packet
    // Packet Version: 0, Type: 0 (TM), Sec Hdr: 0, APID: 0x330 (CAT 048)
    // Seq Flags: 3 (unsegmented), Seq Count: 0x0001
    // Data Length: 0x0003 (4 bytes of data)
    let packet: [u8; 10] = [
        0x03, 0x30, // Version=0, Type=0, SecHdr=0, APID=0x330
        0xC0, 0x01, // SeqFlags=3, SeqCount=1
        0x00, 0x03, // Data Length = 3 (means 4 bytes)
        0x30, 0x00, 0x04, 0x01, // ASTERIX data (CAT 048, len 4)
    ];

    let result = parse_ccsds_header(&packet);
    assert!(result.is_ok());

    let (apid, seq_count, data) = result.unwrap();
    assert_eq!(apid, 0x330);
    assert_eq!(seq_count, 1);
    assert_eq!(data.len(), 4);
}

#[test]
fn test_parse_ccsds_header_too_short() {
    let packet: [u8; 3] = [0x03, 0x30, 0xC0];

    let result = parse_ccsds_header(&packet);
    assert!(result.is_err());

    let err_msg = result.unwrap_err().to_string();
    assert!(err_msg.to_lowercase().contains("too short"));
}

#[test]
fn test_parse_ccsds_header_exactly_header_size() {
    // Just the header, no data
    let packet: [u8; 6] = [
        0x03, 0x30, // APID 0x330
        0xC0, 0x01, // SeqCount 1
        0xFF, 0xFF, // Data length = 65535 (but no actual data)
    ];

    let result = parse_ccsds_header(&packet);
    // Should fail because data length indicates more data than present
    assert!(result.is_err());
}

#[test]
fn test_parse_ccsds_header_telecommand_packet() {
    // Packet Type = 1 (Telecommand)
    let packet: [u8; 10] = [
        0x13, 0x30, // Version=0, Type=1 (TC), SecHdr=0, APID=0x330
        0xC0, 0x02, // SeqFlags=3, SeqCount=2
        0x00, 0x03, // Data Length = 3
        0x30, 0x00, 0x04, 0x01,
    ];

    let result = parse_ccsds_header(&packet);
    assert!(result.is_ok());

    let (apid, seq_count, _data) = result.unwrap();
    assert_eq!(apid, 0x330);
    assert_eq!(seq_count, 2);
}

#[test]
fn test_parse_ccsds_header_with_secondary_header_flag() {
    // Secondary Header Flag = 1
    let packet: [u8; 10] = [
        0x0B, 0x30, // Version=0, Type=0, SecHdr=1, APID=0x330
        0xC0, 0x03, // SeqFlags=3, SeqCount=3
        0x00, 0x03, // Data Length = 3
        0x30, 0x00, 0x04, 0x01,
    ];

    let result = parse_ccsds_header(&packet);
    // Parser should still work even with secondary header flag
    assert!(result.is_ok());
}

#[test]
fn test_parse_ccsds_header_max_apid() {
    // Maximum APID (0x7FF = 2047)
    let packet: [u8; 10] = [
        0x07, 0xFF, // APID = 0x7FF
        0xC0, 0x00, // SeqFlags=3, SeqCount=0
        0x00, 0x03, // Data Length = 3
        0x30, 0x00, 0x04, 0x01,
    ];

    let result = parse_ccsds_header(&packet);
    assert!(result.is_ok());

    let (apid, _, _) = result.unwrap();
    assert_eq!(apid, 0x7FF);
}

#[test]
fn test_parse_ccsds_header_zero_length_data() {
    // Data Length = 0 means 1 byte of data
    let packet: [u8; 7] = [
        0x03, 0x30, // APID 0x330
        0xC0, 0x00, // SeqCount 0
        0x00, 0x00, // Data Length = 0 (means 1 byte)
        0x30, // 1 byte of data
    ];

    let result = parse_ccsds_header(&packet);
    assert!(result.is_ok());

    let (_, _, data) = result.unwrap();
    assert_eq!(data.len(), 1);
}

// =============================================================================
// category_from_apid Tests
// =============================================================================

#[test]
fn test_category_from_apid_cat048() {
    // APID 0x330 with base 0x300 should give category 48
    let category = category_from_apid(0x330, 0x300);
    assert_eq!(category, 48);
}

#[test]
fn test_category_from_apid_cat062() {
    // APID 0x33E with base 0x300 should give category 62
    let category = category_from_apid(0x33E, 0x300);
    assert_eq!(category, 62);
}

#[test]
fn test_category_from_apid_cat001() {
    // APID 0x301 with base 0x300 should give category 1
    let category = category_from_apid(0x301, 0x300);
    assert_eq!(category, 1);
}

#[test]
fn test_category_from_apid_base_apid() {
    // APID equals base_apid should give category 0 (mixed categories)
    let category = category_from_apid(0x300, 0x300);
    assert_eq!(category, 0);
}

#[test]
fn test_category_from_apid_different_base() {
    // APID 0x448 with base 0x400 should give category 72
    let category = category_from_apid(0x448, 0x400);
    assert_eq!(category, 72);
}

#[test]
fn test_category_from_apid_max_category() {
    // APID 0x3FF with base 0x300 should give category 255
    let category = category_from_apid(0x3FF, 0x300);
    assert_eq!(category, 255);
}

#[test]
fn test_category_from_apid_out_of_range() {
    // APID below base range returns 0
    let category = category_from_apid(0x200, 0x300);
    assert_eq!(category, 0);

    // APID above base + 256 range returns 0
    let category = category_from_apid(0x500, 0x300);
    assert_eq!(category, 0);
}

// =============================================================================
// CcsdsSample Tests
// =============================================================================

#[test]
fn test_ccsds_sample_creation() {
    use asterix::transport::ccsds::CcsdsSample;

    let sample = CcsdsSample {
        category: 48,
        apid: 0x330,
        sequence_count: 42,
        data: vec![0x30, 0x00, 0x04],
        timestamp: 1234567890,
        packet_type: CcsdsMode::Telemetry,
    };

    assert_eq!(sample.apid, 0x330);
    assert_eq!(sample.sequence_count, 42);
    assert_eq!(sample.timestamp, 1234567890);
    assert_eq!(sample.data.len(), 3);
    assert_eq!(sample.category, 48);
    assert!(matches!(sample.packet_type, CcsdsMode::Telemetry));
}

#[test]
fn test_ccsds_sample_clone() {
    use asterix::transport::ccsds::CcsdsSample;

    let sample = CcsdsSample {
        category: 62,
        apid: 0x33E,
        sequence_count: 1,
        data: vec![1, 2, 3],
        timestamp: 100,
        packet_type: CcsdsMode::Telecommand,
    };

    let cloned = sample.clone();
    assert_eq!(cloned.apid, sample.apid);
    assert_eq!(cloned.data, sample.data);
    assert_eq!(cloned.category, 62);
    assert!(matches!(cloned.packet_type, CcsdsMode::Telecommand));
}

#[test]
fn test_ccsds_sample_debug() {
    use asterix::transport::ccsds::CcsdsSample;

    let sample = CcsdsSample {
        category: 48,
        apid: 0x330,
        sequence_count: 1,
        data: vec![1, 2, 3],
        timestamp: 100,
        packet_type: CcsdsMode::Telemetry,
    };

    let debug_str = format!("{:?}", sample);
    assert!(debug_str.contains("CcsdsSample"));
    assert!(debug_str.contains("816")); // 0x330 = 816
}
