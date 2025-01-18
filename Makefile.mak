ll: k kp p
	./k &
	./kp &
	./p &

k: k.c 
	gcc k.c -o k

kp: kp.c
	gcc kp.c -o kp

p: p.c 
	gcc p.c -o p

clean:
	rm -f k kp p
