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

#include "XMLParser.h"
#include "Tracer.h"
#include <cstdio>
#include <cstring>

/*!
 * Handling of CDATA
 */
void XMLParser::CharacterHandler(void *userData, const XML_Char *s, int len) {
    XMLParser *p = static_cast<XMLParser *>(userData);

    if (!p) {
        return;
    }

    if (p->m_pstrCData) {
        // assign string value
        p->m_pstrCData->append(s, len);
    } else if (p->m_pintCData) {
        std::string tmpstr(s, len);
        *(p->m_pintCData) = atoi(tmpstr.c_str());
        p->m_pintCData = nullptr;
    }
}

bool XMLParser::GetAttribute(const char *elementName, const char *attrName, std::string *ptrString) {
    if (strcmp(elementName, attrName) == 0) {
        if (!ptrString) {
#ifdef PYTHON_WRAPPER
            PyErr_SetString(PyExc_SyntaxError, "XMLParser : Attribute parsing error");
#else
            Tracer::Error("XMLParser : Attribute parsing error");
#endif
        } else {
            GetCData(ptrString);
        }
        return true;
    }
    return false;
}

bool XMLParser::GetAttribute(const char *elementName, const char *attrName, int *ptrInt) {
    if (strcmp(elementName, attrName) == 0) {
        if (!ptrInt) {
#ifdef PYTHON_WRAPPER
            PyErr_SetString(PyExc_SyntaxError, "XMLParser : Attribute parsing error");
#else
            Tracer::Error("XMLParser : Attribute parsing error");
#endif
        } else {
            GetCData(ptrInt);
        }
        return true;
    }
    return false;
}

void XMLParser::Error(const char *errstr, const char *param1) {
    std::string tmpstr(errstr);
    tmpstr += param1;
    Error(tmpstr.c_str());
}

void XMLParser::Error(const char *errstr) {
    char strLine[1024];
    snprintf(strLine, sizeof(strLine), " in file: %s line: %d", m_pFileName, static_cast<int>(XML_GetCurrentLineNumber(m_Parser)));
    std::string tmpstr(errstr);
    tmpstr += strLine;
    tmpstr += "\n";

#ifdef PYTHON_WRAPPER
    PyErr_SetString(PyExc_SyntaxError, tmpstr.c_str());
#else
    // Use "%s" format to prevent format string injection from XML content
    Tracer::Error("%s", tmpstr.c_str());
#endif

    m_bErrorDetectedStopParsing = true;
}

// =========================================================================
// Helper Methods for Element Handlers
// =========================================================================

bool XMLParser::addFormatToParent(DataItemFormat *pNewFormat, const char *formatName,
                                   bool allowedInVariable, bool allowedInRepetitive,
                                   bool allowedInExplicit, bool allowedInCompound,
                                   bool allowedAsFirstInCompound) {
    if (m_pFormat != nullptr) {
        // Adding to an existing parent format
        if (m_pFormat->isVariable()) {
            if (!allowedInVariable) {
                std::string errMsg = "XMLParser : Error in handling ";
                errMsg += formatName;
                errMsg += " format";
                Error(errMsg.c_str(), m_pDataItem->m_strName.c_str());
                delete pNewFormat;
                return false;
            }
            m_pFormat->m_lSubItems.push_back(pNewFormat);
        } else if (m_pFormat->isRepetitive()) {
            if (!allowedInRepetitive) {
                std::string errMsg = "XMLParser : Error in handling ";
                errMsg += formatName;
                errMsg += " format";
                Error(errMsg.c_str(), m_pDataItem->m_strName.c_str());
                delete pNewFormat;
                return false;
            }
            if (!m_pFormat->m_lSubItems.empty()) {
                std::string errMsg = "XMLParser : Duplicate ";
                errMsg += formatName;
                errMsg += " item in Repetitive";
                Error(errMsg.c_str());
            }
            m_pFormat->m_lSubItems.push_back(pNewFormat);
        } else if (m_pFormat->isExplicit()) {
            if (!allowedInExplicit) {
                std::string errMsg = "XMLParser : Error in handling ";
                errMsg += formatName;
                errMsg += " format";
                Error(errMsg.c_str(), m_pDataItem->m_strName.c_str());
                delete pNewFormat;
                return false;
            }
            if (!m_pFormat->m_lSubItems.empty()) {
                std::string errMsg = "XMLParser : Duplicate ";
                errMsg += formatName;
                errMsg += " item in Explicit";
                Error(errMsg.c_str());
            }
            m_pFormat->m_lSubItems.push_back(pNewFormat);
        } else if (m_pFormat->isCompound()) {
            if (!allowedInCompound) {
                std::string errMsg = "XMLParser : Error in handling ";
                errMsg += formatName;
                errMsg += " format";
                Error(errMsg.c_str(), m_pDataItem->m_strName.c_str());
                delete pNewFormat;
                return false;
            }
            if (m_pFormat->m_lSubItems.empty() && !allowedAsFirstInCompound) {
                std::string errMsg = "XMLParser : First part of <Compound> must be <Variable> and not <";
                errMsg += formatName;
                errMsg += ">";
                Error(errMsg.c_str());
                delete pNewFormat;
                return false;
            }
            m_pFormat->m_lSubItems.push_back(pNewFormat);
        } else {
            std::string errMsg = "XMLParser : Error in handling ";
            errMsg += formatName;
            errMsg += " format in item ";
            Error(errMsg.c_str(), m_pDataItem->m_strName.c_str());
            delete pNewFormat;
            return false;
        }
        pNewFormat->m_pParentFormat = m_pFormat;
        m_pFormat = pNewFormat;
    } else {
        // Top-level format for the data item
        if (m_pDataItem->m_pFormat) {
            Error("XMLParser : Duplicate format in item ", m_pDataItem->m_strName.c_str());
        }
        m_pDataItem->m_pFormat = pNewFormat;
        m_pFormat = pNewFormat;
    }
    return true;
}

