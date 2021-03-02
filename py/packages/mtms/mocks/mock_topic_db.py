class MockTopicDb:
    """A class for mocking topic database.

    """

    _TOPICS = {
        'parameter': [
            'intensity',
            'iti',
        ],
        'command': [
            'stimulate',
        ],
        'state': [
            'error_code',
        ]
    }

    _LATCHED_TOPICS = ['intensity']

    def get_topics_by_type(self, type):
        return self._TOPICS[type] if type in self._TOPICS else []

    def is_topic_latched(self, topic):
        return topic in self._LATCHED_TOPICS
