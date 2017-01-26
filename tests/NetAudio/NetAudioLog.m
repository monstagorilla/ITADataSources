%% Einlesen der Logs
NetAudioLogNet = readtable( 'NetAudioLogNet.txt' );
NetAudioLogStream = readtable( 'NetAudioLogStream.txt' );

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
title('Freie Bl�cke im Ring Buffer')
xlabel('Zeit in s')
ylabel('Anzahl der Bl�cke')

%% Plot Latenz
subplot(2,2,3:4)
DiffTime = diff(NetAudioLogNet.WorldTimeStamp * 1000);
median = mean(DiffTime);
medianVec = zeros(size(DiffTime)); 
medianVec = medianVec + median;
plot( DiffTime, 'b')
hold on
plot( medianVec, 'r')
title('Latenz pro Block')
legend('Latenz', 'Durchschnittliche Latenz')
xlabel('Block Nummer')
ylabel('Zeit in ms')
legend('show') 
