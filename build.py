#!/usr/bin/env python3

import os
import sys

os.makedirs('build', exist_ok=True)
os.chdir('build')

if '--release' in sys.argv:
    os.system('cmake -DCMAKE_BUILD_TYPE=Release ..')
else:
    os.system('cmake -DBUILD_TESTING=On ..')

os.system('make -j`nproc`')
os.chdir('..')

os.makedirs('bin', exist_ok=True)
os.system('cp build/qcc/qcc bin/')
os.system('cp -r build/tools/quixcc-* bin/')
os.system('cp -r build/libquixcc/test/qcc-* bin/')

print('Build complete')