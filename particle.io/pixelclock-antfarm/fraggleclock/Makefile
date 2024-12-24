photon:
	particle compile photon

photon2:
	particle compile photon2

clean:
	rm -f photon*bin

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

austin: roberta
lr_bookcase: rachel
drew: ronaldo
jake: ringo
max: rowan
brewster: rico
leslie: steel_hamster


SQUARES = rosalyn roberta rowan ricky rico steel_hamster
WIDESCREENS = ronaldo rachel ringo river

$(SQUARES) : square
	particle flash $@_raccoon

$(WIDESCREENS) : widescreen
	particle flash $@_raccoon
