CFLAGS=-O2 -W -Wall -I/lib/modules/`uname -r`/build/include
CPPFLAGS=-O2 -W -Wall -I/lib/modules/`uname -r`/build/include
SRCCXX=../../src/textures/textures.cpp 
SRCC=../../src/fifo/utils.c ps3gpu.c
SQ=../../vendors/squish/
SQUISH=$(SQ)squish.cpp $(SQ)alpha.cpp $(SQ)clusterfit.cpp $(SQ)colourblock.cpp $(SQ)colourfit.cpp $(SQ)colourset.cpp $(SQ)maths.cpp $(SQ)rangefit.cpp $(SQ)singlecolourfit.cpp

OBJSQ = $(SQUISH:%.cpp=%.o)
OBJCXX = $(SRCCXX:%.cpp=%.o)
OBJC = $(SRCC:%.c=%.o)

all:gpu

objcxx: $(SRCCXX)
	$(CXX) $(CPPFLAGS) $(SRCXX) -c

objc: $(SRCC)
	$(CC) $(CFLAGS) $(SRCC) -c

objc: $(SQUISH)
	$(CXX) $(CFLAGS) $(SQUISH) -c
	$(AR) crs squish.a $(OBJSQ)

gpu: objc $(OBJC) $(OBJCXX)
	$(CXX) squish.a -lm -o ps3gpu $(OBJC) $(OBJCXX) 


clean:
	$(RM) $(OBJC) $(OBJCXX) $(OBJSQ)
