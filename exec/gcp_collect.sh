#!/bin/bash
python3 collect.py
gsutil cp log.csv gs://<bucket>/ && gsutil cp log_error.txt gs://<bucket>/ && sudo poweroff
