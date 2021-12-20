%%
clear all; clc; close all;
%%
rng(5); % random nr seed
N = 16; % nr of delay lines

% open up the connection
u = udp('127.0.0.1', 6448);
fopen(u);
path = '/juce';

% make random gain values
bGains = -1 + (1- (-1)).*rand(1,N);
cGains = -1 + (1- (-1)).*rand(1,N);

% make random matrix values and reshape matrix
matrix = randomOrthogonal(N);
matrix = reshape(matrix,1,[]);

% Low and high limits of delay
delLowLim = 5;
delHighLim = 15;
delays = round(delLowLim + (delHighLim - delLowLim).*rand(1,N)); % milliseconds

dryWet = 92.5; % percentage

% Decay time
highT60 = 3; % seconds
lowT60 = 4.2; % seconds

% Cutoff Frequency
lowTransFreq = 1000; % Hz
highTransFreq = 3000; %Hz

% Modulation values
modRateWhole = 0.3 + (2 - 0.3).*rand(1,N); % Hz
modRate = 0.2;

modDepthWole = 0.3 + (6 - 0.3).*rand(1,N);
modDepth = 4.4;

% Index for single value updates
index = 3;

% Index for single matrix-value update
idx = [0,0];

% send b gains as whole
for i = 1:length(bGains)
    oscsend(u, path, 'sf', 'bGainWhole', bGains(i));
end

% send single b gain
oscsend(u, path, 'sif', 'bGainSingle', 2, 1.5);

% send c gains as whole
for i = 1:length(cGains)
    oscsend(u, path, 'sf', 'cGainWhole', cGains(i));
end

% send single c gain
oscsend(u, path, 'sif', 'cGainSingle', 0, 1.5);

% send matrix as whole
for i = 1:length(matrix)
    oscsend(u, path, 'sf', 'matrixWhole', matrix(i));
end

% send single matrix value
oscsend(u, path, 'siif', 'matrixSingle', idx(1), idx(2), 0.3);

% send delayValues as whole
for i = 1:length(delays)
    oscsend(u, path, 'sf', 'delayWhole', delays(i));
end
 
% send single delayValue
oscsend(u, path, 'sii', 'delaySingle', 0, 50);

% send dryWet value
oscsend(u, path, 'sf', 'dryWet', dryWet);

% send lowT60 value
oscsend(u, path, 'sf', 'lowT60', lowT60);

% send highT60 value
oscsend(u, path, 'sf', 'highT60', highT60);

% send trans freq value
oscsend(u, path, 'sf', 'lowTransFreq', lowTransFreq);

oscsend(u, path, 'sf', 'highTransFreq', highTransFreq);

% send modulation rate value
oscsend(u, path, 'sif', 'modRateSingle', index, modRate);

% send modulation rate as whole
for i = 1:length(modRateWhole)
    oscsend(u,path, 'sf', 'modRateWhole', modRateWhole(i))
end

% send modulation depth value
oscsend(u, path, 'sif', 'modDepthSingle', 3, modDepth);

% send modulation depth as whole
for i = 1:length(modDepthWole)
    oscsend(u, path, 'sf', 'modDepthWhole', modDepthWole(i));
end

% Turn modulation on/off
oscsend(u,path,'ss', 'modulation', 'off');

fclose(u); % close the connection

