#!/usr/bin/env python

import os
import json
import argparse
import csv

def main():
    parser = argparse.ArgumentParser(
        description='Prepare iperf3 JSON output for MATLAB'
    )
    parser.add_argument(
        'file', nargs='+', help='path to the JSON file'
    )
    args = parser.parse_args()
    for path in args.file:
        with open(path) as fp:
            data = json.load(fp)
        pth = os.path.splitext(path)
        newpath = pth[0] + '.csv'
        with open(newpath,'wb') as ofp:
            writer=csv.writer(ofp)
            title =  data['intervals'][0]['streams'][0].keys()
            if 'omitted' in title: title.remove('omitted')
            writer.writerow(title)
            for interval in data['intervals']:
                del(interval['streams'][0]['omitted'])
                vals = interval['streams'][0].values()
                writer.writerow(vals)
if __name__ == '__main__':
    main()
