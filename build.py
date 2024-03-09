#!/usr/bin/env python3

import os
import sys

os.makedirs('build', exist_ok=True)
os.chdir('build')

if '--release' in sys.argv:
    os.system('cmake -DCMAKE_BUILD_TYPE=Release ..')
else:
    os.system('cmake ..')

os.system('make -j`nproc`')
os.chdir('..')

os.makedirs('bin', exist_ok=True)
os.system('cp build/quixcc/quixcc bin/')
os.system('cp -r build/tools/quixcc-* bin/')

print('Build complete')