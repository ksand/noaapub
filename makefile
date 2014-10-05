all: weather.o parse.o mkarray.o windfrom.o station.o utctime.o disperr.o charpos.o
	cc -o weather weather.o parse.o mkarray.o windfrom.o station.o utctime.o disperr.o charpos.o

weather.o: weather.c
	cc -c -Wunused-variable weather.c

parse.o: parse.c
	cc -c -Wunused-variable parse.c

mkarray.o: mkarray.c
	cc -c mkarray.c

windfrom.o: windfrom.c
	cc -c windfrom.c

station.o: station.c
	cc -c station.c

utctime.o: utctime.c
	cc -c utctime.c

disperr.o: disperr.c
	cc -c disperr.c

charpos.o: charpos.c
	cc -c charpos.c

clean:
	rm *.o
