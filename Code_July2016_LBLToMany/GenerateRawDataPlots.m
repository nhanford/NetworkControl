%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Generates plots of raw data (Tput/RTT active/passive FQ).
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

close all; clear all; clc;

NTrials = 4;


%FQ is ON.

FQON = 1; Fig_Tput = 1; Fig_RTT = 2; Fig_CongW = 3;
ViewData(FQON, NTrials, Fig_Tput, Fig_RTT, Fig_CongW);




% %FQ is OFF.
% 
% FQON = 0; Fig_Tput = 3; Fig_RTT = 4;
% ViewData(FQON, NTrials, Fig_Tput, Fig_RTT);















