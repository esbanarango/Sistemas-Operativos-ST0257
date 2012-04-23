CC   = g++.exe
LIBS =  -L"C:/Dev-Cpp/lib"  
INCS =  -I"C:/Dev-Cpp/include" 
CXXINCS =  -I"C:/Dev-Cpp/lib/gcc/mingw32/3.4.2/include"  -I"C:/Dev-Cpp/include/c++/3.4.2/backward"  -I"C:/Dev-Cpp/include/c++/3.4.2/mingw32"  -I"C:/Dev-Cpp/include/c++/3.4.2"  -I"C:/Dev-Cpp/include" 
CXXFLAGS = $(CXXINCS)  
CFLAGS = $(INCS)  

all: conctrl.exe procesoctrl.exe

conctrl.exe: conctrl.o
	$(CC) conctrl.o -o "conctrl.exe" $(LIBS)

conctrl.o: conctrl.cpp
	$(CC) -c conctrl.cpp -o conctrl.o $(CFLAGS)

procesoctrl.exe: procesoctrl.o
	$(CC) procesoctrl.o -o "procesoctrl.exe" $(LIBS)
	
procesoctrl.o: procesoctrl.cpp
	$(CC) -c procesoctrl.cpp -o procesoctrl.o $(CFLAGS)
	