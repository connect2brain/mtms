FROM python:3.8

WORKDIR /code

RUN apt-get update && \
  apt-get install -y --no-install-recommends \
  sqlite3 \
  dos2unix

# Python dependencies

# Poetry is installed using pip, which is not the recommended way. However, it suits our
# Docker workflow better, as it requires fewer steps and doesn't need modifying PATH manually.
RUN pip install poetry

COPY py/packages py/packages

COPY py/mtms_bridge/pyproject.toml .

COPY py/mtms_bridge/poetry.lock .

RUN poetry install

# Copy and initialize database

COPY db db

RUN dos2unix db/init-db.sh

RUN chmod 755 db/init-db.sh

RUN db/init-db.sh

# Run

CMD [ "poetry", "run", "python", "py/mtms_bridge/src/python_runner.py"]
