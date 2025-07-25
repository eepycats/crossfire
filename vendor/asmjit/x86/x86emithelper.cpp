// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include "../core/api-build_p.h"
#if !defined(ASMJIT_NO_X86)

#include "../core/formatter.h"
#include "../core/funcargscontext_p.h"
#include "../core/string.h"
#include "../core/support.h"
#include "../core/type.h"
#include "../core/radefs_p.h"
#include "../x86/x86emithelper_p.h"
#include "../x86/x86emitter.h"
#include "../x86/x86formatter_p.h"
#include "../x86/x86instapi_p.h"

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

// x86::EmitHelper - Utilities
// ===========================

static constexpr OperandSignature regSizeToGpSignature[8 + 1] = {
  OperandSignature{0},
  OperandSignature{RegTraits<RegType::kGp8Lo>::kSignature},
  OperandSignature{RegTraits<RegType::kGp16>::kSignature},
  OperandSignature{0},
  OperandSignature{RegTraits<RegType::kGp32>::kSignature},
  OperandSignature{0},
  OperandSignature{0},
  OperandSignature{0},
  OperandSignature{RegTraits<RegType::kGp64>::kSignature}
};

[[nodiscard]]
static inline uint32_t getXmmMovInst(const FuncFrame& frame) {
  bool avx = frame.isAvxEnabled();
  bool aligned = frame.hasAlignedVecSR();

  return aligned ? (avx ? Inst::kIdVmovaps : Inst::kIdMovaps)
                 : (avx ? Inst::kIdVmovups : Inst::kIdMovups);
}

//! Converts `size` to a 'kmov?' instruction.
[[nodiscard]]
static inline uint32_t kmovInstFromSize(uint32_t size) noexcept {
  switch (size) {
    case  1: return Inst::kIdKmovb;
    case  2: return Inst::kIdKmovw;
    case  4: return Inst::kIdKmovd;
    case  8: return Inst::kIdKmovq;
    default: return Inst::kIdNone;
  }
}

[[nodiscard]]
static inline uint32_t makeCastOp(TypeId dst, TypeId src) noexcept {
  return (uint32_t(dst) << 8) | uint32_t(src);
}

// x86::EmitHelper - Emit Reg Move
// ===============================