// =========================================================================
// Element Start Handlers
// =========================================================================

void XMLParser::handleCategoryStart(const char **attr) {
    for (int i = 0; attr[i]; i += 2) {
        if (strcmp(attr[i], "id") == 0) {
            int id = 0;
            if (strcmp(attr[i + 1], "BDS") == 0)
                id = BDS_CAT_ID;
            else
                id = atoi(attr[i + 1]);

            if (id >= 0 && id <= MAX_CATEGORIES) {
                m_pCategory = new Category(id);
            } else {
                Error("XMLParser : Wrong category: ", attr[i + 1]);
                break;
            }
        } else if (strcmp(attr[i], "name") == 0) {
            m_pCategory->m_strName = attr[i + 1];
        } else if (strcmp(attr[i], "ver") == 0) {
            m_pCategory->m_strVer = attr[i + 1];
        } else {
            Error("XMLParser : Unknown attribute: ", attr[i]);
        }
    }
}

void XMLParser::handleDataItemStart(const char **attr) {
    if (m_pCategory == nullptr) {
        Error("XMLParser : <DataItem> without <Category> ");
        return;
    }
    for (int i = 0; attr[i]; i += 2) {
        if (strcmp(attr[i], "id") == 0) {
            m_pDataItem = m_pCategory->getDataItemDescription(attr[i + 1]);
        } else if (strcmp(attr[i], "rule") == 0) {
            if (m_pDataItem) {
                if (strcmp(attr[i + 1], "optional") == 0)
                    m_pDataItem->m_eRule = DataItemDescription::DATAITEM_OPTIONAL;
                else if (strcmp(attr[i + 1], "mandatory") == 0)
                    m_pDataItem->m_eRule = DataItemDescription::DATAITEM_MANDATORY;
                else
                    m_pDataItem->m_eRule = DataItemDescription::DATAITEM_UNKNOWN;
            } else {
                Error("XMLParser : DataItem missing for rule");
            }
        } else {
            Error("XMLParser : Unknown attribute: ", attr[i]);
        }
    }
}

void XMLParser::handleDataItemFormatStart(const char **attr) {
    for (int i = 0; attr[i]; i += 2) {
        if (strcmp(attr[i], "desc") == 0) {
            if (m_pDataItem) {
                m_pDataItem->m_strFormat = attr[i + 1];
            } else {
                Error("XMLParser : <Format> without <DataItem>");
            }
        } else {
            Error("XMLParser : Unknown attribute for <DataItemFormat>: ", attr[i]);
        }
    }
}

