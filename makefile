# Template C++ makefile
# 
# Daniel Andrus (used with permission from author)
# Version 1.0


## This is where a target gets it's source files
SOURCE_FILES_dicp = DIPC.cpp
SOURCE_FILES_dicprm = dipcrm.c


CPP_COMPILER=g++
CPP_LINKER=g++
CPP_COMPILE_FLAGS=-std=c++11 -pthread
CPP_LINK_FLAGS=-std=c++11 -pthread

# NOTICE build directory must be set in object targets

BUILD_DIR=.
SOURCE_DIR=.



# EDIT set custom program name
all: dipc dipcrm

#Target is the program name
dipc: $(addprefix $(BUILD_DIR)/, $(SOURCE_FILES_dicp:.cpp=.o))
	$(CPP_LINKER) $(CPP_LINK_FLAGS) -o $@ $^

dipcrm: $(addprefix $(BUILD_DIR)/, $(SOURCE_FILES_dicprm:.cpp=.o))
	$(CPP_LINKER) $(CPP_LINK_FLAGS) -o $@ $^


# EDIT custom build directory

#first part of next line is the build directory
./%.o: $(addprefix $(SOURCE_DIR)/, %.cpp)
	@mkdir -p $(BUILD_DIR)
	$(CPP_COMPILER) $(CPP_COMPILE_FLAGS) -c -o $@ $<

# ./%.o: $(addprefix $(SOURCE_DIR)/, %.cpp)
# 	@mkdir -p $(BUILD_DIR)
# 	$(CPP_COMPILER) $(CPP_COMPILE_FLAGS) -c -o $@ $<

clean:
	@rm -f $(addprefix $(BUILD_DIR)/, $(dipc:.cpp=.o))
	@rm -f dipc
	@rm -f dipcrm
    
    
# g++ -pthread -o dipc DIPC.cpp -std=c++11
# g++ -pthread -o dipcrm dipcrm.c