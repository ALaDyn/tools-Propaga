
FILES = scan-parameters.cpp

EXES = $(addsuffix .exe, $(basename $(FILES)))

OPT = -std=c++11 -I../ext/jsoncons/include -I../ext/utils


all: $(EXES)


%.exe: %.cpp
	$(CXX) $(OPT) -o $@ $<


clean:
	rm -f $(EXES) *~ .*~ 

cleanall: clean


