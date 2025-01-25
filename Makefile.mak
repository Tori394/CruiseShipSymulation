main: k kp p

k: kapitan.c rejs.c rejs.h
	gcc kapitan.c rejs.c -o k

kp: kapitan_portu.c rejs.c rejs.h
	gcc kapitan_portu.c rejs.c -o kp

p: pasazer.c rejs.c rejs.h
	gcc pasazer.c rejs.c -o p

test1:
	./p &
	./k 0 0 0 0 &
	./kp 0 0 0 & 

test2:
	./p &
	./k 10 1 1 1 &
	./kp 5 1 10 & 

test3:
	./p &
	./k 5 10 1 20 &
	./kp 5 20 10 & 

test4:
	./p &
	./k 5 60 5 10 &
	./kp 20 10 0 & 

test5:
	./p &
	./k 2 5 1000 1 &
	./kp 5 1 0 & 

test6:
	./p &
	./k 2 60 5 1 &
	./kp 11 1 100 & 

pokaz:
	./p &
	./k 5 10 2 10 &
	./kp 30 10 15 &

user:
	@echo "Podaj pojemnosc mostka (minimum 2):"
	@read pojemnosc_mostka; \
	echo "Podaj pojemnosc statku (minimum 5):"; \
	read pojemnosc_statku; \
	echo "Podaj ilosc rejsow dzisiaj:"; \
	read ilosc_rejsow_dzis; \
	echo "Podaj czas rejsu (w sekundach):"; \
	read czas_rejsu; \
	echo "Podaj czas miedzy rejsami (w sekundach, musi byc wiekszy niz 10s i mniejszy od czasu rejsu):"; \
	read czas; \
	echo "Podaj szanse na burze (w procentach):"; \
	read szansa_na_burze; \
	./p & \
	./k $$pojemnosc_mostka $$pojemnosc_statku $$ilosc_rejsow_dzis $$czas_rejsu & \
	./kp $$czas $$czas_rejsu $$szansa_na_burze & \

clean:
	rm -f k kp p

fullclean:
	rm -f k kp p rejs.c rejs.h kapitan.c kapitan_portu. pasazer.c Makefile