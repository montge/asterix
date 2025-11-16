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
 * @brief HDLC frame parsing for serial ASTERIX data streams
 *
 * This header provides low-level HDLC (High-Level Data Link Control) frame
 * extraction from serial communication streams containing ASTERIX data.
 * HDLC is a bit-oriented synchronous data link layer protocol commonly used
 * in aviation surveillance systems for reliable serial data transmission.
 *
 * ## Protocol Background
 *
 * HDLC frames encapsulate ASTERIX data for transmission over serial links:
 * - **Frame structure**: FLAG (0x7E) + Data + FCS (16-bit CRC) + FLAG
 * - **Bit stuffing**: Prevents 0x7E appearing in data (transparency)
 * - **FCS validation**: CRC-16 checksum for error detection
 * - **Frame recovery**: Resynchronization on framing errors
 *
 * ## Parsing Pipeline
 *
 * ```
 * Serial Stream → RxBuf (4KB) → Circular Buffer (64KB) → Frame Extraction → FCS Check → ASTERIX Data
 * ```
 *
 * 1. Raw bytes read into RxBuf (4KB receive buffer)
 * 2. Data copied to circular buffer (64KB) via copy_to_cbuf()
 * 3. HDLC frames extracted via get_hdlc_frame() or get_next_hdlc_frame()
 * 4. FCS validated with test_hdlc_fcs()
 * 5. Valid ASTERIX data extracted from frame payload
 *
 * ## Buffer Management
 *
 * - **RxBuf**: 4KB temporary buffer for serial port reads
 * - **Circular Buffer**: 64KB internal buffer for frame assembly
 * - **Frame Buffer**: 512-byte max frame size (MAX_FRM)
 *
 * ## Usage Example
 *
 * ```c
 * #include "asterixhdlcparsing.h"
 *
 * // Read from serial port into RxBuf
 * int bytes_read = read(serial_fd, RxBuf, MAX_RXBUF);
 *
 * // Copy to circular buffer for processing
 * copy_to_cbuf(RxBuf, bytes_read);
 *
 * // Extract HDLC frame
 * int frame_length;
 * unsigned char* frame = get_next_hdlc_frame(&frame_length);
 * if (frame) {
 *     // Validate FCS checksum
 *     if (test_hdlc_fcs(0, frame_length) == 0) {
 *         // Frame valid - extract ASTERIX data (exclude FCS bytes)
 *         unsigned char* asterix_data = frame;
 *         int asterix_length = frame_length - 2;  // Subtract 2-byte FCS
 *         // Parse ASTERIX data...
 *     } else {
 *         // FCS checksum failed
 *         int failed = GetAndResetFailedBytes();
 *     }
 * }
 * ```
 *
 * ## Error Recovery
 *
 * - Failed FCS validation increments error counter (GetAndResetFailedBytes)
 * - Frame desynchronization triggers automatic resynchronization
 * - Invalid frames are discarded with statistics tracking
 *
 * ## Thread Safety
 *
 * **NOT thread-safe** - uses global buffers (RxBuf, internal circular buffer).
 * Concurrent calls will corrupt state. Use external locking or dedicated
 * instances per thread.
 *
 * ## Security Considerations
 *
 * - **Buffer overflow protection**: All buffers have fixed size limits
 * - **FCS validation required**: Always check test_hdlc_fcs() before trusting data
 * - **Frame length limits**: MAX_FRM (512 bytes) prevents oversized frames
 * - **Malformed frame handling**: Parser discards invalid frames gracefully
 *
 * @note C-compatible API (extern "C" linkage)
 * @note Not thread-safe - global buffer state
 * @note Typical use case: Serial port ASTERIX receivers (radar, ADS-B)
 * @note FCS is CRC-16 (ITU-T recommendation)
 *
 * @see asterixhdlcsubformat (high-level HDLC format handler)
 * @see SerialDevice (serial port input device)
 */

#ifndef ASTERIXHDLCPARSING_H_
#define ASTERIXHDLCPARSING_H_

// ============================================================================
// Buffer Size Configuration
// ============================================================================

#define MAX_RXBUF 0x1000                        //!< Receive buffer size (4KB) for serial port reads
//#define MAX_RXBUF 90 //test
#define MAX_CBUF  0x10000                       //!< Circular buffer size (64KB) for frame assembly
#define MAX_FRM   0x200                         //!< Maximum frame size (512 bytes = 2*256)

// ============================================================================
// C API Functions
// ============================================================================

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Global receive buffer for serial port data
 *
 * Temporary buffer for reading data from serial port (COM/UART) before
 * copying to internal circular buffer. Size: MAX_RXBUF (4KB).
 *
 * @note Global state - not thread-safe
 * @warning Do not write beyond MAX_RXBUF bytes
 */
extern unsigned char RxBuf[];

/**
 * @brief Copy received data to internal circular buffer
 *
 * Transfers data from RxBuf to the internal circular buffer (64KB) used
 * for HDLC frame extraction. Handles buffer wraparound automatically.
 *
 * @param RxBuf Pointer to receive buffer containing raw serial data
 * @param Cnt Number of bytes to copy (must be <= MAX_RXBUF)
 * @return Number of bytes successfully copied
 *
 * @note This updates internal circular buffer write pointer
 * @warning Not thread-safe - modifies global buffer state
 */
extern int copy_to_cbuf(unsigned char *RxBuf, int Cnt);

/**
 * @brief Extract HDLC frame from circular buffer
 *
 * Searches circular buffer for valid HDLC frame starting at position iF
 * with minimum payload length MinLen. Handles frame flag detection and
 * bit unstuffing.
 *
 * @param iF Starting position in circular buffer
 * @param MinLen Minimum expected payload length (bytes)
 * @return Frame length if found, negative value on error
 *
 * @note Advances internal read pointer on success
 */
extern int get_hdlc_frame(int iF, int MinLen);

/**
 * @brief Validate HDLC frame FCS (Frame Check Sequence)
 *
 * Computes CRC-16 checksum over frame data and compares with FCS bytes
 * at end of frame. Uses ITU-T CRC-16 polynomial (x^16 + x^12 + x^5 + 1).
 *
 * @param iF Starting position of frame in buffer
 * @param iL Length of frame including FCS (last 2 bytes)
 * @return 0 if FCS valid, non-zero if checksum mismatch
 *
 * @note Always validate FCS before trusting frame contents
 * @note Frame corruption or bit errors will fail FCS check
 */
extern unsigned short test_hdlc_fcs(int iF, int iL);

/**
 * @brief Get next complete HDLC frame from circular buffer
 *
 * High-level function that combines frame extraction and validation.
 * Returns pointer to next valid HDLC frame with FCS validated.
 *
 * @param len Output parameter - receives frame length (including FCS)
 * @return Pointer to frame data, or NULL if no valid frame available
 *
 * @note Returned pointer is valid until next call to get_next_hdlc_frame()
 * @note Subtract 2 from *len to get ASTERIX payload length (exclude FCS)
 * @note Automatically advances to next frame in circular buffer
 */
extern unsigned char *get_next_hdlc_frame(int *len);

/**
 * @brief Get count of failed bytes and reset counter
 *
 * Returns the number of bytes discarded due to FCS validation failures
 * or framing errors since last call. Resets counter to zero.
 *
 * @return Number of bytes failed since last reset
 *
 * @note Useful for monitoring link quality and error rates
 * @note High failure count indicates noisy serial link or baud rate mismatch
 */
extern int GetAndResetFailedBytes();

#ifdef __cplusplus
}
#endif

#endif /* ASTERIXHDLCPARSING_H_ */
