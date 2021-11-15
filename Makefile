LLVM_CONFIG=/home/sycheng/llvm_pass/llvm_build/bin/llvm-config

CXX=`$(LLVM_CONFIG) --bindir`/clang
CXXFLAGS=`$(LLVM_CONFIG) --cppflags` -fPIC -fno-rtti
LDFLAGS=`$(LLVM_CONFIG) --ldflags`

all: hw1.so Test hello_pass.so Demo.so

hw1.o: hw1.cpp
	$(CXX) -c hw1.cpp $(CXXFLAGS) -o hw1.o

hw1.so: hw1.o
	$(CXX) -shared hw1.o $(LDFLAGS) -fPIC -o hw1.so

hello_pass.o: hello_pass.cpp
	$(CXX) -c hello_pass.cpp $(CXXFLAGS) -o hello_pass.o

hello_pass.so: hello_pass.o
	$(CXX) -shared hello_pass.o $(LDFLAGS) -fPIC -o hello_pass.so

Demo.o: Demo.cpp
	$(CXX) -c Demo.cpp $(CXXFLAGS) -o Demo.o

Demo.so: Demo.o
	$(CXX) -shared Demo.o $(LDFLAGS) -fPIC -o Demo.so

Test: test1.c test2.c test3.c
	$(CXX) -S -fno-discard-value-names -emit-llvm test1.c -o test1.ll
	$(CXX) -S -fno-discard-value-names -emit-llvm test2.c -o test2.ll
	$(CXX) -S -fno-discard-value-names -emit-llvm test3.c -o test3.ll

clean:
	rm -f *.o *.ll *.so *.bc
