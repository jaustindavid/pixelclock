photon:
	particle compile photon

photon2:
	particle compile photon2

clean:
	rm -f photon*bin

square:
	cp src/square-aspect.h src/local-settings.h

widescreen:
	cp src/widescreen-aspect.h src/local-settings.h

d1:
	echo "#define PIXEL_PIN D1" >> src/local-settings.h

offline:
	echo "#define OFFLINE_MODE true" >> src/local-settings.h

mega:
	echo "#define MEGA true" >> src/local-settings.h

test:
	echo "#define TESTING" >> src/local-settings.h

local:
	particle flash --local

pause:
	sleep 5

follow:
	particle serial monitor --follow

roxie_raccoon: square d1
	particle flash $@

rachel_raccoon: widescreen mega
	particle flash $@

renee_raccoon: widescreen mega
	particle flash $@

ranier_raccoon: widescreen mega
	particle flash $@

remus_raccoon: square offline
	particle flash $@

rosalyn_raccoon: square mega
	particle flash $@

roberta_raccoon: square mega
	particle flash $@

steel_hamster: square
	particle flash $@

austin: roberta
lr_bookcase: rachel
drew: ronaldo
jake: ringo
max: rowan
dan_singer: rosalyn
brewster: rico
leslie: steel_hamster renee
gjab: river


SQUARES = rosalyn roberta rowan ricky rico remus
WIDESCREENS = ronaldo rachel ringo river renee

$(SQUARES) : square
	particle flash $@_raccoon

$(WIDESCREENS) : widescreen
	particle flash $@_raccoon

uptime:
	@for host in rachel_raccoon ricky_raccoon; \
	do \
          echo -n "$$host: uptime = "; \
	  particle get $$host uptime_h; \
	  particle get $$host statistics; \
	done
