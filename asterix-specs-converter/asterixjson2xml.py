#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import argparse
import json
import hashlib
from copy import copy
from itertools import chain, repeat
from xml.sax.saxutils import escape as xml_escape

def getNumber(value):
    """Get Natural/Real/Rational number as an object."""
    class Integer(object):
        def __init__(self, val):
            self.val = val
        def __str__(self):
            return '{}'.format(self.val)
        def __float__(self):
            return float(self.val)

    class Ratio(object):
        def __init__(self, a, b):
            self.a = a
            self.b = b
        def __str__(self):
            return '{}/{}'.format(self.a, self.b)
        def __float__(self):
            return float(self.a) / float(self.b)

    class Real(object):
        def __init__(self, val):
            self.val = val
        def __str__(self):
            return '{0:f}'.format(self.val).rstrip('0')
        def __float__(self):
            return float(self.val)

    t = value['type']
    val = value['value']

    if t == 'Integer':
        return Integer(int(val))
    if t == 'Ratio':
        x, y = val['numerator'], val['denominator']
        return Ratio(x, y)
    if t == 'Real':
        return Real(float(val))
    raise Exception('unexpected value type {}'.format(t))

def renderRule(rule, caseContextFree, caseDependent):
    rule_type = rule['type']
    if rule_type == 'ContextFree':
        return caseContextFree(rule)
    elif rule_type == 'Dependent':
        return caseDependent(rule)
    else:
        raise Exception('unexpected rule type {}'.format(rule_type))

# 'case' statement as a function
def case(msg, val, *cases):
    for (a,b) in cases:
        if val == a: return b
    raise Exception('unexpected {}: {}'.format(msg, val))

def getVariationSize(variation):
    t = variation['type']
    if t == 'Element':
        return variation['size']
    elif t == 'Group':
        return sum([getItemSize(i) for i in variation['items']])
    elif t == 'Repetitive':
        return getVariationSize(variation['variation'])
    else:
        raise Exception('can not determine item size for type {}'.format(t))

def getItemSize(item):
    """Determine the size of an item with apriory known size."""
    # Handle None items (optional/spare fields in Extended structures)
    if item is None:
        return 0
    if item.get('spare'):
        return item.get('length', 0)
    # If no variation field, item may not be fully converted - return 0 as safe default
    if 'variation' not in item:
        # This can happen with unconverted items - log warning would be good in production
        return 0
    return getVariationSize(item['variation'])

accumulator = []
indentLevel = 0

class Indent(object):
    """Simple indent context manager."""
    def __enter__(self):
        global indentLevel
        indentLevel += 1
    def __exit__(self, exec_type, exec_val, exec_tb):
        global indentLevel
        indentLevel -= 1

indent = Indent()

def replaceString(s, mapping):
    for (key,val) in mapping.items():
        s = s.replace(key, val)
    return s

def evaluate_constraint_value(value_obj):
    """Evaluate a constraint value which can be Integer, Div, Pow, etc.

    Constraint values can have various types:
    - Integer: {type: "Integer", value: 123}
    - Div: {type: "Div", numerator: {...}, denominator: {...}}
    - Pow: {type: "Pow", base: 2, exponent: 10}
    """
    vtype = value_obj.get('type')

    if vtype == 'Integer':
        return value_obj.get('value', 0)
    elif vtype == 'Div':
        numerator = evaluate_constraint_value(value_obj.get('numerator', {}))
        denominator = evaluate_constraint_value(value_obj.get('denominator', {}))
        if denominator != 0:
            return numerator / denominator
        return 0
    elif vtype == 'Pow':
        base = value_obj.get('base', 0)
        exponent = value_obj.get('exponent', 0)
        return base ** exponent
    else:
        # Unknown type, try to get 'value' field as fallback
        return value_obj.get('value', 0)

