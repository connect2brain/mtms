#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from typing import TypedDict

# Type definitions shared across modules.

Intensity = int
Isi = int

class StimulationParameters(TypedDict):
    intensity: Intensity
    isi: Isi
