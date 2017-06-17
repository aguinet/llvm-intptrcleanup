// Copyright 2017 Adrien Guinet <adrien@guinet.me>

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "llvm/Support/raw_ostream.h"


using namespace llvm;

namespace {

struct IntPtrCleanup: public FunctionPass
{
  static char ID;

  IntPtrCleanup():
    FunctionPass(ID)
  {}

  /// Return the name of the pass, for debugging.
  StringRef getPassName() const override {
    return "int-ptr-cleanup";
  }

  static PointerType* getPHIArgPointerType(PHINode const& Phi)
  {
    return cast<PointerType>(cast<PtrToIntInst>(Phi.getIncomingValue(0))->getPointerOperand()->getType());
  }

  static bool isValidPHI(PHINode const& Phi)
  {
    if (!Phi.getType()->isIntegerTy()) {
      return false;
    }

    if (Phi.getNumIncomingValues() == 0) {
      return false;
    }

    Type* PtrTy = nullptr;
    auto ItInV = Phi.op_begin();
    if (auto* PTI = dyn_cast<PtrToIntInst>(ItInV->get()))
      PtrTy = PTI->getPointerOperand()->getType();
    else
      return false;

    if (!std::all_of(++ItInV, Phi.op_end(), [&PtrTy](Use const& U) {
          auto* V = U.get();
          return isa<PtrToIntInst>(V) && cast<PtrToIntInst>(V)->getPointerOperand()->getType() == PtrTy;})) {
      return false;
    }

    // Checks that the PHI users are inttoptr instructions of the good type
    return std::all_of(Phi.user_begin(), Phi.user_end(), [&PtrTy](User const* U) { 
        return isa<IntToPtrInst>(U) && U->getType() == PtrTy; });
  }

  static void transformPhi(PHINode& Phi)
  {
    // Generate a PHINode with the good pointer type and that points directly
    // to the values of the ptrtoint instructions, then make the users of the
    // inttoptr instructions' Phi users use this one. After that, cleanup the
    // inttoptr instructions and the original PHINode.

    auto* PointerType = getPHIArgPointerType(Phi);
    auto* NewPhi = PHINode::Create(PointerType, Phi.getNumIncomingValues(), "", &Phi);
    for (unsigned i = 0; i < Phi.getNumIncomingValues(); ++i) {
      NewPhi->addIncoming(cast<PtrToIntInst>(Phi.getIncomingValue(i))->getPointerOperand(), Phi.getIncomingBlock(i));
    }

    SmallVector<Instruction*, 8> ToRemove;
    // Transform users
    for (User* U: Phi.users()) {
      IntToPtrInst* ITP = cast<IntToPtrInst>(U);
      ITP->replaceAllUsesWith(NewPhi);
      ToRemove.push_back(ITP);
    }
    
    // Cleanup
    for (Instruction* I: ToRemove) {
      I->eraseFromParent();
    }
    Phi.eraseFromParent();
  }

  bool runOnFunction(Function &F) override {

    // First, gather candidates!
    SmallVector<PHINode*, 16> Phis;
    for (auto& BB: F) {
      for (auto& I: BB) {
        if (auto* Phi = dyn_cast<PHINode>(&I)) {
          if (isValidPHI(*Phi)) {
            Phis.push_back(Phi);
          }
        }
      }
    }

    //
    for (PHINode* Phi: Phis) {
      transformPhi(*Phi);
    }

    return Phis.size() > 0;
  }
};

char IntPtrCleanup::ID;

void addPass(const PassManagerBuilder &Builder, legacy::PassManagerBase &PM) {
  PM.add(new IntPtrCleanup());
}

// Register this pass before vectorization!
RegisterStandardPasses S(PassManagerBuilder::EP_VectorizerStart,
                         addPass);

} // anonymous
