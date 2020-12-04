# Project Louhi

This project contains the git repository for development of the mTMS system in C2B.

## Installation

Make sure that you have Python 3.6 installed on your system.

### Install Python 3.6 using Pyenv

	pyenv install -v 3.6.10
	pyenv local 3.6.10

### Install required packages

	pipenv install

### Install Zookeeper and Kafka

After installing, ensure that:

1. If using a Windows system, %KAFKA_HOME% points to the top-level directory in
   which Kafka is installed, and %ZOOKEEPER_HOME% points to the top-level directory
   of Zookeeper installation.

## Makefile

A Makefile is provided, including make targets to run different parts of the system.

Run `make help` for a list of make targets.

## Backend

The backend resides in backend/ directory.

## Frontend

The web frontend resides in frontend/ directory.

Start frontend by running:

    cd frontend
    npm install
    npm run serve

## Scripts

Scripts is used for command line scripts, for example, .bat and .sh scripts.

## Sandbox

Sandbox is used to store experimental scripts, drafts, etc.