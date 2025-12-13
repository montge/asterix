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

#include "DataItemFormat.h"

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
int DataItemFormat::m_nLastPID = PID_LAST;
#endif

DataItemFormat::DataItemFormat(int id)
        : m_pParentFormat(nullptr), m_nID(id) {
#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
    m_nPID = m_nLastPID++;
#endif
}

DataItemFormat::~DataItemFormat() {
    // C++23 Quick Win: Ranges simplify cleanup operations
#if HAS_RANGES_ALGORITHMS
    // Modern ranges-based cleanup - more concise and potentially optimized
    asterix::ranges::for_each(m_lSubItems, [](DataItemFormat* item) { delete item; });
    m_lSubItems.clear();
#else
    // C++17: Traditional iterator-based cleanup
    std::list<DataItemFormat *>::iterator it = m_lSubItems.begin();
    while (it != m_lSubItems.end()) {
        delete (DataItemFormat *) (*it);
        it = m_lSubItems.erase(it);
    }
#endif
}
