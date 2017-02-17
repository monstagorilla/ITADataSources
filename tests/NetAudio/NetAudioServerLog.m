%% Einlesen der Logs
close all;
clear all;
BlockSize = '1024';
NetAudioLogServer = dir(['NetAudioLogServer_BS' BlockSize '*.txt']);
NetAudioLogServer = {NetAudioLogServer.name};
NetAudioLogServerTab = readtable(NetAudioLogServer{1}, 'FileType', 'text', 'Delimiter', '\t');
minTime = NetAudioLogServerTab.WorldTimeStamp(1);
maxTime = NetAudioLogServerTab.WorldTimeStamp(end);
channel = NetAudioLogServer{1}(28:end-4);

for k=2:numel(NetAudioLogServer) 
    temp = readtable(NetAudioLogServer{k}, 'FileType', 'text', 'Delimiter', '\t');
    NetAudioLogServerTab = [NetAudioLogServerTab; temp];
    minTime = min(minTime, temp.WorldTimeStamp(1));
    maxTime = max(maxTime, temp.WorldTimeStamp(end));
    channel = [channel; NetAudioLogServer{k}(28:end-4)];
end
NetAudioLogServerTab.WorldTimeStamp = NetAudioLogServerTab.WorldTimeStamp - minTime;

%% Protocolstatus ersetzten
hallo = [1 2; 3 4 ];
Protocol = {'100', 'NP_CLIENT_OPEN';...
    '101', 'NP_CLIENT_CLOSE';...
    '111', 'NP_CLIENT_SENDING_RINGBUFFER_FREE_SAMPLES';...
    '200', 'NP_SERVER_OPEN';...
    '201', 'NP_SERVER_CLOSE';...
    '211', 'NP_SERVER_GET_RINGBUFFER_FREE_SAMPLES';...
    '222', 'NP_SERVER_SENDING_SAMPLES'};
%NumPro = zeros(size(NetAudioLogServerTab.ProtocolStatus));
for k = (1:size(Protocol,1))
%NumPro( NetAudioLogServerTab.ProtocolStatus == 211) = 'NP_SERVER_GET_RINGBUFFER_FREE_SAMPLES';
end


%% Plot Protocol
plot(NetAudioLogServerTab.WorldTimeStamp, NetAudioLogServerTab.ProtocolStatus)
hold on;
plot(NetAudioLogServerTab.WorldTimeStamp, NetAudioLogServerTab.FreeSamples)
