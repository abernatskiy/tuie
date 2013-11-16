.SUFFIXES = .cpp .o
CC        = g++
CFLAGS    = -g -ggdb -Wall
# -DdTRIMESH_ENABLED -DdDOUBLE -I "./ode-0.12/include"
CPPFLAGS  = ${CFLAGS}
LDFLAGS   = -g -ggdb -Wall
# -lGLU -lGL -lm 
objects		=	main.o rankings.o arrays.o

#.cpp.o:
#	${CC} -c ${CPPFLAGS} $<

test: ${objects}
	${CC} ${LDFLAGS} -o $@ $^

clean:
	${RM} *.o test

all: test
