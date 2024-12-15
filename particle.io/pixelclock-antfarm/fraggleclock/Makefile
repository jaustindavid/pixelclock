photon:
	particle compile photon

photon2:
	particle compile photon2


square:
	cp src/square-aspect.h src/aspect.h

widescreen:
	cp src/widescreen-aspect.h src/aspect.h

local:
	particle flash --local

pause:
	sleep 5

follow:
	particle serial monitor --follow

rachel_raccoon: widescreen
	particle flash $@

ricky_raccoon: square
	particle flash $@

roxie_raccoon: widescreen
	particle flash $@

leslie: steel_hamster
steel_hamster: square
	particle flash $@

pixelclock-dev: ronaldo_raccoon
ronaldo_raccoon:	square
	particle flash $@

pixelclock-mahogany: rowan_raccoon
rowan_raccoon: square
	particle flash $@
