#!/usr/bin/env python

#From http://www.randalolson.com/2012/06/26/using-pandas-dataframes/

from pandas import * 
import numpy as np
import glob  
  
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

numerics = ['int16', 'int32', 'int64', 'float16', 'float32', 'float64']

newDF = keyDF.select_dtypes(include=numerics)
for key in newDF:
    meanDFs[key] = newDF.mean()  
    stderrDFs[key] = newDF.std().div(sqrt(len(dataLists[key]))).mul(2.0)  
      
keyDF = None  
  
# plot data  
for column in meanDFs[key].columns:  
      
    # don't plot generation over generation - that's pointless!  
    if not (column == "generation"):  
      
        figure(figsize=(20, 15))  
        title(column.replace("_", " ").title())  
        ylabel(column.replace("_", " ").title())  
        xlabel("Generation")  
          
        for key in meanDFs.keys():  
  
            errorbar(x=meanDFs[key]["generation"], y=meanDFs[key][column], yerr=stderrDFs[key][column], label=key)  
              
        legend(loc=2)  
