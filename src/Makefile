DEPS=-Wall -Wextra -march=native

ifeq ($(D),1)
OPTIMIZE=-O0
else
OPTIMIZE=-O2
endif

all: processmon processget

processmon:
	gcc -g ${OPTIMIZE} ${DEPS} processmon.c -o processmon

processget:
	gcc -g ${OPTIMIZE} ${DEPS} processget.c -o processget

clean:
	rm -f processmon processget
