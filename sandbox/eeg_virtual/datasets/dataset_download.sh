#!/bin/sh

set -e

OPENNEURO=openneuro
GITANNEX=git-annex

# First check for openneuro command
#command -v "$OPENNEURO" >/dev/null 2>&1 || { echo >&2 "The '$OPENNEURO' tool was not found. Install it with 'npm install -g openneuro-cli'."; exit 1; }

command -v "$GITANNEX" >/dev/null 2>&1 || { echo >&2 "The '$GITANNEX' tool was not found. Install instructions in README.md."; exit 1; }

# Print usage 
usage() {  
	cat <<-END
Usage: $0 [-h | -l | dataset]
------
   -h | --help
     Display this help
   -l | --list
     List available datasets
   dataset
     Load dataset
END
}

list_datasets() {
	cat <<-END
Available datasets:
-------------------
    forrestgump
      "Forrest Gump" BIDS EEG dataset from Openneuro.org
END
}

# Check for arguments
if [ $# -ne 1 ] ; then

	>&2 usage
	exit 1

else

	case "$1" in
		-h | --help)
			usage
			exit
			;;
		-l | --list)
			list_datasets
			exit
			;;
	esac 

	DATASET=$1
fi

case "$DATASET" in
	"forrestgump")
		echo "Downloading dataset into folder 'forrestgump'"
		#openneuro download --snapshot 1.3.0 ds000113 forrestgump/ && echo "Success" && exit
		
		echo "Something went wrong"
		exit 1
esac

echo "Could not find dataset called '$DATASET'"
exit 1
