#!/bin/bash
python3 collect.py
aws s3 cp log.csv s3://<bucket>/
aws s3 cp log_error.txt s3://<bucket>/
poweroff
