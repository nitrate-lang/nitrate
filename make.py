#!/usr/bin/env python3

import os
import sys

cwd = os.getcwd()

if not os.path.exists(os.path.join(cwd, 'libquix-parser')):
    print("Please run this script from the root of the repository.")
    sys.exit(1)

# Check if Docker is installed
if os.system('docker --version') != 0:
    print("Docker is not installed.")
    sys.exit(1)

if '--clean-all' in sys.argv:
    os.system('rm -rf bin build')
    print("Cleaned all.")
    sys.exit(0)

# Check if SNAP Build mode is enabled
if '--snap' in sys.argv:
    # Check if snapcraft is installed
    if os.system('snapcraft --version') != 0:
        print("Snapcraft is not installed.")
        sys.exit(1)
    
    print("Building Snap package...")
    if os.system('snapcraft') != 0:
        print("Snap build failed.")
        sys.exit(1)
    print("Snap build complete.")
    sys.exit(0)

print("Building Docker containers...")

# Build the debug env container
if os.system('docker build -t quixcc-debug:latest -f env/Debug.Dockerfile .') != 0:
    print("Debug build failed.")
    sys.exit(1)

def regenerate_runner():
    if os.system('docker build -t qpkg-run:latest -f env/Runner.Dockerfile .') != 0:
        print("Runner build failed.")
        sys.exit(1)

# Build the release env container
if os.system('docker build -t quixcc-release:latest -f env/Release.Dockerfile .') != 0:
    print("Release build failed.")
    sys.exit(1)

if '--release' in sys.argv:
    print("Building release...")
    if os.system('docker run -v {0}:/app --rm -it quixcc-release:latest'.format(cwd)) != 0:
        print("Release build failed.")
        sys.exit(1)
    if '--strip' in sys.argv:
        for file in os.listdir(os.path.join(cwd, 'bin')):
            if file.endswith('.a'):
                continue
            if os.system('strip {0}'.format(os.path.join(cwd, 'bin', file))) != 0:
                print("Failed to strip {0}".format(file))
                sys.exit(1)
        print("Stripped release binaries.")

    if '--upx-best' in sys.argv:
        files = ['qpkg', 'qld', 'qcc']
        for file in files:
            if os.system('upx --best {0}'.format(os.path.join(cwd, 'bin', file))) != 0:
                print("Failed to UPX {0}".format(file))
                sys.exit(1)

    print("UPX'd release binaries.")
    regenerate_runner()
    print("Release build complete.")
    sys.exit(0)

if '--debug' in sys.argv:
    print("Building debug...")
    if os.system('docker run -v {0}:/app --rm -it quixcc-debug:latest'.format(cwd)) != 0:
        print("Debug build failed.")
        sys.exit(1)
    print("Debug build complete.")

    regenerate_runner()
    sys.exit(0)
