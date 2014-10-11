CC=i686-w64-mingw32-gcc
LIB_WPCAP=./WpdPack/Lib
INC_WPCAP=./WpdPack/Include

client.exe: src/client.c src/raw.c src/vpn.c
	$(CC) -I $(INC_WPCAP) -L $(LIB_WPCAP) src/*.c -o client.exe -lws2_32 -lwpcap

clean:
	rm -f client.exe
