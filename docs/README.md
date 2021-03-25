# Project Louhi Docs

## Initial set up for git

Note that to deploy with Git Bash in Windows requires an installation of *rsync* which does not come by default.

	cd docs/
	git clone git@github.com:connect2brain/project-louhi-docs.git public

### Full set-up using pyenv, virtualenv, and pip-tools

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

### Updating packages

	pip-compile --upgrade

### Installing packages

	echo "package" >> requirements.in
	pip-sync
