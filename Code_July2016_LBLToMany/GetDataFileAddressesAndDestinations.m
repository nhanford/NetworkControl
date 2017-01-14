%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Outputs data file addresses from 4 trials (nominal throughput = 10Gbps, either FQ ON or OFF, not both), and destinations.
%Col 1 = Time (sec)
%Col 3 = RTT (microsec)
%Col 5 = Throughput (bits per sec)
%Col 6 = Data transmitted (bytes)
%Col 8 = Retransmits (packets)
%Col 9 = Congestion window size (bytes)

%FQON == 1 -> TCP + FQ (FQ switches between 'active=is limiting sending rate' and 'passive=NOT limiting sending rate'). 
%FQOFF == 0 -> TCP only.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [DataFileAddresses, Destinations] = GetDataFileAddressesAndDestinations(FQON)

if (FQON == 1), folder = 'TCP_FQON_data\'; FQtype = '_ON_'; endstr = '_FQ';
else folder = 'TCP_FQOFF_data\'; FQtype = '_OFF_'; endstr = '';
end

NTrials = 4; NDestinations = 8; Files = cell(NDestinations,1);
for trial = 1:NTrials
        
    begstr = strcat(folder, 'Trial', num2str(trial),'\', 'Trial', num2str(trial), FQtype);

    Files{1} = strcat(begstr, 'Berkeley-10Gbps-to-Amsterdam-10Gbps_CentOS7',endstr,'.csv');
    Files{2} = strcat(begstr, 'Berkeley-10Gbps-to-Ashburn-VA-US-1Gbps_CentOS7',endstr,'.csv');
    Files{3} = strcat(begstr, 'Berkeley-10Gbps-to-Bethesda-MD-US-10Gbps_CentOS7',endstr,'.csv');
    Files{4} = strcat(begstr, 'Berkeley-10Gbps-to-Boulder-CO-US-10Gbps_CentOS7',endstr,'.csv');
    Files{5} = strcat(begstr, 'Berkeley-10Gbps-to-Chicago-40Gbps_CentOS7',endstr,'.csv');
    Files{6} = strcat(begstr, 'Berkeley-10Gbps-to-East-Lansing-MI-US-10Gbps_CentOS7',endstr,'.csv');
    Files{7} = strcat(begstr, 'Berkeley-10Gbps-to-Pittsburgh-1Gbps_CentOS7',endstr,'.csv');
    Files{8} = strcat(begstr, 'Berkeley-10Gbps-to-UK-1Gbps_CentOS7', endstr, '.csv');

    if trial == 1, Files_Trial1 = Files;
    elseif trial == 2, Files_Trial2 = Files;
    elseif trial ==3, Files_Trial3 = Files;
    else Files_Trial4 = Files;
    end
end

DataFileAddresses = [Files_Trial1; Files_Trial2; Files_Trial3; Files_Trial4]; %Vertically concatenate all files into a single cell.

Destinations = cell(NDestinations,1);
Destinations{1} = 'AMS'; %Amsterdam
Destinations{2} = 'VA'; %Ashburn, VA
Destinations{3} = 'MD'; %Bethesda, MD
Destinations{4} = 'CO'; %Boulder, CO
Destinations{5} = 'IL'; %Chicago, IL
Destinations{6} = 'MI'; %East Lansing, MI
Destinations{7} = 'PA'; %Pittsburgh, PA
Destinations{8} = 'UK';







