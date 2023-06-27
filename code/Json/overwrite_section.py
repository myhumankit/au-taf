#! /usr/bin/env python3

import sys
import lief

if len(sys.argv) != 4:
    sys.exit(-1)
# end if

build = sys.argv[1]
patch = sys.argv[2]
final = sys.argv[3]

parser = lief.parse(build)
section = parser.get_section(".config")
available_size = section.size
patch_offset = section.offset
print("Size %08X Offset %08X" % (available_size,patch_offset))
f = open(patch,"rb")
patch_content = f.read()
f.close()
patch_size = len(patch_content)
print("Size %08X" % patch_size)
if patch_size > available_size:
    print("Config file is too big")
    sys.exit(-1)
# end if

f = open(build,"rb")
build_content = f.read()
f.close()

f = open(final,"wb")
f.write(build_content[0:patch_offset])
f.write(patch_content)
f.write(build_content[patch_offset + patch_size:])
f.close()
sys.exit(0)
