# TINY to Python Code Generated from gcd.py
# K.C. Louden Compiler Project

import sys

u = int(sys.stdin.readline())
v = int(sys.stdin.readline())
if (v == 0):
    v = 0
else:
    while True:
        temp = v
        v = (u - ((u // v) * v))
        u = temp
        if (v == 0): break
print(u)

# End of TINY program execution
