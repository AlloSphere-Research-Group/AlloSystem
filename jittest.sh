rm build/bin/example

# -Wl,-r 

g++ -DOSC_HOST_LITTLE_ENDIAN -Iinclude//allocore/types/jack/ -I/usr/include/ -Iinclude/ -Iosx/include/ -Ibuild//include -undefined dynamic_lookup -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -arch i386 -isysroot /Developer/SDKs/MacOSX10.5.sdk -mmacosx-version-min=10.5 -O3 -fpeel-loops -Wreturn-type -Wformat -Wmissing-braces -Wparentheses -Wswitch -Wunused-variable -Wsign-compare -Wno-unknown-pragmas  -Wnon-virtual-dtor -Woverloaded-virtual -force_load build/lib/liballocore.a -o build/bin/example examples/allojit/example.cpp -L/usr/lib/ -Losx/lib/ -Lbuild//lib -framework Carbon  -lapr-1 -lfreeimage -lassimp -framework AudioUnit -framework AudioToolbox -framework CoreAudio -framework Carbon -lportaudio -framework AGL -framework OpenGL -framework GLUT -lallojit -lalloutil

build/bin/example

nm build/bin/example | grep _ZN2al8MainLoop3getEv