void XMLParser::handleFixedStart(const char **attr) {
    if (!m_pDataItem) {
        Error("XMLParser : <Fixed> without <DataItem>");
        return;
    }

    auto *pFormatFixed = new DataItemFormatFixed(m_pDataItem->m_nID);

    // Fixed is allowed in: Variable, Repetitive(single), Explicit(single), Compound(not first)
    if (!addFormatToParent(pFormatFixed, "Fixed", true, true, true, true, false)) {
        return;
    }

    for (int i = 0; attr[i]; i += 2) {
        if (strcmp(attr[i], "length") == 0) {
            int len = atoi(attr[i + 1]);
            if (len >= 0) {
                pFormatFixed->m_nLength = len;
            } else {
                Error("XMLParser : Wrong DataItem length: ", attr[i + 1]);
            }
        } else {
            Error("XMLParser : Unknown attribute for Fixed: ", attr[i]);
        }
    }
}

void XMLParser::handleVariableStart(const char **attr) {
    (void)attr;  // Variable has no attributes
    if (!m_pDataItem) {
        Error("XMLParser : <Variable> without <DataItem>");
        return;
    }

    auto *pFormatVariable = new DataItemFormatVariable(m_pDataItem->m_nID);

    // Variable is allowed in: Compound(including first), Explicit(single)
    // Not allowed in: Variable, Repetitive
    addFormatToParent(pFormatVariable, "Variable", false, false, true, true, true);
}

void XMLParser::handleCompoundStart(const char **attr) {
    (void)attr;  // Compound has no attributes
    if (!m_pDataItem) {
        Error("XMLParser : <Compound> without <DataItem>");
        return;
    }

    auto *pFormatCompound = new DataItemFormatCompound(m_pDataItem->m_nID);

    // Compound is allowed in: Compound(not first), Explicit(single)
    // Not allowed in: Variable, Repetitive
    addFormatToParent(pFormatCompound, "Compound", false, false, true, true, false);
}

void XMLParser::handleRepetitiveStart(const char **attr) {
    (void)attr;  // Repetitive has no attributes
    if (!m_pDataItem) {
        Error("XMLParser : <Repetitive> without <DataItem>");
        return;
    }

    auto *pFormatRepetitive = new DataItemFormatRepetitive(m_pDataItem->m_nID);

    // Repetitive is allowed in: Compound(not first), Explicit(single)
    // Not allowed in: Variable, Repetitive
    addFormatToParent(pFormatRepetitive, "Repetitive", false, false, true, true, false);
}

void XMLParser::handleExplicitStart(const char **attr) {
    (void)attr;  // Explicit has no attributes
    if (!m_pDataItem) {
        Error("XMLParser : <Explicit> without <DataItem>");
        return;
    }

    auto *pFormatExplicit = new DataItemFormatExplicit(m_pDataItem->m_nID);

    // Explicit is allowed in: Compound(not first)
    // Not allowed in: Variable, Repetitive, Explicit
    addFormatToParent(pFormatExplicit, "Explicit", false, false, false, true, false);
}

void XMLParser::handleBDSStart(const char **attr) {
    (void)attr;  // BDS has no attributes
    Category *m_pBDSCategory = m_pDef->getCategory(BDS_CAT_ID);

    std::list<DataItemDescription *>::iterator it = m_pBDSCategory->m_lDataItems.begin();
    if (it == m_pBDSCategory->m_lDataItems.end()) {
        Error("XMLParser : Missing BDS definition file.");
        return;
    }

    if (!m_pDataItem) {
        Error("XMLParser : <BDS> without <DataItem>");
        return;
    }

    auto *pFormatBDS = new DataItemFormatBDS(m_pDataItem->m_nID);

    for (; it != m_pBDSCategory->m_lDataItems.end(); ++it) {
        auto *dip = *it;
        pFormatBDS->m_lSubItems.push_back(dip->m_pFormat->clone());
    }

    // BDS is allowed in: Variable, Repetitive(single), Explicit(single), Compound(not first)
    addFormatToParent(pFormatBDS, "BDS", true, true, true, true, false);
}

