// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_ARM_A64COMPILER_H_INCLUDED
#define ASMJIT_ARM_A64COMPILER_H_INCLUDED

#include "../core/api-config.h"
#ifndef ASMJIT_NO_COMPILER

#include "../core/compiler.h"
#include "../core/type.h"
#include "../arm/a64emitter.h"

ASMJIT_BEGIN_SUB_NAMESPACE(a64)

//! \addtogroup asmjit_a64
//! \{

//! AArch64 compiler implementation.
class ASMJIT_VIRTAPI Compiler
  : public BaseCompiler,
    public EmitterExplicitT<Compiler> {
public:
  ASMJIT_NONCOPYABLE(Compiler)
  using Base = BaseCompiler;

  //! \name Construction & Destruction
  //! \{

  ASMJIT_API explicit Compiler(CodeHolder* code = nullptr) noexcept;
  ASMJIT_API ~Compiler() noexcept override;

  //! \}

  //! \name Virtual Registers
  //! \{

  //! \cond INTERNAL
  template<typename RegT, typename Type>
  ASMJIT_INLINE_NODEBUG RegT _newRegInternal(const Type& type) {
    RegT reg(Globals::NoInit);
    _newReg(&reg, type, nullptr);
    return reg;
  }

  template<typename RegT, typename Type>
  ASMJIT_INLINE_NODEBUG RegT _newRegInternal(const Type& type, const char* s) {
#ifndef ASMJIT_NO_LOGGING
    RegT reg(Globals::NoInit);
    _newReg(&reg, type, s);
    return reg;
#else
    DebugUtils::unused(s);
    return _newRegInternal<RegT>(type);
#endif
  }

  template<typename RegT, typename Type, typename... Args>
  ASMJIT_INLINE_NODEBUG RegT _newRegInternal(const Type& type, const char* s, Args&&... args) {
#ifndef ASMJIT_NO_LOGGING
    RegT reg(Globals::NoInit);
    _newRegFmt(&reg, type, s, std::forward<Args>(args)...);
    return reg;
#else
    DebugUtils::unused(s, std::forward<Args>(args)...);
    return _newRegInternal<RegT>(type);
#endif
  }
  //! \endcond

  template<typename RegT, typename... Args>
  ASMJIT_INLINE_NODEBUG RegT newSimilarReg(const RegT& ref, Args&&... args) {
    return _newRegInternal<RegT>(ref, std::forward<Args>(args)...);
  }

  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Reg newReg(TypeId typeId, Args&&... args) { return _newRegInternal<Reg>(typeId, std::forward<Args>(args)...); }

  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp newGp(TypeId typeId, Args&&... args) { return _newRegInternal<Gp>(typeId, std::forward<Args>(args)...); }

  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec newVec(TypeId typeId, Args&&... args) { return _newRegInternal<Vec>(typeId, std::forward<Args>(args)...); }

  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp newInt32(Args&&... args) { return _newRegInternal<Gp>(TypeId::kInt32, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp newUInt32(Args&&... args) { return _newRegInternal<Gp>(TypeId::kUInt32, std::forward<Args>(args)...); }

  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp newInt64(Args&&... args) { return _newRegInternal<Gp>(TypeId::kInt64, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp newUInt64(Args&&... args) { return _newRegInternal<Gp>(TypeId::kUInt64, std::forward<Args>(args)...); }

  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp newIntPtr(Args&&... args) { return _newRegInternal<Gp>(TypeId::kIntPtr, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp newUIntPtr(Args&&... args) { return _newRegInternal<Gp>(TypeId::kUIntPtr, std::forward<Args>(args)...); }

  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp newGp32(Args&&... args) { return _newRegInternal<Gp>(TypeId::kUInt32, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp newGp64(Args&&... args) { return _newRegInternal<Gp>(TypeId::kUInt64, std::forward<Args>(args)...); }

  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp newGpw(Args&&... args) { return _newRegInternal<Gp>(TypeId::kUInt32, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp newGpx(Args&&... args) { return _newRegInternal<Gp>(TypeId::kUInt64, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp newGpz(Args&&... args) { return _newRegInternal<Gp>(TypeId::kUIntPtr, std::forward<Args>(args)...); }

  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec newVecS(Args&&... args) { return _newRegInternal<Vec>(TypeId::kFloat32, std::forward<Args>(args)...); }

  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec newVecD(Args&&... args) { return _newRegInternal<Vec>(TypeId::kFloat64, std::forward<Args>(args)...); }

  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec newVecQ(Args&&... args) { return _newRegInternal<Vec>(TypeId::kUInt8x16, std::forward<Args>(args)...); }

  //! \}

  //! \name Stack
  //! \{

  //! Creates a new memory chunk allocated on the current function's stack.
  ASMJIT_INLINE_NODEBUG Mem newStack(uint32_t size, uint32_t alignment, const char* name = nullptr) {
    Mem m(Globals::NoInit);
    _newStack(&m, size, alignment, name);
    return m;
  }

  //! \}

  //! \name Constants
  //! \{

  //! Put data to a constant-pool and get a memory reference to it.
  ASMJIT_INLINE_NODEBUG Mem newConst(ConstPoolScope scope, const void* data, size_t size) {
    Mem m(Globals::NoInit);
    _newConst(&m, scope, data, size);
    return m;
  }

  //! Put a BYTE `val` to a constant-pool (8 bits).
  ASMJIT_INLINE_NODEBUG Mem newByteConst(ConstPoolScope scope, uint8_t val) noexcept { return newConst(scope, &val, 1); }
  //! Put a HWORD `val` to a constant-pool (16 bits).
  ASMJIT_INLINE_NODEBUG Mem newHWordConst(ConstPoolScope scope, uint16_t val) noexcept { return newConst(scope, &val, 2); }
  //! Put a WORD `val` to a constant-pool (32 bits).
  ASMJIT_INLINE_NODEBUG Mem newWordConst(ConstPoolScope scope, uint32_t val) noexcept { return newConst(scope, &val, 4); }
  //! Put a DWORD `val` to a constant-pool (64 bits).
  ASMJIT_INLINE_NODEBUG Mem newDWordConst(ConstPoolScope scope, uint64_t val) noexcept { return newConst(scope, &val, 8); }

  //! Put a WORD `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem newInt16Const(ConstPoolScope scope, int16_t val) noexcept { return newConst(scope, &val, 2); }
  //! Put a WORD `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem newUInt16Const(ConstPoolScope scope, uint16_t val) noexcept { return newConst(scope, &val, 2); }
  //! Put a DWORD `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem newInt32Const(ConstPoolScope scope, int32_t val) noexcept { return newConst(scope, &val, 4); }
  //! Put a DWORD `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem newUInt32Const(ConstPoolScope scope, uint32_t val) noexcept { return newConst(scope, &val, 4); }
  //! Put a QWORD `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem newInt64Const(ConstPoolScope scope, int64_t val) noexcept { return newConst(scope, &val, 8); }
  //! Put a QWORD `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem newUInt64Const(ConstPoolScope scope, uint64_t val) noexcept { return newConst(scope, &val, 8); }

  //! Put a SP-FP `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem newFloatConst(ConstPoolScope scope, float val) noexcept { return newConst(scope, &val, 4); }
  //! Put a DP-FP `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem newDoubleConst(ConstPoolScope scope, double val) noexcept { return newConst(scope, &val, 8); }

  //! \}

  //! \name Instruction Options
  //! \{

  //! Force the compiler to not follow the conditional or unconditional jump.
  ASMJIT_INLINE_NODEBUG Compiler& unfollow() noexcept { _instOptions |= InstOptions::kUnfollow; return *this; }

  //! \}

  //! \name Compiler specific
  //! \{

  //! Special pseudo-instruction that can be used to load a memory address into `o0` GP register.
  //!
  //! \note At the moment this instruction is only useful to load a stack allocated address into a GP register
  //! for further use. It makes very little sense to use it for anything else. The semantics of this instruction
  //! is the same as X86 `LEA` (load effective address) instruction.
  ASMJIT_INLINE_NODEBUG Error loadAddressOf(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdAdr, o0, o1); }

  //! \}

  //! \name Function Call & Ret Intrinsics
  //! \{

  //! Invoke a function call without `target` type enforcement.
  ASMJIT_INLINE_NODEBUG Error invoke_(InvokeNode** out, const Operand_& target, const FuncSignature& signature) {
    return addInvokeNode(out, Inst::kIdBlr, target, signature);
  }

  //! Invoke a function call of the given `target` and `signature` and store the added node to `out`.
  //!
  //! Creates a new \ref InvokeNode, initializes all the necessary members to match the given function `signature`,
  //! adds the node to the compiler, and stores its pointer to `out`. The operation is atomic, if anything fails
  //! nullptr is stored in `out` and error code is returned.
  ASMJIT_INLINE_NODEBUG Error invoke(InvokeNode** out, const Gp& target, const FuncSignature& signature) { return invoke_(out, target, signature); }
  //! \overload
  ASMJIT_INLINE_NODEBUG Error invoke(InvokeNode** out, const Mem& target, const FuncSignature& signature) { return invoke_(out, target, signature); }
  //! \overload
  ASMJIT_INLINE_NODEBUG Error invoke(InvokeNode** out, const Label& target, const FuncSignature& signature) { return invoke_(out, target, signature); }
  //! \overload
  ASMJIT_INLINE_NODEBUG Error invoke(InvokeNode** out, const Imm& target, const FuncSignature& signature) { return invoke_(out, target, signature); }
  //! \overload
  ASMJIT_INLINE_NODEBUG Error invoke(InvokeNode** out, uint64_t target, const FuncSignature& signature) { return invoke_(out, Imm(int64_t(target)), signature); }

  //! Return.
  ASMJIT_INLINE_NODEBUG Error ret() { return addRet(Operand(), Operand()); }
  //! \overload
  ASMJIT_INLINE_NODEBUG Error ret(const Reg& o0) { return addRet(o0, Operand()); }
  //! \overload
  ASMJIT_INLINE_NODEBUG Error ret(const Reg& o0, const Reg& o1) { return addRet(o0, o1); }

  //! \}

  //! \name Jump Tables Support
  //! \{

  using EmitterExplicitT<Compiler>::br;

  //! Adds a jump to the given `target` with the provided jump `annotation`.
  ASMJIT_INLINE_NODEBUG Error br(const Reg& target, JumpAnnotation* annotation) { return emitAnnotatedJump(Inst::kIdBr, target, annotation); }

  //! \}

  //! \name Events
  //! \{

  ASMJIT_API Error onAttach(CodeHolder& code) noexcept override;
  ASMJIT_API Error onDetach(CodeHolder& code) noexcept override;
  ASMJIT_API Error onReinit(CodeHolder& code) noexcept override;

  //! \}

  //! \name Finalize
  //! \{

  ASMJIT_API Error finalize() override;

  //! \}
};

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_COMPILER
#endif // ASMJIT_ARM_A64COMPILER_H_INCLUDED
