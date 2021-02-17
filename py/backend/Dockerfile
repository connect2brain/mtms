FROM python:3.8

WORKDIR /code

RUN apt-get update && \
  apt-get install -y --no-install-recommends \
  sqlite3 \
  dos2unix

# Python dependencies

COPY py/backend/Pipfile .

RUN pip install pipenv

RUN pipenv install --deploy --ignore-pipfile

# Copy and initialize database

COPY db db

RUN dos2unix db/init-db.sh

RUN db/init-db.sh

# Run

CMD [ "pipenv", "run", "python", "-m", "py.backend.src.runner"]
