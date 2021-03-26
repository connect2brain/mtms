#!/bin/sh

cd "$(dirname "$0")"
rm -f topics.db
sqlite3 topics.db ".read topics.sql"
chmod a-w topics.db
