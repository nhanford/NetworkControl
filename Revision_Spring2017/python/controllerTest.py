#!/usr/bin/python
'''
livetest.py -- Applies control to FQ-CoDel pacing qdisc.
livetest is designed for any Linux distribution with tc_fq available in 
its iproute2 package

@author: Nathan Hanford
@contact: nhanford@es.net
@deffield: updated: Updated
'''
import sys,os,re,subprocess,socket,sched,time,datetime,threading,struct,argparse,json,logging,warnings,csv,random,tempfile,shutil
from controller import Controller

# Adaptive filter parameters.
ALPHA = 0.5
BETA = 0.5
P = 5
Q = 1

# Controller parameters.
PSI = 0.0
XI = 0.5
GAMMA = 0.5

# Latency generator parameters.
L_MAX = 1.0
L_SLOPE = 0.01
L_CUT = 0.1
R_COEFF = -0.02
NOISE_SD = 0.0

def pollss():
    '''gets data from ss'''
    out = subprocess.check_output(['ss','-i','-t','-n'])
    out = re.sub('\A.+\n','',out)
    out = re.sub('\n\t','',out)
    out = out.splitlines()
    return out

def parseconnection(connection):
    '''parses a string representing a single TCP connection'''
    #Junk gets filtered in @loadconnections
    try:
        connection = connection.strip()
        ordered = re.sub(':|,|/|Mbps',' ',connection)
        ordered = connection.split()
        ips = re.findall('\d+\.\d+\.\d+\.\d+',connection)
        ports = re.findall('\d:\w+',connection)
        rtt = re.search('rtt:\d+[.]?\d+',connection)
        wscaleavg = re.search('wscale:\d+',connection)
        mss = re.search('mss:\d+',connection)
        cwnd = re.search('cwnd:\d+',connection)
        retrans = re.search('retrans:\d+\/\d+',connection)
    except Exception as e:
        logging.warning('connection {} could not be parsed'.format(connection))
        return -1,-1,-1,-1,-1,-1,-1
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
        retrans = re.sub('retrans:\d+\/','',retrans)
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
    return -1,-1,-1,-1,-1,-1,-1

def findconn(connections):
    for connection in connections:
        ips, ports, rtt, wscaleavg, cwnd, retrans, mss = parseconnection(connection)
        if ips[0]=='10.2.2.2' and ips[1]=='198.129.254.14' and 4999<ports[1]<6000 and cwnd>10:
            return ips,ports,rtt,wscaleavg,cwnd,retrans,mss
    return -1,-1,-1,-1,-1,-1,-1

def setfq(rate):
    subprocess.check_call(['tc','qdisc','change','dev','eth4','root','fq','maxrate','{0:.2f}Gbit'.format(rate)])
    return

def getBytes():
    out = subprocess.check_output(['ifconfig','eth3'])
    out = re.search('TX bytes:\d+',out)
    out = out.group()
    out = out[9:]
    out = int(out)
    return out

def main():
    #Initialize values
    intervalNum = 0
    nominalrtt = -1
    rate,controllerRate = -1,-1
    oldrtt = -1
    flowFound = False
    controller = Controller(PSI, XI, GAMMA, P, Q, ALPHA, BETA)
    #Set qdisc
    subprocess.check_call(['tc','qdisc','add','dev','eth4','root','fq'])
    #Start bwctl
    subprocess.Popen(['bwctl','-c','denv-pt1.es.net','-T','iperf3','-t60','--parsable','-p'])
    #Initialize bytes for throughput count
    oldBytes = getBytes()
    with tempfile.NamedTemporaryFile(suffix='.csv',delete=False) as output:
        writer = csv.writer(output)
        writer.writerow(['ertt','samplertt','controlRate','setRate','throughput','retransmits','cwnd','mss'])
        for i in range(20000):
            time.sleep(.01)
            #Get throughput every 100ms
            if i%10 == 0:
                newBytes = getBytes()
                tput = ((newBytes - oldBytes) * 8) / float(1000)
            #Get flow stats evkery 10ms
            ssout = pollss()
            ips, ports, rtt, wscaleavg, cwnd, retrans, mss = findconn(ssout)
            #When the flow is actually occurring
            if rtt > 0:
                flowFound = True
                #Inversion of RTT to sample RTT
                if oldrtt == -1:
                    oldrtt = rtt
                elif nominalrtt<0 or rtt<nominalrtt:
                    nominalrtt = rtt
                delta = rtt-oldrtt
                samplertt = oldrtt + (delta * 8)
                if samplertt < 0:
                    if nominalrtt > 0:
                        samplertt = nominalrtt
                    else:
                        samplertt = 1
                #Code for calling controller
                rate = controller.Process(samplertt,rate)
                setfq(rate)
                writer.writerow([rtt,samplertt,rate,tput,retrans,cwnd,mss])
            elif flowFound == True:
                break
            oldrtt = rtt
            oldBytes = newBytes
    #
    shutil.copy(output.name, 'output.csv')
    os.unlink(output.name)
    #Delete qdisc
    subprocess.check_call(['tc','qdisc','del','dev','eth4','root'])
if __name__ =='__main__':
    main()
