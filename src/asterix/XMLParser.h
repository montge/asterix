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

/**
 * @file XMLParser.h
 * @brief XML parser for ASTERIX category definition files
 *
 * This file defines the XMLParser class which loads ASTERIX category definitions
 * from XML configuration files (asterix_cat*.xml) that conform to the asterix.dtd
 * specification. The parser uses the Expat SAX-based XML parser to process category
 * definitions, data item descriptions, formats, and User Application Profiles (UAPs).
 *
 * @par XML Structure Overview
 * ASTERIX category definitions follow this hierarchical structure:
 * @code{.xml}
 * <?xml version="1.0" encoding="UTF-8"?>
 * <!DOCTYPE Category SYSTEM "asterix.dtd">
 * <Category id="62" name="SDPS Track Messages" ver="1.21">
 *     <DataItem id="010">
 *         <DataItemName>Data Source Identifier</DataItemName>
 *         <DataItemDefinition>Identification of the system sending the data.</DataItemDefinition>
 *         <DataItemFormat desc="2-octet fixed length data item.">
 *             <Fixed length="2">
 *                 <Bits from="16" to="9">
 *                     <BitsShortName>SAC</BitsShortName>
 *                     <BitsName>System Area Code</BitsName>
 *                 </Bits>
 *                 <Bits from="8" to="1">
 *                     <BitsShortName>SIC</BitsShortName>
 *                     <BitsName>System Identification Code</BitsName>
 *                 </Bits>
 *             </Fixed>
 *         </DataItemFormat>
 *     </DataItem>
 *     <UAP>
 *         <UAPItem bit="0" frn="1">010</UAPItem>
 *         <UAPItem bit="1" frn="2">015</UAPItem>
 *         ...
 *     </UAP>
 * </Category>
 * @endcode
 *
 * @par DTD Specification
 * Category definitions must conform to asterix.dtd which defines:
 * - **Category**: Root element with id, name, ver attributes
 * - **DataItem**: Data item definition (id, rule attributes)
 * - **DataItemFormat**: One of Fixed, Variable, Compound, Repetitive, Explicit, BDS
 * - **Bits**: Bit field definitions with encoding, FX bits, scaling, value meanings
 * - **UAP**: User Application Profile mapping FRNs to data items
 *
 * @par Parsing Process
 * The parser uses a state machine approach with SAX callbacks:
 * 1. **Start element handler** - Creates objects (Category, DataItem, Format)
 * 2. **Character handler** - Collects CDATA (names, definitions, values)
 * 3. **End element handler** - Finalizes objects and adds to parent containers
 * 4. **Error handling** - Reports line numbers and stops on malformed XML
 *
 * @par Supported Format Types
 * - **Fixed**: Fixed-length items (e.g., 2-byte SAC/SIC)
 * - **Variable**: Variable-length with FX (Field Extension) bits
 * - **Compound**: Multiple sub-items with presence bitmap
 * - **Repetitive**: Array of repeated sub-items with count byte
 * - **Explicit**: Explicit length field followed by data
 * - **BDS**: Comm-B Data Buffer register format (Mode S)
 *
 * @par Configuration Files
 * Category XML files are typically located in asterix/config/:
 * - asterix_cat001_1_4.xml (Cat 001 v1.4 - Monoradar Target Reports)
 * - asterix_cat062_1_21.xml (Cat 062 v1.21 - SDPS Track Messages)
 * - asterix_bds.xml (BDS register definitions for Mode S)
 * - asterix.ini (lists all category files to load)
 *
 * @par Example Usage
 * @code
 * #include "XMLParser.h"
 * #include "AsterixDefinition.h"
 * #include <stdio.h>
 *
 * // Initialize ASTERIX definition container
 * AsterixDefinition* pDef = new AsterixDefinition();
 *
 * // Parse category definition file
 * XMLParser parser;
 * FILE* pFile = fopen("asterix/config/asterix_cat062_1_21.xml", "r");
 * if (pFile) {
 *     bool success = parser.Parse(pFile, pDef, "asterix_cat062_1_21.xml");
 *     fclose(pFile);
 *
 *     if (success) {
 *         // Category now registered in pDef
 *         Category* cat62 = pDef->getCategory(62, "1.21");
 *         if (cat62) {
 *             printf("Loaded category: %s\n", cat62->m_strName.c_str());
 *         }
 *     } else {
 *         fprintf(stderr, "Parse error detected\n");
 *     }
 * }
 * @endcode
 *
 * @par Thread Safety
 * XMLParser is NOT thread-safe. Do not access the same XMLParser instance
 * from multiple threads concurrently. Create separate instances per thread.
 *
 * @par Memory Management
 * - XMLParser does not own AsterixDefinition (passed as parameter)
 * - Category, DataItem, Format objects are owned by AsterixDefinition
 * - Parser state is reset between Parse() calls
 *
 * @see AsterixDefinition Container for all loaded category definitions
 * @see Category Represents a single ASTERIX category
 * @see DataItemDescription Metadata for individual data items
 * @see DataItemFormat Base class for format parsers (Fixed, Variable, etc.)
 * @see InputParser High-level interface for loading configurations
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

//! XML parsing buffer size (8 KB for reading XML files in chunks)
#define BUFFSIZE        8192

/**
 * @class XMLParser
 * @brief SAX-based XML parser for ASTERIX category definition files
 *
 * XMLParser loads ASTERIX category definitions from XML configuration files
 * (asterix_cat*.xml) using the Expat SAX parser. It processes category metadata,
 * data item descriptions, format specifications, bit field definitions, and
 * User Application Profiles (UAPs) according to the asterix.dtd specification.
 *
 * @par Parsing Architecture
 * The parser uses a state machine with SAX event callbacks:
 * - **ElementHandlerStart**: Creates objects when entering XML elements
 * - **CharacterHandler**: Collects CDATA text content
 * - **ElementHandlerEnd**: Finalizes objects and adds to parent containers
 * - **Error handling**: Reports errors with line numbers and stops parsing
 *
 * @par Parser State Machine
 * During parsing, member variables track the current context:
 * - m_pCategory: Currently parsed Category element
 * - m_pDataItem: Currently parsed DataItem element
 * - m_pFormat: Currently parsed format (Fixed, Variable, Compound, etc.)
 * - m_pBitsValue: Currently parsed BitsValue element
 * - m_pUAPItem: Currently parsed UAP item
 * - m_pUAP: Currently parsed UAP element
 *
 * @par XML Element Hierarchy
 * The parser handles this element hierarchy:
 * @code
 * Category (id, name, ver)
 * ├── DataItem (id, rule)
 * │   ├── DataItemName (CDATA)
 * │   ├── DataItemDefinition (CDATA)
 * │   ├── DataItemFormat (desc)
 * │   │   ├── Fixed (length)
 * │   │   │   └── Bits (bit|from/to, encode, fx, rep)
 * │   │   │       ├── BitsShortName (CDATA)
 * │   │   │       ├── BitsName (CDATA)
 * │   │   │       ├── BitsValue (val) → (CDATA)
 * │   │   │       ├── BitsUnit (scale, min, max) → (CDATA)
 * │   │   │       └── BitsConst (CDATA)
 * │   │   ├── Variable → Fixed+
 * │   │   ├── Compound → Variable, (Fixed|Variable|Compound|Repetitive|Explicit)*
 * │   │   ├── Repetitive → Fixed+|BDS
 * │   │   ├── Explicit → (Fixed|Variable|Compound|Repetitive|Explicit)
 * │   │   └── BDS (CDATA)
 * │   └── DataItemNote (CDATA)
 * └── UAP (use_if_bit_set, use_if_byte_nr, is_set_to)
 *     └── UAPItem (bit, frn, fx, len) → (CDATA: item id)
 * @endcode
 *
 * @par CDATA Collection Mechanism
 * The parser uses a pointer-based system to collect text content:
 * - GetCData(std::string*) - Sets target for next CDATA content
 * - GetCData(int*) - Sets target for next CDATA integer value
 * - CharacterHandler - Appends/assigns CDATA to registered target
 *
 * @par Error Handling
 * - Malformed XML: Reports Expat parser errors with line numbers
 * - Invalid attributes: Reports missing required attributes
 * - Unsupported elements: Warns about unrecognized elements
 * - Stops parsing on first error (m_bErrorDetectedStopParsing = true)
 *
 * @par Format Type Mapping
 * XML elements map to C++ format classes:
 * - \<Fixed\> → DataItemFormatFixed
 * - \<Variable\> → DataItemFormatVariable
 * - \<Compound\> → DataItemFormatCompound
 * - \<Repetitive\> → DataItemFormatRepetitive
 * - \<Explicit\> → DataItemFormatExplicit
 * - \<BDS\> → DataItemFormatBDS
 *
 * @par Thread Safety
 * XMLParser is NOT thread-safe. Each thread must use its own XMLParser instance.
 * The Expat parser maintains internal state that cannot be shared across threads.
 *
 * @par Memory Management
 * - XMLParser does not own AsterixDefinition (caller-owned)
 * - Created Category/DataItem/Format objects are added to AsterixDefinition
 * - Parser state pointers (m_pCategory, m_pDataItem, etc.) are non-owning
 * - Expat parser is created/destroyed per Parse() call
 *
 * @par Example Usage
 * @code
 * #include "XMLParser.h"
 * #include "AsterixDefinition.h"
 * #include <stdio.h>
 *
 * AsterixDefinition asterixDef;
 * XMLParser parser;
 *
 * // Parse single category file
 * FILE* file = fopen("asterix/config/asterix_cat062_1_21.xml", "r");
 * if (file) {
 *     if (parser.Parse(file, &asterixDef, "asterix_cat062_1_21.xml")) {
 *         printf("Successfully parsed category 62 v1.21\n");
 *     } else {
 *         fprintf(stderr, "Parse error in file\n");
 *     }
 *     fclose(file);
 * }
 *
 * // Access parsed category
 * Category* cat = asterixDef.getCategory(62, "1.21");
 * if (cat) {
 *     printf("Category: %s\n", cat->m_strName.c_str());
 * }
 * @endcode
 *
 * @par DTD Compliance
 * XML files must declare the asterix.dtd document type:
 * @code{.xml}
 * <?xml version="1.0" encoding="UTF-8"?>
 * <!DOCTYPE Category SYSTEM "asterix.dtd">
 * @endcode
 *
 * @see AsterixDefinition Container for all loaded categories
 * @see Category Represents a single ASTERIX category with metadata
 * @see DataItemDescription Metadata for individual data items
 * @see DataItemFormat Base class for format parsers
 * @see InputParser High-level interface for loading all configurations
 * @see UAP User Application Profile (FRN to data item mapping)
 */
