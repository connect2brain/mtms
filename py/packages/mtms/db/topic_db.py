#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from typing import List

import sqlite3
from sqlite3 import Connection, Cursor, Row

class TopicDb:
    """A class to interface with the topic database.

    """
    _DATABASE_FILENAME: str = "db/topics.db"

    def _run_query(self, query: str) -> List[Row]:
        """Query the topic database with a given query, return the query result.

        Parameters
        ----------
        query
            The query string.

        Returns
        -------
            An array of the query result rows.
        """
        connection: Connection = sqlite3.connect(self._DATABASE_FILENAME)
        cursor: Cursor = connection.cursor()
        cursor.execute(query)
        record: List[Row] = cursor.fetchall()
        connection.close()

        return record

    def get_topics(self, type: str = None) -> List[str]:
        """Return topics of the desired type.

        Parameters
        ----------
        type
            The type of the topics to be returned. If not given, return
            all topics.

        Returns
        -------
            An array of topic names.
        """
        query: str
        if type is None:
            query = "select name from topics;"
        else:
            query = "select name from topics where type='{}';".format(type)

        record: List[Row] = self._run_query(query)

        topics: List[str] = []
        row: Row
        for row in record:
            topics.append(row[0])

        return topics

    def is_topic_latched(self, topic: str) -> bool:
        """Return true if the topic is latched.

        Latching means that a new consumer of the topic will receive the latest message that has been
        produced into the topic at the time when the consumer is created.

        Parameters
        ----------
        topic
            The name of the topic.

        Returns
        -------
            True if the topic is latched, False otherwise.
        """
        query: str = "select latch from topics where name='{}';".format(topic)
        record: List[Row] = self._run_query(query)
        return record[0][0]
