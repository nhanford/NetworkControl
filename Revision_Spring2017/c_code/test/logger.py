#!/usr/bin/python

import argparse
import datetime
import glob
import json
import threading
import time
import os

def now():
    dt = datetime.datetime.now()
    return float("{}.{}".format(dt.strftime("%s"), dt.microsecond))

class Logger:
    def __init__(self, output, interval):
        self.output = output
        self.interval = interval

        self.running = True
        self.lock = threading.Lock()
        self.thread = None

    def start(self):
        self.running = True
        self.thread = threading.Thread(target = self.run)
        self.thread.start()

    def stop(self):
        with self.lock:
            self.running = False

        self.thread.join()

    def run(self):
        running = True

        data = []
        outputF = open(self.output, mode = 'w')

        while running:
            t = now()

            for mpc in glob.glob("/sys/kernel/debug/mpc/*"):
                try:
                    with open(mpc + "/rtt_meas_us") as rttF, \
                            open(mpc + "/rtt_pred_us") as rttPF, \
                            open(mpc + "/rate_set") as rateF, \
                            open(mpc + "/probing") as probingF:
                        rtt = int(rttF.read())
                        rttP = int(rttPF.read())
                        rate = int(rateF.read())

                        if probingF.read() == "Y\n":
                            probing = True
                        else:
                            probing = False

                        data.append({'time': t, 'id': os.path.basename(mpc),
                            'rtt_meas_us': rtt, 'rtt_pred_us': rttP,
                            'rate_set': rate, 'probing': probing})

                except FileNotFoundError:
                    pass

            s = self.interval - (now() - t)
            if s > 0:
                time.sleep(s)

            with self.lock:
                running = self.running

        json.dump(data, outputF, indent = 4)