class XMLParser {
public:
    /**
     * @brief Construct a new XMLParser instance
     *
     * Initializes parser state with null pointers. The Expat parser
     * is created during Parse() and destroyed afterward.
     */
    XMLParser();

    /**
     * @brief Destructor
     *
     * Cleans up any remaining parser state. Expat parser is already
     * destroyed by the time destructor is called.
     */
    virtual
    ~XMLParser();

    /**
     * @brief Parse ASTERIX category definition from XML file
     *
     * Parses an XML file containing ASTERIX category definitions and adds
     * the resulting Category object to the AsterixDefinition container.
     * Uses Expat SAX parser to process XML in chunks (BUFFSIZE = 8192 bytes).
     *
     * The parser processes:
     * - Category metadata (id, name, version)
     * - DataItem definitions (name, description, format, notes)
     * - Format specifications (Fixed, Variable, Compound, Repetitive, Explicit, BDS)
     * - Bit field definitions (names, encoding, scaling, value meanings)
     * - UAP (User Application Profile) mappings
     *
     * @param pFile Open file pointer to XML category file (must be readable)
     * @param pDefinition AsterixDefinition container to store parsed category
     * @param filename Filename for error reporting (e.g., "asterix_cat062_1_21.xml")
     *
     * @return true if parsing succeeded without errors
     * @return false if XML is malformed or parsing error occurred
     *
     * @par Error Reporting
     * Errors are printed to stderr with line numbers:
     * - "Error at line X: Malformed XML"
     * - "Error at line X: Missing required attribute 'id'"
     * - "Error at line X: Unknown element 'Foo'"
     *
     * @par Side Effects
     * - Creates Category object and adds to pDefinition
     * - Creates DataItemDescription objects for each DataItem
     * - Creates DataItemFormat objects (Fixed, Variable, etc.)
     * - Sets m_bErrorDetectedStopParsing on error
     *
     * @par Example
     * @code
     * XMLParser parser;
     * AsterixDefinition def;
     * FILE* f = fopen("asterix_cat062_1_21.xml", "r");
     * if (f) {
     *     bool ok = parser.Parse(f, &def, "asterix_cat062_1_21.xml");
     *     fclose(f);
     *     if (!ok) {
     *         fprintf(stderr, "Failed to parse category definition\n");
     *     }
     * }
     * @endcode
     *
     * @note The file pointer must remain valid for the duration of the Parse() call.
     * @note The parser reads the file in 8KB chunks (BUFFSIZE).
     * @note Parsing stops immediately on first error.
     *
     * @see AsterixDefinition::getCategory Access parsed categories
     * @see InputParser::parseDefaultConfiguration Parse all category files
     */
    bool Parse(FILE *pFile, AsterixDefinition *pDefinition, const char *filename);

