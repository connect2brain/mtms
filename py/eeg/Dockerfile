FROM python:3.8

WORKDIR /code

RUN apt-get update && \
  apt-get install -y --no-install-recommends \
  dos2unix

# Python dependencies

COPY py/eeg/Pipfile .

COPY py/eeg/Pipfile.lock .

RUN pip install pipenv

RUN pipenv install --deploy --ignore-pipfile

# Internal packages

COPY py/packages py/packages

RUN pipenv install -e py/packages

# Run

# TODO: runner.py takes the data files as parameters, pass them here.
CMD [ "pipenv", "run", "python", "py/eeg/runner.py"]
