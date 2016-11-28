ALSA_HW=1
SAMPLE_RATE=44100
BLOCK_SIZE=256
NUM_INPUTS=2
NUM_OUTPUTS=2


if [[ ! `pidof jackd` ]]; then
	echo "Starting JACK server..."
	jackd --realtime -P70 --sync -dalsa -dhw:${ALSA_HW},0 -r${SAMPLE_RATE} -p${BLOCK_SIZE} -n2 -i${NUM_INPUTS} -o${NUM_OUTPUTS} -s &
	sleep 5
	echo "Now starting test..."
fi

pidof jackd

bin/ITAJackLoopback
