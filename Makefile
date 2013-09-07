.SUFFIXES = .cpp .o
CC        = g++
CFLAGS    = -DdTRIMESH_ENABLED -DdDOUBLE -I "./ode-0.12/include"
CPPFLAGS  = ${CFLAGS}
LDFLAGS   = -g -O2 -lGLU -lGL -lm -lpthread 
objects		=	BackProp.o envs.o lightSensor.o matrix.o object.o propSensor.o tau.o touchSensor.o environment.o joint.o M3.o neuralNetwork.o optimizer.o robot.o tauOptimizer.o userModel.o
libs			= ./ode-0.12/drawstuff/src/.libs/libdrawstuff.a ./ode-0.12/ode/src/.libs/libode.a 

.cpp.o:
	${CC} -c ${CPPFLAGS} $<

M3: ${objects}
	${CC} ${LDFLAGS} -o $@ $^ ${libs}

clean:
	${RM} *.o M3

all: M3
