%% Test script for importing raw data - Test passed 4/27

filename = 'TCP_FQ_data\File1_Berkeley-10Gbps-to-Amsterdam-10Gbps_CentOS7_FQ.csv';

[Time_Sec, RTT_MicroSec, Thruput_BitsPerSec, DataTransmitted_Bytes, Retransmits_Packets, CongWindow_Bytes] = TagRawData(filename);

if (Time_Sec(11) == 11.0001 && Time_Sec(60) == 60.0001), display('Time most likely correct');
else display('Time NOT correct');
end

if (RTT_MicroSec(3) == 144793 && RTT_MicroSec(19) == 161915), display('RTT most likely correct');
else display('RTT NOT correct');
end

if (Thruput_BitsPerSec(36) == 9889200000 && Thruput_BitsPerSec(59) == 9888030000),  display('Throughput most likely correct');
else display('Throughput NOT correct');
end

if (DataTransmitted_Bytes(5) == 1236008960 && DataTransmitted_Bytes(48) == 1236008960 && DataTransmitted_Bytes(56) == 1237319680), display('Data transmitted most likely correct');
else display('Data transmitted NOT correct');
end

if (Retransmits_Packets(4) == 0 && Retransmits_Packets(18) == 0), display('Retransmits most likely correct');
else display('Retransmits NOT correct');
end

if (CongWindow_Bytes(60) == 409424688 && CongWindow_Bytes(23) == 405684424), display('Congestion window size most likely correct');
else display('Congestion window size NOT correct');
end
