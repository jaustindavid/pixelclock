photon:
	particle compile photon

photon2:
	particle compile photon2

square:
	cp src/square-aspect.h src/aspect.h

widescreen:
	cp src/widescreen-aspect.h src/aspect.h

d1:
	echo "#define PIXEL_PIN D1" >> src/aspect.h

local:
	particle flash --local

pause:
	sleep 5

follow:
	particle serial monitor --follow

roxie_raccoon: square d1
	particle flash $@

leslie: steel_hamster
steel_hamster: square
	particle flash $@

drew: ronaldo

SQUARES = rosalyn roberta rowan ricky rico
WIDESCREENS = ronaldo rachel ringo

$(SQUARES) : square
	particle flash $@_raccoon

$(WIDESCREENS) : widescreen
	particle flash $@_raccoon