    bool m_bErrorDetectedStopParsing; //!< Flag set to true when parsing error occurs (stops further processing)

    AsterixDefinition *m_pDef; //!< Pointer to AsterixDefinition container (non-owning)
    Category *m_pCategory; //!< Currently parsed Category element (non-owning, owned by m_pDef)
    DataItemDescription *m_pDataItem; //!< Currently parsed DataItem element (non-owning)
    DataItemFormat *m_pFormat; //!< Currently parsed Format element (Fixed/Variable/Compound/etc.) (non-owning)
    BitsValue *m_pBitsValue; //!< Currently parsed BitsValue element (non-owning)
    UAPItem *m_pUAPItem; //!< Currently parsed UAPItem element (non-owning)
    UAP *m_pUAP; //!< Currently parsed UAP element (non-owning)

    /**
     * @brief Pointer to string target for next CDATA content
     *
     * When CharacterHandler receives CDATA, it appends to this string.
     * Set via GetCData(std::string*).
     */
    std::string *m_pstrCData;

    /**
     * @brief Pointer to integer target for next CDATA content
     *
     * When CharacterHandler receives CDATA, it converts and assigns to this int.
     * Set via GetCData(int*).
     */
    int *m_pintCData;

    /**
     * @brief Current XML filename being parsed (for error reporting)
     */
    const char *m_pFileName;

