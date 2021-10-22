FROM python:3.8

WORKDIR /code

RUN apt-get update && \
  apt-get install -y --no-install-recommends \
  dos2unix

# Python dependencies

# Poetry is installed using pip, which is not the recommended way. However, it suits our
# Docker workflow better, as it requires fewer steps and doesn't need modifying PATH manually.
RUN pip install poetry

COPY py/integration_tests/pyproject.toml .

COPY py/integration_tests/poetry.lock .

RUN poetry install

# Entrypoint

COPY py/integration_tests/docker-entrypoint.sh .

COPY third-party/wait-for-it/wait-for-it.sh .

RUN dos2unix docker-entrypoint.sh

RUN dos2unix wait-for-it.sh

RUN chmod +x ./docker-entrypoint.sh ./wait-for-it.sh

# Run

CMD ["./docker-entrypoint.sh"]
