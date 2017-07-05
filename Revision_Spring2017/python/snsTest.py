#!/usr/bin/env python

from os.path import splitext
import glob
import numpy as np
import pandas as pd
import matplotlib as mpl
mpl.use('Agg')
import matplotlib.pyplot as plt

#plt.set(context='paper', style='white', rc={'text.usetex':'true', 'font.serif':'Computer Modern Roman'})

plt.style.use('seaborn-paper')


for path in glob.glob('*on-controlOutput.csv'):
    try:
        df = pd.read_csv(path)
        df['time'] = df.index * 20

        ax = df.plot(x='time',y='ertt')
        ax2 = df.plot(x='time', y='controlRate', secondary_y=True, ax=ax)

        ax.set_xlabel('time (milliseconds)')
        ax.set_ylabel('latency (milliseconds)', color='b')
        ax2.set_ylabel('rate set by control (Gbps)', color='g')
        ax.set_title('Latency and control rate over time')
        ax.legend().set_visible(False)

        plt.savefig(splitext(path)[0]+'.pdf',format='pdf')
    except TypeError as e:
        print e

for path in glob.glob('*off-controlOutput.csv'):
    try:
        df = pd.read_csv(path)
        df['time'] = df.index * 20
        ax = df.plot(x='time',y='ertt', color='k')
        ax.set_xlabel('time (milliseconds)')
        ax.set_ylabel('latency (milliseconds)')
        ax.set_title('Latency over time without control')
        ax.legend().set_visible(False)
        plt.savefig(splitext(path)[0]+'.pdf',format='pdf')
    except TypeError as e:
        print e

for path in glob.glob('*iPerfOutput.csv'):
    try:
        df = pd.read_csv(path)
        ax = df.plot(x='end', y='bits_per_second', color='k')
        ax.set_xlabel('time (seconds)')
        ax.set_ylabel('throughput (bps)')
        ax.set_title('Throughput over time')
        ax.legend().set_visible(False)
        plt.savefig(splitext(path)[0]+'.pdf',format='pdf')
    except TypeError as e:
        print e
