LIBS=
INCLUDE=
DEPS=-Wall -Wextra -march=native
SLIBS=

ifeq ($(D),1)
OPTIMIZE=-O0
else
OPTIMIZE=-O2
endif

SRC=processmon.c

all: processmon

processmon:
	gcc -g ${OPTIMIZE} ${DEPS} ${INCLUDE} ${LIBS} ${SRC} ${SLIBS} -o processmon

clean:
	rm -f processmon
