from typing import Dict, List

TopicType = str
Topic = str

class MockTopicDb:
    """A class for mocking topic database.

    """

    _TOPICS: Dict[TopicType, List[Topic]] = {
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

    _LATCHED_TOPICS: List[Topic] = ['intensity']

    def get_topics_by_type(self, type: TopicType) -> List[Topic]:
        return self._TOPICS[type] if type in self._TOPICS else []

    def is_topic_latched(self, topic: Topic) -> bool:
        return topic in self._LATCHED_TOPICS
