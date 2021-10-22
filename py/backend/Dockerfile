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

COPY py/backend/pyproject.toml .

COPY py/backend/poetry.lock .

RUN poetry install

# Copy and initialize database

COPY db db

RUN dos2unix db/init-db.sh

RUN chmod 755 db/init-db.sh

RUN db/init-db.sh

# Entrypoint

COPY py/backend/docker-entrypoint.sh .

COPY third-party/wait-for-it/wait-for-it.sh .

RUN dos2unix docker-entrypoint.sh

RUN dos2unix wait-for-it.sh

RUN chmod +x ./docker-entrypoint.sh ./wait-for-it.sh

# Run

CMD ["./docker-entrypoint.sh"]
