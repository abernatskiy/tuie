.SUFFIXES = .cpp .o
CC        = g++
CFLAGS    = -march=native -O2 -DdTRIMESH_ENABLED -DdDOUBLE -I "./ode-0.12/include"
CPPFLAGS  = ${CFLAGS}
LDFLAGS   = -march=native -O2 -lGLU -lGL -lm -lX11
#-lpthread 
objects		=	BackProp.o envs.o lightSensor.o matrix.o object.o propSensor.o tau.o touchSensor.o environment.o joint.o M3.o neuralNetwork.o optimizer.o robot.o tauOptimizer.o userModel.o interactor.o client.o server.o taus.o rankings/rankings.o rankings/arrays.o
libs			= ./ode-0.12/drawstuff/src/.libs/libdrawstuff.a ./ode-0.12/ode/src/.libs/libode.a 

.cpp.o:
	${CC} -o $@ -c ${CPPFLAGS} $<

.c.o:
	${CC} -o $@ -c ${CPPFLAGS} $<

M3: ${objects}
	${CC} ${LDFLAGS} -o $@ $^ ${libs}

switch-to-opposing:
	sed -i -e 's/return\ .*pos0/return (-1)\*pos0/' robot.cpp

switch-to-normal:
	sed -i -e 's/return\ .*pos0/return pos0/' robot.cpp

M3-opposing: switch-to-opposing M3 switch-to-normal
	mv M3 M3-opposing

clean:
	${RM} ${objects} M3

all: M3