void XMLParser::handleBitsStart(const char **attr) {
    if (!m_pFormat) {
        Error("XMLParser : <Bits> without <Format>");
        return;
    }
    if (!m_pFormat->isFixed()) {
        Error("XMLParser : <Bits> without <Fixed>");
        return;
    }

    auto *pBits = new DataItemBits();
    m_pFormat->m_lSubItems.push_back(pBits);
    pBits->m_pParentFormat = m_pFormat;
    m_pFormat = pBits;

    for (int i = 0; attr[i]; i += 2) {
        if (strcmp(attr[i], "bit") == 0) {
            int bit = atoi(attr[i + 1]);
            if (bit >= 0) {
                pBits->m_nFrom = pBits->m_nTo = bit;
                if (static_cast<DataItemFormatFixed *>(pBits->m_pParentFormat)->m_nLength * 8 < bit) {
                    Error("XMLParser : Bit out of fixed length");
                }
            } else {
                Error("XMLParser : Wrong bit: ", attr[i + 1]);
            }
        } else if (strcmp(attr[i], "from") == 0) {
            int bit = atoi(attr[i + 1]);
            if (bit >= 0) {
                pBits->m_nFrom = bit;
                if (static_cast<DataItemFormatFixed *>(pBits->m_pParentFormat)->m_nLength * 8 < bit) {
                    Error("XMLParser : Bit out of fixed length");
                }
            } else {
                Error("XMLParser : Wrong bit from: ", attr[i + 1]);
            }
        } else if (strcmp(attr[i], "to") == 0) {
            int bit = atoi(attr[i + 1]);
            if (bit >= 0) {
                pBits->m_nTo = bit;
                if (static_cast<DataItemFormatFixed *>(pBits->m_pParentFormat)->m_nLength * 8 < bit) {
                    Error("XMLParser : Bit out of fixed length");
                }
            } else {
                Error("XMLParser : Wrong bit to: ", attr[i + 1]);
            }
        } else if (strcmp(attr[i], "encode") == 0) {
            if (strcmp(attr[i + 1], "unsigned") == 0) {
                pBits->m_eEncoding = DataItemBits::DATAITEM_ENCODING_UNSIGNED;
            } else if (strcmp(attr[i + 1], "6bitschar") == 0) {
                pBits->m_eEncoding = DataItemBits::DATAITEM_ENCODING_SIX_BIT_CHAR;
            } else if (strcmp(attr[i + 1], "hex") == 0) {
                pBits->m_eEncoding = DataItemBits::DATAITEM_ENCODING_HEX_BIT_CHAR;
            } else if (strcmp(attr[i + 1], "octal") == 0) {
                pBits->m_eEncoding = DataItemBits::DATAITEM_ENCODING_OCTAL;
            } else if (strcmp(attr[i + 1], "signed") == 0) {
                pBits->m_eEncoding = DataItemBits::DATAITEM_ENCODING_SIGNED;
            } else if (strcmp(attr[i + 1], "ascii") == 0) {
                pBits->m_eEncoding = DataItemBits::DATAITEM_ENCODING_ASCII;
            } else {
                Error("XMLParser : Wrong encode: ", attr[i]);
            }
        } else if (strcmp(attr[i], "fx") == 0) {
            pBits->m_bExtension = (atoi(attr[i + 1]) != 0);
        } else {
            Error("XMLParser : Unknown attribute: ", attr[i]);
        }
    }
}

void XMLParser::handleBitsValueStart(const char **attr) {
    if (m_pFormat == nullptr || !m_pFormat->isBits()) {
        Error("XMLParser : <BitsValue> without <Bits>");
        return;
    }

    for (int i = 0; attr[i]; i += 2) {
        if (strcmp(attr[i], "val") == 0) {
            int val = atoi(attr[i + 1]);
            m_pBitsValue = new BitsValue(val);
            (static_cast<DataItemBits *>(m_pFormat))->m_lValue.push_back(m_pBitsValue);
        }
    }
    GetCData(m_pBitsValue ? &m_pBitsValue->m_strDescription : nullptr);
}