def convert_rule_to_variation(item):
    """Convert rule-based format (from convertspec) to variation format.

    This function bridges the new JSON schema (post-July 2024) with the
    variation-based format that the XML generator expects.
    """
    # Handle None items (can occur in Extended/Variable structures)
    if item is None:
        return None

    if item.get('spare'):
        return item

    if 'rule' not in item:
        return item

    rule = item['rule']
    rule_type = rule.get('type')

    if rule_type == 'ContextFree':
        value = rule.get('value', {})
        variation_type = value.get('type')

        if variation_type == 'Element':
            # Simple element with size and content rule
            size = value.get('size', 0)
            content_rule = value.get('rule', {})
            variation = {
                'type': 'Element',
                'size': size
            }

            # Add content information and normalize rule format
            rule_type = content_rule.get('type')

            if rule_type == 'ContextFree':
                content_value = content_rule.get('value', {})
                content_type = content_value.get('type')

                if content_type == 'Table':
                    # Normalize: convert 'value' to 'content' for compatibility
                    variation['rule'] = {
                        'type': content_rule['type'],
                        'content': content_value
                    }
                elif content_type == 'Quantity':
                    # Quantity needs lsb → scaling + fractionalBits conversion
                    # lsb format: 1 / 2^n means fractionalBits=n, scaling=1
                    lsb = content_value.get('lsb', {})

                    # Default values if lsb is simple
                    fractional_bits = 0
                    scaling = {'type': 'Integer', 'value': 1}

                    # Parse lsb structure: typically 1 / 2^n
                    if lsb.get('type') == 'Div':
                        numerator = lsb.get('numerator', {})
                        denominator = lsb.get('denominator', {})

                        # Extract scaling (numerator)
                        scaling = numerator

                        # Extract fractional bits from denominator (2^n)
                        if denominator.get('type') == 'Pow' and denominator.get('base') == 2:
                            fractional_bits = denominator.get('exponent', 0)
                    elif lsb.get('type') == 'Integer':
                        # Simple integer scaling
                        scaling = lsb
                        fractional_bits = 0

                    # Create normalized content
                    normalized_content = {
                        'type': 'Quantity',
                        'scaling': scaling,
                        'fractionalBits': fractional_bits,
                        'signed': content_value.get('signed', False),
                        'unit': content_value.get('unit', ''),
                        'constraints': content_value.get('constraints', [])
                    }

                    variation['rule'] = {
                        'type': content_rule['type'],
                        'content': normalized_content
                    }
                elif content_type in ['String', 'Integer', 'Raw', 'Bds']:
                    # Normalize: convert 'value' to 'content' for compatibility
                    # Bds = Mode S BDS (Broadcast Data Services) register data
                    variation['rule'] = {
                        'type': content_rule['type'],
                        'content': content_value
                    }
            elif rule_type == 'Dependent':
                # Dependent rules (context-dependent values)
                # Extract size from default case (all cases should have same size)
                default_case = content_rule.get('default', {})
                if 'size' in default_case:
                    variation['size'] = default_case['size']
                # Pass through as-is - rendering code handles Dependent logic
                variation['rule'] = content_rule
            elif not content_rule or not rule_type:
                # No content rule - add default Raw rule
                # This handles simple Elements with no metadata (just size)
                variation['rule'] = {
                    'type': 'ContextFree',
                    'content': {'type': 'Raw'}
                }

            item['variation'] = variation

        elif variation_type == 'Group':
            # Group of items (Fixed length composite)
            items = value.get('items', [])
            converted_items = []
            for i in items:
                if i is None:
                    converted_items.append(None)
                else:
                    converted_items.append(convert_rule_to_variation(i))
            item['variation'] = {
                'type': 'Group',
                'items': converted_items
            }

        elif variation_type == 'Extended':
            # Variable length with FX extension
            # Extended items pack fields into 8-bit (1-byte) chunks
            # First chunk: 7 bits of data + 1 FX bit = 8 bits total
            # Each extent: 7 bits of data + 1 FX bit = 8 bits total
            items = value.get('items', [])
            converted_items = []
            for i in items:
                if i is None:
                    converted_items.append(None)
                else:
                    converted_items.append(convert_rule_to_variation(i))

            # For Extended/Variable items:
            # - first: number of BITS in first Fixed segment (typically 8 = 1 byte)
            # - extents: number of BITS in each extension segment (typically 8 = 1 byte)
            # The FX bit is implicit and handled by the rendering code
            item['variation'] = {
                'type': 'Extended',
                'first': 8,      # First segment is always 1 byte (8 bits)
                'extents': 8,    # Each extension is always 1 byte (8 bits)
                'items': converted_items
            }

        elif variation_type == 'Repetitive':
            # Repetitive structure - but RepetitiveFx should be Extended!
            rep_info = value.get('rep', {})
            rep_type = rep_info.get('type')

            if rep_type == 'Fx':
                # RepetitiveFx → Extended (Variable with FX extension)
                # This is like item 030 - repetitive with FX bit
                rep_variation = value.get('variation', {})

                # The rep_variation is already a variation dict (has type, size, rule)
                # We need to create an item dict to wrap it
                # First, normalize the rule format (value → content) if needed
                if 'rule' in rep_variation:
                    rule = rep_variation['rule']
                    if 'value' in rule and 'content' not in rule:
                        rule['content'] = rule['value']

                # Create a proper item structure for the Extended items list
                extended_item = {
                    'name': item.get('name', ''),
                    'title': item.get('title', ''),
                    'spare': False,
                    'variation': rep_variation  # Use the variation as-is
                }

                # Wrap as Extended structure
                item['variation'] = {
                    'type': 'Extended',
                    'first': 8,      # First segment is 1 byte
                    'extents': 8,    # Extensions are 1 byte each
                    'items': [extended_item]  # Properly structured item
                }
            else:
                # Regular Repetitive (RepetitiveRegular)
                rep_variation = value.get('variation', {})

                # rep_variation is a variation dict (Group, Element, etc), not an item dict
                # We need to convert the items inside it based on its type
                converted_variation = dict(rep_variation)  # shallow copy

                var_type = rep_variation.get('type')
                if var_type == 'Group':
                    # Convert group items recursively
                    items = rep_variation.get('items', [])
                    converted_items = []
                    for i in items:
                        if i is None:
                            converted_items.append(None)
                        else:
                            converted_items.append(convert_rule_to_variation(i))
                    converted_variation['items'] = converted_items
                elif var_type == 'Element':
                    # Element variation might have a nested rule that needs normalization
                    if 'rule' in rep_variation:
                        rule = rep_variation['rule']
                        if 'value' in rule and 'content' not in rule:
                            rule['content'] = rule['value']

                item['variation'] = {
                    'type': 'Repetitive',
                    'variation': converted_variation
                }

        elif variation_type == 'Explicit':
            # Explicit (length-prefixed)
            item['variation'] = {
                'type': 'Explicit'
            }

        elif variation_type == 'Compound':
            # Compound with FSPEC
            fspec = value.get('fspec', 0)
            items = value.get('items', [])
            # Items can be None for optional compound items
            converted_items = []
            for i in items:
                if i is None:
                    converted_items.append(None)
                else:
                    converted_items.append(convert_rule_to_variation(i))
            item['variation'] = {
                'type': 'Compound',
                'fspec': fspec,
                'items': converted_items
            }

    elif rule_type == 'Dependent':
        # Top-level Dependent rules (context-dependent items)
        # These are Elements with Dependent content
        # Extract size from default case
        default_case = rule.get('default', {})
        size = default_case.get('size', 0)

        item['variation'] = {
            'type': 'Element',
            'size': size,
            'rule': rule
        }

    return item

