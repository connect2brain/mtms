# Project Louhi Docs

## Initial set up for git

Note that to deploy with Git Bash in Windows requires an installation of _rsync_ which does not come by default.

    cd docs/
    git clone https://github.com/connect2brain/project-louhi-docs public

## AsyncAPI generator setup

    npm install -g @asyncapi/generator openapi-sampler

## Python setup

### Full Python set-up using pyenv, virtualenv, and pip-tools

    pyenv install -s 3.8.5
    pyenv local 3.8.5
    pip install virtualenv   # Install virtualenv (globally)
    virtualenv venv    # Create virtualenv in folder called venv
    source venv/bin/activate   # Activate the virtualenv
    pip install pip-tools
    pip-compile requirements.in   # Create requirements.txt
    pip-sync   # Install packages

### (Windows) Full set-up using pyenv, virtualenv, and pip-tools

    pyenv install 3.8.5
    pyenv local 3.8.5
    pip install virtualenv   # Install virtualenv (globally)
    virtualenv venv    # Create virtualenv in folder called venv
    source venv/Scripts/activate   # Activate the virtualenv
    pip install pip-tools
    pip-compile requirements.in   # Create requirements.txt
    pip-sync   # Install packages

### Set-up with only virtualenv

    python -v   # Check version is 3.8.5 or compatible
    pip install virtualenv   # Install virtualenv
    virtualenv venv   # Create virtualenv folder called venv
    source venv/bin/activate    # Activte the virtualenv
    pip install -r requirements.txt

### Updating Python packages

    pip-compile --upgrade

### Installing Python packages

    echo "package" >> requirements.in
    pip-sync

## Generating documentation

    make asyncapi
    make html

or

    make github

If you receive the error "Error: "[...]/project-louhi/docs/source" is in a git repository with unstaged changes. Please commit your changes before proceeding or add proper directory to .gitignore file. You can also use the --force-write flag to skip this rule (not recommended)."

    ASYNCAPIBUILDOPTS=--force-write make asyncapi

## Deploying documentation

    ./deploy.sh   # unix-like

## Running documentation locally

### Install NodeJS dependencies

    nvm use node   # If using NVM
    npm install node-static

### Running documentation server

    node doc-server.js

Browse to http://localhost:5000
