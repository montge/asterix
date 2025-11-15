"""
Asterix is utility used for reading and parsing of EUROCONTROL ASTERIX protocol data from stdin, file or network multicast stream.

ASTERIX stands for All Purpose STructured EUROCONTROL SuRveillance Information EXchange.
It is an ATM Surveillance Data Binary Messaging Format which allows transmission of harmonised information between any surveillance and automation system.
ASTERIX defines the structure of the data to be exchanged over a communication medium, from the encoding of every bit of information up to the organisation of the data within a block of data - without any loss of information during the whole process.
More about ASTERIX protocol you can find here: http://www.eurocontrol.int/services/asterix

Asterix application was developed by Croatia Control Ltd.

This program comes with ABSOLUTELY NO WARRANTY. This is free software, and you are welcome to redistribute it under certain conditions. See COPYING file for details.

**Thread Safety Warning**:
    This module is NOT thread-safe. The ASTERIX parser uses global state to manage category
    definitions (AsterixDefinition singleton in C++). Concurrent calls to parse(), init(), or
    describe() from multiple threads will cause race conditions and undefined behavior.

    If you need to parse ASTERIX data from multiple threads, use one of these approaches:
    - Use a process-level lock (threading.Lock or multiprocessing.Lock) around all asterix calls
    - Use separate processes instead of threads (multiprocessing.Process)
    - Ensure all asterix operations happen in a single thread

Example:

    # Import the asterix module::
    import asterix

    # Prepare your asterix binary packet. E.g.::
    asterix_packet = bytearray([0x30, 0x00, 0x30, 0xfd, 0xf7, 0x02, 0x19, 0xc9, 0x35, 0x6d, 0x4d, 0xa0, 0xc5, 0xaf, 0xf1, 0xe0,
                        0x02, 0x00, 0x05, 0x28, 0x3c, 0x66, 0x0c, 0x10, 0xc2, 0x36, 0xd4, 0x18, 0x20, 0x01, 0xc0, 0x78,
                        0x00, 0x31, 0xbc, 0x00, 0x00, 0x40, 0x0d, 0xeb, 0x07, 0xb9, 0x58, 0x2e, 0x41, 0x00, 0x20, 0xf5])


    # Parse the packet with asterix module. It will return the list of Asterix records::
    parsed = asterix.parse(asterix_packet)
    print(parsed)

    # If you want to see textual presentation of asterix packet use describe::
	print(asterix.describe(parsed))

    # If you do not need description of items and meaning of bit values, you can disable the verbose mode, to just get values::
    parsed = asterix.parse(asterix_packet, verbose=false)
    print(parsed)

    # All Asterix categories are defined with XML configuration file.
    # You can get a list of configuration files ith following command::
    config = asterix.list_configuration_files()
    print(config)

    # And then get specific configuration file::
    configfile = asterix.get_configuration_file('cat048')
    with open(configfile, "rt") as f:
        config = f.read()
        print(config)

    # If you want to create your own Asterix definition file, create XML file using dtd definition::
    dtd = asterix.get_configuration_file('dtd')

    # And then initialize asterix with your configuration::
    asterix.init(path_to_your_config_file)

"""
import _asterix

try:
    from lxml import etree
    lxml_found = True
except ImportError:
    lxml_found = False

import os
import sys
from datetime import datetime
from .version import __version__


# def set_callback(callback):
#    return _asterix.set_callback(callback)


# def hello(world):
#    return _asterix.hello(world)


def init(filename: str) -> int:
    """Initializes asterix with XML category definition.

    This function loads a custom ASTERIX category definition from an XML file.
    Use this when you need to add support for custom ASTERIX categories or
    modify existing category definitions. All default categories are automatically
    loaded on module import.

    Warning:
        This function modifies global state and is NOT thread-safe. Do not call
        from multiple threads concurrently.

    Args:
        filename (str): Absolute or relative path to XML definition file. The file
            must conform to the ASTERIX DTD specification (see asterix.dtd).

    Returns:
        int: 0 if initialization successful.

    Raises:
        SyntaxError: If there is a syntax error in XML file.
        RuntimeError: If there is an internal parsing problem.
        IOError: If XML file is not found or cannot be read.
        ValueError: If parameter is not a string.
        TypeError: If incorrect number of parameters provided.

    Example:
        >>> import asterix
        >>> # Get DTD file for reference
        >>> dtd_file = asterix.get_configuration_file('dtd')
        >>> # Initialize with custom category definition
        >>> asterix.init('/path/to/custom_cat999_1_0.xml')
        0
        >>> # Now you can parse CAT999 data
        >>> data = b'\\x99\\x00\\x10...'  # CAT999 data
        >>> records = asterix.parse(data)
    """
    return _asterix.init(filename)


