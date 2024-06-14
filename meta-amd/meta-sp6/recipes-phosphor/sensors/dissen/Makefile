SRC = dissen-monitor.cpp
OUTPUT=dissen-monitor
CFLAGS+= -Wno-unused-result -std=c++17
LIB = -lsdbusplus -lsystemd -lpthread -lgpiodcxx
all: $(SRC)
	$(CXX) $(SRC) -o  $(OUTPUT) $(CFLAGS) $(LIB) 
clean:
	-rm -rf $(OUTPUT)