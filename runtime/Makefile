CFLAGS = -Wall -Wextra -Werror -pedantic -O3

linux: 
# Build runtime static object
	$(CC) -o nitrate-linux-rt.o $(CFLAGS) -c nitrate-linux-rt.c
	$(AR) rcs libnitrate-linux-rt.a nitrate-linux-rt.o

# Build mock application
	$(CC) -o mock-app $(CFLAGS) -L. -lnitrate-linux-rt mock-app.c -nostdlib -static -lc -s


clean:
	rm -r *.o *.a mock-app
