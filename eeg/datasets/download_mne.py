#!/usr/bin/env python3
# -*- coding: utf8 -*-

import sys
import urllib
import os.path
import argparse
import mne
import mne.datasets


if __name__ == "__main__":
    def value_range(input_str):
        try:
            val_from, val_to = input_str.split("-")
            int_from = int(val_from.strip())
            int_to = int(val_to.strip())

            if int_from > int_to:
                raise argparse.ArgumentTypeError("'to' value cannot be smaller than 'from' value")

            return range(int_from, int_to+1)
        except TypeError as e:
            raise argparse.ArgumentTypeError("'{}' is not a valid range of type 'a-b'".format(input_str))

    parser = argparse.ArgumentParser(description = "Download MNE example data")
    parser.add_argument('dataset', help="Identifier of dataset to download, for example 'eegbci'.")
    parser.add_argument('--subject', type=int, default=1, help="Identifier of subject in the dataset, default=1. Acceptable values depend on the dataset.")
    parser.add_argument('--runs', type=value_range, default="1-2", help="The runs to use. See dataset description for more information.")
    parser.add_argument('--dryrun', default=False, action='store_true', help="Dry-run, ie. don't actually download anything.")

    args = parser.parse_args()

    #datadir = os.path.abspath(os.path.join(os.path.dirname(os.path.realpath(__file__)), "../"))
    #print("Using datadir: {}".format(datadir))

    #print(os.path.dirname(os.path.realpath(__file__)))
    current_dir = os.path.dirname(os.path.join(os.getcwd(), "datasets"))
    print("Using datasets directory ('{}') as base for data.".format(current_dir))

    dataset = args.dataset
    subject = args.subject
    runs = args.runs
    dryrun = args.dryrun

    #print("dataset = {}".format(dataset))
    #print("subject = {}".format(subject))
    #print("runs = {}".format(runs))

    if dataset == 'eegbci':
        from mne.datasets import eegbci
        if not dryrun:
            try:
                raw_fnames = eegbci.load_data(subject, runs, path=current_dir, update_path=False)
                print("Downloaded files:\n{}".format("\n".join(raw_fnames)))
            except urllib.error.HTTPError as e:
                sys.stderr.write("[ERROR] Could not find dataset. Error message: '{}'.\n".format(e))
                sys.exit(1)
        else:
            print("Dry-run: Nothing downloaded.")

    else:
        sys.stderr.write("[ERROR] Dataset with name '{}' not recognized.\n".format(dataset))
        sys.exit(1)

