# Project Louhi Docs

The docs are automatically updated when new commits are pushed into `origin/main`.

Below are the instructions for previewing the documentation locally and also updating them manually,
if needed.

## Initial set up for git

Note that to deploy with Git Bash in Windows requires an installation of _rsync_ which does not come by default.

    cd docs/
    git clone https://github.com/connect2brain/project-louhi-docs public

## AsyncAPI generator setup

    npm install -g @asyncapi/generator openapi-sampler

## Python setup

Python version 3.8 or later is assumed.

### Full Python set-up using virtualenv, and pip-tools

    pip install virtualenv   # Install virtualenv (globally)
    virtualenv venv    # Create virtualenv in folder called venv

    source venv/bin/activate   # Activate the venv (MacOS/Unix/Linux)
    venv\Scripts\activate.bat   # Activate the venv (Microsoft Windows)

    pip install pip-tools
    pip-compile requirements.in   # Create requirements.txt
    pip-sync   # Install packages

## Generating documentation

    make asyncapi
    make html

or

    make github

If you receive the error "Error: "[...]/project-louhi/docs/source" is in a git repository with unstaged changes. Please commit your changes before proceeding or add proper directory to .gitignore file. You can also use the --force-write flag to skip this rule (not recommended)."

    ASYNCAPIBUILDOPTS=--force-write make asyncapi

## Deploying documentation

    ./deploy.sh   # unix-like

### Running documentation server locally

    cd public && python -m http.server 5000 && cd ..

Browse to http://localhost:5000
