#!/usr/bin/env python3

import sys
import os

args = sys.argv[1:]

ret = os.system('docker run --rm -v {0}:/app qpkg-run:latest {1}'.format(os.getcwd(), ' '.join(args)))
sys.exit(ret)