def parse(data: bytes, verbose: bool = True) -> list:
    """Parse raw ASTERIX data from bytes.

    Parses ASTERIX binary data and returns a list of decoded records. Each record
    contains the category, timestamp, data items, and their decoded values. This
    is the primary parsing function for processing ASTERIX surveillance data.

    Warning:
        This function uses global state and is NOT thread-safe. Do not call from
        multiple threads concurrently.

    Args:
        data (bytes): Raw ASTERIX binary data to parse. Can contain one or more
            ASTERIX data blocks. Supports CAT001-CAT255 if category definitions
            are loaded.
        verbose (bool, optional): If True, include descriptions, meanings, and
            min/max values for each data item. If False, return only raw values.
            Defaults to True.

    Returns:
        list: List of dictionaries, one per ASTERIX record. Each dictionary contains:
            - 'category' (int): ASTERIX category number (e.g., 48, 62)
            - 'len' (int): Record length in bytes
            - 'crc' (str): CRC checksum
            - 'ts' (int): Timestamp in milliseconds since epoch
            - 'hexdata' (str): Hexadecimal representation of raw data
            - 'I...' (dict/list): Data items (e.g., 'I010', 'I040') with parsed values

    Example:
        >>> import asterix
        >>> # CAT048 sample data (Monoradar Target Reports)
        >>> data = bytes([0x30, 0x00, 0x30, 0xfd, 0xf7, 0x02, 0x19, 0xc9,
        ...               0x35, 0x6d, 0x4d, 0xa0, 0xc5, 0xaf, 0xf1, 0xe0,
        ...               0x02, 0x00, 0x05, 0x28, 0x3c, 0x66, 0x0c, 0x10,
        ...               0xc2, 0x36, 0xd4, 0x18, 0x20, 0x01, 0xc0, 0x78,
        ...               0x00, 0x31, 0xbc, 0x00, 0x00, 0x40, 0x0d, 0xeb,
        ...               0x07, 0xb9, 0x58, 0x2e, 0x41, 0x00, 0x20, 0xf5])
        >>> # Parse with descriptions
        >>> records = asterix.parse(data)
        >>> print(f"Category: {records[0]['category']}")
        Category: 48
        >>> print(f"Items: {[k for k in records[0].keys() if k.startswith('I')]}")
        Items: ['I010', 'I040', 'I070', 'I220', ...]
        >>> # Parse without descriptions (faster, less memory)
        >>> records_compact = asterix.parse(data, verbose=False)
    """
    if 'verbose' in kwargs and not kwargs['verbose']:
        verbose = 0
    else:
        verbose = 1

    return _asterix.parse(bytes(data), verbose)


