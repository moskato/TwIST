#Makefile to compile the project

CXX = g++
CXXFLAGS = -g 
RM = rm -f

PROJECT = main.o
SOURCE = main.cpp twist.cpp AT.cpp Af.cpp diff.cpp hadamard.cpp non_zero.cpp phi_function.cpp prod_c_v.cpp psi_function.cpp sum.cpp vector_res.cpp vector_prod.cpp vector_sum.cpp
HEADER = functions.h
OBJS = $(subst .cc,.o,$(SOURCE)))

#*********************

all: $(PROJECT)
	
$(PROJECT):
	$(CXX) $(CXXFLAGS) $(HEADER) $(SOURCE) -o $(PROJECT)

clean: 
	$(RM) $(PROJECT) 
