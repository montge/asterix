/*
 *  Copyright (c) 2013 Croatia Control Ltd. (www.crocontrol.hr)
 *
 *  This file is part of Asterix.
 *
 *  Asterix is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Asterix is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Asterix.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * AUTHORS: Damir Salantic, Croatia Control Ltd.
 *
 */

/**
 * @file asterixhdlcparsing.h
 * @brief HDLC (High-Level Data Link Control) frame parser for ASTERIX data
 *
 * This file provides functionality for extracting ASTERIX data blocks from
 * HDLC-framed transport layer encapsulation. HDLC framing is commonly used
 * for serial line communication and some network protocols in ATM/ATC systems.
 *
 * @par HDLC Frame Structure
 * The HDLC frame encapsulates ASTERIX data with the following structure:
 * @code
 * +--------+--------+------------------+--------+--------+
 * | ADDR   | CTRL   | ASTERIX DATA     | FCS_H  | FCS_L  |
 * | (1B)   | (1B)   | (N bytes)        | (1B)   | (1B)   |
 * +--------+--------+------------------+--------+--------+
 * | 0x01   | 0x03   | ASTERIX records  | CRC-16 CCITT    |
 * +--------+--------+------------------+--------+--------+
 * @endcode
 *
 * - **Address field (1 byte)**: Fixed to 0x01
 * - **Control field (1 byte)**: Fixed to 0x03 (Unnumbered Information frame)
 * - **Data field (variable)**: One or more ASTERIX data records
 * - **Frame Check Sequence (2 bytes)**: CRC-16-CCITT for error detection
 *
 * @par ASTERIX Encapsulation
 * The data field contains one or more ASTERIX records, each with:
 * @code
 * [CAT (1B)] [LEN_H (1B)] [LEN_L (1B)] [PAYLOAD (N bytes)]
 * @endcode
 *
 * Example multi-record HDLC frame:
 * @code
 * 0x01 0x03 [CAT=048] [LEN=0x0020] [ASTERIX DATA...]
 *           [CAT=062] [LEN=0x0015] [ASTERIX DATA...]
 *           [FCS_H] [FCS_L]
 * @endcode
 *
 * @par Bit Stuffing
 * HDLC uses bit stuffing to prevent flag sequence (0x7E) from appearing in data:
 * - After five consecutive 1-bits in data, a 0-bit is inserted
 * - Receiver removes stuffed bits during decoding
 * - This implementation assumes pre-destuffed data from hardware/driver
 *
 * @par Error Detection
 * CRC-16-CCITT (polynomial 0x1021) is used for error detection:
 * - **Initialization**: 0xFFFF
 * - **Final XOR**: 0xF0B8 (expected good FCS value)
 * - **Detection**: Single-bit errors, burst errors up to 16 bits
 *
 * @par Circular Buffer Architecture
 * Uses a 64KB circular buffer for continuous stream processing:
 * - Handles fragmented frames across multiple read operations
 * - Detects frame boundaries using address/control field pattern matching
 * - Validates frames using FCS before extraction
 *
 * @par Usage Example
 * @code
 * // Initialize buffers (done internally)
 * unsigned char data[256];
 * int bytes_read = read_serial_port(data, sizeof(data));
 *
 * // Copy to circular buffer
 * copy_to_cbuf(data, bytes_read);
 *
 * // Extract HDLC frames and parse ASTERIX
 * int frame_len = 0;
 * unsigned char* asterix_data = get_next_hdlc_frame(&frame_len);
 * if (asterix_data) {
 *     // asterix_data points to ASTERIX payload (without HDLC header/FCS)
 *     // frame_len is the ASTERIX data length
 *     parse_asterix(asterix_data, frame_len);
 * }
 * @endcode
 *
 * @par Thread Safety
 * This module is NOT thread-safe. Global state (circular buffer, indices)
 * must not be accessed concurrently from multiple threads.
 *
 * @par Performance Considerations
 * - Circular buffer minimizes memory allocations
 * - Pattern matching uses bitmask for efficient frame detection
 * - FCS validation uses pre-computed lookup table (256 entries)
 *
 * @see asterixhdlcsubformat.cpp For HDLC input format handler
 * @see InputParser For generic ASTERIX parsing interface
 * @see DataBlock For parsed ASTERIX data representation
 *
 * @note Based on AtxRecorder.cpp by Hrvoje Kalinovcic, Croatia Control Ltd.
 */

