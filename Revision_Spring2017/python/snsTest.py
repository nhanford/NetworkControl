#!/usr/bin/env python

from os.path import splitext
import glob
import numpy as np
import pandas as pd
import matplotlib as mpl
mpl.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import matplotlib.units as units

#plt.set(context='paper', style='white', rc={'text.usetex':'true', 'font.serif':'Computer Modern Roman'})

plt.style.use('seaborn-paper')

for path in glob.glob('*on-controlOutput.csv'):
    try:
        df = pd.read_csv(path).head(3000)
        df['time'] = df.index
        ax = df.plot(x='time',y='ertt')
        ax2 = df.plot(x='time', y='controlRate', secondary_y=True, ax=ax)
        ax.set_ylim(ymin=0)
        ax.set_xlabel('time (milliseconds)')
        ax.set_ylabel('latency (milliseconds)', color='b')
        ax2.set_ylabel('rate set by control (Gbps)', color='g')
        ax2.set_ylim(ymin=0)
        ax.set_title('Latency and control rate over time')
        ax.legend().set_visible(False)
        fig = plt.gcf()
        fig.set_size_inches(4,2.75,forward=True)
        fig.subplots_adjust(bottom=.15)
        fig.savefig(splitext(path)[0]+'.pdf',format='pdf')
        plt.close()
    except TypeError as e:
        print e

for path in glob.glob('*on-controlOutput.csv'):
    try:
        df = pd.read_csv(path).head(3000)
        df['time'] = df.index
        ax = df.plot(x='time',y='ertt')
        ax.set_xlabel('time (milliseconds)')
        ax.set_ylabel('latency (milliseconds)', color='k')
        ax2 = df.plot(x='time', y='lHat', secondary_y=False, ax=ax)
        ax.set_title('Latency and predicted latency over time')
        blue_patch = mpatches.Patch(color='blue', label='Measured latency')
        green_patch = mpatches.Patch(color='green', label='Predicted latency')
        plt.legend(handles=[blue_patch, green_patch])
        fig = plt.gcf()
        fig.set_size_inches(4,2.75,forward=True)
        fig.subplots_adjust(bottom=.15)
        plt.savefig(splitext(path)[0]+'lHat.pdf', format='pdf')
        plt.close()
    except TypeError as e:
        print e

for path in glob.glob('*off-controlOutput.csv'):
    try:
        df = pd.read_csv(path).head(3000)
        df['time'] = df.index
        ax = df.plot(x='time',y='ertt')
        ax.set_xtick
        ax.set_xlabel('time (milliseconds)')
        ax.set_ylabel('latency (milliseconds)', color='k')
        ax2 = df.plot(x='time', y='lHat', secondary_y=False, ax=ax)
        ax.set_title('Latency and predicted latency over time without control')
        blue_patch = mpatches.Patch(color='blue', label='Measured latency')
        green_patch = mpatches.Patch(color='green', label='Predicted latency')
        plt.legend(handles=[blue_patch, green_patch])
        fig = plt.gcf()
        fig.set_size_inches(4,2.75,forward=True)
        fig.subplots_adjust(bottom=.15)
        plt.savefig(splitext(path)[0]+'lHat.pdf', format='pdf')
        plt.close()
    except TypeError as e:
        print e

for path in glob.glob('*iPerfOutput.csv'):
    try:
        df = pd.read_csv(path)
        ax = df.plot(x='end', y='bits_per_second', color='k')
        ax.set_ylim(ymin=0)
        ax.set_xlabel('time (seconds)')
        ax.set_ylabel('throughput (bps)')
        ax.set_title('Throughput over time')
        ax.legend().set_visible(False)
        fig = plt.gcf()
        fig.set_size_inches(4,2.75,forward=True)
        fig.subplots_adjust(bottom=.15)
        plt.savefig(splitext(path)[0]+'.pdf',format='pdf')
        plt.close()
    except TypeError as e:
        print e
