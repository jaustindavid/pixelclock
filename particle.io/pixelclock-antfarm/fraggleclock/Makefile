photon:
	particle compile photon

photon2:
	particle compile photon2

follow:
	particle serial monitor --follow

square:
	cp src/square-aspect.h src/aspect.h

widescreen:
	cp src/widescreen-aspect.h src/aspect.h

local:
	particle flash --local

rachel_raccoon: widescreen
	particle flash $@

roxie_raccoon: widescreen
	particle flash $@

steel_hamster: square
	particle flash $@

pixelclock-dev: square
	particle flash $@

pixelclock-mahogany: square
	particle flash "pixelclock mahogany"