ASMJIT_FAVOR_SIZE Error EmitHelper::emitRegMove(
  const Operand_& dst_,
  const Operand_& src_, TypeId typeId, const char* comment) {

  // Invalid or abstract TypeIds are not allowed.
  ASMJIT_ASSERT(TypeUtils::isValid(typeId) && !TypeUtils::isAbstract(typeId));

  Operand dst(dst_);
  Operand src(src_);

  InstId instId = Inst::kIdNone;
  uint32_t memFlags = 0;
  uint32_t overrideMemSize = 0;

  enum MemFlags : uint32_t {
    kDstMem = 0x1,
    kSrcMem = 0x2
  };

  // Detect memory operands and patch them to have the same size as the register. BaseCompiler always sets memory size
  // of allocs and spills, so it shouldn't be really necessary, however, after this function was separated from Compiler
  // it's better to make sure that the size is always specified, as we can use 'movzx' and 'movsx' that rely on it.
  if (dst.isMem()) { memFlags |= kDstMem; dst.as<Mem>().setSize(src.as<Mem>().size()); }
  if (src.isMem()) { memFlags |= kSrcMem; src.as<Mem>().setSize(dst.as<Mem>().size()); }

  switch (typeId) {
    case TypeId::kInt8:
    case TypeId::kUInt8:
    case TypeId::kInt16:
    case TypeId::kUInt16:
      // Special case - 'movzx' load.
      if (memFlags & kSrcMem) {
        instId = Inst::kIdMovzx;
        dst.setSignature(Reg::_signatureOf<RegType::kGp32>());
        break;
      }

      if (!memFlags) {
        // Change both destination and source registers to GPD (safer, no dependencies).
        dst.setSignature(Reg::_signatureOf<RegType::kGp32>());
        src.setSignature(Reg::_signatureOf<RegType::kGp32>());
      }
      [[fallthrough]];

    case TypeId::kInt32:
    case TypeId::kUInt32:
    case TypeId::kInt64:
    case TypeId::kUInt64:
      instId = Inst::kIdMov;
      break;

    case TypeId::kMmx32:
      instId = Inst::kIdMovd;
      if (memFlags) break;
      [[fallthrough]];

    case TypeId::kMmx64 : instId = Inst::kIdMovq ; break;
    case TypeId::kMask8 : instId = Inst::kIdKmovb; break;
    case TypeId::kMask16: instId = Inst::kIdKmovw; break;
    case TypeId::kMask32: instId = Inst::kIdKmovd; break;
    case TypeId::kMask64: instId = Inst::kIdKmovq; break;

    default: {
      TypeId scalarTypeId = TypeUtils::scalarOf(typeId);
      if (TypeUtils::isVec32(typeId) && memFlags) {
        overrideMemSize = 4;
        if (scalarTypeId == TypeId::kFloat32) {
          instId = _avxEnabled ? Inst::kIdVmovss : Inst::kIdMovss;
        }
        else {
          instId = _avxEnabled ? Inst::kIdVmovd : Inst::kIdMovd;
        }
        break;
      }

      if (TypeUtils::isVec64(typeId) && memFlags) {
        overrideMemSize = 8;
        if (scalarTypeId == TypeId::kFloat64) {
          instId = _avxEnabled ? Inst::kIdVmovsd : Inst::kIdMovsd;
        }
        else {
          instId = _avxEnabled ? Inst::kIdVmovq : Inst::kIdMovq;
        }
        break;
      }

      if (scalarTypeId == TypeId::kFloat32) {
        instId = _avxEnabled ? Inst::kIdVmovaps : Inst::kIdMovaps;
      }
      else if (scalarTypeId == TypeId::kFloat64) {
        instId = _avxEnabled ? Inst::kIdVmovapd : Inst::kIdMovapd;
      }
      else if (!_avx512Enabled) {
        instId = _avxEnabled ? Inst::kIdVmovdqa : Inst::kIdMovdqa;
      }
      else {
        instId = Inst::kIdVmovdqa32;
      }
      break;
    }
  }

  if (!instId) {
    return DebugUtils::errored(kErrorInvalidState);
  }

  if (overrideMemSize) {
    if (dst.isMem()) {
      dst.as<Mem>().setSize(overrideMemSize);
    }

    if (src.isMem()) {
      src.as<Mem>().setSize(overrideMemSize);
    }
  }

  _emitter->setInlineComment(comment);
  return _emitter->emit(instId, dst, src);
}

// x86::EmitHelper - Emit Arg Move
// ===============================

