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

#include "DataItemFormatRepetitive.h"
#include "Tracer.h"
#include <climits>  // For LONG_MAX
#include "asterixformat.hxx"

DataItemFormatRepetitive::DataItemFormatRepetitive(int id)
        : DataItemFormat(id) {
}

DataItemFormatRepetitive::DataItemFormatRepetitive(const DataItemFormatRepetitive &obj)
        : DataItemFormat(obj.m_nID) {
    // Use range-based for to avoid casting away const
    for (const auto* di : obj.m_lSubItems) {
        m_lSubItems.push_back(di->clone());
    }

    m_pParentFormat = obj.m_pParentFormat;
}

DataItemFormatRepetitive::~DataItemFormatRepetitive() {
}

long DataItemFormatRepetitive::getLength(const unsigned char *pData) {
    DataItemFormat *pF = !m_lSubItems.empty() ? static_cast<DataItemFormatFixed *>(m_lSubItems.front()) : nullptr;
    if (pF == nullptr) {
        Tracer::Error("Wrong data in Repetitive");
        return 0;
    }
    unsigned char nRepetition = *pData;
    long fixedLength = pF->getLength(pData + 1);

    // SECURITY FIX (VULN-001): Check for integer overflow
    if (nRepetition > 0 && fixedLength > (LONG_MAX - 1) / nRepetition) {
        Tracer::Error("Integer overflow in repetitive item length calculation: nRepetition=%d, fixedLength=%ld",
                      nRepetition, fixedLength);
        return 0;
    }

    long totalLength = 1 + static_cast<long>(nRepetition) * fixedLength;

    // SECURITY FIX (VULN-001): Additional sanity check - maximum reasonable ASTERIX item size
    // ASTERIX data blocks are limited to 64KB, individual items should be much smaller
    const long MAX_ASTERIX_ITEM_SIZE = 65536;
    if (totalLength > MAX_ASTERIX_ITEM_SIZE) {
        Tracer::Error("Repetitive item exceeds maximum allowed size: %ld bytes (max: %ld)",
                      totalLength, MAX_ASTERIX_ITEM_SIZE);
        return 0;
    }

    return totalLength;
}

bool DataItemFormatRepetitive::getText(std::string &strResult, std::string &strHeader, const unsigned int formatType,
                                       unsigned char *pData, long nLength) {
    bool ret = false;
    DataItemFormatFixed *pF = !m_lSubItems.empty() ? static_cast<DataItemFormatFixed *>(m_lSubItems.front()) : nullptr;
    if (pF == nullptr) {
        Tracer::Error("Wrong data in Repetitive");
        return true;
    }

    int fixedLength = pF->getLength(pData);
    unsigned char nRepetition = *pData;

    if (1 + nRepetition * fixedLength != nLength) {
        Tracer::Error("Wrong length in Repetitive");
        return true;
    }

    pData++;

    switch (formatType) {
        case CAsterixFormat::EJSON:
        case CAsterixFormat::EJSONH:
        case CAsterixFormat::EJSONE: {
            std::string tmpStr = format("[");
            if (nRepetition == 0) {
                ret = true;
            } else {
                while (nRepetition--) {
                    ret |= pF->getText(tmpStr, strHeader, formatType, pData, fixedLength);
                    pData += fixedLength;

                    if (nRepetition > 0)
                        tmpStr += format(",");
                }
            }
            tmpStr += format("]");

            if (ret)
                strResult += tmpStr;

            break;
        }
        default: {
            if (nRepetition == 0) {
                ret = true;
            } else {
                while (nRepetition--) {
                    ret |= pF->getText(strResult, strHeader, formatType, pData, fixedLength);
                    pData += fixedLength;
                }   
            }
            break;
        }
    }
    return ret;
}

std::string DataItemFormatRepetitive::printDescriptors(std::string header) {
    DataItemFormatFixed *pFixed = !m_lSubItems.empty() ? static_cast<DataItemFormatFixed *>(m_lSubItems.front()) : nullptr;
    if (pFixed == nullptr) {
        Tracer::Error("Wrong data in Repetitive");
        return "Wrong data in Repetitive";
    }

    return pFixed->printDescriptors(header);
}

