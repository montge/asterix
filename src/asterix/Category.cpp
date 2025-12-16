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

#include "Category.h"
#include "Utils.h"

// FSPEC (Field Specification) extension bit - indicates if FSPEC continues to next byte
namespace {
    constexpr unsigned char FSPEC_FX_BIT = 0x01;
}

Category::Category(int id)
        : m_id(id), m_bFiltered(false) {
}

Category::~Category() {
    deleteAndClear(m_lDataItems);
    deleteAndClear(m_lUAPs);
}

DataItemDescription *Category::getDataItemDescription(std::string id) {
    for (auto* di : m_lDataItems) {
        if (di->m_strID == id) {
            return di;
        }
    }

    // create new DataItemDescription
    auto* di = new DataItemDescription(id);
    m_lDataItems.push_back(di);

    return di;
}

const char *Category::getDescription(const char *item, const char *field, const char *value) const {
    std::string item_number = format("%s", &item[1]);

    for (const auto* di : m_lDataItems) {
        if (di->m_strID.compare(item_number) == 0) {
            if (field == nullptr)
                return di->m_strName.c_str();
            return di->m_pFormat->getDescription(field, value);
        }
    }
    return nullptr;
}

UAP *Category::newUAP() {
    UAP *uap = new UAP();
    m_lUAPs.push_back(uap);
    return uap;
}

UAP *Category::getUAP(const unsigned char *data, unsigned long len) const {
    for (auto* uap : m_lUAPs) {
        if (uap) {
            if (uap->m_nUseIfBitSet) { // check if bit matches
                unsigned long bittomatch = uap->m_nUseIfBitSet;

                unsigned long pos = 0;

                // skip FSPEC
                while (pos < len && (data[pos] & FSPEC_FX_BIT))
                    pos++;

                pos++;

                pos += (bittomatch - 1) / 8;

                unsigned char mask = FSPEC_FX_BIT;
                mask <<= (7 - (bittomatch - 1) % 8);

                if (pos < len && (data[pos] & mask))
                    return uap;
            } else if (uap->m_nUseIfByteNr) { // check if byte matches
                unsigned long pos = 0;

                // skip FSPEC
                while (pos < len && (data[pos] & FSPEC_FX_BIT))
                    pos++;

                pos++;

                pos += uap->m_nUseIfByteNr - 1;

                if (pos < len && data[pos] == uap->m_nIsSetTo) {
                    return uap;
                }
            } else { // no need to match
                return uap;
            }
        }
    }
    return nullptr;
}

std::string Category::printDescriptors() const {
    std::string strDef;
    char header[32];

    for (const auto* di : m_lDataItems) {
        snprintf(header, sizeof(header), "CAT%03d:I%s:", m_id, di->m_strID.c_str());
        strDef += di->m_pFormat->printDescriptors(header);
    }

    return strDef;
}

bool Category::filterOutItem(std::string item, const char *name) {
    // At least one item of category shall be printed when filter is applied
    m_bFiltered = true;

    for (auto* di : m_lDataItems) {
        if (di->m_strID == item) {
            return di->m_pFormat->filterOutItem(name);
        }
    }
    return false;
}

bool Category::isFiltered(std::string item, const char *name) const {
    for (const auto* di : m_lDataItems) {
        if (di->m_strID == item && di->m_pFormat->isFiltered(name)) {
            return true;
        }
    }
    return false;
}

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
fulliautomatix_definitions* Category::getWiresharkDefinitions()
{
  fulliautomatix_definitions* startDef = nullptr;
  fulliautomatix_definitions* def = nullptr;

  // get definitions for UAPs
  for (auto* ui : m_lUAPs) {
    if (def) {
      def->next = ui->getWiresharkDefinitions();
    } else {
      startDef = def = ui->getWiresharkDefinitions();
    }
    while (def->next)
      def = def->next;
  }

  // get definitions for items
  for (auto* di : m_lDataItems) {
    if (def) {
      def->next = di->m_pFormat->getWiresharkDefinitions();
    } else {
      startDef = def = di->m_pFormat->getWiresharkDefinitions();
    }
    while (def->next) {
      def = def->next;
    }
  }

  return startDef;
}
#endif
