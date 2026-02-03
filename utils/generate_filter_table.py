#!/usr/bin/env python3

"""
Generates the lookup table for src/lib/audio.c audio_resample function
see docs/resample.py
"""

import numpy as np

L = 64
Nz = 13

# TODO: implement Kaiser Window
x = np.linspace(0, Nz, L * Nz + 1, dtype="float32")
h_s = np.sinc(x)
print(np.array2string(h_s, separator=','))