def parse_with_offset(data: bytes, offset: int = 0, blocks_count: int = 1000, verbose: bool = True) -> tuple:
    """Parse raw ASTERIX data with incremental offset tracking.

    This function enables incremental/streaming parsing of large ASTERIX data streams.
    It parses a specified number of data blocks starting from a given offset and
    returns the new offset for the next parsing iteration. Useful for processing
    continuous network streams or large files without loading all data into memory.

    Warning:
        This function uses global state and is NOT thread-safe. Do not call from
        multiple threads concurrently.

    Args:
        data (bytes): Raw ASTERIX binary data buffer to parse. Can contain multiple
            ASTERIX data blocks.
        offset (int, optional): Byte offset in data buffer to start parsing from.
            Use 0 for first call, then use returned offset for subsequent calls.
            Defaults to 0.
        blocks_count (int, optional): Maximum number of ASTERIX data blocks to parse
            in this call. Limits memory usage for large streams. Defaults to 1000.
        verbose (bool, optional): If True, include descriptions and metadata for
            each data item. If False, return only raw values. Defaults to True.

    Returns:
        tuple: A 2-element tuple containing:
            - records (list): List of parsed ASTERIX records (same format as parse())
            - new_offset (int): Byte offset where parsing stopped. Use this value
              as offset parameter in next call for incremental parsing.

    Example:
        >>> import asterix
        >>> # Read large PCAP file
        >>> with open('multicast_stream.pcap', 'rb') as f:
        ...     data = f.read()
        >>> # Incremental parsing: process 100 blocks at a time
        >>> offset = 0
        >>> all_records = []
        >>> while offset < len(data):
        ...     records, offset = asterix.parse_with_offset(
        ...         data, offset=offset, blocks_count=100, verbose=False
        ...     )
        ...     if not records:
        ...         break  # No more data
        ...     all_records.extend(records)
        ...     print(f"Parsed {len(records)} records, offset now at {offset}")
        Parsed 100 records, offset now at 15234
        Parsed 100 records, offset now at 30891
        ...
        >>> print(f"Total records parsed: {len(all_records)}")
    """
    verbose_flag = 0 if not verbose else 1
    return _asterix.parse_with_offset(bytes(data), offset, blocks_count, verbose_flag)


def describeXML(parsed, descriptions=False):
    """ Describe all elements in Asterix data in a lxml ElementTree.
    Content is identical to the command "./asterix --xml filename".
    Args:
        parsed: Parsed Asterix packet returned by asterix.parse
    Returns:
         lxml ElementTree describing all Asterix data
         None if lxml module is not installed
    """
    if lxml_found:
        xml = etree.Element('ASTERIXSTART')
        for record in parsed:
            xml_Record = etree.SubElement(xml, 'ASTERIX')
            cat = record['category']
            ts = record['ts']
            xml_Record.set('ver', '%d' % 1)
            xml_Record.set('length', '%ld' % record['len'])
            xml_Record.set('crc', '%s' % record['crc'])
            xml_Record.set('timestamp', '%ld' % ts)
            if 'hexdata' in record:
                xml_Record.set('hexdata', record['hexdata'])
            xml_Record.set('cat', '%03d' % cat)
            if descriptions:
                xml_Record.set('description', '%s' % _asterix.describe(cat))

            for key, value in record.items():
                if key[0] == 'I':
                    xml_Item = etree.SubElement(xml_Record, key)
                    description = '%s' % _asterix.describe(cat, str(key))
                    if description and descriptions:
                        xml_Item.set('description', description)
                    if isinstance(value, dict):
                        for ikey, ival in value.items():
                            xml_Field = etree.SubElement(xml_Item, ikey)
                            if 'val' in ival:
                                description1 = _asterix.describe(cat, str(key), ikey)
                                description2 = _asterix.describe(cat, str(key), ikey,
                                                                 str(ival['val']))
                                if description2 and descriptions:
                                    xml_Field.set('description', description2)
                                xml_Field.text = str(ival['val'])
                            else:
                                for ikey2, ival2 in ival.items():
                                    xml_subItem = etree.SubElement(xml_Field, ikey2)
                                    description1 = _asterix.describe(cat, str(key), ikey2)
                                    description2 = _asterix.describe(cat, str(key), ikey2,
                                                                     str(ival2['val']))
                                    if description2 and descriptions:
                                        xml_subItem.set('description', description2)
                                    xml_subItem.text = str(ival2['val'])
                    elif isinstance(value, list):
                        for it in value:
                            for ikey, ival in it.items():
                                xml_Field = etree.SubElement(xml_Item, ikey)
                                description1 = _asterix.describe(cat, str(key), ikey)
                                description2 = _asterix.describe(cat, str(key), ikey,
                                                                 str(ival['val']))
                                if description2 and descriptions:
                                    xml_Field.set('description', description2)
                                xml_Field.text = str(ival['val'])
                    else:
                        xml_Item.text = str(value)

        return xml
    return None

