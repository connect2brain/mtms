#!/bin/sh

set -e

OPENNEURO=openneuro
GITANNEX=git-annex
DATALAD=datalad

check_openneuro() {
	command -v "$OPENNEURO" >/dev/null 2>&1 || { echo >&2 "Warning. The command '$OPENNEURO' was not found. It is needed for some of the datasets. Install it with 'npm install -g openneuro-cli'."; return 1; }
}

check_gitannex() {
	command -v "$GITANNEX" >/dev/null 2>&1 || { echo >&2 "Warning. The command '$GITANNEX' was not found. It is needed for some of the datasets. Install instructions in README.md."; return 1; }
}

check_datalad() {
	command -v "$DATALAD" >/dev/null 2>&1 || { echo >&2 "Warning. The command '$DATALAD' was not found. It is needed for some of the datasets. Install with 'pip install datalad'."; return 1; }
}

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
       "EEG-BCI" dataset from MNE datasets. https://mne.tools/stable/overview/datasets_index.html#eegbci-motor-imagery
    restingstatetms
    	"Resting State TMS" dataset from Openneuro.org. https://dx.doi.org/10.18112/openneuro.ds001832.v1.0.1
    hypnosis
      "Hypnosis TMS-EEG dataset" from osf.io. https://dx.doi.org/10.17605/OSF.IO/E2PKT
    dbseeg
      Deep brain stimulation in treatment resistant depression (EEG). https://dx.doi.org/10.18112/openneuro.ds001784.v1.1.2

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
		pipenv run python download_mne.py --subject 1 --runs 1-14 eegbci && echo "Success." && exit
		echo "Something went wrong."
		exit 1
		;;
	restingstatetms)
		echo "Downloading dataset into folder 'restingstatetms'"
		check_openneuro && $OPENNEURO download --snapshot 1.0.1 ds001832 restingstatetms/ && echo "Success." && exit
		check_datalad && $DATALAD install --get-data --source https://github.com/OpenNeuroDatasets/ds001832.git restingstatetms && echo "Success." && exit
		echo "Error: Manual download required. Go to https://openneuro.org/datasets/ds001832/versions/1.0.1." && exit 1
		;;
	hypnosis)
		echo "Downloading dataset into folder 'hypnosis'"
		echo "Error: Manual download required. Go to https://osf.io/m6ky2/." && exit 1
		;;
	dbseeg)
		echo "Downloading dataset into folder 'dbseeg'"
		check_datalad && datalad install --get-data --source https://github.com/OpenNeuroDatasets/ds001784.git dbseeg && echo "Success." && exit
		echo "Something went wrong"
		exit 1
		;;
esac

echo "Could not find dataset called '$DATASET'"
exit 1
