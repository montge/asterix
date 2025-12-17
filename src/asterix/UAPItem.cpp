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

#include "UAPItem.h"
#include <cstring>  // For memset

UAPItem::UAPItem()
        : m_nBit(0), m_nFRN(0), m_bFX(false), m_nLen(0) {
}

UAPItem::UAPItem(const UAPItem &obj)
        : DataItemFormat(obj.m_nID) {
    for (const auto* subItem : obj.m_lSubItems) {
        m_lSubItems.push_back(subItem->clone());
    }

    m_pParentFormat = obj.m_pParentFormat;
    m_nBit = obj.m_nBit;
    m_nFRN = obj.m_nFRN;
    m_bFX = obj.m_bFX;
    m_nLen = obj.m_nLen;
    m_strItemID = obj.m_strItemID;
}

UAPItem::~UAPItem() {
}

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
fulliautomatix_definitions* UAPItem::getWiresharkDefinitions()
{
  // Use calloc instead of malloc+memset for cleaner initialization (SonarCloud)
  // Note: calloc is appropriate here for C interop with Wireshark plugin
  auto* def = static_cast<fulliautomatix_definitions*>(
      calloc(1, sizeof(fulliautomatix_definitions)));
  if (def == nullptr) {
      return nullptr;
  }

  def->pid = getPID();

  if (m_bFX)
  {
    def->name = strdup("Field extension");
    def->abbrev = strdup("FX");
  }
  else
  {
    char tmp[128];

    snprintf(tmp, sizeof(tmp), "Item %s", m_strItemID.c_str());
    def->name = strdup(tmp);

    snprintf(tmp, sizeof(tmp), "UAP%s", m_strItemID.c_str());
    def->abbrev = strdup(tmp);
  }
  def->type = FA_FT_UINT8;
  def->display = FA_BASE_DEC;
  def->bitmask = 0x01;
  def->bitmask <<= (7-m_nBit%8);

  def->strings = (fulliautomatix_value_string*)malloc(3 * sizeof(fulliautomatix_value_string));
  // Security fix: Check malloc return value to prevent null pointer dereference
  if (def->strings == nullptr) {
    return def;
  }
  def->strings[0].value = 0;
  def->strings[0].strptr = strdup(" Not present");
  def->strings[1].value = 1;
  def->strings[1].strptr = strdup(" Present");
  def->strings[2].value = 0;
  def->strings[2].strptr = nullptr;

  def->blurb = nullptr;
  def->next = nullptr;
  return def;
}

fulliautomatix_data* UAPItem::getData(unsigned char* pData, long len, int byteoffset)
{
  int byteShift = m_nBit/8;
  if (byteShift<len)
    return newDataUL(nullptr, getPID(), byteoffset+byteShift, 1, *(pData+byteShift));
  return nullptr;
}
#endif
