MATHPATH ?= /usr/local/Wolfram/Mathematica/11.3
MAPLE ?= /opt/maple2015
CXX := clang++

.PHONY : all
all: msc header
	${CXX} backend_sample.cpp -o backend_sample -I${MATHPATH}/SystemFiles/Links/WSTP/DeveloperKit/Linux-x86-64/CompilerAdditions -L${MATHPATH}/SystemFiles/Links/WSTP/DeveloperKit/Linux-x86-64/CompilerAdditions -I${MAPLE}/extern/include -L${MAPLE}/bin.X86_64_LINUX -lWSTP64i4 -lmaplec -lm -lpthread -lrt -lstdc++ -ldl -luuid -lc

.PHONY : msc
msc: msc.cpp
	$(CXX) -o msc msc.cpp

.PHONY : header
header: ../src/backend/mathematica/math_source/load_first/*.m ../src/backend/mathematica/math_source/*.m msc.cpp
	cat ../src/backend/mathematica/math_source/load_first/*.m ../src/backend/mathematica/math_source/*.m | \
		./msc backend_sample > \
		backend_sample.h

.PHONY : clean
clean:
	rm -f backend_sample msc backend_sample.h
