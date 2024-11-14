#!/usr/bin/env python3

from sys import argv, exit
import matplotlib.pyplot as plt
from math import log2
import random

if len(argv) != 2:
    print("Usage: binview.py <filename>")
    exit(1)

block_sizes = [64, 128, 512, 1024]
pad_byte = b'\x00'


def process_block(block):
    size = len(block)
    byte_count = {}
    for byte in block:
        byte_count[byte] = byte_count.get(byte, 0) + 1

    entropy = 0
    for byte in byte_count:
        p = byte_count[byte] / size
        entropy -= p * (p and log2(p))

    return entropy / 8.0


filename = argv[1]

holistic_entropy_density = []

with open(filename, 'rb') as f:
    for block_size in block_sizes:
        offset = 0
        f.seek(0)

        print(f'Processing block size: {block_size}')

        entropy_field = []
        while True:
            block = f.read(block_size)
            if not block:
                break

            # Pad the block if needed
            if len(block) < block_size:
                block += pad_byte * (block_size - len(block))

            entropy_field.append((offset, process_block(block)))
            offset += block_size

        print(f'Processed block size: {block_size}')

        holistic_entropy_density.append(entropy_field)


fig, axs = plt.subplots(len(block_sizes))

for i, entropy_field in enumerate(holistic_entropy_density):
    x, y = zip(*entropy_field)
    axs[i].plot(x, y)
    axs[i].set_title(f'Entropy Density (block size: {block_sizes[i]})')

plt.suptitle(f'Entropy Density of {filename}')
plt.tight_layout()
plt.show()
