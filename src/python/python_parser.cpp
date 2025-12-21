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

#define LOGDEBUG(cond, ...)
#define LOGERROR(cond, ...)

#ifdef _WIN32
  #include "../engine/win32_compat.h"
#endif

#include "python_parser.h"
#include "AsterixDefinition.h"
#include "XMLParser.h"
#include "InputParser.h"
#include <string>

#ifdef _WIN32
  #include <time.h>
#else
  #include <sys/time.h>
#endif

static AsterixDefinition *pDefinition = nullptr;
static InputParser *inputParser = nullptr;
bool gFiltering = false;
bool gSynchronous = false;
const char *gAsterixDefinitionsFile = nullptr;
bool gVerbose = false;
bool gForceRouting = false;
int gHeartbeat = 0;

static void debug_trace(char const *format, ...) {
    /* TODO
    char buffer[1024];
    va_list args;
    va_start (args, format);
    vsnprintf (buffer, 1023, format, args);
    va_end (args);
    strcat(buffer, "\n");
    LOGERROR(1, "%s", buffer); // TODO
    */
    // TODO PyErr_SetString(PyExc_RuntimeError, buffer);
}

/*
 * Initialize Asterix Python with XML configuration file
 */
int python_init(const char *xml_config_file) {
    // MEDIUM-005 FIX: Add exception handling to prevent crashes
    try {
        Tracer::Configure(debug_trace);

        if (!pDefinition)
            pDefinition = new AsterixDefinition();

        if (!inputParser)
            inputParser = new InputParser(pDefinition);

        FILE *fp = fopen(xml_config_file, "rt");
        if (!fp) {
            PyErr_SetString(PyExc_IOError, "Input file not found.");
            return -1;
        }
        // parse format file
        XMLParser Parser;
        if (!Parser.Parse(fp, pDefinition, xml_config_file)) {
            fclose(fp);
            // XMLParser.Parse() already sets PyErr via Tracer, don't override it
            return -2;
        }
        fclose(fp);
        return 0;

    } catch (const std::bad_alloc& e) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory during initialization.");
        return -3;
    } catch (const std::exception& e) {
        std::string error_msg = std::string("C++ exception during initialization: ") + e.what();
        PyErr_SetString(PyExc_RuntimeError, error_msg.c_str());
        return -4;
    } catch (...) {
        PyErr_SetString(PyExc_RuntimeError, "Unknown C++ exception during initialization.");
        return -5;
    }
}

PyObject *python_parse(const unsigned char *pBuf, Py_ssize_t len, int verbose, int strict) {
    // MEDIUM-005 FIX: Add exception handling to prevent crashes
    try {
        // get current timstamp in ms since epoch
        struct timeval tp;
        gettimeofday(&tp, nullptr);
        unsigned long nTimestamp = tp.tv_sec * 1000 + tp.tv_usec / 1000;

        if (inputParser) {
            AsterixData *pData = inputParser->parsePacket(pBuf, len, nTimestamp);
            if (pData) {
                // Check for parse errors in strict mode
                if (strict) {
                    // Check if any records had parsing errors
                    for (auto* block : pData->m_lDataBlocks) {
                        if (block) {
                            for (auto* record : block->m_lDataRecords) {
                                if (record && !record->m_bFormatOK) {
                                    std::string error_msg = "Parse error in CAT" +
                                        std::to_string(block->m_pCategory ? block->m_pCategory->m_id : 0) +
                                        ": malformed record";
                                    delete pData;
                                    PyErr_SetString(PyExc_RuntimeError, error_msg.c_str());
                                    return nullptr;
                                }
                            }
                        }
                    }
                }
                // convert to Python format
                PyObject *lst = pData->getData(verbose);
                delete pData;
                return lst;
            } else if (strict) {
                // No data returned - could indicate parse failure
                PyErr_SetString(PyExc_RuntimeError, "Parse error: no valid ASTERIX data found");
                return nullptr;
            }
        }
        return nullptr;

    } catch (const std::bad_alloc& e) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory during parsing.");
        return nullptr;
    } catch (const std::exception& e) {
        std::string error_msg = std::string("C++ exception during parsing: ") + e.what();
        PyErr_SetString(PyExc_RuntimeError, error_msg.c_str());
        return nullptr;
    } catch (...) {
        PyErr_SetString(PyExc_RuntimeError, "Unknown C++ exception during parsing.");
        return nullptr;
    }
}

PyObject *
python_parse_with_offset(const unsigned char *pBuf, Py_ssize_t len, unsigned int offset, unsigned int blocks_count,
                         int verbose, int strict)