#ifndef ASTERIXHDLCPARSING_H_
#define ASTERIXHDLCPARSING_H_

/**
 * @defgroup HDLCBufferSizes HDLC Buffer Size Constants
 * @brief Buffer size definitions for HDLC frame processing
 * @{
 */

/**
 * @def MAX_RXBUF
 * @brief Maximum size of receive buffer for raw input data
 *
 * 4KB buffer for reading data from serial port or network stream.
 * This is the staging buffer before data is copied to circular buffer.
 */
#define MAX_RXBUF 0x1000                        // 4K

/**
 * @def MAX_CBUF
 * @brief Maximum size of circular buffer for frame assembly
 *
 * 64KB circular buffer for storing incoming data and assembling HDLC frames.
 * Large enough to handle multiple fragmented frames and network jitter.
 */
#define MAX_CBUF  0x10000                       // 64K

/**
 * @def MAX_FRM
 * @brief Maximum size of a single HDLC frame
 *
 * 512 bytes (2 * 256) maximum frame size after extraction from circular buffer.
 * Sufficient for typical ASTERIX data blocks in HDLC encapsulation.
 */
#define MAX_FRM   0x200                         // 2*256 byte

/** @} */ // end of HDLCBufferSizes group

/**
 * @defgroup HDLCGlobalData HDLC Global Data Structures
 * @brief Global buffers and state for HDLC parsing
 *
 * These global variables maintain the state of the HDLC parser across
 * multiple invocations. The module uses a circular buffer architecture
 * to handle streaming data from serial ports or network connections.
 *
 * @warning Not thread-safe. Do not access from multiple threads.
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @var RxBuf
 * @brief Receive buffer for raw input data from serial port or network
 *
 * Staging buffer of MAX_RXBUF (4KB) size used to temporarily hold data
 * read from input source before copying to circular buffer.
 *
 * @see MAX_RXBUF
 * @see copy_to_cbuf
 */
extern unsigned char RxBuf[];

/** @} */ // end of HDLCGlobalData group

/**
 * @defgroup HDLCParsing HDLC Frame Parsing Functions
 * @brief Functions for extracting and validating HDLC frames
 * @{
 */

/**
 * @brief Copy received bytes from RxBuf to internal circular buffer
 *
 * Copies data from the receive buffer to the internal 64KB circular buffer
 * (cBuf) for frame assembly. This function handles wrap-around of the
 * circular buffer automatically.
 *
 * @param[in] RxBuf Pointer to receive buffer containing raw input data
 * @param[in] Cnt Number of bytes to copy from RxBuf
 *
 * @return Number of bytes copied (equal to Cnt)
 *
 * @par Example
 * @code
 * unsigned char data[256];
 * int bytes_read = read(serial_fd, data, sizeof(data));
 * if (bytes_read > 0) {
 *     copy_to_cbuf(data, bytes_read);
 * }
 * @endcode
 *
 * @note This function updates internal circular buffer index (iBuf)
 * @see MAX_CBUF
 * @see get_next_hdlc_frame
 */
extern int copy_to_cbuf(unsigned char *RxBuf, int Cnt);

/**
 * @brief Extract HDLC frame from circular buffer starting at given index
 *
 * Attempts to extract a complete HDLC frame from the circular buffer,
 * validating the Frame Check Sequence (FCS/CRC). Searches for the end
 * of the frame by testing FCS at each position.
 *
 * @param[in] iF Index of frame start in circular buffer
 * @param[in] MinLen Minimum expected frame length (typically MIN_BLK=10)
 *
 * @return Length of valid HDLC frame if found, 0 otherwise
 * @retval 0 Frame incomplete or invalid FCS
 * @retval >0 Valid frame length including header and FCS
 *
 * @note Internal function, typically called by get_next_hdlc_frame()
 * @see test_hdlc_fcs
 * @see get_next_hdlc_frame
 */
extern int get_hdlc_frame(int iF, int MinLen);

