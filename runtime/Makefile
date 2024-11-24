
# Build the nitrate runtime

all: nitrate-rt.a nitrate-start.a 

nitrate-rt.a: 
	$(MAKE) -C common

nitrate-start.a:
	$(MAKE) -C platform

clean:
	$(MAKE) -C common clean
	$(MAKE) -C platform clean
