#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Parse ASTERIX category XML definitions and generate SQL schemas.

This example demonstrates how to parse ASTERIX category definition XML files
and extract structured information about data items, fields, and their
descriptions. It's a utility script for:
- Understanding ASTERIX category structures
- Generating database schemas from ASTERIX definitions
- Creating data dictionaries for ASTERIX categories
- Analyzing field hierarchies and relationships

The script processes XML files that follow the ASTERIX DTD specification,
extracting:
- Data item IDs (e.g., I010, I062, I500)
- Field presences (compound field indicators)
- Bit definitions and their meanings
- Field descriptions and value enumerations

Output Formats:
    1. Tab-separated text: Field definitions with descriptions
    2. JSON: Structured field dictionary
    3. SQL: CREATE TABLE statement for database schema

XML Structure Parsed:
    - Fixed format items: Single fixed-length fields
    - Variable format items: Extensible fields with FX bit
    - Compound items: Multiple sub-fields with presence indicators
    - Repetitive items: Variable-length repeated structures
    - Explicit items: Complex nested structures

Usage:
    python xml_parser.py

    Output is printed to stdout in three sections:
    1. Tab-separated field definitions
    2. JSON representation
    3. SQL CREATE TABLE statement

Requirements:
    - Python 3.x standard library (xml.dom.minidom)
    - ASTERIX category XML file (default: asterix_cat062_1_18.xml)

Example Output:
    I010	SAC		System Area Code
    I010	SIC		System Identification Code
    I015	SI		Service Identification
    ...

    ===================================================================================
    ======                         json                                        =======
    ===================================================================================
    {
        "i010_sac": "System Area Code",
        "i010_sic": "System Identification Code",
        ...
    }

    ===================================================================================
    ======                    create table sql                                 =======
    ===================================================================================
    CREATE TABLE IF NOT EXISTS TABLE_NAME (
        `i010_sac` STRING COMMENT "System Area Code",
        `i010_sic` STRING COMMENT "System Identification Code",
        ...
    );

Configuration:
    xml_file: Path to ASTERIX category XML file
    Default: "asterix/config/asterix_cat062_1_18.xml"

See Also:
    - ASTERIX specification: asterix/config/asterix.dtd
    - asterix-specs-converter: Tools for XML generation
    - read_raw_file.py: Using parsed ASTERIX data
