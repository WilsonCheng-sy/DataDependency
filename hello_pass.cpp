#include "llvm/Pass.h"    
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
  //declare a “Hello” class that is a subclass of FunctionPass. 
  struct Hello : public FunctionPass {
    //pass identifier used by LLVM to identify pass
    static char ID;
    Hello() : FunctionPass(ID) {}
    //override an abstract virtual method inherited from FunctionPass. 
    virtual bool runOnFunction(Function &F) {
      errs() << "Hello: ";
      errs().write_escaped(F.getName()) << '\n';
      return false;
    }
  };
}

char Hello::ID = 0;

//register class 
/*
hello ->  command line argument
Hello world pass -> name
*/
static RegisterPass<Hello> X("hello", "Hello World Pass");