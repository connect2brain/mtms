#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import csv

def get_parameter_topics():
    """Return parameter-containing topics and corresponding ActiveX control names.

    Returns
    -------
    array_like
        An array of topic names.
    dict
        A dictionary with topic names as keys and corresponding ActiveX control names as values.
    """
    control_names = {}
    topics = []
    # XXX: Consider using SQLite database instead of csv file if the structure becomes more complicated.
    with open('db/parameter_topics.csv', mode='r') as csv_file:
        csv_reader = csv.DictReader(csv_file, delimiter=';', quotechar='"')
        for row in csv_reader:
            topic = row['topic']
            control_name = row['control_name']

            topics.append(topic)
            control_names[topic] = control_name

    return topics, control_names