ASMJIT_FAVOR_SIZE Error EmitHelper::emitArgMove(
  const Reg& dst_, TypeId dstTypeId,
  const Operand_& src_, TypeId srcTypeId, const char* comment) {

  // Deduce optional `dstTypeId`, which may be `TypeId::kVoid` in some cases.
  if (dstTypeId == TypeId::kVoid) {
    dstTypeId = RegUtils::typeIdOf(dst_.regType());
  }

  // Invalid or abstract TypeIds are not allowed.
  ASMJIT_ASSERT(TypeUtils::isValid(dstTypeId) && !TypeUtils::isAbstract(dstTypeId));
  ASMJIT_ASSERT(TypeUtils::isValid(srcTypeId) && !TypeUtils::isAbstract(srcTypeId));

  Reg dst(dst_.as<Reg>());
  Operand src(src_);

  uint32_t dstSize = TypeUtils::sizeOf(dstTypeId);
  uint32_t srcSize = TypeUtils::sizeOf(srcTypeId);

  InstId instId = Inst::kIdNone;

  // Not a real loop, just 'break' is nicer than 'goto'.
  for (;;) {
    if (TypeUtils::isInt(dstTypeId)) {
      // Sign extend.
      if (TypeUtils::isInt(srcTypeId)) {
        uint32_t castOp = makeCastOp(dstTypeId, srcTypeId);

        if (castOp == makeCastOp(TypeId::kInt16, TypeId::kInt8 ) ||
            castOp == makeCastOp(TypeId::kInt32, TypeId::kInt8 ) ||
            castOp == makeCastOp(TypeId::kInt64, TypeId::kInt8 ) ||
            castOp == makeCastOp(TypeId::kInt32, TypeId::kInt16) ||
            castOp == makeCastOp(TypeId::kInt64, TypeId::kInt16) ||
            castOp == makeCastOp(TypeId::kInt64, TypeId::kInt32)) {
          // Sign extend by using 'movsx' or 'movsxd'.
          instId = (castOp == makeCastOp(TypeId::kInt64, TypeId::kInt32)) ? Inst::kIdMovsxd : Inst::kIdMovsx;

          dst.setSignature(regSizeToGpSignature[dstSize]);
          if (src.isReg()) {
            src.setSignature(regSizeToGpSignature[srcSize]);
          }
          break;
        }
      }

      // Zero extend.
      if (TypeUtils::isInt(srcTypeId) || src_.isMem()) {
        uint32_t movSize = Support::min(srcSize, dstSize);
        if (movSize <= 4) {
          dstSize = 4;
        }

        // Zero extend by using 'movzx' or 'mov'.
        instId = movSize < 4 ? Inst::kIdMovzx : Inst::kIdMov;
        srcSize = Support::min(srcSize, movSize);

        dst.setSignature(regSizeToGpSignature[dstSize]);
        if (src.isReg()) {
          src.setSignature(regSizeToGpSignature[srcSize]);
        }
        break;
      }

      // NOTE: The previous branch caught all memory sources, from here it's always register to register conversion,
      // so catch the remaining cases.
      srcSize = Support::min(srcSize, dstSize);

      if (TypeUtils::isMmx(srcTypeId)) {
        // 64-bit move.
        instId = Inst::kIdMovq;
        if (srcSize == 8) {
          break;
        }

        // 32-bit move.
        instId = Inst::kIdMovd;
        dst.setSignature(Reg::_signatureOf<RegType::kGp32>());
        break;
      }

      if (TypeUtils::isMask(srcTypeId)) {
        instId = kmovInstFromSize(srcSize);
        dst.setSignature(srcSize <= 4 ? Reg::_signatureOf<RegType::kGp32>()
                                      : Reg::_signatureOf<RegType::kGp64>());
        break;
      }

      if (TypeUtils::isVec(srcTypeId)) {
        // 64-bit move.
        instId = _avxEnabled ? Inst::kIdVmovq : Inst::kIdMovq;
        if (srcSize == 8) {
          break;
        }

        // 32-bit move.
        instId = _avxEnabled ? Inst::kIdVmovd : Inst::kIdMovd;
        dst.setSignature(Reg::_signatureOf<RegType::kGp32>());
        break;
      }
    }

    if (TypeUtils::isMmx(dstTypeId)) {
      instId = Inst::kIdMovq;
      srcSize = Support::min(srcSize, dstSize);

      if (TypeUtils::isInt(srcTypeId) || src.isMem()) {
        // 64-bit move.
        if (srcSize == 8) {
          break;
        }

        // 32-bit move.
        instId = Inst::kIdMovd;
        if (src.isReg()) {
          src.setSignature(Reg::_signatureOf<RegType::kGp32>());
        }
        break;
      }

      if (TypeUtils::isMmx(srcTypeId)) {
        break;
      }

      // This will hurt if AVX is enabled.
      instId = Inst::kIdMovdq2q;
      if (TypeUtils::isVec(srcTypeId)) {
        break;
      }
    }

    if (TypeUtils::isMask(dstTypeId)) {
      srcSize = Support::min(srcSize, dstSize);

      if (TypeUtils::isInt(srcTypeId) || TypeUtils::isMask(srcTypeId) || src.isMem()) {
        instId = kmovInstFromSize(srcSize);
        if (src.isGp() && srcSize <= 4) {
          src.setSignature(Reg::_signatureOf<RegType::kGp32>());
        }
        break;
      }
    }

    if (TypeUtils::isVec(dstTypeId)) {
      // By default set destination to XMM, will be set to YMM|ZMM if needed.
      dst.setSignature(Reg::_signatureOf<RegType::kVec128>());

      // This will hurt if AVX is enabled.
      if (src.isMmReg()) {
        // 64-bit move.
        instId = Inst::kIdMovq2dq;
        break;
      }

      // Argument conversion.
      TypeId dstScalarId = TypeUtils::scalarOf(dstTypeId);
      TypeId srcScalarId = TypeUtils::scalarOf(srcTypeId);

      if (dstScalarId == TypeId::kFloat32 && srcScalarId == TypeId::kFloat64) {
        srcSize = Support::min(dstSize * 2, srcSize);
        dstSize = srcSize / 2;

        if (srcSize <= 8) {
          instId = _avxEnabled ? Inst::kIdVcvtss2sd : Inst::kIdCvtss2sd;
        }
        else {
          instId = _avxEnabled ? Inst::kIdVcvtps2pd : Inst::kIdCvtps2pd;
        }

        if (dstSize == 32) {
          dst.setSignature(Reg::_signatureOf<RegType::kVec256>());
        }
        if (src.isReg()) {
          src.setSignature(RegUtils::signatureOfVecSize(srcSize));
        }
        break;
      }

      if (dstScalarId == TypeId::kFloat64 && srcScalarId == TypeId::kFloat32) {
        srcSize = Support::min(dstSize, srcSize * 2) / 2;
        dstSize = srcSize * 2;

        if (srcSize <= 4) {
          instId = _avxEnabled ? Inst::kIdVcvtsd2ss : Inst::kIdCvtsd2ss;
        }
        else {
          instId = _avxEnabled ? Inst::kIdVcvtpd2ps : Inst::kIdCvtpd2ps;
        }

        dst.setSignature(RegUtils::signatureOfVecSize(dstSize));
        if (src.isReg() && srcSize >= 32) {
          src.setSignature(Reg::_signatureOf<RegType::kVec256>());
        }
        break;
      }

      srcSize = Support::min(srcSize, dstSize);
      if (src.isGp() || src.isMem()) {
        // 32-bit move.
        if (srcSize <= 4) {
          instId = _avxEnabled ? Inst::kIdVmovd : Inst::kIdMovd;
          if (src.isReg()) {
            src.setSignature(Reg::_signatureOf<RegType::kGp32>());
          }
          break;
        }

        // 64-bit move.
        if (srcSize == 8) {
          instId = _avxEnabled ? Inst::kIdVmovq : Inst::kIdMovq;
          break;
        }
      }

      if (src.isVec() || src.isMem()) {
        instId = _avxEnabled ? Inst::kIdVmovaps : Inst::kIdMovaps;

        if (src.isMem() && srcSize < _emitter->environment().stackAlignment()) {
          instId = _avxEnabled ? Inst::kIdVmovups : Inst::kIdMovups;
        }

        OperandSignature signature = RegUtils::signatureOfVecSize(srcSize);
        dst.setSignature(signature);
        if (src.isReg()) {
          src.setSignature(signature);
        }
        break;
      }
    }

    return DebugUtils::errored(kErrorInvalidState);
  }

  if (src.isMem())
    src.as<Mem>().setSize(srcSize);

  _emitter->setInlineComment(comment);
  return _emitter->emit(instId, dst, src);
}

