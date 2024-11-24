
# Build the nitrate-rt.a

all: nitrate-rt.a

CFILES = $(wildcard *.c)
OFILES = $(CFILES:.c=.o)

nitrate-rt.a: $(OFILES)
	$(AR) rcs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ -O3 -fPIC -fno-stack-protector -Wall -Wextra -Werror

clean:
	rm -f *.o *.a
