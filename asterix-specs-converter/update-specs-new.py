#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Script that downloads JSON asterix specifications from upstream repository,
# converts them from new to old JSON format, and converts them to XML specification.

import urllib.request, json
import hashlib
from asterixjson2xml import AsterixJson2XML, convert_rule_to_variation
from convertspec import handle_asterix

upstream_repo = 'https://zoranbosnjak.github.io/asterix-specs'

with urllib.request.urlopen(upstream_repo+'/manifest.json') as url:
    data = json.loads(url.read().decode())
    for category in data:
        for ver in category['cats']:
            vermajor, verminor = ver.split('.')
            filename = upstream_repo + '/specs/cat%s/cats/cat%s/definition.json' % (category['category'], ver)
            print('Processing: ' + filename)
            with urllib.request.urlopen(filename) as caturl:
                cat_input_new = caturl.read()

                # Convert from new JSON format to old JSON format
                root_new = json.loads(cat_input_new)
                root_old = handle_asterix(root_new)

                # Convert rule-based format to variation-based format for each catalogue item
                for item in root_old['catalogue']:
                    convert_rule_to_variation(item)

                cat_input = json.dumps(root_old, ensure_ascii=False, sort_keys=True, indent=4).encode('utf-8')

                ref_input = b''
                ref = None
                # todo
                #if args.ref:
                #    ref_input = args.ref.read()
                #    ref = json.loads(ref_input)

                cks = hashlib.sha1(cat_input + ref_input).hexdigest()
                cat = AsterixJson2XML(root_old, cks, re=ref)
                result = cat.parse()

                outfilename = 'specs/asterix_cat%s_%s_%s.xml' % (category['category'], vermajor, verminor)
                with open(outfilename, 'wt') as jsf:
                    jsf.write(result)

with urllib.request.urlopen(upstream_repo+'/gitrev.txt') as url:
    rev = url.read().decode()[0:7]
    print('Done!')
    print('You might want to examine the changes, then proceed with:')
    print('')
    print('git add specs/*xml')
    print('git status specs')
    print('git commit -m "synced with asterix-specs upstream, revision #'+rev+'"')
    print('git push')
