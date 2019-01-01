#!/usr/bin/python3

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
    def __init__(self, output, interval, verbose = False):
        self.output = output
        self.interval = interval
        self.verbose = verbose

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
                    with open(mpc + "/loss_meas") as lossF, \
                            open(mpc + "/loss_pred") as lossPF, \
                            open(mpc + "/rate_meas") as rateMF, \
                            open(mpc + "/rate_set") as rateSF, \
                            open(mpc + "/rb") as rbF:
                        loss = int(lossF.read())
                        lossP = int(lossPF.read())
                        rateM = int(rateMF.read())
                        rateS = int(rateSF.read())
                        rb = int(rbF.read())

                        info = {'time': t, 'id': os.path.basename(mpc),
                            'loss_meas': loss, 'loss_pred': lossP,
                            'rate_meas': rateM, 'rate_set': rateS,
                            'rb': rb}

                        data.append(info)

                        if self.verbose:
                            print("RTT (ms): {}, Set Rate (B/s): {}".format(
                                info['loss_meas'], info['rate_set']))

                except FileNotFoundError as err:
                    print(err)
                    pass

            s = self.interval - (now() - t)
            if s > 0:
                time.sleep(s)

            with self.lock:
                running = self.running

        json.dump(data, outputF, indent = 4)
