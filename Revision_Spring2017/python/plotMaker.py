#!/usr/bin/env python

#From http://www.randalolson.com/2012/06/26/using-pandas-dataframes/

from pandas import * 
import numpy as np
import glob
import matplotlib as mpl
import matplotlib.pyplot as plt
  
dataLists = {}  
  
# read data  
for folder in glob.glob("2017-06-08-04-40/*"):  
      
    dataLists[folder.split("/")[1]] = [] 
    print dataLists 
      
    for datafile in glob.glob(folder + "/*.csv"):  
  
        dataLists[folder.split("/")[1]].append(read_csv(datafile))  
  
# calculate stats for data  
meanDFs = {}  
stderrDFs = {}  

for key in dataLists.keys():  
    keyDF = (concat(dataLists[key], axis=1, keys=range(len(dataLists[key])))  
            .swaplevel(0, 1, axis=1)  
            .sortlevel(axis=1)  
            .groupby(level=0, axis=1))
    for key in keyDF:
        meanDFs[key] = newDF.mean()  
        stderrDFs[key] = newDF.std().div(sqrt(len(dataLists[key]))).mul(2.0)  

    keyDF = None

def sinplot(flip=1):
    x = np.linspace(0, 14, 100)
    for i in range(1, 7):
        plt.plot(x, np.sin(x + i * .5) * (7 - i) * flip)
