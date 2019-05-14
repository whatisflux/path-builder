CPP = g++
CPPFLAGS = -g -std=c++17

SRCS = test.cpp PathBuilder.cpp

LIB= -L"SFML-2.5.1/lib" -lsfml-graphics -lsfml-window -lsfml-system
INC= -I"SFML-2.5.1/include"

OBJS = $(subst .cpp,.o,$(SRCS))

test: $(OBJS)
	$(CPP) -o test $(OBJS) $(LIB)

depend: .depend

.depend: $(SRCS)
	rm -f ./.depend
	$(CPP) $(CPPFLAGS) $(INC) -MM $^>>./.depend

clean:
	rm -f $(OBJS) udp-img.o

udp-img: udp-img.cpp
	$(CPP) -o udp-img udp-img.cpp $(INC) $(LIB) -lws2_32

include .depend