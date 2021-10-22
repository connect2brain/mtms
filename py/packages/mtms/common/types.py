#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from typing import TypedDict

# Type definitions shared across modules.

Intensity = int
Iti = int

class StimulationParameters(TypedDict):
    intensity: Intensity
    iti: Iti
