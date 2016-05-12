TARGETS = winshell.dll

all: $(TARGETS) install

winshell.dll: winshell.o
	gcc -shared -o $@ -Wl,--export-all-symbols -Wl,--enable-auto-import -Wl,--whole-archive $^ -Wl,--no-whole-archive -lstdc++ ../../lib/liblua.a ../../lib/liblualib.a

%.o: %.c
	gcc -c -o $@ $^ 

clean:
	rm *.o $(TARGETS)
	
install:
	cp $(TARGETS) ../..