Error EmitHelper::emitRegSwap(
  const Reg& a,
  const Reg& b, const char* comment) {

  if (a.isGp() && b.isGp()) {
    _emitter->setInlineComment(comment);
    return _emitter->emit(Inst::kIdXchg, a, b);
  }
  else {
    return DebugUtils::errored(kErrorInvalidState);
  }
}

// x86::EmitHelper - Emit Prolog & Epilog
// ======================================

static inline Error X86Internal_setupSaveRestoreInfo(RegGroup group, const FuncFrame& frame, Reg& xReg, uint32_t& xInst, uint32_t& xSize) noexcept {
  switch (group) {
    case RegGroup::kVec:
      xReg = xmm(0);
      xInst = getXmmMovInst(frame);
      xSize = xReg.size();
      return kErrorOk;

    case RegGroup::kMask:
      xReg = k(0);
      xInst = Inst::kIdKmovq;
      xSize = xReg.size();
      return kErrorOk;

    case RegGroup::kX86_MM:
      xReg = mm(0);
      xInst = Inst::kIdMovq;
      xSize = xReg.size();
      return kErrorOk;

    default:
      // This would be a bug in AsmJit if hit.
      return DebugUtils::errored(kErrorInvalidState);
  }
}

ASMJIT_FAVOR_SIZE Error EmitHelper::emitProlog(const FuncFrame& frame) {
  Emitter* emitter = _emitter->as<Emitter>();
  uint32_t gpSaved = frame.savedRegs(RegGroup::kGp);

  Gp zsp = emitter->zsp();   // ESP|RSP register.
  Gp zbp = emitter->zbp();   // EBP|RBP register.
  Gp gpReg = zsp;            // General purpose register (temporary).
  Gp saReg = zsp;            // Stack-arguments base pointer.

  // Emit: 'endbr32' or 'endbr64' (indirect branch protection).
  if (frame.hasIndirectBranchProtection()) {
    InstId instId = emitter->is32Bit() ? Inst::kIdEndbr32 : Inst::kIdEndbr64;
    ASMJIT_PROPAGATE(emitter->emit(instId));
  }

  // Emit: 'push zbp'
  //       'mov  zbp, zsp'.
  if (frame.hasPreservedFP()) {
    gpSaved &= ~Support::bitMask(Gp::kIdBp);
    ASMJIT_PROPAGATE(emitter->push(zbp));
    ASMJIT_PROPAGATE(emitter->mov(zbp, zsp));
  }

  // Emit: 'push gp' sequence.
  {
    Support::BitWordIterator<RegMask> it(gpSaved);
    while (it.hasNext()) {
      gpReg.setId(it.next());
      ASMJIT_PROPAGATE(emitter->push(gpReg));
    }
  }

  // Emit: 'mov saReg, zsp'.
  uint32_t saRegId = frame.saRegId();
  if (saRegId != Reg::kIdBad && saRegId != Gp::kIdSp) {
    saReg.setId(saRegId);
    if (frame.hasPreservedFP()) {
      if (saRegId != Gp::kIdBp) {
        ASMJIT_PROPAGATE(emitter->mov(saReg, zbp));
      }
    }
    else {
      ASMJIT_PROPAGATE(emitter->mov(saReg, zsp));
    }
  }

  // Emit: 'and zsp, StackAlignment'.
  if (frame.hasDynamicAlignment()) {
    ASMJIT_PROPAGATE(emitter->and_(zsp, -int32_t(frame.finalStackAlignment())));
  }

  // Emit: 'sub zsp, StackAdjustment'.
  if (frame.hasStackAdjustment()) {
    ASMJIT_PROPAGATE(emitter->sub(zsp, frame.stackAdjustment()));
  }

  // Emit: 'mov [zsp + DAOffset], saReg'.
  if (frame.hasDynamicAlignment() && frame.hasDAOffset()) {
    Mem saMem = ptr(zsp, int32_t(frame.daOffset()));
    ASMJIT_PROPAGATE(emitter->mov(saMem, saReg));
  }

  // Emit 'movxxx [zsp + X], {[x|y|z]mm, k}'.
  {
    Mem xBase = ptr(zsp, int32_t(frame.extraRegSaveOffset()));

    for (RegGroup group : Support::EnumValues<RegGroup, RegGroup(1), RegGroup::kMaxVirt>{}) {
      Support::BitWordIterator<RegMask> it(frame.savedRegs(group));
      if (it.hasNext()) {
        Reg xReg;
        uint32_t xInst = 0;
        uint32_t xSize = 0;
        ASMJIT_PROPAGATE(X86Internal_setupSaveRestoreInfo(group, frame, xReg, xInst, xSize));
        do {
          xReg.setId(it.next());
          ASMJIT_PROPAGATE(emitter->emit(xInst, xBase, xReg));
          xBase.addOffsetLo32(int32_t(xSize));
        } while (it.hasNext());
      }
    }
  }

  return kErrorOk;
}

