#!/usr/bin/python3

import threading
import time
import os

mpcccSysfs = '/sys/kernel/mpccc'

class RateSysfs:
    """
    Sets the minimum and maximum pacing rate for a process. It does this
    continually in a separate thread, so that when the process opens a port it
    gets set.
    """
    def __init__(self, proc, minRate, maxRate, delay=0.25):
        self.proc = proc
        self.minRate = minRate
        self.maxRate = maxRate
        self.delay = delay

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

        while running:
            if self.proc.poll() is None:
                self.setMinMaxRate()
                time.sleep(self.delay)
            else:
                return

            with self.lock:
                running = self.running

    def setMinMaxRate(self):
        if self.minRate is None and self.maxRate is None:
            return

        for c in self.proc.connections():
            port = c.laddr.port

            try:
                subdirs = os.listdir(mpcccSysfs)
                for mpcid in subdirs:
                    filename = mpcccSysfs + '/' + mpcid
                    sockNum = 0

                    with open(filename + '/sock_num') as f:
                        sockNum = int(f.read())

                    if sockNum == port:
                        if self.minRate is not None:
                            with open(filename + '/min_rate', 'w') as f:
                                f.write(str(self.minRate))

                        if self.maxRate is not None:
                            with open(filename + '/max_rate', 'w') as f:
                                f.write(str(self.maxRate))
            except:
                print("Could not set min/max rate for port {}.".format(port))