    /**
     * @brief Register a string target for next CDATA content
     *
     * Sets m_pstrCData to point to the provided string. When CharacterHandler
     * is called, it appends CDATA to this string.
     *
     * @param pstr Pointer to string that will receive CDATA content
     *
     * @par Example
     * @code
     * std::string itemName;
     * parser.GetCData(&itemName);
     * // Next CDATA in CharacterHandler will append to itemName
     * @endcode
     */
    void GetCData(std::string *pstr) { m_pstrCData = pstr; }

    /**
     * @brief Register an integer target for next CDATA content
     *
     * Sets m_pintCData to point to the provided int and clears m_pstrCData.
     * When CharacterHandler is called, it converts CDATA to integer and assigns.
     *
     * @param pint Pointer to integer that will receive CDATA content (as integer)
     *
     * @par Example
     * @code
     * int categoryId;
     * parser.GetCData(&categoryId);
     * // Next CDATA in CharacterHandler will be converted to int and stored
     * @endcode
     */
    void GetCData(int *pint) {
        m_pintCData = pint;
        m_pstrCData = nullptr;
    }

    /**
     * @brief Expat SAX callback for XML element start tags
     *
     * Called by Expat when entering an XML element (e.g., \<Category\>, \<DataItem\>).
     * Creates appropriate C++ objects (Category, DataItem, Format) and extracts
     * attributes (id, name, ver, length, etc.).
     *
     * @param data User data pointer (XMLParser* instance)
     * @param el Element name (e.g., "Category", "DataItem", "Fixed")
     * @param attr Null-terminated array of attribute name/value pairs
     *
     * @par Element Actions
     * - "Category": Creates Category object, extracts id/name/ver
     * - "DataItem": Creates DataItemDescription, extracts id/rule
     * - "Fixed": Creates DataItemFormatFixed, extracts length
     * - "Variable": Creates DataItemFormatVariable
     * - "Compound": Creates DataItemFormatCompound
     * - "Repetitive": Creates DataItemFormatRepetitive
     * - "Explicit": Creates DataItemFormatExplicit
     * - "BDS": Creates DataItemFormatBDS
     * - "Bits": Creates Bits object, extracts bit/from/to/encode/fx
     * - "UAP": Creates UAP object, extracts use_if_* attributes
     * - "UAPItem": Creates UAPItem, extracts bit/frn/fx/len
     *
     * @note This is a static callback for Expat; it casts data to XMLParser*
     * @note Errors set m_bErrorDetectedStopParsing and call Error()
     *
     * @see ElementHandlerEnd For closing tag handler
     */
    static void XMLCALL
    ElementHandlerStart(void *data, const char *el, const char **attr);

