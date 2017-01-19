NetAudioLogNet = readtable( 'NetAudioLogNet.txt' );
NetAudioLogStream = readtable( 'NetAudioLogStream.txt' );

plot( NetAudioLogNet.WorldTimeStamp, NetAudioLogNet.FreeSamples/NetAudioLogNet.NumSamplesTransmitted )
hold on
plot( NetAudioLogStream.WorldTimeStamp, NetAudioLogStream.FreeSamples/NetAudioLogNet.NumSamplesTransmitted )