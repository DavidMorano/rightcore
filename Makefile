# MAKEFILE


default:
	echo "we can only clean up stuff ; make what you want individually"

clean:
	makebelow clean

libdam:
	makebelow libdam32

libut:
	makebelow libut32

libpcs:
	makebelow libpcs32

libb:
	makebelow libb32

libuc:
	makebelow libuc32

libu:
	makebelow libu32