def replaceOutput(s):
    return replaceString(s, {
        u'–': '-',
        u'“': '',
        u'”': '',
        u'°': 'deg',
    })

def tell(s):
    s = replaceOutput(s) # If unicode is not supported on the target.
    s = ' '*indentLevel*4 + s
    accumulator.append(s.rstrip())

def xmlquote(s):
    return replaceString(s, {
        '"': "&quot;",
        "&": "&amp;",
        "'": "&apos;",
        "’": "&apos;",
        "‘": "&apos;",
        "<":  "&lt;",
        ">": "&gt;",
    })

class Fx(object):
    def render(self):
        tell('<Bits bit="1" fx="1">')
        with indent:
            tell('<BitsShortName>FX</BitsShortName>')
            tell('<BitsName>Extension Indicator</BitsName>')
            tell('<BitsValue val="0">End of Data Item</BitsValue>')
            tell('<BitsValue val="1">Extension</BitsValue>')
        tell('</Bits>')

class Bits(object):

    def __init__(self, parent, item, bitsFrom, bitsTo):
        self.parent = parent
        self.item = item
        self.bitsFrom = bitsFrom
        self.bitsTo = bitsTo

    @property
    def cat(self):
        return self.parent.cat

    @property
    def name(self):
        old_name = self.item['name']
        try:
            int(old_name)
            words = self.parent.item['title'].split()
            new_name = ''
            for word in words:
                new_name += word.strip('()')[0:1]
            # print('Renamed:%s  %s -> %s' % (self.parent.full_name, self.parent.item['title'], new_name))
            return new_name
        except ValueError:
            pass
        return old_name

    def render(self):
        item, bitsFrom, bitsTo = self.item, self.bitsFrom, self.bitsTo
        if item['spare']:
            if bitsFrom == bitsTo:
                tell('<Bits bit="{}">'.format(bitsFrom))
            else:
                tell('<Bits from="{}" to="{}">'.format(bitsFrom, bitsTo))
            with indent:
                tell('<BitsShortName>spare</BitsShortName>')
                tell('<BitsName>Spare bit(s) set to 0</BitsName>')
                tell('<BitsConst>0</BitsConst>')
            tell('</Bits>')
        else:
            variation = item['variation']
            vt = variation['type']
            if vt == 'Element':
                content = variation['rule']
            elif vt == 'Repetitive':
                content = variation['variation']['rule']
            elif vt == 'Group':
                # Group items have multiple subitems, treat as Raw for bit rendering
                content = {
                    'type': 'ContextFree',
                    'content': {'type': 'Raw'}
                }
            else:
                raise Exception('unexpected variation type {}'.format(vt))

            # defined content
            def case1(val):
                rule = val['content']
                t = rule['type']

                if t == 'Raw':
                    if bitsFrom == bitsTo:
                        tell('<Bits bit="{}">'.format(bitsFrom))
                    else:
                        tell('<Bits from="{}" to="{}">'.format(bitsFrom, bitsTo))
                    with indent:
                        tell('<BitsShortName>{}</BitsShortName>'.format(xmlquote(self.name)))
                        if item['title']:
                            tell('<BitsName>{}</BitsName>'.format(xmlquote(item['title'])))
                    tell('</Bits>')

                elif t == 'Table':
                    if bitsFrom == bitsTo:
                        tell('<Bits bit="{}">'.format(bitsFrom))
                    else:
                        tell('<Bits from="{}" to="{}">'.format(bitsFrom, bitsTo))
                    with indent:
                        tell('<BitsShortName>{}</BitsShortName>'.format(xmlquote(self.name)))
                        if item['title']:
                            tell('<BitsName>{}</BitsName>'.format(xmlquote(item['title'])))
                        for key,value in sorted(rule['values']):
                            tell('<BitsValue val="{}">{}</BitsValue>'.format(key, xmlquote(value)))
                    tell('</Bits>')

                elif t == 'String':
                    variation = case('string variation', rule['variation'],
                        ('StringAscii', 'ascii'),
                        ('StringICAO', '6bitschar'),
                        ('StringOctal', 'octal'),
                    )
                    assert bitsFrom != bitsTo
                    tell('<Bits from="{}" to="{}" encode="{}">'.format(bitsFrom, bitsTo, variation))
                    with indent:
                        tell('<BitsShortName>{}</BitsShortName>'.format(xmlquote(self.name)))
                        if item['title']:
                            tell('<BitsName>{}</BitsName>'.format(xmlquote(item['title'])))
                    tell('</Bits>')

                elif t == 'Integer':
                    signed = 'signed' if rule['signed'] else 'unsigned'
                    constraints = rule['constraints']
                    if bitsFrom == bitsTo:
                        tell('<Bits bit="{}">'.format(bitsFrom))
                    else:
                        tell('<Bits from="{}" to="{}" encode="{}">'.format(bitsFrom, bitsTo, signed))
                    with indent:
                        tell('<BitsShortName>{}</BitsShortName>'.format(xmlquote(self.name)))
                        if item['title']:
                            tell('<BitsName>{}</BitsName>'.format(xmlquote(item['title'])))
                        msg = '<BitsUnit'
                        for c in constraints:
                            if c['type'] in ['>=', '>']:
                                msg += ' min="{}"'.format(c['value']['value'])
                                break
                        for c in constraints:
                            if c['type'] in ['<=', '<']:
                                msg += ' max="{}"'.format(c['value']['value'])
                                break
                        msg +='></BitsUnit>'
                        tell(msg)
                    tell('</Bits>')

                elif t == 'Quantity':
                    signed = 'signed' if rule['signed'] else 'unsigned'
                    k = getNumber(rule['scaling'])
                    fract = rule['fractionalBits']
                    unit = rule['unit']
                    constraints = rule['constraints']
                    if fract > 0:
                        scale = format(float(k) / (pow(2, fract)), '.29f')
                        scale = scale.rstrip('0')
                    else:
                        scale = format(float(k))

                    if scale[-1] == '.':
                        scale += '0'

                    if bitsFrom == bitsTo:
                        tell('<Bits bit="{}">'.format(bitsFrom))
                    else:
                        tell('<Bits from="{}" to="{}" encode="{}">'.format(bitsFrom, bitsTo, signed))
                    with indent:
                        tell('<BitsShortName>{}</BitsShortName>'.format(xmlquote(self.name)))
                        if item['title']:
                            tell('<BitsName>{}</BitsName>'.format(xmlquote(item['title'])))
                        msg = '<BitsUnit scale="{}"'.format(scale)
                        for c in constraints:
                            if c['type'] in ['>=', '>']:
                                msg += ' min="{}"'.format(evaluate_constraint_value(c['value']))
                                break
                        for c in constraints:
                            if c['type'] in ['<=', '<']:
                                msg += ' max="{}"'.format(evaluate_constraint_value(c['value']))
                                break
                        msg +='>{}</BitsUnit>'.format(unit)

                        tell(msg)
                    tell('</Bits>')

                elif t == 'Bds':
                    tell('<BDS/>')

                else:
                    raise Exception('unexpected value type {}'.format(t))

            # complex content rule (process the same as raw)
            def case2(val):
                if bitsFrom == bitsTo:
                    tell('<Bits bit="{}">'.format(bitsFrom))
                else:
                    tell('<Bits from="{}" to="{}">'.format(bitsFrom, bitsTo))
                with indent:
                    tell('<BitsShortName>{}</BitsShortName>'.format(xmlquote(self.name)))
                    if item['title']:
                        tell('<BitsName>{}</BitsName>'.format(xmlquote(item['title'])))
                tell('</Bits>')

            renderRule(content, case1, case2)

