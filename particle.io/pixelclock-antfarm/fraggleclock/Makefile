PARTICLE_CLI=/Users/austin/bin/particle

photon:
	${PARTICLE_CLI} compile photon

photon2:
	${PARTICLE_CLI} compile photon2

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
	${PARTICLE_CLI} flash --local

pause:
	sleep 5

follow:
	${PARTICLE_CLI} serial monitor --follow

roxie_raccoon: square d1
	${PARTICLE_CLI} flash $@

rachel_raccoon: widescreen mega
	${PARTICLE_CLI} flash $@

renee_raccoon: widescreen mega
	${PARTICLE_CLI} flash $@

ranier_raccoon: widescreen mega
	${PARTICLE_CLI} flash $@

remus_raccoon: square offline
	${PARTICLE_CLI} flash $@

rosalyn_raccoon: square mega
	${PARTICLE_CLI} flash $@

roberta_raccoon: square mega
ricky_raccoon: square mega
	${PARTICLE_CLI} flash $@
	${PARTICLE_CLI} flash $@

steel_hamster: square
	${PARTICLE_CLI} flash $@

austin: roberta_raccoon
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
	${PARTICLE_CLI} flash $@_raccoon

$(WIDESCREENS) : widescreen
	${PARTICLE_CLI} flash $@_raccoon

uptime:
	@for host in rachel_raccoon ricky_raccoon; \
	do \
          echo -n "$$host: uptime = "; \
	  ${PARTICLE_CLI} get $$host uptime_h; \
	  ${PARTICLE_CLI} get $$host statistics; \
	done
