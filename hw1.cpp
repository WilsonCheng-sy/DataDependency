#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringRef.h"

using namespace std;
using namespace llvm;

#define DEBUG_TYPE "loop"

StringRef *LoopCond = new StringRef("for.cond");
StringRef *LoopBody = new StringRef("for.body");
StringRef *Entry = new StringRef("entry");

namespace {

    struct Array_name_and_expend {
        string arrayName;
        vector<int> index;
    };
    struct Array_name_and_expend Arr[10][50]; // List all Arr value

    int Add[10][2], Sub[10][2], Mul[10][2]; // Fro calculate the value. Ex. A[2*i-1]
    int Loop_from = 0, Loop_to = 0; // for loop (from, to)
    int line;
    bool outputFlag = false, antiFlag = false, flowFlag = false; //Once found dependence, no need to do more

    vector<string> arrayOrder; 

    struct Dependence : public ModulePass {
        static char ID; // Pass identification, replacement for typeid
        Dependence() : ModulePass(ID) {}

        void getLoadDef(Value *inv) {    //recursive traverse the IR
            if(Instruction *I = dyn_cast<Instruction>(inv)){   //make sure *inv is an Instruction

                //when you call getName for llvm temp variable such as %1 %2 ... is empty char
                //so when getName is empty char means you must search deeper inside the IR
                if(I->getName() != ""){
                    if(I->getOpcode() == Instruction::GetElementPtr) {
                        Value *tmp = I->getOperand(0);
                        //%ArrayName = alloca [20 x i32], align 16
                        string name = tmp->getName();
                    }
                }
                else{
                    //recursive to the upper define-use chain
                    for(User::op_iterator OI = (*I).op_begin(), e = (*I).op_end(); OI != e; ++OI) {
                        Value *v = *OI;
                        getLoadDef(v);
                    }
                }
            }
        }

        void initialize() { // Used in handle_calculate(). Initialize ADD, SUB, MUL
            for(int i = 0; i < 10; i++) {
                for(int j = 0; j < 2; j++) {
                    Add[i][j] = 0;
                    Sub[i][j] = 0;
                    Mul[i][j] = 1;
                }
            }
        }


        void Loop_info(Function::iterator BB){ // Get Loop from, to
            if(!BB->getName().find(*Entry, 0)){ // inside Entry
                errs() << "--- Loop info ---\n";
                for (BasicBlock::iterator itrIns = (*BB).begin(); itrIns != (*BB).end(); itrIns++){
                    if(const StoreInst *SI =dyn_cast<StoreInst>(itrIns)){ 
                        Value *val0 = itrIns->getOperand(0);
                        Value *val1 = itrIns->getOperand(1);
                        
                        auto name = itrIns->getOperand(1)->getName();

                        if (name.compare("retval") != 0){
                            if(ConstantInt* Integer = dyn_cast<ConstantInt>(itrIns->getOperand(0))) {
                                Loop_from = Integer->getZExtValue();
                                errs() << "Loop Min Value: " << Loop_from << '\n'; // find loop_from
                            }
                        }
                    }
                }
            }

            if(!BB->getName().find(*LoopCond, 0)) {
                for (BasicBlock::iterator itrIns = (*BB).begin(); itrIns != (*BB).end(); itrIns++) {
                    if(!strcmp("icmp", itrIns->getOpcodeName())) {
                        if(ConstantInt* Integer = dyn_cast<ConstantInt>(itrIns->getOperand(1))) {
                            Loop_to = Integer->getZExtValue();
                            errs() << "Loop Max Value: " << Loop_to << '\n'; // //find loop_to
                        }
                    }
                }
            }
        }


        void calculate_expr(Function::iterator BB){ // Store in ADD, SUB, MUL to calculate value
            if(!BB->getName().find(*LoopBody, 0)) {
                for (BasicBlock::iterator itrIns = (*BB).begin(); itrIns != (*BB).end(); itrIns++) {
                    if(dyn_cast<Instruction>(itrIns)) {
                        if(itrIns->getOpcode() == Instruction::Store) {
                            //tmp1 = tmp2 in c code.
                            Value *tmp1 = itrIns->getOperand(0);
                            Value *tmp2 = itrIns->getOperand(1);

                            if(!tmp1->hasName()) {
                                getLoadDef(tmp1);
                            }

                            if(Instruction *I = dyn_cast<Instruction>(tmp2)) {
                                if(I->getOpcode() == Instruction::GetElementPtr) {
                                    Value *tmp = I->getOperand(0);
                                    string name = tmp->getName();
                                }
                            }
                        }

                        //record the order of each array appeared for fucutre loop expand.
                        else if(itrIns->getOpcode() == Instruction::GetElementPtr) {
                            Value *tmp = itrIns->getOperand(0);
                            std::string name = tmp->getName();
                            arrayOrder.push_back(name);
                        }

                        else if(itrIns->getOpcode() == Instruction::Sub) {
                            int currentLine = arrayOrder.size()/2;
                            int currentLoc = !(arrayOrder.size()%2);

                            Value *val1 = itrIns->getOperand(1);

                            //errs() << "Sub Value1 ::" << *val1 << "\n\n";
                            if(ConstantInt *Int = dyn_cast<ConstantInt>(val1)) {
                                int value = Int->getZExtValue();
                                Sub[currentLine][currentLoc] = value;
                            }
                        }

                        else if(itrIns->getOpcode() == Instruction::Add) {
                            int currentLine = arrayOrder.size()/2;
                            int currentLoc = !(arrayOrder.size()%2);

                            Value *val1 = itrIns->getOperand(1);

                            if(ConstantInt *Int = dyn_cast<ConstantInt>(val1)) {
                                int value = Int->getZExtValue();
                                Add[currentLine][currentLoc] = value;
                            }
                        }

                        else if(itrIns->getOpcode() == Instruction::Mul) {
                            int currentLine = arrayOrder.size()/2;
                            int currentLoc = !(arrayOrder.size()%2);

                            Value *val1 = itrIns->getOperand(0);

                            if(ConstantInt *Int = dyn_cast<ConstantInt>(val1)) {
                                int value = Int->getZExtValue();
                                //errs() << "Mul value: " << value << '\n';
                                Mul[currentLine][currentLoc] = value;
                            }
                        }
                    }
                }
            }
        }


