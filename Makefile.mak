main: k kp p

k: k.c rejs.c rejs.h
	gcc k.c rejs.c -o k

kp: kp.c rejs.c rejs.h
	gcc kp.c rejs.c -o kp

p: p.c
	gcc p.c -o p

test1:
	./k < test1k &
	./kp < test1kp &
	./p &

test2:
	./k < test2k &
	./kp < test2kp &
	./p &

test3:
	./k < test3k &
	./kp < test3kp &
	./p &

user:
	@echo "Podaj pojemnosc mostka:"
	@read pojemnosc_mostka; \
	@echo "Podaj pojemnosc statku:"; \
	@read pojemnosc_statku; \
	@echo "Podaj ilosc rejsow dzisiaj:"; \
	@read ilosc_rejsow_dzis; \
	@echo "Podaj czas rejsu (w sekundach):"; \
	@read czas_rejsu; \
	@echo "Podaj czas miedzy rejsami (w sekundach):"; \
	@read czas; \
	@./k $$pojemnosc_mostka $$pojemnosc_statku $$ilosc_rejsow_dzis $$czas_rejsu & \
	@./kp $$czas $$czas_rejsu & \
	@./p &

clean:
	rm -f k kp p