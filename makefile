CPP = g++
CPPFLAGS = -g -std=c++17 -ID:/dev/opengl/SFML-2.5.1/include

SRCS = test.cpp CalculatePathPoints.cpp

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
	rm -f $(OBJS)

include .depend