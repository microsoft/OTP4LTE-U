CXX        = g++
DEBUGFLAGS = -g -w  #-DTEST 
CXXFLAGS   = $(DEBUGFLAGS) $(PARAFLAGS) 
CXXFLAGS += --std=c++11
LIBS = -lpcap -ljsoncpp -lpthread 

wifilteu: main.o wifi_packet.o wifi_node.o monitor.o udp_socket.o csat.o
	$(CXX) $(CXXFLAGS) $^ -o wifilteu $(LIBS)

%.o: ./src/%.cc ./src/%.h
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -rf wifilteu *.o *~ 


