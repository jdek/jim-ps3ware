CPPFLAGS=-O3 -I ../../vendors/squish/

SQ=../../vendors/squish/

SQUISH=$(SQ)squish.cpp $(SQ)alpha.cpp $(SQ)clusterfit.cpp $(SQ)colourblock.cpp $(SQ)colourfit.cpp $(SQ)colourset.cpp $(SQ)maths.cpp $(SQ)rangefit.cpp $(SQ)singlecolourfit.cpp

OBJSQ = $(SQUISH:%.cpp=%.o)

all:tooldxt

objc: $(SQUISH)
	$(CXX) $(CPPFLAGS) $(SQUISH) -c 


tooldxt: $(OBJSQ) tooldxt.cpp
	$(CXX) $(CPPFLAGS) $(OBJSQ) tooldxt.cpp -o tooldxt


clean:
	$(RM) *.o $(OBJSQ) tooldxt
