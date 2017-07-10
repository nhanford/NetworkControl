#!/usr/bin/env python
'''
controllerTest.py -- Applies control to FQ-CoDel pacing qdisc.
livetest is designed for any Linux distribution with tc_fq available in
its iproute2 package

Apologies for the lack of neatness. This is scratch code.

@author: Nathan Hanford
@contact: nhanford@ucdavis.edu
@deffield: updated: Updated

'''
import sys
import os
import re
import logging
import subprocess
import time
import argparse
import json
import csv
import tempfile
import shutil
import socket
from controller import Controller
from datetime import datetime

# Adaptive filter parameters.
ALPHA = 0.5
P = 5
Q = 1

# Controller parameters.
GAMMA = 0.5

# Latency generator parameters.
L_MAX = 1.0
L_SLOPE = 0.01
L_CUT = 0.1
R_COEFF = -0.02
NOISE_SD = 0.0

def pollss():
    '''gets data from ss'''
    out = subprocess.check_output(['ss', '-i', '-t', '-n'])
    out = re.sub(r'\A.+\n', '', out)
    out = re.sub(r'\n\t', '', out)
    out = out.splitlines()
    return out

def parseconnection(connection):
    '''parses a string representing a single TCP connection'''
    #Junk gets filtered in @loadconnections
    try:
        connection = connection.strip()
        ordered = re.sub(r':|,|/|Mbps', ' ', connection)
        ordered = connection.split()
        ips = re.findall(r'\d+\.\d+\.\d+\.\d+', connection)
        ports = re.findall(r'\d:\w+', connection)
        rtt = re.search(r'rtt:\d+[.]?\d+', connection)
        wscaleavg = re.search(r'wscale:\d+', connection)
        mss = re.search(r'mss:\d+', connection)
        cwnd = re.search(r'cwnd:\d+', connection)
        retrans = re.search(r'retrans:\d+\/\d+', connection)
    except Exception as e:
        logging.warning('connection {} could not be parsed'.format(connection))
        return -1, -1, -1, -1, -1, -1, -1
    if rtt:
        rtt = float(rtt.group(0)[4:])
    else:
        rtt = -1
    if wscaleavg:
        wscaleavg = wscaleavg.group(0)[7:]
        wscaleavg = int(wscaleavg)
    else:
        wscaleavg = -1
    if cwnd:
        cwnd = cwnd.group(0)[5:]
        cwnd = int(cwnd)
    else:
        cwnd = -1
    if retrans:
        retrans = retrans.group(0)
        retrans = re.sub(r'retrans:\d+\/', '', retrans)
        retrans = int(retrans)
    else:
        retrans = -1
    if mss:
        mss = mss.group(0)[4:]
        mss = int(mss)
    else:
        mss = -1
    if len(ips) > 1 and len(ports) > 1 and rtt and wscaleavg and cwnd and retrans:
        ports[0] = int(ports[0][2:])
        ports[1] = int(ports[1][2:])
        return ips, ports, rtt, wscaleavg, cwnd, retrans, mss
    logging.warning('connection {} could not be parsed'.format(connection))
    return -1, -1, -1, -1, -1, -1, -1

def findconn(connections,dest):
    '''given a list of connections, return the connection matching the hardcoded values'''
    ip = socket.gethostbyname(dest)
    for connection in connections:
        ips, ports, rtt, wscaleavg, cwnd, retrans, mss = parseconnection(connection)
        if ips[0] == '10.40.1.2' and ips[1] == ip and 4999 < ports[1] < 6000 and cwnd > 10:
            return ips, ports, rtt, wscaleavg, cwnd, retrans, mss
    return -1, -1, -1, -1, -1, -1, -1

def setfq(rate):
    '''set the fq pacing rate'''
    subprocess.check_call(['tc', 'qdisc', 'change', 'dev', 'eth4', 'root', 'fq', 'maxrate', '{0:.2f}Gbit'.format(rate)])
    return

def getBytes():
    out = subprocess.check_output(['ifconfig', 'eth4'])
    out = re.search(r'TX bytes:\d+', out)
    out = out.group()
    out = out[9:]
    out = int(out)
    return out

