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

# Sleep forever here to allow the user to control the EEG service by separately executing
# commands ínside the container.
CMD ["sleep", "infinity"]
