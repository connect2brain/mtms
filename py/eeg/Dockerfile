FROM python:3.8

WORKDIR /code

RUN apt-get update && \
  apt-get install -y --no-install-recommends \
  sqlite3 \
  dos2unix

# Python dependencies

COPY py/eeg/Pipfile .

COPY py/eeg/Pipfile.lock .

RUN pip install pipenv

RUN pipenv install --deploy --ignore-pipfile

# Internal packages

COPY py/packages py/packages

RUN pipenv install -e py/packages

# Integration tests

COPY py/eeg/integration-tests.sh .

COPY third-party/wait-for-it/wait-for-it.sh .

RUN dos2unix integration-tests.sh

RUN dos2unix wait-for-it.sh

RUN chmod +x ./integration-tests.sh ./wait-for-it.sh

# Copy and initialize database

COPY db db

RUN dos2unix db/init-db.sh

RUN chmod 755 db/init-db.sh

RUN db/init-db.sh

# Sleep forever here to allow the user to control the EEG service by separately executing
# commands ínside the container.
CMD ["sleep", "infinity"]