bool DataItemFormatRepetitive::filterOutItem(const char *name) {
    DataItemFormatFixed *pFixed = !m_lSubItems.empty() ? static_cast<DataItemFormatFixed *>(m_lSubItems.front()) : nullptr;
    if (pFixed == nullptr) {
        Tracer::Error("Wrong data in Repetitive");
        return false;
    }

    return pFixed->filterOutItem(name);
}

bool DataItemFormatRepetitive::isFiltered(const char *name) {
    DataItemFormatFixed *pFixed = !m_lSubItems.empty() ? static_cast<DataItemFormatFixed *>(m_lSubItems.front()) : nullptr;
    if (pFixed == nullptr) {
        Tracer::Error("Wrong data in Repetitive");
        return false;
    }

    return pFixed->isFiltered(name);
}

const char *DataItemFormatRepetitive::getDescription(const char *field, const char *value = nullptr) {
    for (auto* subItem : m_lSubItems) {
        auto *bv = static_cast<DataItemBits *>(subItem);
        const char *desc = bv->getDescription(field, value);
        if (desc != nullptr)
            return desc;
    }
    return nullptr;
}

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
fulliautomatix_definitions* DataItemFormatRepetitive::getWiresharkDefinitions()
{
    DataItemFormatFixed* pFixed = !m_lSubItems.empty() ? static_cast<DataItemFormatFixed*>(m_lSubItems.front()) : nullptr;
    if (!pFixed)
    {
        Tracer::Error("Wrong format of repetitive item");
        return nullptr;
    }
    return pFixed->getWiresharkDefinitions();
}

fulliautomatix_data* DataItemFormatRepetitive::getData(unsigned char* pData, long len, int byteoffset)
{
    fulliautomatix_data *lastData = nullptr, *firstData = nullptr;
    DataItemFormatFixed* pFixed = !m_lSubItems.empty() ? static_cast<DataItemFormatFixed*>(m_lSubItems.front()) : nullptr;
    if (!pFixed)
    {
        Tracer::Error("Wrong format of repetitive item");
        return nullptr;
    }

    int fixedLength = pFixed->getLength(pData);
    unsigned char nRepetition = *pData;

    firstData = lastData = newDataUL(nullptr, PID_REP, byteoffset, 1, nRepetition);
    byteoffset+=1;

    if (1+nRepetition*fixedLength != len)
    {
        Tracer::Error("Wrong length in Repetitive");
        return firstData;
    }

    pData++;

    while(nRepetition--)
    {
        lastData->next = pFixed->getData(pData, fixedLength, byteoffset);
        while(lastData->next)
            lastData = lastData->next;

        pData += fixedLength;
        byteoffset += fixedLength;
    }

    return firstData;
}
#endif

#if defined(PYTHON_WRAPPER)

PyObject* DataItemFormatRepetitive::getObject(unsigned char* pData, long nLength, int verbose)
{
    PyObject* p = PyList_New(0);

     DataItemFormatFixed* pFixed = !m_lSubItems.empty() ? static_cast<DataItemFormatFixed*>(m_lSubItems.front()) : nullptr;
      if (!pFixed)
      {
        PyObject* p1 = Py_BuildValue("s", "Wrong format of Repetitive item");
        PyList_Append(p, p1);
        Py_DECREF(p1);
        return p;
      }

      int fixedLength = pFixed->getLength(pData);
      unsigned char nRepetition = *pData;

      if (1+nRepetition*fixedLength != nLength)
      {
        PyObject* p1 = Py_BuildValue("s", "Wrong length in Repetitive item");
        PyList_Append(p, p1);
        Py_DECREF(p1);
        return p;
      }

      pData++;

      while(nRepetition--)
      {
        PyObject* p1 = pFixed->getObject(pData, fixedLength, verbose);
        PyList_Append(p, p1);
        Py_DECREF(p1);
        pData += fixedLength;
      }

    return p;
}


void DataItemFormatRepetitive::insertToDict(PyObject* p, unsigned char* pData, long nLength, int verbose)
{
 // Not supported
}
#endif
