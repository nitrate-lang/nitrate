
# Build the nitrate-start.a

all: nitrate-start.a

CFILES = $(wildcard *.c)
OFILES = $(CFILES:.c=.o)

nitrate-start.a: $(OFILES)
	$(AR) rcs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ -O3 -fPIC -fno-stack-protector -Wall -Wextra -Werror

clean:
	rm -f *.o *.a
