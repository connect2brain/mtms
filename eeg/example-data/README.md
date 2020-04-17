# Example data

## Downloading the data

The data in this directory are some of the example data in mne-python package.
It can be re-downloaded by running the following in Python:

    from mne.datasets import eegbci

    subject = 1
    runs = range(1, 15)
    raw_fnames = eegbci.load_data(subject, runs)
