
import json
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd


def lastRowWhere(d, cond):
    last = None

    for row in d.iterrows():
        if not cond(row[1]):
            return last
        else:
            last = row[1]


class Data(object):
    """
    A class to collect and present data on tests.

    :stream: Data from BWCtl's streams.
    :module: Data from the kernel MPC module.

    :rawBWCtl: Raw BWCtl data.
    :rawModule: Raw module data.
    """

    def __init__(self, test):
        """
        :test: The name of the test to gather information for.
        """
        bwctlFile = test + '-bwctl.json'
        moduleFile = test + '-module.json'
        timeFile = test + '-time.txt'

        with open(bwctlFile) as data:
            pdata = json.load(data)
            bwctlStartTime = pdata['start']['timestamp']['timesecs']
            strms = list(map(lambda i: i['streams'], pdata['intervals']))
            strms = [x for y in strms for x in y] # Flatten

            self.rawBWCtl = pdata
            self.stream = pd.DataFrame(strms)

        with open(moduleFile) as data:
            pdata = json.load(data)

            self.rawModule = pdata

            # columns is for when there are no entries.
            self.module = pd.DataFrame(list(filter(lambda e: e != {}, pdata)),
                    columns = ["time", "rtt_meas_us", "rate_set"])

            # Normalize times to be the same as those from BWCtl.
            self.module.time -= bwctlStartTime
