%% Einlesen der Logs
close all;
clear all;
NetAudioLogNet = readtable( 'NetAudioLogNet.txt' );
NetAudioLogStream = readtable( 'NetAudioLogStream.txt' );
NetAudioLogClient = readtable( 'NetAudioLogClient.txt' );

minTime = min(NetAudioLogStream.WorldTimeStamp(1),NetAudioLogNet.WorldTimeStamp(1));
TimeNet = NetAudioLogNet.WorldTimeStamp - minTime;
TimeStream = NetAudioLogStream.WorldTimeStamp - minTime;
BlockLenght = NetAudioLogNet.NumSamplesTransmitted(1);

TimeState = [TimeStream NetAudioLogStream.StreamingStatus];
TimeUnderrun = TimeState(find(TimeState(:,2)==3), 1);
TimeOverrun = TimeState(find(TimeState(:,2)==4), 1);
TimeStream = TimeState(find(TimeState(:,2)==2), 1);

%% Plot Freie Smaples und Bufferstutus
Data = [NetAudioLogNet.WorldTimeStamp NetAudioLogNet.FreeSamples; NetAudioLogStream.WorldTimeStamp NetAudioLogStream.FreeSamples];
Data = sortrows(Data,1 );
Data(:,1) = Data(:,1) - minTime();
Data(:,2) = (Data(:,2)) / BlockLenght;
medianBlock = (max(Data(:,2)) - min(Data(:,2))) / 2;
subplot(2,2,1:2)
plot( Data(:,1), Data(:,2) )
hold on
plot( TimeUnderrun, zeros(size(TimeUnderrun)) + max(Data(:,2)),'r*')
plot( TimeOverrun, zeros(size(TimeOverrun)) + min(Data(:,2)),'r+')
title('Freie Blöcke im Ring Buffer')
xlabel('Zeit in s')
ylabel('Anzahl der Blöcke')

%% Plot Latenz
DiffTime = diff(NetAudioLogClient.WorldTimeStamp * 1000);
median = mean(DiffTime(:,1));
medianVec = zeros(size(DiffTime(:,1))); 
medianVec = medianVec + median;
DiffTime = [NetAudioLogClient.WorldTimeStamp(2:end) - minTime DiffTime NetAudioLogClient.ProtocolStatus(2:end)];
DiffTime = sortrows(DiffTime,3);
LatenzWaiting = DiffTime(find(DiffTime(:,3)<222), (1:2));
LatenzRunnning = DiffTime(find(DiffTime(:,3)>221), (1:2));
medianRunning = mean(LatenzRunnning(:,2));
medianRunningVec = zeros(size(LatenzRunnning(:,1))); 
medianRunningVec = medianRunningVec + medianRunning;
sollLatenz = (BlockLenght / 44100) * 1000;
sollLatenzVec = zeros(size(LatenzRunnning(:,1))); 
sollLatenzVec = sollLatenzVec + sollLatenz

subplot(2,2,3:4)
plot( LatenzRunnning(:,1), LatenzRunnning(:,2))
hold on
plot( LatenzRunnning(:,1), medianRunningVec, 'r')
plot( LatenzRunnning(:,1), sollLatenzVec, 'g')
plot( TimeUnderrun, zeros(size(TimeUnderrun)) + sollLatenz,'r*')
plot( TimeOverrun, zeros(size(TimeOverrun)) + medianRunning,'r*')
title('Latenz pro Block')
legend('Latenz', 'Durchschnittliche Latenz', 'SollLatenz', 'Underruns', 'Overruns')
xlabel('Zeit in ms')
ylabel('Latenz in ms')
legend('show') 
