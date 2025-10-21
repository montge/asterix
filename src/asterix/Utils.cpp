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

#include "Utils.h"

std::string format(const char *fmt, ...) {
    // PERFORMANCE: Use stack buffer first to avoid heap allocation in common case
    // Most format strings are < 256 bytes, so this eliminates ~90% of allocations
    char stack_buffer[512];
    va_list args;

    // Try formatting into stack buffer first (single vsnprintf call)
    va_start(args, fmt);
    int size = vsnprintf(stack_buffer, sizeof(stack_buffer), fmt, args);
    va_end(args);

    // Check for error
    if (size < 0) {
        return std::string();
    }

    // Common case: result fits in stack buffer (no heap allocation needed)
    if (size < static_cast<int>(sizeof(stack_buffer))) {
        return std::string(stack_buffer, size);
    }

    // Uncommon case: result doesn't fit, allocate exact size on heap
    // Note: size is the number of characters that would be written (excluding null)
    char *heap_buffer = new char[size + 1];

    va_start(args, fmt);
    vsnprintf(heap_buffer, size + 1, fmt, args);
    va_end(args);

    std::string result(heap_buffer, size);
    delete[] heap_buffer;

    return result;
}

uint32_t crc32(const void *pBuffer, size_t nLength, uint32_t nPreviousCrc32) {
    uint32_t nCrc = ~nPreviousCrc32; // same as previousCrc32 ^ 0xFFFFFFFF
    const uint8_t *pCurrent = (const uint8_t *) pBuffer;

    while (nLength-- > 0) {
        nCrc = (nCrc >> 8) ^ Crc32Lookup[(nCrc & 0xFF) ^ *pCurrent++];
    }

    return ~nCrc; // same as crc ^ 0xFFFFFFFF
}