"""

# Created by xiaojian on 2020/6/23
# Read xml file and parser for create table sql

__author__ = 'xiaojian'

import logging
import json
# Security Note: These XML modules are used to parse LOCAL, TRUSTED ASTERIX config
# files shipped with this project. The XML files are static definitions, not
# user-provided or external data. XXE and other XML attacks are not applicable.
import xml.dom.minidom  # nosec B408 - parsing trusted local config files
from xml.dom import Node  # nosec B408
from xml.dom.minidom import Text  # nosec B408
from xml.dom.minidom import Element  # nosec B408
from xml.dom.minidom import parseString  # nosec B408

logging.basicConfig(level=logging.INFO)

# XML file path - update this to parse different ASTERIX categories
xml_file = "asterix/config/asterix_cat062_1_18.xml"

# Parse XML document
# nosec B318 - parsing trusted local ASTERIX config file, not untrusted input
dom = xml.dom.minidom.parse(xml_file)  # nosec B318
root = dom.documentElement

# Global counter for variable format items
VariableId = 1

# Dictionary to store parsed results
# Key format: "i010_sac" (lowercase, underscore-separated)
# Value: Field description
resultJS = {}


def printJS(line):
    """
    Parse tab-separated line and add to result dictionary.

    This function processes field definitions in the format:
    ItemID<TAB>Presence<TAB>Field<TAB>PresenceInfo<TAB>FieldInfo

    It constructs dictionary keys by joining non-empty components with
    underscores and converting to lowercase.

    Args:
        line (str): Tab-separated field definition line

    Returns:
        None

    Side Effects:
        Updates global resultJS dictionary with field definitions

    Example:
        >>> printJS("I010\tSAC\t\t\tSystem Area Code")
        # Adds to resultJS: {"i010_sac": "System Area Code"}

        >>> printJS("I500\tDOP\tDOP_x\tDilation of Precision\tX-component")
        # Adds to resultJS: {"i500_dop_dop_x": "X-component"}
    """
    print(line)
    row = line.split("\t", -1)
    item_id = row[0]
    presence = row[1]
    presence_info = row[3]
    field = row[2]
    field_info = row[4]

    # Build dictionary key from components
    if len(presence) > 0:
        if len(field) > 0:
            # Compound field with sub-field: "i010_compound_field"
            resultJS[str.lower("_".join([item_id, presence, field]))] = field_info
        else:
            # Compound field without sub-field: "i010_compound"
            resultJS[str.lower("_".join([item_id, presence]))] = presence_info
    elif len(field) > 0:
        # Simple field: "i010_field"
        resultJS[str.lower("_".join([item_id, field]))] = field_info


def getBit(Bit, **kwargs):
    """
    Extract text content from a Bits element.

    Args:
        Bit: XML element containing bit information
        **kwargs: Optional context parameters (unused)

    Returns:
        str: Text content of the element, or None if empty

    Example:
        >>> # XML: <BitsShortName>SAC</BitsShortName>
        >>> element = ... # XML element
        >>> text = getBit(element)
        >>> print(text)
        SAC
    """
    for text in Bit.childNodes:
        return text.nodeValue


def printBits(Bits, **kwargs):
    """
    Parse and print bit field definition.

    Extracts bit field information including short name, description,
    and enumerated values.

    Args:
        Bits: XML Bits element
        **kwargs: Optional context including:
            - DataItemId: Parent data item identifier
            - Variable: Dictionary of presence indicators for compound items

    Side Effects:
        Prints field definition and updates resultJS dictionary
    """
    BitsShortName = ""
    BitsName = ""
    valIndex = 0

    for Bit in Bits.childNodes:
        if type(Bit) is not Element:
            continue
        if Bit.nodeName == "BitsShortName":
            BitsShortName = getBit(Bit, **kwargs)
        if Bit.nodeName == "BitsName":
            BitsName = getBit(Bit, **kwargs)
            if BitsName is None:
                BitsName = ""
        if Bit.nodeName == "BitsValue":
            # Append enumerated values: "0:value0;1:value1;..."
            BitsName += "{}:{};".format(valIndex, getBit(Bit, **kwargs))
            valIndex += 1

    # Format output based on whether this is part of a compound item
    if "Variable" in kwargs:
        global VariableId
        printJS("{}\t{}\t{}\t{}\t{}".format(
            kwargs["DataItemId"],
            kwargs["Variable"][str(VariableId)][0],
            BitsShortName,
            kwargs["Variable"][str(VariableId)][1],
            BitsName
        ))
    else:
        printJS("{}\t{}\t{}\t{}\t{}".format(
            kwargs["DataItemId"], "", BitsShortName, "", BitsName
        ))


def getBits(Bits, **kwargs):
    """
    Extract bit field metadata for compound items.

    Retrieves presence indicator, short name, and description for a
    bit field within a compound data item.

    Args:
        Bits: XML Bits element
        **kwargs: Optional context parameters

    Returns:
        list: [BitsPresence, [BitsShortName, BitsName]]
            Special cases:
            - "spare" fields get presence = "-1"
            - "FX" extension fields get presence = "0"
    """
    BitsPresence = ""
    BitsShortName = ""
    BitsName = ""

    for Bit in Bits.childNodes:
        if type(Bit) is not Element:
            continue
        if Bit.nodeName == "BitsPresence":
            BitsPresence = getBit(Bit, **kwargs)
        if Bit.nodeName == "BitsShortName":
            BitsShortName = getBit(Bit, **kwargs)
        if Bit.nodeName == "BitsName":
            BitsName = getBit(Bit, **kwargs)

    # Handle special field types
    if BitsShortName == "spare":
        BitsPresence = "-1"
    if BitsShortName == "FX":
        BitsPresence = "0"

    return [BitsPresence, [BitsShortName, BitsName]]


def printFixed(Fixed, **kwargs):
    """
    Process and print Fixed format data item.

    Fixed format items have a predetermined length with multiple bit fields.

    Args:
        Fixed: XML Fixed element
        **kwargs: Context including DataItemId

    Side Effects:
        Prints all bit fields and increments global VariableId
    """
    global VariableId
    for Bits in Fixed.childNodes:
        if type(Bits) is not Element:
            continue
        printBits(Bits, **kwargs)
    VariableId += 1


def gitFixed(Fixed, **kwargs):
    """
    Extract field definitions from Fixed format item.

    Builds a dictionary mapping presence indicators to field metadata.

    Args:
        Fixed: XML Fixed element
        **kwargs: Optional context parameters

    Returns:
        dict: Mapping of presence indicators to [shortname, description]
    """
    Variable = {}
    for Bits in Fixed.childNodes:
        if type(Bits) is not Element:
            continue
        [BitsPresence, [BitsShortName, BitsName]] = getBits(Bits, **kwargs)
        if BitsShortName is None:
            BitsShortName = ""
        if BitsName is None:
            BitsName = ""
        Variable[BitsPresence] = [BitsShortName, BitsName]
    return Variable


def getDataItemId(DataItem):
    """
    Extract data item ID from XML element.

    Args:
        DataItem: XML DataItem element with 'id' attribute

    Returns:
        str: Data item ID prefixed with 'I' (e.g., "I010"), or -1 if not found

    Example:
        >>> # XML: <DataItem id="010">...</DataItem>
        >>> element = ... # XML element
        >>> item_id = getDataItemId(element)
        >>> print(item_id)
        I010

    Note:
        Returns -1 (integer) instead of None when the id attribute is missing.
        This is used for error handling in the parsing logic.
    """
    if type(DataItem._attrs) == dict:
        return "I" + DataItem._attrs["id"]._value
    else:
        return -1


def printVariable(Variable, **kwargs):
    """
    Process and print Variable format data item.

    Variable format items have extensible length using FX (extension) bits.

    Args:
        Variable: XML Variable element
        **kwargs: Context including DataItemId
    """
    for Fixed in Variable.childNodes:
        if type(Fixed) is not Element:
            continue
        if Fixed.nodeName == "Fixed":
            printFixed(Fixed, DataItemId=DataItemId)


def getVariable(Variable, **kwargs):
    """
    Extract all field definitions from Variable format item.

    Args:
        Variable: XML Variable element
        **kwargs: Optional context parameters

    Returns:
        dict: Combined dictionary of all presence indicators and their fields
    """
    Variables = {}
    for Fixed in Variable.childNodes:
        if type(Fixed) is not Element:
            continue
        if Fixed.nodeName == "Fixed":
            Variable = gitFixed(Fixed, DataItemId=DataItemId)
            for key in Variable.keys():
                Variables[key] = Variable[key]
    return Variables


def printRepetitive(Repetitive, **kwargs):
    """
    Process and print Repetitive format data item.

    Repetitive items contain a repetition count followed by repeated structures.

    Args:
        Repetitive: XML Repetitive element
        **kwargs: Context including DataItemId
    """
    for DataItemFormat in Repetitive.childNodes:
        if type(DataItemFormat) is not Element:
            continue
        if DataItemFormat.nodeName == "Fixed":
            printFixed(DataItemFormat, **kwargs)


def printCompound(Compound, **kwargs):
    """
    Process and print Compound format data item.

    Compound items contain multiple optional sub-items indicated by presence bits.

    Args:
        Compound: XML Compound element
        **kwargs: Context including DataItemId

    Side Effects:
        Processes Variable sub-items first to build presence dictionary,
        then processes Fixed and Repetitive sub-items
    """
    for DataItemFormat in Compound.childNodes:
        if type(DataItemFormat) is not Element:
            continue
        if DataItemFormat.nodeName == "Variable":
            kwargs["Variable"] = getVariable(DataItemFormat)
        else:
            if DataItemFormat.nodeName == "Fixed":
                printFixed(DataItemFormat, **kwargs)
            elif DataItemFormat.nodeName == "Repetitive":
                printRepetitive(DataItemFormat, **kwargs)

    # Print special fields (spare and FX)
    for BitsPresence in ["-1", "0"]:
        if BitsPresence in kwargs["Variable"]:
            printJS("{}\t{}\t{}\t{}\t{}".format(
                kwargs["DataItemId"],
                kwargs["Variable"][BitsPresence][0],
                "",
                kwargs["Variable"][BitsPresence][1],
                ""
            ))


def printExplicit(Explicit, **kwargs):
    """
    Process and print Explicit format data item.

    Explicit items have a length indicator followed by format-specific data.

    Args:
        Explicit: XML Explicit element
        **kwargs: Context including DataItemId
    """
    for DataItemFormat in Explicit.childNodes:
        if type(DataItemFormat) is not Element:
            continue
        if DataItemFormat.nodeName == "Compound":
            printCompound(DataItemFormat, DataItemId=DataItemId)


def map2sql(map, key, depth):
    """
    Convert nested dictionary to SQL struct definition.

    Recursively builds SQL struct types for nested field hierarchies.

    Args:
        map (dict): Nested dictionary of field definitions
        key (str): Current field key
        depth (int): Nesting depth (for indentation)

    Returns:
        str: SQL struct definition
    """
    rows = []
    for k in map.keys():
        if type(map[k]) == str:
            rows.append(struct_str2sql(map, k, depth + 1))
        elif type(map[k]) == dict:
            rows.append(map2sql(map[k], k, depth + 1))

    if depth == 1:
        return """{}`_{}` struct \n{}<\n{}\n{}>""".format(
            "\t\t" * depth, key, "\t\t" * depth, ",\n".join(rows), "\t\t" * depth
        )
    else:
        return """{}`_{}`:struct \n{}<\n{}\n{}>""".format(
            "\t\t" * depth, key, "\t\t" * depth, ",\n".join(rows), "\t\t" * depth
        )


def str2sql(map, key, depth):
    """
    Convert simple field to SQL column definition with COMMENT.

    Args:
        map (dict): Field dictionary
        key (str): Field key
        depth (int): Indentation depth

    Returns:
        str: SQL column definition
    """
    return """ {}`{}` STRING comment "{}" """.format("\t\t" * depth, key, map[key])


def struct_str2sql(map, key, depth):
    """
    Convert field to SQL struct member definition (no comment).

    Args:
        map (dict): Field dictionary
        key (str): Field key
        depth (int): Indentation depth

    Returns:
        str: SQL struct member definition
    """
    return """ {}`{}`:STRING""".format("\t\t" * depth, key)


def json2sql(map):
    """
    Convert complete field dictionary to SQL CREATE TABLE statement.

    This function processes the entire parsed ASTERIX category definition
    and generates SQL column definitions suitable for database schema creation.

    Args:
        map (dict): Complete field dictionary from ASTERIX XML parsing

    Returns:
        list: List of SQL column definitions (strings)

    Example:
        >>> field_map = {
        ...     "i010_sac": "System Area Code",
        ...     "i010_sic": "System Identification Code"
        ... }
        >>> sql_lines = json2sql(field_map)
        >>> for line in sql_lines:
        ...     print(line)
         `i010_sac` STRING comment "System Area Code"
         `i010_sic` STRING comment "System Identification Code"
    """
    rows = []
    for key in map.keys():
        if type(map[key]) == str:
            rows.append(str2sql(map, key, 1))
        elif type(map[key]) == dict:
            rows.append(map2sql(map[key], key, 1))
    return rows


def main():
    """
    Parse ASTERIX category XML and generate output in multiple formats.

    This function:
    1. Iterates through all DataItem elements in the XML
    2. Processes each data item format (Fixed, Variable, Compound, Explicit)
    3. Prints field definitions as tab-separated text
    4. Generates JSON representation
    5. Creates SQL CREATE TABLE statement

    The script processes ASTERIX category definitions following the DTD
    structure and extracts comprehensive field information.

    Returns:
        None

    Output Structure:
        1. Tab-separated text (for parsing/processing)
        2. JSON (for programmatic use)
        3. SQL (for database schema creation)

    Example:
        >>> main()
        I010	SAC		System Area Code
        I010	SIC		System Identification Code
        I015	SI		Service Identification
        ...

        ===================================================================================
        ======                         json                                        =======
        ===================================================================================
        {
            "i010_sac": "System Area Code",
            "i010_sic": "System Identification Code",
            ...
        }

        ===================================================================================
        ======                    create table sql                                 =======
        ===================================================================================
        CREATE TABLE IF NOT EXISTS TABLE_NAME (
            `i010_sac` STRING COMMENT "System Area Code",
            `i010_sic` STRING COMMENT "System Identification Code",
            ...
        );

    Note:
        To parse a different ASTERIX category, modify the xml_file variable
        at the top of the script:

        >>> xml_file = "asterix/config/asterix_cat048_1_27.xml"

        The script supports all ASTERIX categories defined in the asterix/config/
        directory, including CAT001, CAT002, CAT008, CAT019, CAT020, CAT021,
        CAT023, CAT025, CAT034, CAT048, CAT062, CAT063, CAT065, and CAT252.
    """
    # Process all data items in the XML
    for DataItem in root.childNodes:
        if type(DataItem) is not Element:
            continue

        DataItemId = getDataItemId(DataItem)

        for DataItemFormats in DataItem.childNodes:
            if type(DataItemFormats) is not Element:
                continue
            if DataItemFormats.nodeName == "DataItemFormat":
                for DataItemFormat in DataItemFormats.childNodes:
                    global VariableId
                    VariableId = 1

                    if type(DataItemFormat) is not Element:
                        continue

                    # Process each format type
                    if DataItemFormat.nodeName == "Fixed":
                        printFixed(DataItemFormat, DataItemId=DataItemId)
                    if DataItemFormat.nodeName == "Variable":
                        printVariable(DataItemFormat, DataItemId=DataItemId)
                    if DataItemFormat.nodeName == "Compound":
                        printCompound(DataItemFormat, DataItemId=DataItemId)
                    if DataItemFormat.nodeName == "Explicit":
                        printExplicit(DataItemFormat, DataItemId=DataItemId)

    # Output JSON representation
    print("\n")
    print("=" * 86)
    print("======                         json                                            =======")
    print("=" * 86)
    print(json.dumps(resultJS, sort_keys=True, indent=4))

    # Output SQL CREATE TABLE statement
    print("\n")
    print("=" * 86)
    print("======                    create table sql                                     =======")
    print("=" * 86)
    sql = "CREATE TABLE IF NOT EXISTS TABLE_NAME (\n{}\n);".format(",\n".join(json2sql(resultJS)))
    print(sql)


if __name__ == '__main__':
    main()
