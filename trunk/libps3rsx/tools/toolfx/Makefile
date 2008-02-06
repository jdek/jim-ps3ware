CPPFLAGS= -Wall -O3 -lxml2 -I/usr/include/libxml2

all:toolfx


toolfx: toolfx.cpp
	$(CXX) $(CPPFLAGS) toolfx.cpp -o toolfx


clean:
	$(RM) *.o $(OBJSQ) toolfx