ASMJIT_FAVOR_SIZE Error EmitHelper::emitEpilog(const FuncFrame& frame) {
  Emitter* emitter = _emitter->as<Emitter>();

  uint32_t i;
  uint32_t regId;

  uint32_t registerSize = emitter->registerSize();
  uint32_t gpSaved = frame.savedRegs(RegGroup::kGp);

  Gp zsp = emitter->zsp();   // ESP|RSP register.
  Gp zbp = emitter->zbp();   // EBP|RBP register.
  Gp gpReg = emitter->zsp(); // General purpose register (temporary).

  // Don't emit 'pop zbp' in the pop sequence, this case is handled separately.
  if (frame.hasPreservedFP()) {
    gpSaved &= ~Support::bitMask(Gp::kIdBp);
  }

  // Emit 'movxxx {[x|y|z]mm, k}, [zsp + X]'.
  {
    Mem xBase = ptr(zsp, int32_t(frame.extraRegSaveOffset()));

    for (RegGroup group : Support::EnumValues<RegGroup, RegGroup(1), RegGroup::kMaxVirt>{}) {
      Support::BitWordIterator<RegMask> it(frame.savedRegs(group));
      if (it.hasNext()) {
        Reg xReg;
        uint32_t xInst;
        uint32_t xSize;
        ASMJIT_PROPAGATE(X86Internal_setupSaveRestoreInfo(group, frame, xReg, xInst, xSize));
        do {
          xReg.setId(it.next());
          ASMJIT_PROPAGATE(emitter->emit(xInst, xReg, xBase));
          xBase.addOffsetLo32(int32_t(xSize));
        } while (it.hasNext());
      }
    }
  }

  // Emit 'emms' and/or 'vzeroupper'.
  if (frame.hasMmxCleanup()) {
    ASMJIT_PROPAGATE(emitter->emms());
  }

  if (frame.hasAvxCleanup()) {
    ASMJIT_PROPAGATE(emitter->vzeroupper());
  }

  if (frame.hasPreservedFP()) {
    // Emit 'mov zsp, zbp' or 'lea zsp, [zbp - x]'
    int32_t count = int32_t(frame.pushPopSaveSize() - registerSize);
    if (!count) {
      ASMJIT_PROPAGATE(emitter->mov(zsp, zbp));
    }
    else {
      ASMJIT_PROPAGATE(emitter->lea(zsp, ptr(zbp, -count)));
    }
  }
  else {
    if (frame.hasDynamicAlignment() && frame.hasDAOffset()) {
      // Emit 'mov zsp, [zsp + DsaSlot]'.
      Mem saMem = ptr(zsp, int32_t(frame.daOffset()));
      ASMJIT_PROPAGATE(emitter->mov(zsp, saMem));
    }
    else if (frame.hasStackAdjustment()) {
      // Emit 'add zsp, StackAdjustment'.
      ASMJIT_PROPAGATE(emitter->add(zsp, int32_t(frame.stackAdjustment())));
    }
  }

  // Emit 'pop gp' sequence.
  if (gpSaved) {
    i = gpSaved;
    regId = 16;

    do {
      regId--;
      if (i & 0x8000) {
        gpReg.setId(regId);
        ASMJIT_PROPAGATE(emitter->pop(gpReg));
      }
      i <<= 1;
    } while (regId != 0);
  }

  // Emit 'pop zbp'.
  if (frame.hasPreservedFP()) {
    ASMJIT_PROPAGATE(emitter->pop(zbp));
  }

  // Emit 'ret' or 'ret x'.
  if (frame.hasCalleeStackCleanup()) {
    ASMJIT_PROPAGATE(emitter->emit(Inst::kIdRet, int(frame.calleeStackCleanup())));
  }
  else {
    ASMJIT_PROPAGATE(emitter->emit(Inst::kIdRet));
  }

  return kErrorOk;
}

