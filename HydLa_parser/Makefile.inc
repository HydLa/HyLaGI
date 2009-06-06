TARGET	 := ../hydla
INCLUDES += -I. -I/usr/local/Wolfram/Mathematica/6.0/SystemFiles/Links/MathLink/DeveloperKit/Linux/CompilerAdditions/
LDFLAGS	 += -L/usr/local/Wolfram/Mathematica/6.0/SystemFiles/Links/MathLink/DeveloperKit/Linux/CompilerAdditions/
LIBS	 += -lML32i3 -lm -lrt -lboost_program_options # -lboost_thread -lboost_date_time -lpthread 
DEFINES  += 
C_OPT    += # -pthread #-O3 -g


# INCLUDES += -I/home/hirose/boost/include
# LDFLAGS	 += -L/home/hirose/boost/lib