def get_bit_size(item):
    """Return bit size of a (spare) item."""
    if item is None:
        return 0
    if item.get('spare'):
        return item.get('length', 0)
    else:
        return item.get('variation', {}).get('size', 0)

def ungroup(item):
    """Convert group of items of known size to element"""
    n = sum([get_bit_size(i) for i in item['variation']['items']])
    result = copy(item)
    result['variation'] = {
        'rule': {
            'content': {'type': 'Raw'},
            'type': 'ContextFree',
        },
        'size': n,
        'type': 'Element',
    }
    return result

class Variation(object):

    @staticmethod
    def create(parent, item):
        if not 'variation' in item:
           return None

        variation = item['variation']
        vt = variation['type']

        # single item Fixed variation
        if vt == 'Element':
            size = getItemSize(item)
            return Fixed(parent, item, size, [item])

        if vt == 'Group':
            size = getItemSize(item)
            items = variation['items']
            return Fixed(parent, item, size, items)

        if vt == 'Extended':
            n1 = variation['first']
            n2 = variation['extents']
            items = []
            for i in variation['items']:
                if i is not None and i.get('variation') is not None:
                    if i['variation']['type'] == 'Group':
                        i = ungroup(i)
                items.append(i)
            return Variable(parent, item, n1, n2, items)

        if vt == 'Repetitive':
            size = getVariationSize(variation)
            return Repetitive(parent, item, size, variation['variation'])

        if vt == 'Explicit':
            return Explicit(parent, item)

        if vt == 'Compound':
            fspec = variation['fspec']
            items = variation['items']
            return Compound(parent, item, fspec, items)

        raise Exception('unexpected variation type {}'.format(vt))

    @property
    def cat(self):
        return self.parent.cat

    @property
    def full_name(self):
        return tuple(list(self.parent.full_name) + [self.item['name']])

    def __init__(self, parent, item, *args):
        self.parent = parent
        self.item = item
        self.args = args

        self.oldRender = self.render
        self.render = self.realRender

    @property
    def desc(self):
        return '{} data item.'.format(self.__class__.__name__)

    def realRender(self):
        tell('<DataItemFormat desc="{}">'.format(self.desc))
        with indent:
            self.oldRender()
        tell('</DataItemFormat>')

