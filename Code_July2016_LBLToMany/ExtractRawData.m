%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Extracts TCP data from .csv file. Input = data file address.

%Col 1 = time (s)
%Col 3 = l = latency* (mus)
%Col 5 = t = throughput (bps)
%Col 6 = d = data transmitted (B)
%Col 8 = r = retransmits (packets)
%Col 9 = c = sender congestion window size ('snd_cwnd') (B)

%*measured via rtt
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%UNITS
%B = bytes
%bps = bits per second
%cs = centiseconds (100th of a second)
%Gb = gigabits
%mus = microseconds
%s = seconds
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [time_s, l_mus, t_bps, d_B, r_packets, c_B] = ExtractRawData(DataFileAddress)

RowOffset = 1; %Start reading at row #2 due to header in row #1.
ColOffset = 0; %Start reading at col #1.

RawData = csvread(DataFileAddress,RowOffset,ColOffset);

time_s = RawData(:,1); 

l_mus = RawData(:,3); 

t_bps = RawData(:,5); 

d_B = RawData(:,6); 

r_packets = RawData(:,8); 

c_B = RawData(:,9);