/**
 * @brief Test HDLC Frame Check Sequence (FCS) for data integrity
 *
 * Validates the CRC-16-CCITT checksum of an HDLC frame. Uses lookup table
 * for efficient computation. The FCS is calculated over the entire frame
 * including address, control, data, and FCS fields.
 *
 * @param[in] iF Start index in Frame buffer
 * @param[in] iL End index in Frame buffer (inclusive)
 *
 * @return FCS validation result
 * @retval 0x0000 Valid frame (FCS matches expected value FCS_GOOD=0xF0B8)
 * @retval non-zero Invalid frame (FCS mismatch indicates transmission error)
 *
 * @par CRC-16-CCITT Parameters
 * - **Polynomial**: 0x1021
 * - **Initial value**: 0xFFFF (FCS_INIT)
 * - **Final XOR**: 0xF0B8 (FCS_GOOD)
 * - **Lookup table**: 256-entry pre-computed table (FCStab)
 *
 * @par Example
 * @code
 * // After extracting frame to Frame[] buffer
 * if (test_hdlc_fcs(0, frame_length - 1) == 0) {
 *     // Frame is valid, FCS check passed
 *     process_frame();
 * } else {
 *     // Frame corrupted, discard
 * }
 * @endcode
 *
 * @note Uses global Frame[] buffer for data
 * @see get_hdlc_frame
 */
extern unsigned short test_hdlc_fcs(int iF, int iL);

/**
 * @brief Get next valid HDLC frame from circular buffer
 *
 * Main entry point for HDLC frame extraction. Searches for HDLC frame
 * boundaries in the circular buffer, validates FCS, and returns pointer
 * to ASTERIX payload (without HDLC header and FCS).
 *
 * This function performs the following steps:
 * 1. Complete any partially-received frame from previous call
 * 2. Search for new frame start using address/control field pattern (0x01 0x03)
 * 3. Validate frame using FCS check
 * 4. Return pointer to ASTERIX data (skipping HDLC header/FCS)
 *
 * @param[out] len Pointer to integer receiving ASTERIX data length
 *                 (frame length minus 4 bytes for header and FCS)
 *
 * @return Pointer to ASTERIX payload data, or nullptr if no complete frame available
 * @retval non-nullptr Pointer to ASTERIX data in Frame buffer (&Frame[2])
 * @retval nullptr No complete valid frame available yet
 *
 * @par Usage Pattern
 * @code
 * // Read data from serial port
 * int bytes = read(fd, data, sizeof(data));
 * copy_to_cbuf(data, bytes);
 *
 * // Extract all available frames
 * int frame_len;
 * unsigned char* asterix_data;
 * while ((asterix_data = get_next_hdlc_frame(&frame_len)) != nullptr) {
 *     // Process ASTERIX data
 *     parse_asterix_records(asterix_data, frame_len);
 * }
 * @endcode
 *
 * @warning The returned pointer is valid only until the next call to
 *          get_next_hdlc_frame(). Copy data if needed for later use.
 *
 * @note Updates internal error counters (nFrmOK, nFrmErr, nBytesErr)
 * @see copy_to_cbuf
 * @see get_hdlc_frame
 * @see test_hdlc_fcs
 */
extern unsigned char *get_next_hdlc_frame(int *len);

/**
 * @brief Get and reset count of failed bytes since last call
 *
 * Returns the cumulative count of bytes that failed HDLC validation
 * (corrupt frames, invalid FCS) since the last call to this function,
 * then resets the counter to zero.
 *
 * @return Number of failed bytes since last call
 *
 * @par Use Case
 * Useful for monitoring link quality and detecting transmission errors:
 * @code
 * int failed = GetAndResetFailedBytes();
 * if (failed > 0) {
 *     fprintf(stderr, "Warning: %d bytes corrupted\n", failed);
 * }
 * @endcode
 *
 * @note This is a stateful function - counter is reset after each call
 * @see get_next_hdlc_frame
 */
extern int GetAndResetFailedBytes();

/** @} */ // end of HDLCParsing group

#ifdef __cplusplus
}
#endif

#endif /* ASTERIXHDLCPARSING_H_ */