class Fixed(Variation):

    @property
    def desc(self):
        bitSize, items = self.args
        byteSize = bitSize // 8
        return '{}-octet fixed length data item.'.format(byteSize)

    def render(self):
        bitSize, items = self.args
        if (bitSize % 8) != 0:
            # Better error message showing which item has alignment issue
            item_info = ""
            if items:
                item = items[0]
                if isinstance(item, dict):
                    item_info = f" in item {item.get('name', '?')} (title: {item.get('title', '?')})"
            raise AssertionError(f"bit alignment error: bitSize={bitSize} (not multiple of 8){item_info}")
        byteSize = bitSize // 8
        bitsFrom = bitSize

        # Check if single item is BDS type (defensive access)
        is_bds = (len(items) == 1 and
                  items[0].get('variation', {}).get('rule', {}).get('content', {}).get('type') == 'Bds')

        if is_bds:
            # Example: CAT062/I380/ACS/BDS
            n = getItemSize(items[0])
            bitsTo = bitsFrom - n + 1
            Bits(self, items[0], bitsFrom, bitsTo).render()
        else:
            tell('<Fixed length="{}">'.format(byteSize))
            for item in items:
                n = getItemSize(item)
                # Skip zero-sized items to avoid invalid bit ranges
                if n <= 0:
                    continue
                bitsTo = bitsFrom - n + 1
                with indent:
                    Bits(self, item, bitsFrom, bitsTo).render()
                bitsFrom -= n
            tell('</Fixed>')

