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

#ifndef XMLPARSER_H_
#define XMLPARSER_H_

#include "AsterixDefinition.h"
#include "expat.h"
#include "DataItemFormatFixed.h"
#include "DataItemFormatVariable.h"
#include "DataItemFormatCompound.h"
#include "DataItemFormatExplicit.h"
#include "DataItemFormatRepetitive.h"
#include "DataItemFormatBDS.h"
#include "UAP.h"

//! XML parsing buffer size (8KB chunks)
#define BUFFSIZE        8192

/**
 * @class XMLParser
 * @brief XML configuration file parser for ASTERIX category definitions
 *
 * XMLParser loads ASTERIX category specifications from XML files conforming
 * to the `asterix.dtd` format. It constructs the internal data structures
 * (Category, DataItemDescription, DataItemFormat, UAP) needed for parsing
 * ASTERIX binary data.
 *
 * ## Configuration File Structure
 *
 * XML files define:
 * - **Categories**: ASTERIX category number and version (e.g., CAT062 v1.18)
 * - **Data Items**: Field definitions with IDs (e.g., I062/010)
 * - **Formats**: Binary encoding (Fixed, Variable, Compound, Repetitive, Explicit, BDS)
 * - **UAPs**: User Application Profiles mapping FSPEC bits to data items
 * - **Value Meanings**: Enumerated value descriptions
 *
 * ## Parsing Process
 *
 * ```
 * XML File → expat → Element Handlers → AsterixDefinition → Category/UAP/DataItems
 * ```
 *
 * ## Configuration Files Location
 *
 * - Source: `asterix/config/asterix_cat*.xml`
 * - Installed: `install/share/asterix/config/`
 * - DTD spec: `asterix/config/asterix.dtd`
 *
 * ## Usage Example
 *
 * ```cpp
 * AsterixDefinition* def = AsterixDefinition::getInstance();
 * XMLParser parser;
 *
 * FILE* f = fopen("asterix/config/asterix_cat062_1_18.xml", "r");
 * if (parser.Parse(f, def, "asterix_cat062_1_18.xml")) {
 *     // Category loaded successfully
 * }
 * fclose(f);
 * ```
 *
 * ## Error Handling
 *
 * The parser stops on first error and sets `m_bErrorDetectedStopParsing = true`.
 * Error messages include line numbers and element context.
 *
 * @note Uses expat (libexpat) for XML parsing
 * @note Not thread-safe - use one parser instance per thread
 * @note All parsed data is stored in the AsterixDefinition singleton
 *
 * @see AsterixDefinition
 * @see Category
 * @see UAP
 * @see DataItemDescription
 */
class XMLParser {
public:
    /**
     * @brief Construct a new XML parser
     * @note Initializes expat parser and internal state
     */
    XMLParser();

    /**
     * @brief Destructor - cleans up expat parser
     */
    virtual
    ~XMLParser();

    /**
     * @brief Parse an ASTERIX category definition XML file
     * @param pFile Open FILE pointer to XML file (must be readable)
     * @param pDefinition AsterixDefinition to populate with parsed data
     * @param filename Name of file being parsed (for error messages)
     * @return true if parsing successful, false on error
     * @note Stops parsing on first error and sets m_bErrorDetectedStopParsing
     * @note Reads file in 8KB chunks (BUFFSIZE)
     * @note Caller is responsible for opening and closing the FILE*
     */
    bool Parse(FILE *pFile, AsterixDefinition *pDefinition, const char *filename);

    bool m_bErrorDetectedStopParsing; //!< Flag to stop parsing if error detected

    AsterixDefinition *m_pDef; //!< Destination for parsed category data
    Category *m_pCategory; //!< Currently parsed <Category> element
    DataItemDescription *m_pDataItem; //!< Currently parsed <DataItem> element
    DataItemFormat *m_pFormat; //!< Currently parsed <Format> element
    BitsValue *m_pBitsValue; //!< Currently parsed <BitsValue> element
    UAPItem *m_pUAPItem; //!< Currently parsed <UAPItem> element
    UAP *m_pUAP; //!< Currently parsed <UAP> element

    std::string *m_pstrCData; //!< Pointer to string for next CDATA content
    int *m_pintCData; //!< Pointer to int for next CDATA content
    const char *m_pFileName; //!< Current file name (for error messages)

    /**
     * @brief Set destination for next CDATA content (string)
     * @param pstr Pointer to std::string to receive CDATA
     * @note Internal helper for XML element handlers
     */
    void GetCData(std::string *pstr) { m_pstrCData = pstr; }

    /**
     * @brief Set destination for next CDATA content (integer)
     * @param pint Pointer to int to receive CDATA
     * @note Internal helper for XML element handlers
     */
    void GetCData(int *pint) {
        m_pintCData = pint;
        m_pstrCData = NULL;
    }

    /**
     * @brief expat callback for XML element start tags
     * @param data Pointer to XMLParser instance (this)
     * @param el Element name (e.g., "Category", "DataItem")
     * @param attr Array of attribute name/value pairs
     * @note Static callback required by expat library API
     */
    static void XMLCALL
    ElementHandlerStart(void *data, const char *el, const char **attr);

    /**
     * @brief expat callback for XML element end tags
     * @param data Pointer to XMLParser instance (this)
     * @param el Element name
     * @note Static callback required by expat library API
     */
    static void XMLCALL
    ElementHandlerEnd(void *data, const char *el);

    /**
     * @brief expat callback for XML character data (CDATA)
     * @param userData Pointer to XMLParser instance (this)
     * @param s Character data buffer
     * @param len Length of character data
     * @note Static callback required by expat library API
     */
    static void XMLCALL
    CharacterHandler(void *userData, const XML_Char *s, int len);

    /**
     * @brief Print error message with XML line number
     * @param errstr Error description
     * @note Sets m_bErrorDetectedStopParsing to true
     */
    void Error(const char *errstr);

    /**
     * @brief Print error message with parameter and XML line number
     * @param errstr Error description format string
     * @param param1 Parameter to include in error message
     * @note Sets m_bErrorDetectedStopParsing to true
     */
    void Error(const char *errstr, const char *param1);

private:
    XML_Parser m_Parser; //!< expat XML parser instance

    char m_pBuff[BUFFSIZE]; //!< 8KB buffer for reading XML file

    /**
     * @brief Extract string attribute from current XML element
     * @param elementName Element name (for error reporting)
     * @param attrName Attribute name to extract
     * @param ptrString Destination for attribute value
     * @return true if attribute found, false otherwise
     */
    bool GetAttribute(const char *elementName, const char *attrName, std::string *ptrString);

    /**
     * @brief Extract integer attribute from current XML element
     * @param elementName Element name (for error reporting)
     * @param attrName Attribute name to extract
     * @param ptrInt Destination for attribute value (parsed as int)
     * @return true if attribute found and valid integer, false otherwise
     */
    bool GetAttribute(const char *elementName, const char *attrName, int *ptrInt);
};

#endif /* XMLPARSER_H_ */
