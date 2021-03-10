FROM python:3.8

WORKDIR /code

RUN apt-get update && \
  apt-get install -y --no-install-recommends \
  dos2unix

# Python dependencies

COPY py/integration_tests/Pipfile .

COPY py/integration_tests/Pipfile.lock .

RUN pip install pipenv

RUN pipenv install --deploy --ignore-pipfile

# Entrypoint

COPY py/integration_tests/docker-entrypoint.sh .

COPY third-party/wait-for-it/wait-for-it.sh .

RUN dos2unix docker-entrypoint.sh

RUN dos2unix wait-for-it.sh

RUN chmod +x ./docker-entrypoint.sh ./wait-for-it.sh

# Run

CMD ["./docker-entrypoint.sh"]
