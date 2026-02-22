#!/usr/bin/env python3

import csv
import subprocess
import sys

if len(sys.argv) != 3:
    print(f"Usage: {sys.argv[0]} matches.csv your_job.fcl")
    sys.exit(1)

csv_file = sys.argv[1]
fcl_file = sys.argv[2]

with open(csv_file) as f:
    reader = csv.DictReader(f)

    for row in reader:
        filename = row["original_file"]
        event = row["event"]
        index = row["index"]
        cryo = row["cryo"]

        print(f"### Event {event} (cryo {cryo})")

        try:
            path = subprocess.check_output(
                ["samweb", "get-file-access-url", "--schema", "root", filename],
                text=True
            ).strip()
        except subprocess.CalledProcessError:
            print(f"# ERROR: could not resolve path for {filename}\n")
            continue

        print(f"lar -c {fcl_file} {path} -n 1 --nskip {index}\n")