        void findAntiDependency() {
            // errs() << "in anti";
            for(int i = 0; i < line; i++) {
                for(int j = 0; j < line; j++) {
                    if(Arr[i][1].arrayName == Arr[j][0].arrayName) {
                        for(int m = 0; m < Loop_to - Loop_from; m++) {
                            for(int n = 0; n < Loop_to - Loop_from; n++) {
                                if(Arr[i][1].index[m] == Arr[j][0].index[n] && m < n) {
                                    if(!antiFlag) {
                                        errs() << "=====Anti Dependence=====\n";
                                        errs() << "(i = " << m + Loop_from << ", i = " << n + Loop_from << ")\n";
                                        errs() << Arr[i][1].arrayName << " : ";
                                        errs() << 'S' << i+1 << " ---> S" << j+1 << '\n';
                                        antiFlag = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        void findOutputDependency() {
            for(int i = 0; i < line; i++) {
                for(int j = i+1; j < line; j++) {
                    if(Arr[i][0].arrayName == Arr[j][0].arrayName) {
                        for(int m = 0; m < Loop_to - Loop_from; m++) {
                            for(int n = 0; n < Loop_to - Loop_from; n++) {
                                if(Arr[i][0].index[m] == Arr[j][0].index[n]) {
                                    if(!outputFlag) {
                                        errs() << "=====Output Dependence=====\n";
                                        errs() << "(i = " << m + Loop_from << ", i = " << n + Loop_from << ")\n";
                                        errs() << Arr[i][0].arrayName << " : ";
                                        errs() << 'S' << i+1 << " ---> S" << j+1 << '\n';
                                        outputFlag = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        void findFlowDependency() {
            for(int i = 0; i < line; i++) {
                for(int j = i+1; j < line; j++) {
                    if(Arr[i][0].arrayName == Arr[j][1].arrayName) {
                        for(int m = 0; m < Loop_to - Loop_from; m++) {
                            for(int n = 0; n < Loop_to - Loop_from; n++) {
                                if(Arr[i][0].index[m] == Arr[j][1].index[n] && m <= n) {
                                    if(!flowFlag) {
                                        errs() << "=====Flow Dependence=====\n";
                                        errs() << "(i = " << m + Loop_from << ", i = " << n + Loop_from << ")\n";
                                        errs() << Arr[i][0].arrayName << " : ";
                                        errs() << 'S' << i+1 << " ---> S" << j+1 << '\n';
                                        flowFlag = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }


        void expandLoop() { // Used in runOnModule(). Expend the result

            for(int i = 0; i < arrayOrder.size(); i++) {
                Arr[i/2][!(i%2)].arrayName = arrayOrder[i];
            }

            for(int i = 0; i < line; i++) {
                for(int j = 0; j < 2; j++) {
                    for(int k = Loop_from; k < Loop_to; k++) {
                        int idx = k*Mul[i][j]+Add[i][j]-Sub[i][j];
                        Arr[i][j].index.push_back(idx);
                    }
                }
            }

            // for(int i = 0; i < line; i++) {
            //     for(int j = 0; j < 2; j++) {
            //         // errs() << "(i, j)" << i << "," << j << "\n";
            //         errs() << "name: " << Arr[i][j].arrayName << "\n";
            //         // errs() << "print vector" << Arr[i][j].index.size() << "\n";
            //         for (auto i : Arr[i][j].index) errs() << i << " ";
            //         errs() << "\n";
            //     }
            // }
        }

        virtual bool runOnModule(Module &M) { //override ModulePass
            // loop info
            initialize(); // initialize ADD, SUB, MUL

            for (Module::iterator F = M.begin(); F != M.end(); F++){ // in Module
                for (Function::iterator BB = (*F).begin(); BB != (*F).end(); BB++) { // In Function
                    Loop_info(BB);               
                    calculate_expr(BB);
                }
            }

            line = arrayOrder.size()/2;
            errs() << "Loop Body Statement Counts: " << line <<"\n\n";
            errs() << "--- Dependence Analysis ---" <<"\n";

            expandLoop();
            
            findOutputDependency();
            findFlowDependency();
            findAntiDependency();

            return false;
        }
    };
 }

//initialize identifier
char Dependence::ID = 0;
//"dependence" is the name of pass
//"Simple program to find data dependency " is the explaination of your pass
static RegisterPass<Dependence> GS("dependence", "Simple program to find data dependency");