
import json
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd


def getTestData(inFile):
    with open(inFile) as data:
        pdata = json.load(data)
        strms = list(map(lambda i: i['streams'], pdata['intervals']))
        strms = [x for y in strms for x in y] # Flatten

        return (pdata, pd.DataFrame(strms))

def getModuleData(inFile):
    with open(inFile) as data:
        pdata = json.load(data)

        # columns is for when there are no entries.
        df = pd.DataFrame(list(filter(lambda e: e != {}, pdata)),
                columns = ["time", "id", "loss_meas", "loss_pred",
                    "rate_meas", "rate_set", "rb"])

        return (pdata, df)


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
        testFile = test + '-test.json'
        moduleFile = test + '-module.json'

        (self.rawTest, self.stream) = getTestData(testFile)
        startTime = self.rawTest['start']['timestamp']['timesecs']

        (self.rawModule, self.module) = getModuleData(moduleFile)

        # Normalize times to be the same as those from BWCtl.
        self.module.time -= startTime
