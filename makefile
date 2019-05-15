CPP = g++
CPPFLAGS = -g -std=c++17

SRCS = test.cpp PathBuilder.cpp

LIB= -L"SFML-2.5.1/lib" -lsfml-graphics -lsfml-window -lsfml-system
INC= -I"SFML-2.5.1/include"

OBJS = $(subst .cpp,.o,$(SRCS))

test: $(OBJS)
	$(CPP) -o test $(OBJS) $(LIB)

udp-img: udp-img.o PathBuilder.o
	$(CPP) -o udp-img udp-img.o PathBuilder.o $(INC) $(LIB) -lws2_32

# depend: .depend

# .depend: $(SRCS)
# 	rm -f ./.depend
# 	$(CPP) $(CPPFLAGS) $(INC) -MM $^>>./.depend

test.o: test.cpp
	$(CPP) $(CPPFLAGS) -c test.cpp -o test.o $(INC)
PathBuilder.o: PathBuilder.cpp
	$(CPP) $(CPPFLAGS) -c PathBuilder.cpp -o PathBuilder.o $(INC)

udp-img.o: udp-img.cpp
	$(CPP) $(CPPFLAGS) -c udp-img.cpp -o udp-img.o $(INC)

clean:
	rm -f $(OBJS) udp-img.o

# include .depend