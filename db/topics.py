#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sqlite3

class TopicDb:
    _DATABASE_LOCATION = "db/topics.db"

    def _run_query(self, query):
        """Query the topic database with a given query, return the query result.

        Parameters
        ----------
        query : str
            The query string.

        Returns
        -------
        array_like
            An array of the query result rows.
        """
        connection = sqlite3.connect(self._DATABASE_LOCATION)
        cursor = connection.cursor()
        cursor.execute(query)
        record = cursor.fetchall()
        connection.close()

        return record

    def get_parameter_topics(self):
        """Return topics of the type 'parameter' and their ActiveX control names.

        Returns
        -------
        array_like
            An array of topic names.
        dict
            A dictionary with topic names as keys and ActiveX control names as values.
        """
        record = self._run_query("select name, activex_control_name from topics where type='parameter';")

        topics = []
        control_names = {}
        for name, activex_control_name in record:
            topics.append(name)
            control_names[name] = activex_control_name

        return topics, control_names