class Variable(Variation):
    def render(self):
        n1, n2, items = self.args
        chunks = chain(repeat(n1,1), repeat(n2))
        tell('<Variable>')
        with indent:
            while True:
                bitSize = next(chunks)
                if (bitSize % 8) != 0:
                    item_name = self.item.get('name', '?')
                    item_title = self.item.get('title', '?')
                    raise Exception(f"bit alignment error in item {item_name} ({item_title}): bitSize={bitSize}, expected multiple of 8")
                byteSize = bitSize // 8
                tell('<Fixed length="{}">'.format(byteSize))
                bitsFrom = bitSize
                with indent:
                    while True:
                        # Check if items list is empty
                        if not items:
                            break
                        item = items[0]
                        items = items[1:]
                        # Skip None items (optional fields in Extended structures)
                        if item is None:
                            if not items:
                                break
                            continue
                        n = getItemSize(item)
                        # Skip zero-sized items to avoid invalid bit ranges
                        if n <= 0:
                            continue
                        bitsTo = bitsFrom - n + 1
                        # Ensure bitsTo doesn't go negative or below FX bit position
                        # In Variable structures with FX, minimum bitsTo is 2 (bit 1 is FX)
                        if bitsTo < 2:
                            # Item size exceeds available space, clamp to minimum
                            bitsTo = 2
                            # Ensure bitsTo doesn't exceed bitsFrom (would create invalid range)
                            if bitsTo > bitsFrom:
                                # No room for this item, skip it
                                break
                            n = bitsFrom - bitsTo + 1  # Recalculate actual size used
                        Bits(self, item, bitsFrom, bitsTo).render()
                        bitsFrom -= n
                        if bitsFrom <= 1:
                            break
                    Fx().render()
                tell('</Fixed>')
                if not items:
                    break
        tell('</Variable>')

class Repetitive(Variation):
    def render(self):
        bitSize, variation = self.args
        if (bitSize % 8) != 0:
            item_name = self.item.get('name', '?')
            item_title = self.item.get('title', '?')
            raise Exception(f"bit alignment error in Repetitive item {item_name} ({item_title}): bitSize={bitSize}, expected multiple of 8")
        byteSize = bitSize // 8
        tell('<Repetitive>')
        items = variation.get('items')
        if items is None:
            # Single item repetitive - check if it's BDS
            item = self.item
            content_type = variation.get('rule', {}).get('content', {}).get('type')

            # Check if BDS type
            is_bds = (content_type == 'Bds')

            if is_bds:
                # BDS should NOT be wrapped in Fixed
                with indent:
                    tell('<BDS/>')
            else:
                # Wrap other types in Fixed
                bitSize = getItemSize(item)
                bitsFrom = bitSize
                bitsTo = bitsFrom - bitSize + 1
                with indent:
                    tell('<Fixed length="{}">'.format(byteSize))
                    with indent:
                        Bits(self, item, bitsFrom, bitsTo).render()
                    tell('</Fixed>')
        else:
            # Check if this is a BDS-only repetitive (defensive access)
            # For BDS, the variation itself has the Bds content type
            is_bds_only = (variation.get('rule', {}).get('content', {}).get('type') == 'Bds')

            with indent:
                if is_bds_only:
                    # BDS items should NOT be wrapped in Fixed
                    # Example: CAT011/I380/MBS - <Repetitive><BDS/></Repetitive>
                    tell('<BDS/>')
                else:
                    tell('<Fixed length="{}">'.format(byteSize))
                    bitsFrom = bitSize
                    for item in items:
                        n = getItemSize(item)
                        # Skip zero-sized items to avoid invalid bit ranges
                        if n <= 0:
                            continue
                        bitsTo = bitsFrom - n + 1
                        with indent:
                            Bits(self, item, bitsFrom, bitsTo).render()
                        bitsFrom -= n
                    tell('</Fixed>')
        tell('</Repetitive>')