static Error ASMJIT_CDECL Emitter_emitProlog(BaseEmitter* emitter, const FuncFrame& frame) {
  EmitHelper emitHelper(emitter, frame.isAvxEnabled(), frame.isAvx512Enabled());
  return emitHelper.emitProlog(frame);
}

static Error ASMJIT_CDECL Emitter_emitEpilog(BaseEmitter* emitter, const FuncFrame& frame) {
  EmitHelper emitHelper(emitter, frame.isAvxEnabled(), frame.isAvx512Enabled());
  return emitHelper.emitEpilog(frame);
}

static Error ASMJIT_CDECL Emitter_emitArgsAssignment(BaseEmitter* emitter, const FuncFrame& frame, const FuncArgsAssignment& args) {
  EmitHelper emitHelper(emitter, frame.isAvxEnabled(), frame.isAvx512Enabled());
  return emitHelper.emitArgsAssignment(frame, args);
}

void initEmitterFuncs(BaseEmitter* emitter) noexcept {
  emitter->_funcs.emitProlog = Emitter_emitProlog;
  emitter->_funcs.emitEpilog = Emitter_emitEpilog;
  emitter->_funcs.emitArgsAssignment = Emitter_emitArgsAssignment;

#ifndef ASMJIT_NO_LOGGING
  emitter->_funcs.formatInstruction = FormatterInternal::formatInstruction;
#endif
}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_X86
