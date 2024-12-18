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

rachel_raccoon: widescreen
	particle flash $@

rico_raccoon: square
	particle flash $@

ricky_raccoon: square
	particle flash $@

roxie_raccoon: square d1
	particle flash $@

rosalyn_raccoon: square
	particle flash $@

roberta_raccoon: square
	particle flash $@

leslie: steel_hamster
steel_hamster: square
	particle flash $@

ronaldo_raccoon:	square
	particle flash $@

rowan_raccoon: square
	particle flash $@

froge: rico_raccoon rowan_raccoon roxie_raccoon ricky_raccoon \
	ronaldo_raccoon rosalyn_raccoon roberta_raccoon
