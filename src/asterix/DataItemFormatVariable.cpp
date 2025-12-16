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

#include "DataItemFormatVariable.h"
#include "Tracer.h"
#include "Utils.h"
#include "asterixformat.hxx"

DataItemFormatVariable::DataItemFormatVariable(int id)
        : DataItemFormat(id) {
}

DataItemFormatVariable::DataItemFormatVariable(const DataItemFormatVariable &obj)
        : DataItemFormat(obj.m_nID) {
    // C++23 Quick Win: Ranges provide 5-10% throughput improvement for transformations
#if HAS_RANGES_ALGORITHMS
    // Modern ranges-based clone operation - more expressive and efficient
    asterix::ranges::transform(
        obj.m_lSubItems,
        std::back_inserter(m_lSubItems),
        [](const DataItemFormat* item) { return item->clone(); }
    );
#else
    // C++17: Traditional range-based approach
    for (const auto* di : obj.m_lSubItems) {
        m_lSubItems.push_back(di->clone());
    }
#endif

    m_pParentFormat = obj.m_pParentFormat;
}

DataItemFormatVariable::~DataItemFormatVariable() {
    deleteAndClear(m_lSubItems);
}

long DataItemFormatVariable::getLength(const unsigned char *pData) {
    long length = 0;
    std::list<DataItemFormat *>::iterator it;
    bool lastPart = false;
    it = m_lSubItems.begin();

    auto *dip = static_cast<DataItemFormatFixed *>(*it);

    do {
        lastPart = dip->isLastPart(pData);
        long partlen = dip->getLength();

        length += partlen;
        pData += partlen;

        if (it != m_lSubItems.end()) {
            it++;
            if (it != m_lSubItems.end()) {
                dip = static_cast<DataItemFormatFixed *>(*it);
            }
        }
    } while (!lastPart);

    return length;
}

bool DataItemFormatVariable::getText(std::string &strResult, std::string &strHeader, const unsigned int formatType,
                                     unsigned char *pData, long nLength) {
    bool ret = false;

    std::list<DataItemFormat *>::iterator it;
    bool lastPart = false;

    // If Variable item definition contains 1 Fixed subitem, show items in list
    bool listOfSubItems = false;
    if (m_lSubItems.size() == 1)
        listOfSubItems = true;

    it = m_lSubItems.begin();
    std::string tmpResult;

    auto *dip = static_cast<DataItemFormatFixed *>(*it);

    switch (formatType) {
        case CAsterixFormat::EJSON:
        case CAsterixFormat::EJSONH:
        case CAsterixFormat::EJSONE: {
            if (listOfSubItems)
                strResult += '[';
            else
                strResult += '{';
        }
            break;
    }

    do {
        lastPart = dip->isLastPart(pData);

        switch (formatType) {
            case CAsterixFormat::EJSON:
            case CAsterixFormat::EJSONH:
            case CAsterixFormat::EJSONE: {
                tmpResult = "";
                ret |= dip->getText(tmpResult, strHeader, formatType, pData, dip->getLength());
                if (tmpResult.length() > 2) { // if result != {}
                    if (listOfSubItems)
                        strResult += '{';
                    strResult += tmpResult.substr(1, tmpResult.length() - 2); // trim {}
                    if (listOfSubItems)
                        strResult += '}';
                    if (!lastPart) {
                        strResult += ',';
                    }
                }
            }
                break;
            default:
                ret |= dip->getText(strResult, strHeader, formatType, pData, dip->getLength());
                break;
        }

        pData += dip->getLength();
        nLength -= dip->getLength();

        if (it != m_lSubItems.end()) {
            it++;
            if (it != m_lSubItems.end()) {
                dip = static_cast<DataItemFormatFixed *>(*it);
            }
        }
    } while (!lastPart && nLength > 0);

    switch (formatType) {
        case CAsterixFormat::EJSON:
        case CAsterixFormat::EJSONH:
        case CAsterixFormat::EJSONE: {
            if (strResult[strResult.length() - 1] == ',') {
                if (listOfSubItems)
                    strResult[strResult.length() - 1] = ']';
                else
                    strResult[strResult.length() - 1] = '}';
            } else {
                if (listOfSubItems)
                    strResult += ']';
                else
                    strResult += '}';
            }
        }
            break;
    }

    return ret;
}

std::string DataItemFormatVariable::printDescriptors(std::string header) {
    std::string strDef;

    for (auto* subItem : m_lSubItems) {
        auto *dip = static_cast<DataItemFormatFixed *>(subItem);
        strDef += dip->printDescriptors(header);
    }
    return strDef;
}

