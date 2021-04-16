# Minimal makefile for Sphinx documentation
#

# You can set these variables from the command line, and also
# from the environment for the first two.
SPHINXOPTS    ?=
SPHINXBUILD   ?= sphinx-build
SOURCEDIR     = source
BUILDDIR      = build
ASYNCAPIBUILD ?= ag
APISOURCEFILE  = api/asyncapi.yaml
APIBUILDDIR   = source
APITEMPLATE   = https://github.com/asyncapi/markdown-template
ASYNCAPIBUILDOPTS ?= --force-write

# Put it first so that "make" without argument is like "make help".
help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

.PHONY: help Makefile

# Catch-all target: route all unknown targets to Sphinx using the new
# "make mode" option.  $(O) is meant as a shortcut for $(SPHINXOPTS).
%: Makefile
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

asyncapi: 
	@$(ASYNCAPIBUILD) "$(APISOURCEFILE)" "$(APITEMPLATE)" -o "$(APIBUILDDIR)" $(ASYNCAPIBUILDOPTS) 
	
github:
	@make asyncapi
	@make html
