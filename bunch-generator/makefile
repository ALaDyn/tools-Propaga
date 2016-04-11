
FILES = BuGe.cpp
DEPFILES = NumberGenerator.cpp

EXES = $(addsuffix .exe, $(basename $(FILES)))
DEPS = $(addsuffix .o, $(basename $(DEPFILES)))

OPT = -std=c++11


all: $(DEPS)
all: $(EXES)


%.o: %.cpp
	$(CXX) $(OPT) -c -o $@ $<

%.exe: %.cpp
	$(CXX) $(OPT) -o $@ $(DEPS) $<


clean:
	rm -f $(EXES) *~ .*~ 

cleanall: clean