void XMLParser::handleBitsUnitStart(const char **attr) {
    if (m_pFormat == nullptr || !m_pFormat->isBits()) {
        Error("XMLParser : <BitsUnit> without <Bits>");
        return;
    }

    for (int i = 0; attr[i]; i += 2) {
        if (strcmp(attr[i], "scale") == 0) {
            (static_cast<DataItemBits *>(m_pFormat))->m_dScale = atof(attr[i + 1]);
        } else if (strcmp(attr[i], "min") == 0) {
            (static_cast<DataItemBits *>(m_pFormat))->m_dMinValue = atof(attr[i + 1]);
            (static_cast<DataItemBits *>(m_pFormat))->m_bMinValueSet = true;
        } else if (strcmp(attr[i], "max") == 0) {
            (static_cast<DataItemBits *>(m_pFormat))->m_dMaxValue = atof(attr[i + 1]);
            (static_cast<DataItemBits *>(m_pFormat))->m_bMaxValueSet = true;
        }
    }

    GetCData(&(static_cast<DataItemBits *>(m_pFormat))->m_strUnit);
}

void XMLParser::handleBitsConstStart(const char **attr) {
    (void)attr;  // BitsConst has no attributes
    if (m_pFormat == nullptr || !m_pFormat->isBits()) {
        Error("XMLParser : <BitsConst> without <Bits>");
        return;
    }

    (static_cast<DataItemBits *>(m_pFormat))->m_bIsConst = true;
    GetCData(&(static_cast<DataItemBits *>(m_pFormat))->m_nConst);
}

void XMLParser::handleUAPStart(const char **attr) {
    if (m_pCategory == nullptr) {
        Error("XMLParser : Missing <UAP> outside <Category>");
        return;
    }

    m_pUAP = m_pCategory->newUAP();

    for (int i = 0; attr[i]; i += 2) {
        if (strcmp(attr[i], "use_if_bit_set") == 0) {
            m_pUAP->m_nUseIfBitSet = atoi(attr[i + 1]);
        } else if (strcmp(attr[i], "use_if_byte_nr") == 0) {
            m_pUAP->m_nUseIfByteNr = atoi(attr[i + 1]);
        } else if (strcmp(attr[i], "is_set_to") == 0) {
            m_pUAP->m_nIsSetTo = atoi(attr[i + 1]);
        } else {
            Error("XMLParser : Unknown property for UAP: ", attr[i + 1]);
        }
    }
}

void XMLParser::handleUAPItemStart(const char **attr) {
    if (m_pCategory == nullptr) {
        Error("XMLParser : Missing <UAPItem> outside <Category>");
        return;
    }
    if (m_pUAP == nullptr) {
        Error("XMLParser : Missing <UAPItem> outside <UAP>");
        return;
    }

    m_pUAPItem = m_pUAP->newUAPItem();

    for (int i = 0; attr[i]; i += 2) {
        if (strcmp(attr[i], "bit") == 0) {
            m_pUAPItem->m_nBit = atoi(attr[i + 1]);
        } else if (strcmp(attr[i], "frn") == 0) {
            if (strcmp(attr[i + 1], "FX") == 0) {
                m_pUAPItem->m_bFX = true;
            } else {
                m_pUAPItem->m_bFX = false;
                m_pUAPItem->m_nFRN = atoi(attr[i + 1]);
            }
        } else if (strcmp(attr[i], "len") == 0) {
            m_pUAPItem->m_nLen = atoi(attr[i + 1]);
        } else {
            Error("XMLParser : Unknown property for UAPItem: ", attr[i + 1]);
        }
    }

    GetCData(m_pUAPItem ? &m_pUAPItem->m_strItemID : nullptr);
}

// =========================================================================
// Element End Handlers
// =========================================================================

void XMLParser::handleCategoryEnd() {
    if (m_pCategory) {
        m_pDef->setCategory(m_pCategory);
        m_pCategory = nullptr;
    } else {
        Error("Closing unopened tag: ", "Category");
    }
}

void XMLParser::handleDataItemEnd() {
    if (m_pDataItem) {
        m_pDataItem = nullptr;
    } else {
        Error("Closing unopened tag: ", "DataItem");
    }
}

void XMLParser::handleFormatEnd(const char *formatName) {
    bool validFormat = false;

    if (strcmp(formatName, "Fixed") == 0) {
        validFormat = m_pFormat != nullptr && m_pFormat->isFixed();
    } else if (strcmp(formatName, "Variable") == 0) {
        validFormat = m_pFormat != nullptr && m_pFormat->isVariable();
    } else if (strcmp(formatName, "Explicit") == 0) {
        validFormat = m_pFormat != nullptr && m_pFormat->isExplicit();
    } else if (strcmp(formatName, "Repetitive") == 0) {
        validFormat = m_pFormat != nullptr && m_pFormat->isRepetitive();
    } else if (strcmp(formatName, "BDS") == 0) {
        validFormat = m_pFormat != nullptr && m_pFormat->isBDS();
    } else if (strcmp(formatName, "Compound") == 0) {
        validFormat = m_pFormat != nullptr && m_pFormat->isCompound();
    }

    if (validFormat) {
        m_pFormat = m_pFormat->m_pParentFormat;
    } else {
        Error("Closing unopened tag: ", formatName);
    }
}

