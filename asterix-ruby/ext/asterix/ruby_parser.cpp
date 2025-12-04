/*
 *  Copyright (c) 2025 ASTERIX Contributors
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
 */

#define LOGDEBUG(cond, ...)
#define LOGERROR(cond, ...)

#ifdef _WIN32
  #include "../../src/engine/win32_compat.h"
#endif

#include "ruby_parser.h"
#include "../../src/asterix/AsterixDefinition.h"
#include "../../src/asterix/XMLParser.h"
#include "../../src/asterix/InputParser.h"
#include "../../src/asterix/Tracer.h"

#ifdef _WIN32
  #include <time.h>
#else
  #include <sys/time.h>
#endif

static AsterixDefinition *pDefinition = NULL;
static InputParser *inputParser = NULL;
bool gFiltering = false;
bool gSynchronous = false;
const char *gAsterixDefinitionsFile = NULL;
bool gVerbose = false;
bool gForceRouting = false;
int gHeartbeat = 0;

static void debug_trace(char const *format, ...) {
    // Error tracing callback for Asterix library
    // Errors are set via rb_raise instead
}

/*
 * Initialize Asterix with XML configuration file
 */
extern "C" int ruby_init_asterix(const char *xml_config_file) {
    try {
        Tracer::Configure(debug_trace);

        if (!pDefinition)
            pDefinition = new AsterixDefinition();

        if (!inputParser)
            inputParser = new InputParser(pDefinition);

        FILE *fp = fopen(xml_config_file, "rt");
        if (!fp) {
            rb_raise(rb_eIOError, "Configuration file not found: %s", xml_config_file);
            return -1;
        }

        // parse format file
        XMLParser Parser;
        if (!Parser.Parse(fp, pDefinition, xml_config_file)) {
            fclose(fp);
            rb_raise(rb_eSyntaxError, "Failed to parse XML configuration file: %s", xml_config_file);
            return -2;
        }
        fclose(fp);
        return 0;

    } catch (const std::bad_alloc& e) {
        rb_raise(rb_eNoMemError, "Out of memory during initialization");
        return -3;
    } catch (const std::exception& e) {
        rb_raise(rb_eRuntimeError, "C++ exception during initialization: %s", e.what());
        return -4;
    } catch (...) {
        rb_raise(rb_eRuntimeError, "Unknown C++ exception during initialization");
        return -5;
    }
}

extern "C" VALUE ruby_parse(const unsigned char *pBuf, size_t len, int verbose) {
    try {
        // get current timestamp in ms since epoch
        struct timeval tp;
        gettimeofday(&tp, NULL);
        unsigned long nTimestamp = tp.tv_sec * 1000 + tp.tv_usec / 1000;

        if (inputParser) {
            AsterixData *pData = inputParser->parsePacket(pBuf, len, nTimestamp);
            if (pData) {
                // Convert to Ruby format - AsterixData has a getRubyData method
                VALUE rb_result = pData->getRubyData(verbose);
                delete pData;
                return rb_result;
            }
        }
        return Qnil;

    } catch (const std::bad_alloc& e) {
        rb_raise(rb_eNoMemError, "Out of memory during parsing");
        return Qnil;
    } catch (const std::exception& e) {
        rb_raise(rb_eRuntimeError, "C++ exception during parsing: %s", e.what());
        return Qnil;
    } catch (...) {
        rb_raise(rb_eRuntimeError, "Unknown C++ exception during parsing");
        return Qnil;
    }
}

extern "C" VALUE ruby_parse_with_offset(const unsigned char *pBuf, size_t len,
                                        unsigned int offset, unsigned int blocks_count, int verbose) {
    try {
        // Bounds checking at parser level (defense-in-depth)
        if (offset >= len) {
            // Return empty array and original offset
            VALUE rb_empty = rb_ary_new();
            VALUE rb_offset = UINT2NUM(offset);
            return rb_ary_new_from_args(2, rb_empty, rb_offset);
        }

        // get current timestamp in ms since epoch
        struct timeval tp;
        gettimeofday(&tp, NULL);
        unsigned long nTimestamp = tp.tv_sec * 1000 + tp.tv_usec / 1000;

        if (inputParser) {
            AsterixData *pData = new AsterixData();
            unsigned int m_nPos = offset;
            unsigned int current_blocks_count = 0;

            while (m_nPos < len && current_blocks_count < blocks_count) {
                // Explicit bounds check before subtraction
                if (m_nPos >= len) {
                    break;  // Prevent underflow
                }

                unsigned int m_nDataLength = len - m_nPos;

                // Ensure we have minimum data to parse (at least 3 bytes for ASTERIX header)
                while (m_nDataLength > 3 && current_blocks_count < blocks_count) {
                    // Validate pointer arithmetic before use
                    if (m_nPos >= len) {
                        break;  // Prevent buffer overflow
                    }

                    const unsigned char *pBuf_offset = (pBuf + m_nPos);
                    DataBlock *block = inputParser->parse_next_data_block(
                            pBuf_offset, m_nPos, len, nTimestamp, m_nDataLength);
                    if (block) {
                        pData->m_lDataBlocks.push_back(block);
                        current_blocks_count++;
                    }
                }
            }

            if (pData) {
                VALUE rb_result = pData->getRubyData(verbose);
                delete pData;
                VALUE rb_offset = UINT2NUM(m_nPos);
                VALUE rb_output = rb_ary_new_from_args(2, rb_result, rb_offset);
                return rb_output;
            }
        }

        // Return empty array and original offset on failure
        VALUE rb_empty = rb_ary_new();
        VALUE rb_offset = UINT2NUM(offset);
        return rb_ary_new_from_args(2, rb_empty, rb_offset);

    } catch (const std::bad_alloc& e) {
        rb_raise(rb_eNoMemError, "Out of memory during parsing with offset");
        return Qnil;
    } catch (const std::exception& e) {
        rb_raise(rb_eRuntimeError, "C++ exception during parsing with offset: %s", e.what());
        return Qnil;
    } catch (...) {
        rb_raise(rb_eRuntimeError, "Unknown C++ exception during parsing with offset");
        return Qnil;
    }
}

extern "C" VALUE ruby_describe(int category, const char *item, const char *field, const char *value) {
    try {
        if (!pDefinition)
            return rb_str_new_cstr("Not initialized");

        const char *description = pDefinition->getDescription(category, item, field, value);
        if (description == NULL)
            return rb_str_new_cstr("");
        return rb_str_new_cstr(description);

    } catch (const std::exception& e) {
        rb_raise(rb_eRuntimeError, "C++ exception in describe: %s", e.what());
        return Qnil;
    } catch (...) {
        rb_raise(rb_eRuntimeError, "Unknown C++ exception in describe");
        return Qnil;
    }
}
