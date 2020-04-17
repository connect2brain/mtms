#!/bin/sh

set -e

OPENNEURO=openneuro
GITANNEX=git-annex
DATALAD=datalad

# First check for openneuro command
command -v "$OPENNEURO" >/dev/null 2>&1 || { echo >&2 "The '$OPENNEURO' tool was not found. Install it with 'npm install -g openneuro-cli'."; exit 1; }

command -v "$GITANNEX" >/dev/null 2>&1 || { echo >&2 "The '$GITANNEX' tool was not found. Install instructions in README.md."; exit 1; }

command -v "$DATALAD" >/dev/null 2>&1 || { echo >&2 "The '$DATALAD' tool was not found. Install with `pip install datalad`."; exit 1; }

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
    eegbci
       "EEG-BCI" dataset from MNE datasets.
    forrestgump
      "Forrest Gump" BIDS EEG dataset from Openneuro.org
    restingstatetms
    	"Resting State TMS" dataset from Openneuro.org
    tesaexample
      Example TMS-EEG dataset from TESA/EEGLAB.
    hypnosis
      "Hypnosis TMS-EEG dataset" from osf.io.
    dbseeg
      Deep brain stimulation in treatment resistant depression (EEG)

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
	eegbci)
		echo "Downloading dataset into folder 'MNE-eegbci-data/files/eegmmidb/1.0.0'"
		pipenv run src/download_mne.py --subject 1 --runs 1-15 eegbci && echo "Success." && exit
		echo "Something went wrong"
		exit 1
		;;
	forrestgump)
		echo "Downloading dataset into folder 'forrestgump'"
		#openneuro download --snapshot 1.3.0 ds000113 forrestgump/ && echo "Success" && exit
		$DATALAD install --get-data --source https://github.com/OpenNeuroDatasets/ds000113.git forrestgump && echo "Success." && exit
		echo "Something went wrong"
		exit 1
		;;
	restingstatetms)
		echo "Downloading dataset into folder 'restingstatetms'"
		$OPENNEURO download --snapshot 1.0.1 ds001832 restingstatetms/ && echo "Success." && exit
		#$DATALAD install --get-data --source https://github.com/OpenNeuroDatasets/ds001832.git restingstatetms && echo "Success." && exit
		echo "Error: Manual download required. Go to https://openneuro.org/datasets/ds001832/versions/1.0.1." && exit 1
		;;
	hypnosis)
		echo "Downloading dataset into folder 'hypnosis'"
		echo "Error: Manual download required. Go to https://osf.io/m6ky2/." && exit 1
		;;
	dbseeg)
		echo "Downloading dataset into folder 'dbseeg'"
		datalad install --get-data --source https://github.com/OpenNeuroDatasets/ds001784.git dbseeg && echo "Success." && exit
		echo "Something went wrong"
		exit 1
		;;
esac

echo "Could not find dataset called '$DATASET'"
exit 1