def main():
    #Parse input values
    parser = argparse.ArgumentParser()
    parser.add_argument('XI', type=float)
    parser.add_argument('PSI', type=float)
    parser.add_argument('DEST')
    parser.add_argument('TESTNO')
    parser.add_argument('--on', action='store_true')
    #parser.add_argument('RTT', type=float)
    args = parser.parse_args()
    XI = args.XI
    PSI = args.PSI
    dest = args.DEST
    add = args.TESTNO
    on = args.on
    #nominalrtt = args.RTT
    #Initialize values
    intervalNum = 0
    rate, controllerRate = -1, -1
    oldrtt = -1
    flowFound = False
    controller = Controller(PSI, XI, GAMMA, P, Q, ALPHA)
    #Set qdisc
    subprocess.check_call(['tc', 'qdisc', 'add', 'dev', 'eth4', 'root', 'fq'])
    #Start bwctl
    subprocess.Popen(['bwctl', '-c', dest, '-T', 'iperf3', '-i.1', '-w150m', '-t60', '--parsable', '-p'])
    #Initialize bytes for throughput count
    oldBytes = getBytes()
    #Initialize timedelta
    startTime = datetime.now()
    with tempfile.NamedTemporaryFile(suffix='.csv', delete=False) as output:
        writer = csv.writer(output)
        writer.writerow(['end','ertt', 'lHat', 'samplertt', 'controlRate', 'throughput', 'retransmits', 'cwnd', 'mss', 'txPort', 'rxPort'])
        for i in range(30000):
            time.sleep(.01)
            #Get throughput every 100ms
            if i%10 == 0:
                newBytes = getBytes()
                tput = ((newBytes - oldBytes) * 8) / float(1000)
            #Get flow stats every 10ms
            ssout = pollss()
            ips, ports, rtt, wscaleavg, cwnd, retrans, mss = findconn(ssout,dest)
            #When the flow is actually occurring
            if rtt != -1:
                flowFound = True
                #Inversion of RTT to sample RTT
                #First time seeing this flow
                if oldrtt == -1:
                    oldrtt = rtt
                    startTime = datetime.now()
                #elif nominalrtt<0 or rtt<nominalrtt: #stopped finding the lowest RTT
                #    nominalrtt = rtt
                delta = rtt-oldrtt
                samplertt = oldrtt + (delta * 8)
                #if samplertt < nominalrtt:
                    #samplertt = nominalrtt
                    #print('samplertt was less than nominal')
                    #if nominalrtt > 0:
                    #    samplertt = nominalrtt
                    #else:
                    #    samplertt = 1
                #Code for calling controller
                rate, lHat = controller.Process(rtt) 
                if on:
                    setfq(rate)
                end = startTime - datetime.now()
                writer.writerow([end, rtt, lHat, samplertt, rate, tput, retrans, cwnd, mss, ports[0], ports[1]])
            elif flowFound:
                break
            oldrtt = rtt
            oldBytes = newBytes
    controlStat = 'off'
    if on:
        controlStat = 'on'
    #Write out the temporary controller logic file
    shutil.copy2(output.name, 'XI-'+str(XI)+'-PSI-'+str(PSI)+'-'+dest+'-'+add+'-'+controlStat+'-controlOutput.csv')
    os.unlink(output.name)
    #Wait 10 seconds for bwctl output
    time.sleep(30)
    #Convert the bwctl json file to csv
    for basename in os.listdir('.'):
        if basename.endswith('.bw'):
            with open(basename) as fp:
                data = json.load(fp)
            newpath = 'XI-'+str(XI)+'-PSI-'+str(PSI)+'-'+dest+'-'+add+'-'+controlStat+'-iPerfOutput.csv'
            with open(newpath, 'wb') as ofp:
                writer = csv.writer(ofp)
                title = data['intervals'][0]['streams'][0].keys()
                if 'omitted' in title: title.remove('omitted')
                writer.writerow(title)
                for interval in data['intervals']:
                    del interval['streams'][0]['omitted']
                    vals = interval['streams'][0].values()
                    writer.writerow(vals)
    #Delete qdisc
    subprocess.check_call(['tc', 'qdisc', 'del', 'dev', 'eth4', 'root'])
    sys.exit()
if __name__ == '__main__':
    main()