def describe(parsed):
    """ Describe all elements in Asterix data in textual format.
    Args:
        parsed: Parsed Asterix packet returned by ateris.parse
    Returns:
         Formatted string describing all Asterix data
    """
    i = 0
    txt = ''
    for record in parsed:
        i += 1
        txt += '\n\nAsterix record: %d ' % i
        len = record['len']
        txt += '\nLen: %ld' % (len)
        crc = record['crc']
        txt += '\nCRC: %s' % (crc)
        ts = record['ts']
        strts = datetime.fromtimestamp(ts / 1000.)

        txt += '\nTimestamp: %ld (%s)' % (ts, strts)
        cat = record['category']
        txt += '\nCategory: %d (%s)' % (cat, _asterix.describe(cat))
        txt += '\nHexdata: %s' % record['hexdata']

        for key, value in record.items():
            if key != 'category':
                if isinstance(value, dict):
                    for ikey, ival in value.items():
                        txt += '\nItem: %s (%s)' % (str(key), _asterix.describe(cat, str(key)))

                        if 'val' in ival:
                            txt += '\n\t%s (%s): %s %s' % (
                            ikey, _asterix.describe(cat, str(key), ikey), str(ival['val']),
                            _asterix.describe(cat, str(key), ikey, str(ival['val'])))
                        else:
                            txt += '\n\t%s' % ikey
                            for ikey2, ival2 in ival.items():
                                txt += '\n\t\t%s (%s): %s %s' % (
                                ikey2, _asterix.describe(cat, str(key), ikey2), str(ival2['val']),
                                _asterix.describe(cat, str(key), ikey2, str(ival2['val'])))
                elif isinstance(value, list):
                    txt += '\nItem: %s (%s)' % (str(key), _asterix.describe(cat, str(key)))

                    for it in value:
                        for ikey, ival in it.items():
                            txt += '\n\t%s (%s): %s %s' % (
                            ikey, _asterix.describe(cat, str(key), ikey), str(ival['val']),
                            _asterix.describe(cat, str(key), ikey, str(ival['val'])))

    return txt


def list_sample_files():
    """ Return the list of Asterix format sample files from the package
    Returns:
         list of Asterix sample files
    Example:
        >>> list_sample_files()
    """
    sample_files = []
    filepath = os.path.join(os.path.dirname(__file__), 'sample_data')
    for fn in sorted(os.listdir(filepath)):
        f = os.path.join(filepath, fn)
        if os.path.isfile(f):
            sample_files.append(f)
    return sample_files


def get_sample_file(match):
    """ Returns first Asterix sample file matching the parameter string
    Args:
        match: Search string for sample file
    Returns:
         Sample file path
    """
    filepath = os.path.join(os.path.dirname(__file__), 'sample_data')
    for fn in sorted(os.listdir(filepath)):
        f = os.path.join(filepath, fn)
        if os.path.isfile(f) and match in fn:
            return f


def list_configuration_files():
    """ Return the list of Asterix configuration files from the package
    Returns:
        list of Asterix configuration files
    """
    sample_files = []
    filepath = os.path.join(os.path.dirname(__file__), 'config')
    for fn in sorted(os.listdir(filepath)):
        f = os.path.join(filepath, fn)
        if os.path.isfile(f):
            sample_files.append(f)
    return sample_files


def get_configuration_file(match):
    """ Returns first Asterix configuration file matching the parameter string
    Args:
        match: Search string for configuration file
    Returns:
         Configuration file path
    """
    filepath = os.path.join(os.path.dirname(__file__), 'config')
    for fn in sorted(os.listdir(filepath)):
        f = os.path.join(filepath, fn)
        if os.path.isfile(f) and match in fn:
            return f


# default callback function
# def callback(arg):
#    for a in arg:
#        print(a)

# initialize asterix with default configuration files

filepath = os.path.join(os.path.dirname(__file__), 'config')
for fn in sorted(os.listdir(filepath)):
    f = os.path.join(filepath, fn)
    if os.path.isfile(f) and f.endswith(".xml"):
        _asterix.init(f)

# set default callback
# _asterix.set_callback(callback)
