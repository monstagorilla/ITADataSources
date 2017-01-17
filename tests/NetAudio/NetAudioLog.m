NetAudioLogNet = readtable( 'NetAudioLogNet.txt' );
NetAudioLogStream = readtable( 'NetAudioLogStream.txt' );

plot( NetAudioLogNet.WorldTimeStamp, [ NetAudioLogNet.NumSamplesTransmitted NetAudioLogNet.FreeSamples ] )
hold on
plot( NetAudioLogStream.WorldTimeStamp, NetAudioLogStream.FreeSamples )