void XMLParser::handleBitsEnd() {
    if (m_pFormat != nullptr && m_pFormat->isBits()) {
        m_pFormat = m_pFormat->m_pParentFormat;
    } else {
        Error("Closing unopened tag: ", "Bits");
    }
}

void XMLParser::handleBitsValueEnd() {
    if (m_pBitsValue) {
        m_pBitsValue = nullptr;
    } else {
        Error("Closing unopened tag: ", "BitsValue");
    }
}

void XMLParser::handleUAPEnd() {
    if (m_pUAP) {
        m_pUAP = nullptr;
    } else {
        Error("Closing unopened tag: ", "UAP");
    }
}

void XMLParser::handleUAPItemEnd() {
    if (m_pUAPItem) {
        m_pUAPItem = nullptr;
    } else {
        Error("Closing unopened tag: ", "UAPItem");
    }
}

// =========================================================================
// Main Element Handler (delegates to individual handlers)
// =========================================================================

/*!
 * Handling of element start
 */
void XMLParser::ElementHandlerStart(void *data, const char *el, const char **attr) {
    XMLParser *p = static_cast<XMLParser *>(data);

    if (!p) {
#ifdef PYTHON_WRAPPER
        PyErr_SetString(PyExc_RuntimeError, "Missing Parser!");
#else
        Tracer::Error("Missing Parser!");
#endif
        return;
    }

    if (p->m_bErrorDetectedStopParsing)
        return;

    if (strcmp(el, "Category") == 0) {
        p->handleCategoryStart(attr);
    } else if (strcmp(el, "DataItem") == 0) {
        p->handleDataItemStart(attr);
    } else if (p->GetAttribute(el, "DataItemName", p->m_pDataItem ? &p->m_pDataItem->m_strName : nullptr)) {
        // Handled by GetAttribute
    } else if (p->GetAttribute(el, "DataItemDefinition", p->m_pDataItem ? &p->m_pDataItem->m_strDefinition : nullptr)) {
        // Handled by GetAttribute
    } else if (p->GetAttribute(el, "DataItemNote", p->m_pDataItem ? &p->m_pDataItem->m_strNote : nullptr)) {
        // Handled by GetAttribute
    } else if (strcmp(el, "DataItemFormat") == 0) {
        p->handleDataItemFormatStart(attr);
    } else if (strcmp(el, "BDS") == 0) {
        p->handleBDSStart(attr);
    } else if (strcmp(el, "Fixed") == 0) {
        p->handleFixedStart(attr);
    } else if (strcmp(el, "Explicit") == 0) {
        p->handleExplicitStart(attr);
    } else if (strcmp(el, "Repetitive") == 0) {
        p->handleRepetitiveStart(attr);
    } else if (strcmp(el, "Variable") == 0) {
        p->handleVariableStart(attr);
    } else if (strcmp(el, "Compound") == 0) {
        p->handleCompoundStart(attr);
    } else if (strcmp(el, "Bits") == 0) {
        p->handleBitsStart(attr);
    } else if (p->GetAttribute(el, "BitsShortName", (p->m_pFormat && p->m_pFormat->isBits())
                                                    ? &(static_cast<DataItemBits *>(p->m_pFormat))->m_strShortName
                                                    : nullptr)) {
        // Handled by GetAttribute
    } else if (p->GetAttribute(el, "BitsName",
                               (p->m_pFormat && p->m_pFormat->isBits()) ? &(static_cast<DataItemBits *>(p->m_pFormat))->m_strName
                                                                        : nullptr)) {
        // Handled by GetAttribute
    } else if (p->GetAttribute(el, "BitsPresence", (p->m_pFormat && p->m_pFormat->isBits())
                                                   ? &(static_cast<DataItemBits *>(p->m_pFormat))->m_nPresenceOfField
                                                   : nullptr)) {
        // Handled by GetAttribute
    } else if (strcmp(el, "BitsValue") == 0) {
        p->handleBitsValueStart(attr);
    } else if (strcmp(el, "BitsUnit") == 0) {
        p->handleBitsUnitStart(attr);
    } else if (strcmp(el, "BitsConst") == 0) {
        p->handleBitsConstStart(attr);
    } else if (strcmp(el, "UAP") == 0) {
        p->handleUAPStart(attr);
    } else if (strcmp(el, "UAPItem") == 0) {
        p->handleUAPItemStart(attr);
    } else {
        p->Error("Unknown tag: ", el);
    }
}