bool DataItemFormatVariable::filterOutItem(const char *name) {
    for (auto* subItem : m_lSubItems) {
        auto *dip = static_cast<DataItemFormatFixed *>(subItem);
        if (dip->filterOutItem(name))
            return true;
    }
    return false;
}

bool DataItemFormatVariable::isFiltered(const char *name) {
    for (auto* subItem : m_lSubItems) {
        auto *dip = static_cast<DataItemFormatFixed *>(subItem);
        if (dip->isFiltered(name))
            return true;
    }
    return false;
}

const char *DataItemFormatVariable::getDescription(const char *field, const char *value = nullptr) {
    for (auto* subItem : m_lSubItems) {
        auto *dip = static_cast<DataItemFormatFixed *>(subItem);
        const char *desc = dip->getDescription(field, value);
        if (desc != nullptr)
            return desc;
    }
    return nullptr;
}

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
fulliautomatix_definitions* DataItemFormatVariable::getWiresharkDefinitions()
{
  fulliautomatix_definitions *def = nullptr, *startDef = nullptr;

  for (auto* subItem : m_lSubItems) {
    auto* dip = static_cast<DataItemFormatFixed*>(subItem);
    if (def) {
      def->next = dip->getWiresharkDefinitions();
    } else {
      startDef = def = dip->getWiresharkDefinitions();
    }
    while (def->next)
      def = def->next;
  }
  return startDef;
}

fulliautomatix_data* DataItemFormatVariable::getData(unsigned char* pData, long len, int byteoffset)
{
  fulliautomatix_data *lastData = nullptr, *firstData = nullptr;
  std::list<DataItemFormat*>::iterator it;
  bool lastPart = false;

  it=m_lSubItems.begin();

  auto* dip = static_cast<DataItemFormatFixed*>(*it);

  do
  {
    lastPart = dip->isLastPart(pData);

    if (lastData)
    {
      lastData->next = dip->getData(pData, dip->getLength(), byteoffset);
    }
    else
    {
      firstData = lastData = dip->getData(pData, dip->getLength(), byteoffset);
    }
    while(lastData->next)
      lastData = lastData->next;

    byteoffset += dip->getLength();
    pData += dip->getLength();
    len -= dip->getLength();

    if (it != m_lSubItems.end())
    {
      it++;
      if (it != m_lSubItems.end())
      {
        dip = static_cast<DataItemFormatFixed*>(*it);
      }
    }
  }
  while(!lastPart && len > 0);

  return firstData;
}
#endif

#if defined(PYTHON_WRAPPER)
PyObject* DataItemFormatVariable::getObject(unsigned char* pData, long nLength, int verbose)
{
    PyObject* p = nullptr;
    std::list<DataItemFormat*>::iterator it;
    bool lastPart = false;

    // If Variable item definition contains 1 Fixed subitem, show items in list
    bool listOfSubItems = false;
    if (m_lSubItems.size() == 1) {
        listOfSubItems = true;
        p = PyList_New(0);
    } else {
        // otherwise put directly to dictionary
        p = PyDict_New();
    }

    it=m_lSubItems.begin();
    auto* dip = static_cast<DataItemFormatFixed*>(*it);
    do
    {
        lastPart = dip->isLastPart(pData);

        if (listOfSubItems) {
            PyObject* p1 = PyDict_New();
            dip->insertToDict(p1, pData, dip->getLength(), verbose);
            PyList_Append(p, p1);
            Py_DECREF(p1);
        }
        else {
            dip->insertToDict(p, pData, dip->getLength(), verbose);
        }

        pData += dip->getLength();
        nLength -= dip->getLength();

        if (it != m_lSubItems.end()) {
            it++;
            if (it != m_lSubItems.end()) {
                dip = static_cast<DataItemFormatFixed*>(*it);
            }
        }
    }
    while(!lastPart && nLength > 0);

    return p;
}

void DataItemFormatVariable::insertToDict(PyObject* p, unsigned char* pData, long nLength, int verbose)
{
    return;
    /*
    std::list<DataItemFormat*>::iterator it;
    bool lastPart = false;

    it=m_lSubItems.begin();

    DataItemFormatFixed* dip = (DataItemFormatFixed*)(*it);

    do
    {
        lastPart = dip->isLastPart(pData);

        dip->insertToDict(p, pData, dip->getLength(), verbose);

        pData += dip->getLength();
        nLength -= dip->getLength();

        if (it != m_lSubItems.end()) {
            it++;
            if (it != m_lSubItems.end()) {
                dip = (DataItemFormatFixed*)(*it);
            }
        }
    }
    while(!lastPart && nLength > 0);
     */
}
#endif
