#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sqlite3

class TopicDb:
    """A class to interface with the topic database.

    """
    _DATABASE_FILENAME = "db/topics.db"

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
        connection = sqlite3.connect(self._DATABASE_FILENAME)
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