/*!
 * Handling of element end
 */
void XMLParser::ElementHandlerEnd(void *data, const char *el) {
    auto *p = static_cast<XMLParser *>(data);

    if (!p) {
#ifdef PYTHON_WRAPPER
        PyErr_SetString(PyExc_RuntimeError, "Missing Parser!");
#else
        Tracer::Error("Missing Parser!");
#endif
        return;
    }

    if (p->m_bErrorDetectedStopParsing)
        return;

    if (strcmp(el, "Category") == 0) {
        p->handleCategoryEnd();
    } else if (strcmp(el, "DataItem") == 0) {
        p->handleDataItemEnd();
    } else if (strcmp(el, "Fixed") == 0 || strcmp(el, "Variable") == 0 ||
               strcmp(el, "Explicit") == 0 || strcmp(el, "Repetitive") == 0 ||
               strcmp(el, "BDS") == 0 || strcmp(el, "Compound") == 0) {
        p->handleFormatEnd(el);
    } else if (strcmp(el, "Bits") == 0) {
        p->handleBitsEnd();
    } else if (strcmp(el, "BitsValue") == 0) {
        p->handleBitsValueEnd();
    } else if (strcmp(el, "UAP") == 0) {
        p->handleUAPEnd();
    } else if (strcmp(el, "UAPItem") == 0) {
        p->handleUAPItemEnd();
    }

    p->m_pstrCData = nullptr;
    p->m_pintCData = nullptr;
}

/*!
 * XMLParser constructor
 */
XMLParser::XMLParser()
        : m_bErrorDetectedStopParsing(false), m_pDef(nullptr), m_pCategory(nullptr), m_pDataItem(nullptr), m_pFormat(nullptr),
          m_pBitsValue(nullptr), m_pUAPItem(nullptr), m_pUAP(nullptr), m_pstrCData(nullptr), m_pintCData(nullptr), m_pFileName(nullptr) {
    m_Parser = XML_ParserCreate(nullptr);
    if (!m_Parser) {
#ifdef PYTHON_WRAPPER
        PyErr_SetString(PyExc_RuntimeError, "Couldn't allocate memory for parser");
#else
        Tracer::Error("Couldn't allocate memory for parser");
#endif
    }

    XML_SetElementHandler(m_Parser, ElementHandlerStart, ElementHandlerEnd);

    XML_SetCharacterDataHandler(m_Parser, CharacterHandler);

    XML_SetUserData(m_Parser, this);

}

/*!
 * XMLParser destructor
 */
XMLParser::~XMLParser() {
    XML_ParserFree(m_Parser);
}

/*!
 * Parse XML file and fill definition object
 */
bool XMLParser::Parse(FILE *pFile, AsterixDefinition *pDefinition, const char *filename) {
    m_pDef = pDefinition;
    m_pFileName = filename;

    while (true) {
        int done;
        int len;

        len = static_cast<int>(fread(m_pBuff, 1, BUFFSIZE, pFile));
        if (ferror(pFile)) {
#ifdef PYTHON_WRAPPER
            PyErr_SetString(PyExc_IOError, "Format file read error.");
#else
            Tracer::Error("Format file read error.");
#endif
            return false;
        }
        done = feof(pFile);

        if (XML_Parse(m_Parser, m_pBuff, len, done) == XML_STATUS_ERROR) {
            std::string tmpStr("Format file parse error: ");
            tmpStr += XML_ErrorString(XML_GetErrorCode(m_Parser));
            Error(tmpStr.c_str());
            return false;
        }

        if (done)
            break;
    }

    return !m_bErrorDetectedStopParsing;
}
