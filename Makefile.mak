main: k kp p

k: kapitan.c rejs.c rejs.h
	gcc kapitan.c rejs.c -o k

kp: kapitan_portu.c rejs.c rejs.h
	gcc kapitan_portu.c rejs.c -o kp -pthread

p: pasazer.c rejs.c rejs.h
	gcc pasazer.c rejs.c -o p

test1:
	./p &
	./k 5 10 1 20 &
	./kp 5 20 10 & 

test2:
	./p &
	./k 5 60 5 10 &
	./kp 20 10 0 & 

test3:
	./p &
	./k 2 5 5 1 &
	./kp 60 1 0 & 

test4:
	./p &
	./k 2 60 5 1 &
	./kp 11 1 100 & 

user:
	@echo "Podaj pojemnosc mostka:"
	@read pojemnosc_mostka; \
	echo "Podaj pojemnosc statku:"; \
	read pojemnosc_statku; \
	echo "Podaj ilosc rejsow dzisiaj:"; \
	read ilosc_rejsow_dzis; \
	echo "Podaj czas rejsu (w sekundach):"; \
	read czas_rejsu; \
	echo "Podaj czas miedzy rejsami (w sekundach, musi byc wiekszy od czasu rejsu):"; \
	read czas; \
	echo "Podaj szanse na burze (w procentach):"; \
	read szansa_na_burze; \
	./p & \
	./k $$pojemnosc_mostka $$pojemnosc_statku $$ilosc_rejsow_dzis $$czas_rejsu & \
	./kp $$czas $$czas_rejsu $$szansa_na_burze & \

clean:
	rm -f k kp p

fullclean:
	rm -f k kp p rejs.c rejs.h kapitan.c kapitan_portu.c pasazer.c Makefile