#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from typing import Any, Callable, List

def drop_unique(l: List[Any], cond: Callable[[Any], bool]) -> int:
    """Given a list and a condition, drop the element for which the condition is True.
    Modify the list in place. Return the index of the dropped element.

    If the condition is True for none of the elements or more than one element, raise an error.

    Parameters
    ----------
    l
        The list of interest.
    cond
        A function that takes an element of the list and return a boolean.
    """
    index: int = None
    for i, element in enumerate(l):
        if cond(element):
            assert index is None, "Condition is True for more than one element"
            index = i
    assert index is not None, "Condition is True for none of the elements"
    del l[index]

    return index

def clamp(value: Any, min_value: Any, max_value: Any) -> Any:
    """Given a value of any type supported by > and < operators, and minimum and
    maximum values, clamp the given value with the following rules:

    - If value > max_value, return max_value.
    - If value < min_value, return min_value.
    - Otherwise return value.

    Parameters
    ----------
    value
        A value supported by > and < operators.
    min_value
        The minimum value.
    max_value
        The maximum value.
    """
    if value > max_value:
        return max_value
    elif value < min_value:
        return min_value
    else:
        return value