    /**
     * @brief Expat SAX callback for XML element end tags
     *
     * Called by Expat when leaving an XML element (e.g., \</Category\>, \</DataItem\>).
     * Finalizes objects and adds them to parent containers.
     *
     * @param data User data pointer (XMLParser* instance)
     * @param el Element name (e.g., "Category", "DataItem", "Fixed")
     *
     * @par Element Actions
     * - "Category": Adds Category to AsterixDefinition
     * - "DataItem": Adds DataItemDescription to Category
     * - "Fixed/Variable/Compound/etc.": Links Format to DataItem
     * - "Bits": Adds Bits to Format
     * - "UAPItem": Adds UAPItem to UAP
     * - "UAP": Adds UAP to Category
     *
     * @note This is a static callback for Expat; it casts data to XMLParser*
     * @note Clears state pointers after adding to parent containers
     *
     * @see ElementHandlerStart For opening tag handler
     */
    static void XMLCALL
    ElementHandlerEnd(void *data, const char *el);

    /**
     * @brief Expat SAX callback for character data (CDATA)
     *
     * Called by Expat when processing text content between XML tags.
     * Appends or assigns content to registered target (m_pstrCData or m_pintCData).
     *
     * @param userData User data pointer (XMLParser* instance)
     * @param s Pointer to character data (not null-terminated)
     * @param len Length of character data in bytes
     *
     * @par Behavior
     * - If m_pstrCData is set: Appends s[0..len-1] to string
     * - If m_pintCData is set: Converts s[0..len-1] to integer and assigns
     * - If neither is set: Ignores character data (whitespace between elements)
     *
     * @note CDATA may be delivered in multiple chunks; use append, not assign
     * @note Character data is NOT null-terminated; must use len parameter
     *
     * @par Example XML Processing
     * @code{.xml}
     * <DataItemName>Data Source Identifier</DataItemName>
     * @endcode
     * CharacterHandler is called with s="Data Source Identifier", len=23
     *
     * @see GetCData(std::string*) Register string target
     * @see GetCData(int*) Register integer target
     */
    static void XMLCALL
    CharacterHandler(void *userData, const XML_Char *s, int len);

    /**
     * @brief Report parsing error with line number
     *
     * Prints error message to stderr with current XML line number and filename.
     * Sets m_bErrorDetectedStopParsing to true.
     *
     * @param errstr Error message to display
     *
     * @par Output Format
     * @code
     * Error at line 42 in file asterix_cat062_1_21.xml: errstr
     * @endcode
     *
     * @par Example
     * @code
     * Error("Missing required attribute 'id'");
     * // Output: Error at line 42 in file asterix_cat062_1_21.xml: Missing required attribute 'id'
     * @endcode
     */
    void Error(const char *errstr);

