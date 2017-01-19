NetAudioLogNet = readtable( 'NetAudioLogNet.txt' );
NetAudioLogStream = readtable( 'NetAudioLogStream.txt' );

plot( NetAudioLogNet.WorldTimeStamp, [ NetAudioLogNet.NumSamplesTransmitted/1024 NetAudioLogNet.FreeSamples/1024 ] )
hold on
plot( NetAudioLogStream.WorldTimeStamp, NetAudioLogStream.FreeSamples/1024 )