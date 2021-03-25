# Project Louhi Docs

## Initial set up for git

	cd docs/
	git submodule update --init --recursive

### Full set-up using pyenv, virtualenv, and pip-tools

	pyenv install -s 3.8.5
	pyenv local 3.8.5
	pip install virtualenv   # Install virtualenv (globally)
	virtualenv venv    # Create virtualenv in folder called venv
	source venv/bin/activate   # Activate the virtualenv
	pip install pip-tools   
	pip-compile requirements.in   # Create requirements.txt
	pip-sync   # Install packages

### Set-up with only virtualenv

	python -v   # Check version is 3.8.5 or compatible
	pip install virtualenv   # Install virtualenv
	virtualenv venv   # Create virtualenv folder called venv
	source venv/bin/activate    # Activte the virtualenv
	pip install -r requirements.txt

### Updating packages


	pip-compile --upgrade

### Installing packages

	echo "package" >> requirements.in
	pip-sync
