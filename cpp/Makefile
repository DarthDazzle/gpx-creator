all: gpx_creator

gpx_creator: example.hpp opendlv-standard-message-set.hpp gpx_creator.cpp
	g++ -std=c++14 -o gpx_creator -pthread -I. -g gpx_creator.cpp

example.hpp: example.odvd cluon-msc
	./cluon-msc --cpp --out=example.hpp example.odvd

opendlv-standard-message-set.hpp: opendlv-standard-message-set-v0.9.9.odvd cluon-msc
	./cluon-msc --cpp --out=opendlv-standard-message-set.hpp opendlv-standard-message-set-v0.9.9.odvd 

cluon-msc: cluon-complete.cpp
	g++ -DHAVE_CLUON_MSC -std=c++14 -pthread -o cluon-msc cluon-complete.cpp

cluon-complete.cpp: cluon-complete.hpp
	ln -sf cluon-complete.hpp cluon-complete.cpp

clean:
	rm -f example.hpp opendlv-standard-message-set.hpp cluon-complete.cpp printContentFromRecFile
	rm -f cluon-msc
