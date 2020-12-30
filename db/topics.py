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

    def get_topics_by_type(self, type):
        """Return topics of the desired type.

        Parameters
        ----------
        type : str
            The type of the topics to be returned.

        Returns
        -------
        array_like
            An array of topic names.
        """
        query = "select name from topics where type='{}';".format(type)
        record = self._run_query(query)

        topics = []
        for row in record:
            topics.append(row[0])

        return topics

    def get_control_names_for_topics(self, topics):
        """Return activex control names for an array of topics.

        Parameters
        ----------
        topics : array_like
            An array of topics for which the control names are returned.

        Returns
        -------
        dict
            A dictionary with topic names as keys and ActiveX control names as values.
        """
        topics_str = ', '.join(['\'' + topic + '\'' for topic in topics])
        query = "select name, activex_control_name from topics where name in ({});".format(topics_str)
        record = self._run_query(query)

        control_names = {}
        for name, activex_control_name in record:
            control_names[name] = activex_control_name

        return control_names

    def is_topic_latched(self, topic):
        """Return true if the topic is latched.

        Latching means that a new consumer of the topic will receive the latest message that has been
        produced into the topic at the time when the consumer is created.

        Parameters
        ----------
        topic : str
            The name of the topic.

        Returns
        -------
        boolean
            True if the topic is latched, False otherwise.
        """
        query = "select latch from topics where name='{}';".format(topic)
        record = self._run_query(query)
        return record[0][0]
