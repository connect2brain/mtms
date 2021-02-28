FROM python:3.8

WORKDIR /code

RUN apt-get update && \
  apt-get install -y --no-install-recommends \
  sqlite3 \
  dos2unix

# Python dependencies

COPY py/backend/Pipfile .

COPY py/backend/Pipfile.lock .

RUN pip install pipenv

RUN pipenv install --deploy --ignore-pipfile

# Internal packages

COPY py/packages py/packages

RUN pipenv install -e py/packages

# Copy and initialize database

COPY db db

RUN dos2unix db/init-db.sh

RUN chmod 755 db/init-db.sh

RUN db/init-db.sh

# Run

CMD [ "pipenv", "run", "python", "py/mtms_bridge/src/python_runner.py"]
