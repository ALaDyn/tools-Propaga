COMPILER = g++
OPT = -O3 -std=c++0x

dataproc_exe = dataproc/dataproc
converter_exe = converter/converter

SRC1 = dataproc/dataproc.cpp
SRC2 = converter/converter.cpp


debug : OPT = -O0 -g -std=c++11 
debug : all

all : dataproc converter

dataproc : $(SRC1)
	$(COMPILER) $(OPT) $(SRC1) -o $(dataproc_exe)

converter : $(SRC2)
	$(COMPILER) $(OPT) $(SRC2) -o $(converter_exe)

clean :
	rm -f $(dataproc_exe) $(converter_exe) 