    /**
     * @brief Report parsing error with line number and parameter
     *
     * Prints error message to stderr with current XML line number, filename,
     * and an additional parameter (e.g., element name, attribute name).
     * Sets m_bErrorDetectedStopParsing to true.
     *
     * @param errstr Error message format string (may contain %s)
     * @param param1 Parameter to insert into error message
     *
     * @par Output Format
     * @code
     * Error at line 42 in file asterix_cat062_1_21.xml: errstr param1
     * @endcode
     *
     * @par Example
     * @code
     * Error("Unknown element", "Foo");
     * // Output: Error at line 42 in file asterix_cat062_1_21.xml: Unknown element Foo
     * @endcode
     */
    void Error(const char *errstr, const char *param1);

private:
    XML_Parser m_Parser; //!< Expat XML parser instance (created during Parse())

    char m_pBuff[BUFFSIZE]; //!< Buffer for reading XML file in chunks (8 KB)

    /**
     * @brief Extract string attribute from current XML element
     *
     * Searches for attribute in current element's attribute list and assigns
     * value to the provided string pointer.
     *
     * @param elementName Element name (for error reporting)
     * @param attrName Attribute name to search for
     * @param ptrString Pointer to string that will receive attribute value
     *
     * @return true if attribute found and assigned
     * @return false if attribute not found (reports error)
     */
    bool GetAttribute(const char *elementName, const char *attrName, std::string *ptrString);

    /**
     * @brief Extract integer attribute from current XML element
     *
     * Searches for attribute in current element's attribute list, converts
     * value to integer, and assigns to the provided int pointer.
     *
     * @param elementName Element name (for error reporting)
     * @param attrName Attribute name to search for
     * @param ptrInt Pointer to integer that will receive attribute value
     *
     * @return true if attribute found, converted, and assigned
     * @return false if attribute not found (reports error)
     */
    bool GetAttribute(const char *elementName, const char *attrName, int *ptrInt);

    // =========================================================================
    // Element Handler Methods (refactored from ElementHandlerStart)
    // =========================================================================

    /**
     * @brief Add a format to the current parent format or data item
     *
     * Common logic for adding Fixed, Variable, Compound, Repetitive, Explicit,
     * and BDS format objects to their parent container.
     *
     * @param pNewFormat The new format object to add
     * @param formatName Name of the format for error messages (e.g., "Fixed")
     * @param allowedInVariable Whether this format can be nested in Variable
     * @param allowedInRepetitive Whether this format can be nested in Repetitive
     * @param allowedInExplicit Whether this format can be nested in Explicit
     * @param allowedInCompound Whether this format can be nested in Compound
     * @param allowedAsFirstInCompound Whether this can be the first item in Compound
     * @return true if format was added successfully, false on error
     */
    bool addFormatToParent(DataItemFormat *pNewFormat, const char *formatName,
                           bool allowedInVariable, bool allowedInRepetitive,
                           bool allowedInExplicit, bool allowedInCompound,
                           bool allowedAsFirstInCompound);

    // Helper methods for handleBitsStart complexity reduction
    void parseBitRange(DataItemBits *pBits, const char *attrValue, int &target, const char *errorLabel);
    bool parseEncodeAttribute(DataItemBits *pBits, const char *value);

    // Element start handlers
    void handleCategoryStart(const char **attr);
    void handleDataItemStart(const char **attr);
    void handleDataItemFormatStart(const char **attr);
    void handleFixedStart(const char **attr);
    void handleVariableStart(const char **attr);
    void handleCompoundStart(const char **attr);
    void handleRepetitiveStart(const char **attr);
    void handleExplicitStart(const char **attr);
    void handleBDSStart(const char **attr);
    void handleBitsStart(const char **attr);
    void handleBitsValueStart(const char **attr);
    void handleBitsUnitStart(const char **attr);
    void handleBitsConstStart(const char **attr);
    void handleUAPStart(const char **attr);
    void handleUAPItemStart(const char **attr);

    // Element end handlers
    void handleCategoryEnd();
    void handleDataItemEnd();
    void handleFormatEnd(const char *formatName);
    void handleBitsEnd();
    void handleBitsValueEnd();
    void handleUAPEnd();
    void handleUAPItemEnd();
};

#endif /* XMLPARSER_H_ */
