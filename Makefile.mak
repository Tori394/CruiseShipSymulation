main: k kp p

k: k.c rejs.c rejs.h
	gcc k.c rejs.c -o k

kp: kp.c rejs.c rejs.h
	gcc kp.c rejs.c -o kp

p: p.c rejs.c rejs.h
	gcc p.c rejs.c -o p

test1:
	./k 0 0 0 0 &
	./kp 0 0 &
	./p &

test2:
	./k 10 1 1 1 &
	./kp 5 1 &
	./p &

test3:
	./k 5 10 1 20 &
	./kp 5 20 &
	./p &

test4:
	./k 5 10 3 15 &
	./kp 30 15 &
	./p &

test5:
	./k 15 40 8 30 &
	./kp 120 30 &
	./p &


user:
	@echo "Podaj pojemnosc mostka:"
	@read pojemnosc_mostka; \
	echo "Podaj pojemnosc statku:"; \
	read pojemnosc_statku; \
	echo "Podaj ilosc rejsow dzisiaj:"; \
	read ilosc_rejsow_dzis; \
	echo "Podaj czas rejsu (w sekundach):"; \
	read czas_rejsu; \
	echo "Podaj czas miedzy rejsami (w sekundach, musi byc wiekszy niz 10s i mniejszy od czasu rejsu):"; \
	read czas; \
	./k $$pojemnosc_mostka $$pojemnosc_statku $$ilosc_rejsow_dzis $$czas_rejsu & \
	./kp $$czas $$czas_rejsu & \
	./p & 

clean:
	rm -f k kp p