class Explicit(Variation):
    def render(self):
        definition = None
        if self.parent.item['name'] == 'RE':
            definition = self.parent.category.re
            if definition:
                definition['name'] = 'RE'
        if self.parent.item['name'] == 'SP':
            definition = self.parent.category.sp
            if definition:
                definition['name'] = 'SP'
        tell('<Explicit>')
        if definition:
            Variation.create(self, definition).render()
        else:
            with indent:
                tell('<Fixed length="1">')
                with indent:
                    tell('<Bits from="8" to="1">')
                    with indent:
                        tell('<BitsShortName>VAL</BitsShortName>')
                    tell('</Bits>')
                tell('</Fixed>')
        tell('</Explicit>')

class Compound(Variation):
    def render(self):
        (fspec, items) = self.args
        tell('<Compound>')

        with indent:
            tell('<Variable>')

            # FSPEC with predefined length
            if fspec is not None:
                assert (fspec % 8) == 0, "bit alignment error"
                assert fspec == len(items), "item length mismatch"
                with indent:
                    tell('<Fixed length="{}">'.format(fspec // 8))
                    n = len(items)
                    bp = 1
                    for item in items:
                        with indent:
                            if item:
                                tell('<Bits bit="{}">'.format(n))
                                with indent:
                                    tell('<BitsShortName>{}</BitsShortName>'.format(xmlquote(item['name'])))
                                    tell('<BitsName>{}</BitsName>'.format(xmlquote(item['title'])))
                                    tell('<BitsPresence>{}</BitsPresence>'.format(bp))
                                tell('</Bits>')
                                bp += 1
                            else:
                                tell('<Bits bit="{}">'.format(n))
                                with indent:
                                    tell('<BitsShortName>spare</BitsShortName>')
                                    tell('<BitsName>Spare bits set to 0</BitsName>')
                                tell('</Bits>')
                            n -= 1
                    tell('</Fixed>')

            # FSPEC with FX extension
            else:
                with indent:
                    bp = 1
                    while True:
                        tell('<Fixed length="1">')
                        n = 8
                        while True:
                            if items:
                                item = items[0]
                                items = items[1:]
                            else:
                                item = None
                            with indent:
                                if item:
                                    tell('<Bits bit="{}">'.format(n))
                                    with indent:
                                        tell('<BitsShortName>{}</BitsShortName>'.format(xmlquote(item['name'])))
                                        tell('<BitsName>{}</BitsName>'.format(xmlquote(item['title'])))
                                        tell('<BitsPresence>{}</BitsPresence>'.format(bp))
                                    tell('</Bits>')
                                    bp += 1
                                else:
                                    tell('<Bits bit="{}">'.format(n))
                                    with indent:
                                        tell('<BitsShortName>spare</BitsShortName>')
                                        tell('<BitsName>Spare bits set to 0</BitsName>')
                                    tell('</Bits>')
                                n -= 1
                                if n <= 1:
                                    tell('<Bits bit="1" fx="1">')
                                    with indent:
                                        tell('<BitsShortName>FX</BitsShortName>')
                                        tell('<BitsName>Extension indicator</BitsName>')
                                        tell('<BitsValue val="0">no extension</BitsValue>')
                                        tell('<BitsValue val="1">extension</BitsValue>')
                                    tell('</Bits>')
                                    break
                        tell('</Fixed>')
                        if not items:
                            break

            tell('</Variable>')

            # item list
            (fspec,items) = self.args
            for item in items:
                tell('')
                if item is not None:
                    obj = Variation.create(self, item)
                    obj.oldRender()
        tell('</Compound>')

class TopItem(object):
    def __init__(self, category, item):
        self.category = category
        self.item = item

    @property
    def cat(self):
        return self.category.cat

    @property
    def full_name(self):
        return (self.item['name'],)

    def render(self):
        item = self.item
        tell('')
        tell('<DataItem id="{}">'.format(item['name']))
        title = item['title']
        definition = item['definition']
        remark = item['remark']

        with indent:
            if title:
                tell('<DataItemName>{}</DataItemName>'.format(xmlquote(title)))
            if definition:
                tell('<DataItemDefinition>')
                with indent:
                    for line in definition.splitlines():
                        tell(xmlquote(line))
                tell('</DataItemDefinition>')
            variation = Variation.create(self, item)
            if variation:
                variation.render()
            if remark:
                tell('<DataItemNote>')
                with indent:
                    for line in remark.splitlines():
                        tell(xmlquote(line))
                tell('</DataItemNote>')
        tell('</DataItem>')

class Category(object):
    def __init__(self, root, cks, re=None, sp=None):
        self.root = root
        self.cks = cks
        self.re = re
        self.sp = sp
        self.items = [TopItem(self, item) for item in root['catalogue']]

    @property
    def cat(self):
        # Handle both old ('number') and new ('category') schema
        return self.root.get('category', self.root.get('number'))

    def render(self):
        category = self.cat
        edition = self.root['edition']
        # Handle both old (string "1.4") and new (dict {'major': 1, 'minor': 4}) edition formats
        if isinstance(edition, str):
            major, minor = edition.split('.')
            edition = {'major': int(major), 'minor': int(minor)}
        title = self.root['title']
        tell('<?xml version="1.0" encoding="UTF-8"?>')
        tell('<!DOCTYPE Category SYSTEM "asterix.dtd">')
        tell('')
        tell('<!--')
        with indent:
            tell('')
            tell('Asterix Category {:03d} v{}.{} definition'.format(category, edition['major'], edition['minor']))
            tell('')
            tell('Do not edit this file!')
            tell('')
            tell('This file is auto-generated from json specs file.')
            tell('sha1sum of concatenated json input(s): {}'.format(self.cks))
            tell('')
            tell('See asterix-specs-converter/README.md for details.')
        tell('-->')
        tell('')
        tell('<Category id="{:d}" name="{}" ver="{}.{}">'.format(category, title, edition['major'], edition['minor']))
        with indent:
            for i in self.items:
                i.render()
        with indent:
            self.renderUap()
        tell('')
        tell('</Category>')

    def renderUap(self):
        uap = self.root['uap']
        ut = uap['type']
        if ut == 'uap':
            variations = [{'name': 'uap', 'items': uap['items']}]
        elif ut == 'uaps':
            variations = uap['variations']
        else:
            raise Exception('unexpected uap type {}'.format(ut))

        for var in variations:
            tell('')
            tell('<UAP>')
            name = var['name']
            items = var['items']
            with indent:
                bit = 0
                frn = 1
                while True:
                    chunk = items[0:7]
                    items = items[7:]
                    for i in chunk:
                        tell('<UAPItem bit="{}" frn="{}">{}</UAPItem>'.format(bit,frn, i or '-'))
                        bit += 1
                        frn += 1
                    if not items:
                        while bit % 8 != 7:
                            tell('<UAPItem bit="{}" frn="{}">-</UAPItem>'.format(bit,frn))
                            bit += 1
                            frn += 1
                    tell('<UAPItem bit="{}" frn="FX" len="-">-</UAPItem>'.format(bit,frn))
                    bit += 1
                    if not items:
                        break
            tell('</UAP>')


class AsterixJson2XML(object):
    def __init__(self, root, cks, re=None, sp=None):
        # Handle new JSON schema with 'contents' wrapper
        if 'contents' in root:
            root = root['contents']
        self.cat = Category(root, cks, re, sp)

    def parse(self):
        global accumulator
        accumulator = []
        self.cat.render()
        result = ''.join([line + '\n' for line in accumulator])
        return result

# main
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Render asterix specs from json to custom xml.')
    parser.add_argument('--cat', nargs='?', type=argparse.FileType('rb'), default=sys.stdin.buffer, help="input CATegory JSON file")
    parser.add_argument('--ref', nargs='?', type=argparse.FileType('rb'), help="input REF JSON file")
    parser.add_argument('--outfile', nargs='?', type=argparse.FileType('wt'), default=sys.stdout)

    args = parser.parse_args()

    cat_input = args.cat.read()
    root = json.loads(cat_input)

    ref_input = b''
    ref = None
    if args.ref:
        ref_input = args.ref.read()
        ref = json.loads(ref_input)

    cks = hashlib.sha1(cat_input+ref_input).hexdigest()
    cat = AsterixJson2XML(root, cks, re=ref)
    result = cat.parse()
    args.outfile.write(result)

