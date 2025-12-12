# TINY to Python Code Generated from fact.py
# K.C. Louden Compiler Project

import sys

x = int(sys.stdin.readline())
if (0 < x):
    fact = 1
    while True:
        fact = (fact * x)
        x = (x - 1)
        if (x == 0): break
    print(fact)

# End of TINY program execution