/* AUTHOR: Krzysztof Rutkowski, ICM UW, krutk@icm.edu.pl
*/
{
    // MEDIUM-005 FIX: Add exception handling to prevent crashes
    try {
        // CRITICAL-002 FIX: Additional bounds checking at parser level
        // Note: Validation already done in python_wrapper.cpp, but defense-in-depth
        if (offset >= len) {
            return nullptr;  // Return empty result for out-of-bounds offset
        }

        // get current timstamp in ms since epoch
        struct timeval tp;
        gettimeofday(&tp, nullptr);
        unsigned long nTimestamp = tp.tv_sec * 1000 + tp.tv_usec / 1000;

        if (inputParser) {
            AsterixData *pData = new AsterixData();
            unsigned int m_nPos = offset;
            unsigned int current_blocks_count = 0;
            while (m_nPos < len && current_blocks_count < blocks_count) {
                // CRITICAL-002 FIX: Explicit bounds check before subtraction
                if (m_nPos >= len) {
                    break;  // Prevent underflow
                }

                unsigned int m_nDataLength = len - m_nPos;

                // Ensure we have minimum data to parse (at least 3 bytes for ASTERIX header)
                while (m_nDataLength > 3 && current_blocks_count < blocks_count) {
                    // CRITICAL-002 FIX: Validate pointer arithmetic before use
                    if (m_nPos >= len) {
                        break;  // Prevent buffer overflow
                    }

                    const unsigned char *pBuf_offset = (pBuf + m_nPos);
                    DataBlock *block = inputParser->parse_next_data_block(
                            pBuf_offset, m_nPos, len, nTimestamp, m_nDataLength);
                    if (block) {
                        // Check for parse errors in strict mode
                        if (strict) {
                            for (auto* record : block->m_lDataRecords) {
                                if (record && !record->m_bFormatOK) {
                                    std::string error_msg = "Parse error in CAT" +
                                        std::to_string(block->m_pCategory ? block->m_pCategory->m_id : 0) +
                                        ": malformed record at offset " + std::to_string(m_nPos);
                                    delete pData;
                                    PyErr_SetString(PyExc_RuntimeError, error_msg.c_str());
                                    return nullptr;
                                }
                            }
                        }
                        pData->m_lDataBlocks.push_back(block);
                        current_blocks_count++;
                    }
                }
            }
            if (pData) { // convert to Python format
                PyObject *lst = pData->getData(verbose);
                delete pData;
                PyObject *py_m_nPos = Py_BuildValue("l", m_nPos);
                PyObject *py_output = PyTuple_Pack(2, lst, py_m_nPos);
                // Decrease references since the tuple holds references to them now
                Py_DECREF(lst);
                Py_DECREF(py_m_nPos);
                return py_output;
            }
        }
        return nullptr;

    } catch (const std::bad_alloc& e) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory during parsing with offset.");
        return nullptr;
    } catch (const std::exception& e) {
        std::string error_msg = std::string("C++ exception during parsing with offset: ") + e.what();
        PyErr_SetString(PyExc_RuntimeError, error_msg.c_str());
        return nullptr;
    } catch (...) {
        PyErr_SetString(PyExc_RuntimeError, "Unknown C++ exception during parsing with offset.");
        return nullptr;
    }
}

PyObject *python_describe(int category, const char *item = nullptr, const char *field = nullptr, const char *value = nullptr) {
    // MEDIUM-005 FIX: Add exception handling to prevent crashes
    try {
        if (!pDefinition)
            return Py_BuildValue("s", "Not initialized");

        const char *description = pDefinition->getDescription(category, item, field, value);
        if (description == nullptr)
            return Py_BuildValue("s", "");
        return Py_BuildValue("s", description);

    } catch (const std::exception& e) {
        std::string error_msg = std::string("C++ exception in describe: ") + e.what();
        PyErr_SetString(PyExc_RuntimeError, error_msg.c_str());
        return nullptr;
    } catch (...) {
        PyErr_SetString(PyExc_RuntimeError, "Unknown C++ exception in describe.");
        return nullptr;
    }

/*
    Category* cat = pDefinition->getCategory(category);
    if (!cat)
    {
        return Py_BuildValue("s", "Unknown category");
    }

    if (item == nullptr && field == nullptr && value == nullptr)
    {   // return Category description
        return Py_BuildValue("s", cat->m_strName.c_str());
    }

	std::list<DataItemDescription*>::iterator it;
	DataItemDescription* di = nullptr;

    std::string item_number = format("%s", &item[1]);
	for (it = cat->m_lDataItems.begin(); it != cat->m_lDataItems.end(); ++it)
    {
        di = *it;
        if (di->m_strID == item_number)
            break;
        di = nullptr;
    }
    if (di == nullptr)
        return Py_BuildValue("s", "Unknown item");

    if (field == nullptr && value == nullptr)
    { // Return Item name and description
        return Py_BuildValue("s", (di->m_strName+" ("+di->m_strDefinition+" )").c_str());
    }

    if (value == nullptr)
    {
        return Py_BuildValue("s", "field todo");
    }
    return Py_BuildValue("s", "value todo");
*/
}



/*
	CAsterixFormatDescriptor& Descriptor((CAsterixFormatDescriptor&)formatDescriptor);
	PyObject *lst = Descriptor.m_pAsterixData->getData();
	PyObject *arg = Py_BuildValue("(O)", lst);
	PyObject *result = PyObject_CallObject(my_callback, arg);
	Py_DECREF(lst);
	if (result != nullptr)
		/// use result...
		Py_DECREF(result);
	return true;
*/
