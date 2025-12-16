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

#include "AsterixData.h"
#include "Utils.h"
#include <time.h>
#include "asterixformat.hxx"

AsterixData::AsterixData() {
}

AsterixData::~AsterixData() {
    deleteAndClear(m_lDataBlocks);
}

/*
 * appends Asterix data description to strResult
 */
bool AsterixData::getText(std::string &strResult, const unsigned int formatType) {
    static int i = 1;

    for (auto* db : m_lDataBlocks) {
        if (db != nullptr) {
            switch (formatType) {
                case CAsterixFormat::ETxt:
                    strResult += format("\n\n-------------------------\nData Block %d", i++);
                    break;
            }
            db->getText(strResult, formatType);
        }
    }
    return true;
}


#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
fulliautomatix_data* AsterixData::getData()
{
  fulliautomatix_data *firstData = nullptr, *lastData = nullptr;
  int byteoffset = 0;

  for (auto* db : m_lDataBlocks) {
    if (db != nullptr) {
      if (!lastData) {
        firstData = lastData = db->getData(byteoffset);
      } else {
        lastData->next = db->getData(byteoffset);
      }
      while (lastData->next) {
        lastData = lastData->next;
      }
      byteoffset = lastData->bytenr + lastData->length;
    }
  }
  return firstData;
}
#endif

#if defined(PYTHON_WRAPPER)
PyObject* AsterixData::getData(int verbose)
{
    PyObject* hp = PyList_New(0);

    for (auto* db : m_lDataBlocks) {
        if (db != nullptr) {
            db->getData(hp, verbose);
        }
    }
    return hp;
}
#endif
