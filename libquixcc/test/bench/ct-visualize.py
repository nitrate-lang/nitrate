import matplotlib.pyplot as plt
import numpy as np
import json
import sys

if len(sys.argv) < 2:
    print("usage: python ct-visualize.py <filename.json>")
    sys.exit(1)

filename = sys.argv[1]

with open(filename) as f:
    data = json.load(f)

# [double,double,...]

data = np.array(data, dtype=np.float64)

print(data)

plt.plot(data)
plt.grid()
plt.ylabel('Compile time (ns)')
plt.xlabel('Iteration')
plt.title('Compile time vs Iteration (Same source, same flags, same process)')
plt.show()