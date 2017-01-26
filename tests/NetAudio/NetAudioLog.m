%% Einlesen der Logs
NetAudioLogNet = readtable( 'NetAudioLogNet.txt' );
NetAudioLogStream = readtable( 'NetAudioLogStream.txt' );
NetAudioLogClient = readtable( 'NetAudioLogClient.txt' );

%% Plot Freie Smaples und Bufferstutus
minTime = min(NetAudioLogStream.WorldTimeStamp(1),NetAudioLogNet.WorldTimeStamp(1));
TimeNet = NetAudioLogNet.WorldTimeStamp - minTime;
TimeStream = NetAudioLogStream.WorldTimeStamp - minTime;
BlockLenght = NetAudioLogNet.NumSamplesTransmitted(1);
BufferSize = 100 * BlockLenght;
Data = [NetAudioLogNet.WorldTimeStamp NetAudioLogNet.FreeSamples; NetAudioLogStream.WorldTimeStamp NetAudioLogStream.FreeSamples];
Data = sortrows(Data,1 );
Data(:,1) = Data(:,1) - minTime();
Data(:,2) = (Data(:,2)) / BlockLenght;
subplot(2,2,1:2)
plot(Data(:,1), Data(:,2))
title('Freie Blöcke im Ring Buffer')
xlabel('Zeit in s')
ylabel('Anzahl der Blöcke')

%% Plot Latenz
subplot(2,2,3:4)
DiffTime = diff(NetAudioLogClient.WorldTimeStamp * 1000);
median = mean(DiffTime(:,1));
medianVec = zeros(size(DiffTime(:,1))); 
medianVec = medianVec + median;
DiffTime = [DiffTime NetAudioLogClient.ProtocolStatus(2:end)];
DiffTime = sortrows(DiffTime,2);
LatenzWaiting = DiffTime(find(DiffTime(:,2)<222),1);
LatenzRunnning = DiffTime(find(DiffTime(:,2)>221),1);
medianRunning = mean(LatenzRunnning(:,1));
medianRunningVec = zeros(size(LatenzRunnning(:,1))); 
medianRunningVec = medianRunningVec + medianRunning;
plot( LatenzRunnning, 'b')
hold on
plot( medianRunningVec, 'r')
title('Latenz pro Block')
legend('Latenz', 'Durchschnittliche Latenz')
xlabel('Block Nummer')
ylabel('Zeit in ms')
legend('show') 
