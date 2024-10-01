# Makefile for building qrcodeScanner.exe using i686-w64-mingw32-g++
# Variables for compiler, flags, and directories
CC := i686-w64-mingw32-g++
CFLAGS := -fpermissive -fdata-sections -ffunction-sections -Os -static-libgcc -static-libstdc++ -s
INCLUDES := -I./zbar/include/ -I./openssl/include/ -I.
LDFLAGS := -L. -lgdi32 -l:libcrypto-3.dll -l:libzbar-0.dll
TARGET := build/qrcodeScanner.exe
COMPRESSED_TARGET := build/qrcodeScanner.exe
DEP_FILE := run.bat libcrypto-3.dll libzbar-0.dll libiconv-2.dll
ZIP_FILE := qrcodeScanner_release.zip

SRC := qrScan.cpp crypto.cc base64.cc

$(TARGET): $(SRC)
	mkdir -p build
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(INCLUDES) $(LDFLAGS)
pack: $(TARGET)
	cp $(DEP_FILE) build/
	-upx --best $(TARGET)
	cd build && zip -r ../$(ZIP_FILE) *
clean:
	rm -rf build $(ZIP_FILE)
test: $(TARGET)
	@wine $(TARGET)
# Phony targets
.PHONY: clean pack test
