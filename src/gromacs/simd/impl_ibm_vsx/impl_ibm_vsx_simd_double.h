/*
 * This file is part of the GROMACS molecular simulation package.
 *
 * Copyright (c) 2014,2015, by the GROMACS development team, led by
 * Mark Abraham, David van der Spoel, Berk Hess, and Erik Lindahl,
 * and including many others, as listed in the AUTHORS file in the
 * top-level source directory and at http://www.gromacs.org.
 *
 * GROMACS is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * GROMACS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GROMACS; if not, see
 * http://www.gnu.org/licenses, or write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
 *
 * If you want to redistribute modifications to GROMACS, please
 * consider that scientific software is very special. Version
 * control is crucial - bugs must be traceable. We will be happy to
 * consider code for inclusion in the official distribution, but
 * derived work must not be called official GROMACS. Details are found
 * in the README & COPYING files - if they are missing, get the
 * official version at http://www.gromacs.org.
 *
 * To help us fund GROMACS development, we humbly ask that you cite
 * the research papers on the package. Check out http://www.gromacs.org.
 */

#ifndef GMX_SIMD_IMPLEMENTATION_IBM_VSX_SIMD_DOUBLE_H
#define GMX_SIMD_IMPLEMENTATION_IBM_VSX_SIMD_DOUBLE_H

#include "config.h"

#include "gromacs/utility/basedefinitions.h"

#include "impl_ibm_vsx_definitions.h"

namespace gmx
{

class SimdDouble
{
    public:
        SimdDouble() {}

        // gcc-4.9 does not recognize that we use the parameter
        SimdDouble(double gmx_unused d) : simdInternal_(vec_splats(d)) {}

        // Internal utility constructor to simplify return statements
        SimdDouble(__vector double simd) : simdInternal_(simd) {}

        __vector double  simdInternal_;
};

class SimdDInt32
{
    public:
        SimdDInt32() {}

        // gcc-4.9 does not recognize that we use the parameter
        SimdDInt32(std::int32_t gmx_unused i) : simdInternal_(vec_splats(i)) {}

        // Internal utility constructor to simplify return statements
        SimdDInt32(__vector signed int simd) : simdInternal_(simd) {}

        __vector signed int  simdInternal_;
};

class SimdDBool
{
    public:
        SimdDBool() {}

        SimdDBool(bool b) : simdInternal_(reinterpret_cast<__vector vsxBool long long>(vec_splats( b ? 0xFFFFFFFFFFFFFFFFULL : 0))) {}

        // Internal utility constructor to simplify return statements
        SimdDBool(__vector vsxBool long long simd) : simdInternal_(simd) {}

        __vector vsxBool long long simdInternal_;
};

class SimdDIBool
{
    public:
        SimdDIBool() {}

        SimdDIBool(bool b) : simdInternal_(reinterpret_cast<__vector vsxBool int>(vec_splats( b ? 0xFFFFFFFF : 0))) {}

        // Internal utility constructor to simplify return statements
        SimdDIBool(__vector vsxBool int simd) : simdInternal_(simd) {}

        __vector vsxBool int  simdInternal_;
};

// The VSX load & store operations are a bit of a mess. The interface is different
// for xlc version 12, xlc version 13, and gcc. Long-term IBM recommends
// simply using pointer dereferencing both for aligned and unaligned loads.
// That's nice, but unfortunately xlc still bugs out when the pointer is
// not aligned. Sticking to vec_xl/vec_xst isn't a solution either, since
// that appears to be buggy for some _aligned_ loads :-)
//
// For now, we use pointer dereferencing for all aligned load/stores, and
// for unaligned ones with gcc. On xlc we use vec_xlw4/vec_xstw4 for
// unaligned memory operations. The latest docs recommend using the overloaded
// vec_xl/vec_xst, but that is not supported on xlc version 12. We'll
// revisit things once xlc is a bit more stable - for now you probably want
// to stick to gcc...

static inline SimdDouble gmx_simdcall
load(const double *m)
{
    return {
               *reinterpret_cast<const __vector double *>(m)
    };
}

static inline void gmx_simdcall
store(double *m, SimdDouble a)
{
    *reinterpret_cast<__vector double *>(m) = a.simdInternal_;
}

static inline SimdDouble gmx_simdcall
loadU(const double *m)
{
#if defined(__ibmxl__) || defined(__xlC__)
    return {
               vec_xlw4(0, const_cast<double *>(m))
    }
#else
    return {
               *reinterpret_cast<const __vector double *>(m)
    };
#endif
}

static inline void gmx_simdcall
storeU(double *m, SimdDouble a)
{
#if defined(__ibmxl__) || defined(__xlC__)
    vec_xstw4(a.simdInternal_, 0, m);
#else
    *reinterpret_cast<__vector double *>(m) = a.simdInternal_;
#endif
}

static inline SimdDouble gmx_simdcall
setZeroD()
{
    return {
               vec_splats(0.0)
    };
}

static inline SimdDInt32 gmx_simdcall
loadDI(const std::int32_t * m)
{
    __vector signed int          t0, t1;
    const __vector unsigned char perm = { 0, 1, 2, 3, 0, 1, 2, 3, 16, 17, 18, 19, 16, 17, 18, 19 };
    t0 = vec_splats(m[0]);
    t1 = vec_splats(m[1]);
    return {
               vec_perm(t0, t1, perm)
    };
}

// gcc-4.9 does not understand that arguments to vec_extract() are used
static inline void gmx_simdcall
store(std::int32_t * m, SimdDInt32 gmx_unused x)
{
    m[0] = vec_extract(x.simdInternal_, 0);
    m[1] = vec_extract(x.simdInternal_, 2);
}

static inline SimdDInt32 gmx_simdcall
loadUDI(const std::int32_t *m)
{
    return loadDI(m);
}

static inline void gmx_simdcall
storeU(std::int32_t * m, SimdDInt32 a)
{
    return store(m, a);
}

static inline SimdDInt32 gmx_simdcall
setZeroDI()
{
    return {
               vec_splats(static_cast<int>(0))
    };
}

// gcc-4.9 does not detect that vec_extract() uses its argument
template<int index>
static inline std::int32_t gmx_simdcall
extract(SimdDInt32 gmx_unused a)
{
    return vec_extract(a.simdInternal_, 2*index);
}

static inline SimdDouble gmx_simdcall
operator&(SimdDouble a, SimdDouble b)
{
    return {
               vec_and(a.simdInternal_, b.simdInternal_)
    };
}

static inline SimdDouble gmx_simdcall
andNot(SimdDouble a, SimdDouble b)
{
    return {
               vec_andc(b.simdInternal_, a.simdInternal_)
    };
}

static inline SimdDouble gmx_simdcall
operator|(SimdDouble a, SimdDouble b)
{
    return {
               vec_or(a.simdInternal_, b.simdInternal_)
    };
}

static inline SimdDouble gmx_simdcall
operator^(SimdDouble a, SimdDouble b)
{
    return {
               vec_xor(a.simdInternal_, b.simdInternal_)
    };
}

static inline SimdDouble gmx_simdcall
operator+(SimdDouble a, SimdDouble b)
{
    return {
               vec_add(a.simdInternal_, b.simdInternal_)
    };
}

static inline SimdDouble gmx_simdcall
operator-(SimdDouble a, SimdDouble b)
{
    return {
               vec_sub(a.simdInternal_, b.simdInternal_)
    };
}

static inline SimdDouble gmx_simdcall
operator-(SimdDouble x)
{
    return {
               -x.simdInternal_
    };
}

static inline SimdDouble gmx_simdcall
operator*(SimdDouble a, SimdDouble b)
{
    return {
               vec_mul(a.simdInternal_, b.simdInternal_)
    };
}

static inline SimdDouble gmx_simdcall
fma(SimdDouble a, SimdDouble b, SimdDouble c)
{
    return {
               vec_madd(a.simdInternal_, b.simdInternal_, c.simdInternal_)
    };
}

static inline SimdDouble gmx_simdcall
fms(SimdDouble a, SimdDouble b, SimdDouble c)
{
    return {
               vec_msub(a.simdInternal_, b.simdInternal_, c.simdInternal_)
    };
}

static inline SimdDouble gmx_simdcall
fnma(SimdDouble a, SimdDouble b, SimdDouble c)
{
    return {
               vec_nmsub(a.simdInternal_, b.simdInternal_, c.simdInternal_)
    };
}

static inline SimdDouble gmx_simdcall
fnms(SimdDouble a, SimdDouble b, SimdDouble c)
{
    return {
               vec_nmadd(a.simdInternal_, b.simdInternal_, c.simdInternal_)
    };
}

static inline SimdDouble gmx_simdcall
rsqrt(SimdDouble x)
{
    return {
               vec_rsqrte(x.simdInternal_)
    };
}

static inline SimdDouble gmx_simdcall
rcp(SimdDouble x)
{
    return {
               vec_re(x.simdInternal_)
    };
}

static inline SimdDouble gmx_simdcall
maskAdd(SimdDouble a, SimdDouble b, SimdDBool m)
{
    return {
               vec_add(a.simdInternal_, vec_and(b.simdInternal_, reinterpret_cast<__vector double>(m.simdInternal_)))
    };
}

static inline SimdDouble gmx_simdcall
maskzMul(SimdDouble a, SimdDouble b, SimdDBool m)
{
    SimdDouble prod = a * b;

    return {
               vec_and(prod.simdInternal_, reinterpret_cast<__vector double>(m.simdInternal_))
    };
}

static inline SimdDouble gmx_simdcall
maskzFma(SimdDouble a, SimdDouble b, SimdDouble c, SimdDBool m)
{
    SimdDouble prod = fma(a, b, c);

    return {
               vec_and(prod.simdInternal_, reinterpret_cast<__vector double>(m.simdInternal_))
    };
}

static inline SimdDouble gmx_simdcall
maskzRsqrt(SimdDouble x, SimdDBool m)
{
#ifndef NDEBUG
    x.simdInternal_ = vec_sel(vec_splats(1.0f), x.simdInternal_, m.simdInternal_);
#endif
    return {
               vec_and(vec_rsqrte(x.simdInternal_), reinterpret_cast<__vector double>(m.simdInternal_))
    };
}

static inline SimdDouble gmx_simdcall
maskzRcp(SimdDouble x, SimdDBool m)
{
#ifndef NDEBUG
    x.simdInternal_ = vec_sel(vec_splats(1.0f), x.simdInternal_, m.simdInternal_);
#endif
    return {
               vec_and(vec_re(x.simdInternal_), reinterpret_cast<__vector double>(m.simdInternal_))
    };
}

static inline SimdDouble gmx_simdcall
abs(SimdDouble x)
{
    return {
               vec_abs( x.simdInternal_ )
    };
}

static inline SimdDouble gmx_simdcall
max(SimdDouble a, SimdDouble b)
{
    return {
               vec_max(a.simdInternal_, b.simdInternal_)
    };
}

static inline SimdDouble gmx_simdcall
min(SimdDouble a, SimdDouble b)
{
    return {
               vec_min(a.simdInternal_, b.simdInternal_)
    };
}

static inline SimdDouble gmx_simdcall
round(SimdDouble x)
{
#if defined(__GNUC__) && !defined(__ibmxl__) && !defined(__xlC__)
// gcc up to at least version 4.9 does not have vec_round() in double precision - use inline asm
    __vector double res;
    __asm__ ("xvrdpi %x0,%x1" : "=wd" (res) : "wd" (x.simdInternal_));
    return {
               res
    };
#else
    return {
               vec_round( x.simdInternal_ )
    };
#endif
}

static inline SimdDouble gmx_simdcall
trunc(SimdDouble x)
{
    return {
               vec_trunc( x.simdInternal_ )
    };
}

static inline SimdDouble
frexp(SimdDouble value, SimdDInt32 * exponent)
{
    // Don't use _mm_set1_epi64x() - on MSVC it is only supported for 64-bit builds
    const __vector double     exponentMask = reinterpret_cast<__vector double>(vec_splats(0x7FF0000000000000ULL));
    const __vector signed int exponentBias = vec_splats(1022);
    const __vector double     half         = vec_splats(0.5);
    __vector signed int       iExponent;

    iExponent               = reinterpret_cast<__vector signed int>(vec_and(value.simdInternal_, exponentMask));
    // The data is in the upper half of each double (corresponding to elements 1 and 3).
    // First shift 52-32=20bits, and then permute to swap element 0 with 1 and element 2 with 3
    // For big endian they are in opposite order, so then we simply skip the swap.
    iExponent               = vec_sr(iExponent, vec_splats(20U));
#ifndef __BIG_ENDIAN__
    const __vector unsigned char perm = {4, 5, 6, 7, 0, 1, 2, 3, 12, 13, 14, 15, 8, 9, 10, 11};
    iExponent               = vec_perm(iExponent, iExponent, perm);
#endif
    iExponent               = vec_sub(iExponent, exponentBias);
    exponent->simdInternal_ = iExponent;

    return {
               vec_or(vec_andc(value.simdInternal_, exponentMask), half)
    };
}

static inline SimdDouble
ldexp(SimdDouble value, SimdDInt32 exponent)
{
    const __vector signed int    exponentBias = vec_splats(1023);
    __vector signed int          iExponent;
#ifdef __BIG_ENDIAN__
    const __vector unsigned char perm = {0, 1, 2, 3, 16, 17, 18, 19, 8, 9, 10, 11, 16, 17, 18, 19};
#else
    const __vector unsigned char perm = {16, 17, 18, 19, 0, 1, 2, 3, 16, 17, 18, 19, 8, 9, 10, 11};
#endif

    iExponent = vec_add(exponent.simdInternal_, exponentBias);
    // exponent is now present in pairs of integers; 0011.
    // Elements 0/2 already correspond to the upper half of each double,
    // so we only need to shift by another 52-32=20 bits.
    // The remaining elements are set to zero.
    iExponent = vec_sl(iExponent, vec_splats(20U));
    iExponent = vec_perm(iExponent, vec_splats(0), perm);

    return {
               vec_mul(value.simdInternal_, reinterpret_cast<__vector double>(iExponent))
    };
}

static inline double gmx_simdcall
reduce(SimdDouble x)
{
    const __vector unsigned char perm = { 8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7 };
#ifdef __xlC__
    /* old xlc version 12 does not understand vec_perm() with double arguments */
    x.simdInternal_ = vec_add(x.simdInternal_,
                              reinterpret_cast<__vector double>(vec_perm(reinterpret_cast<__vector signed int>(x.simdInternal_),
                                                                         reinterpret_cast<__vector signed int>(x.simdInternal_), perm)));
#else
    x.simdInternal_ = vec_add(x.simdInternal_, vec_perm(x.simdInternal_, x.simdInternal_, perm));
#endif
    return vec_extract(x.simdInternal_, 0);
}

static inline SimdDBool gmx_simdcall
operator==(SimdDouble a, SimdDouble b)
{
    return {
               vec_cmpeq(a.simdInternal_, b.simdInternal_)
    };
}

static inline SimdDBool gmx_simdcall
operator!=(SimdDouble a, SimdDouble b)
{
    return {
               reinterpret_cast<__vector vsxBool long long>(vec_or(reinterpret_cast<__vector signed int>(vec_cmpgt(a.simdInternal_, b.simdInternal_)),
                                                                   reinterpret_cast<__vector signed int>(vec_cmplt(a.simdInternal_, b.simdInternal_))))
    };
}

static inline SimdDBool gmx_simdcall
operator<(SimdDouble a, SimdDouble b)
{
    return {
               vec_cmplt(a.simdInternal_, b.simdInternal_)
    };
}

static inline SimdDBool gmx_simdcall
operator<=(SimdDouble a, SimdDouble b)
{
    return {
               vec_cmple(a.simdInternal_, b.simdInternal_)
    };
}

static inline SimdDBool gmx_simdcall
testBits(SimdDouble a)
{
#ifdef __POWER8_VECTOR__
    return {
               vec_cmpgt(reinterpret_cast<__vector unsigned long long>(a.simdInternal_), vec_splats(0ULL))
    };
#else
    return {
               reinterpret_cast<__vector vsxBool long long>(vec_nor(reinterpret_cast<__vector signed int>(vec_cmpeq(a.simdInternal_, vec_splats(0.0))), vec_splats(0)))
    };
#endif
}

static inline SimdDBool gmx_simdcall
operator&&(SimdDBool a, SimdDBool b)
{
    return {
               reinterpret_cast<__vector vsxBool long long>(vec_and(reinterpret_cast<__vector signed int>(a.simdInternal_), reinterpret_cast<__vector signed int>(b.simdInternal_)))
    };
}

static inline SimdDBool gmx_simdcall
operator||(SimdDBool a, SimdDBool b)
{
    return {
               reinterpret_cast<__vector vsxBool long long>(vec_or(reinterpret_cast<__vector signed int>(a.simdInternal_), reinterpret_cast<__vector signed int>(b.simdInternal_)))
    };
}

static inline bool gmx_simdcall
anyTrue(SimdDBool a)
{
    return vec_any_ne(reinterpret_cast<__vector vsxBool int>(a.simdInternal_), reinterpret_cast<__vector vsxBool int>(vec_splats(0)));
}

static inline SimdDouble gmx_simdcall
selectByMask(SimdDouble a, SimdDBool m)
{
    return {
               vec_and(a.simdInternal_, reinterpret_cast<__vector double>(m.simdInternal_))
    };
}

static inline SimdDouble gmx_simdcall
selectByNotMask(SimdDouble a, SimdDBool m)
{
    return {
               vec_andc(a.simdInternal_, reinterpret_cast<__vector double>(m.simdInternal_))
    };
}

static inline SimdDouble gmx_simdcall
blend(SimdDouble a, SimdDouble b, SimdDBool sel)
{
    return {
               vec_sel(a.simdInternal_, b.simdInternal_, sel.simdInternal_)
    };
}

static inline SimdDInt32 gmx_simdcall
operator<<(SimdDInt32 a, int n)
{
    return {
               vec_sl(a.simdInternal_, vec_splats(static_cast<unsigned int>(n)))
    };
}

static inline SimdDInt32 gmx_simdcall
operator>>(SimdDInt32 a, int n)
{
    return {
               vec_sr(a.simdInternal_, vec_splats(static_cast<unsigned int>(n)))
    };
}

static inline SimdDInt32 gmx_simdcall
operator&(SimdDInt32 a, SimdDInt32 b)
{
    return {
               vec_and(a.simdInternal_, b.simdInternal_)
    };
}

static inline SimdDInt32 gmx_simdcall
andNot(SimdDInt32 a, SimdDInt32 b)
{
    return {
               vec_andc(b.simdInternal_, a.simdInternal_)
    };
}

static inline SimdDInt32 gmx_simdcall
operator|(SimdDInt32 a, SimdDInt32 b)
{
    return {
               vec_or(a.simdInternal_, b.simdInternal_)
    };
}

static inline SimdDInt32 gmx_simdcall
operator^(SimdDInt32 a, SimdDInt32 b)
{
    return {
               vec_xor(a.simdInternal_, b.simdInternal_)
    };
}

static inline SimdDInt32 gmx_simdcall
operator+(SimdDInt32 a, SimdDInt32 b)
{
    return {
               vec_add(a.simdInternal_, b.simdInternal_)
    };
}

static inline SimdDInt32 gmx_simdcall
operator-(SimdDInt32 a, SimdDInt32 b)
{
    return {
               vec_sub(a.simdInternal_, b.simdInternal_)
    };
}

static inline SimdDInt32 gmx_simdcall
operator*(SimdDInt32 a, SimdDInt32 b)
{
    return {
               a.simdInternal_ * b.simdInternal_
    };
}

static inline SimdDIBool gmx_simdcall
operator==(SimdDInt32 a, SimdDInt32 b)
{
    return {
               vec_cmpeq(a.simdInternal_, b.simdInternal_)
    };
}

static inline SimdDIBool gmx_simdcall
testBits(SimdDInt32 a)
{
    return {
               vec_cmpgt( reinterpret_cast<__vector unsigned int>(a.simdInternal_), vec_splats(0U))
    };
}

static inline SimdDIBool gmx_simdcall
operator<(SimdDInt32 a, SimdDInt32 b)
{
    return {
               vec_cmplt(a.simdInternal_, b.simdInternal_)
    };
}

static inline SimdDIBool gmx_simdcall
operator&&(SimdDIBool a, SimdDIBool b)
{
    return {
               vec_and(a.simdInternal_, b.simdInternal_)
    };
}

static inline SimdDIBool gmx_simdcall
operator||(SimdDIBool a, SimdDIBool b)
{
    return {
               vec_or(a.simdInternal_, b.simdInternal_)
    };
}

static inline bool gmx_simdcall
anyTrue(SimdDIBool a)
{
    return vec_any_ne(a.simdInternal_, reinterpret_cast<__vector vsxBool int>(vec_splats(0)));
}

static inline SimdDInt32 gmx_simdcall
selectByMask(SimdDInt32 a, SimdDIBool m)
{
    return {
               vec_and(a.simdInternal_, reinterpret_cast<__vector signed int>(m.simdInternal_))
    };
}

static inline SimdDInt32 gmx_simdcall
selectByNotMask(SimdDInt32 a, SimdDIBool m)
{
    return {
               vec_andc(a.simdInternal_, reinterpret_cast<__vector signed int>(m.simdInternal_))
    };
}

static inline SimdDInt32 gmx_simdcall
blend(SimdDInt32 a, SimdDInt32 b, SimdDIBool sel)
{
    return {
               vec_sel(a.simdInternal_, b.simdInternal_, sel.simdInternal_)
    };
}

static inline SimdDInt32 gmx_simdcall
cvttR2I(SimdDouble a)
{
#if defined(__GNUC__) && !defined(__ibmxl__) && !defined(__xlC__)
// gcc up to at least version 4.9 is missing intrinsics for converting double to/from int - use inline asm
    const __vector unsigned char perm = {4, 5, 6, 7, 0, 1, 2, 3, 12, 13, 14, 15, 8, 9, 10, 11};
    __vector double              ix;

    __asm__ ("xvcvdpsxws %x0,%x1" : "=wa" (ix) : "wd" (a.simdInternal_));

    return {
               reinterpret_cast<__vector signed int>(vec_perm(ix, ix, perm))
    };
#else
    return {
               vec_cts(a.simdInternal_, 0)
    };
#endif
}

static inline SimdDInt32 gmx_simdcall
cvtR2I(SimdDouble a)
{
    return cvttR2I(round(a));
}

static inline SimdDouble gmx_simdcall
cvtI2R(SimdDInt32 a)
{
#if defined(__GNUC__) && !defined(__ibmxl__) && !defined(__xlC__)
// gcc up to at least version 4.9 is missing intrinsics for converting double to/from int - use inline asm
    const __vector unsigned char perm = {4, 5, 6, 7, 0, 1, 2, 3, 12, 13, 14, 15, 8, 9, 10, 11};
    __vector double              x;

    a.simdInternal_ = vec_perm(a.simdInternal_, a.simdInternal_, perm);
    __asm__ ("xvcvsxwdp %x0,%x1" : "=wd" (x) : "wa" (a.simdInternal_));

    return {
               x
    };
#else
    return {
               vec_ctd(a.simdInternal_, 0)
    };
#endif
}

static inline SimdDIBool gmx_simdcall
cvtB2IB(SimdDBool a)
{
    return {
               reinterpret_cast<__vector vsxBool int>(a.simdInternal_)
    };
}

static inline SimdDBool gmx_simdcall
cvtIB2B(SimdDIBool a)
{
    return {
               reinterpret_cast<__vector vsxBool long long>(a.simdInternal_)
    };
}

static inline void gmx_simdcall
cvtF2DD(SimdFloat f, SimdDouble *d0, SimdDouble *d1)
{
    __vector float fA, fB;
    fA  = vec_mergeh(f.simdInternal_, f.simdInternal_); /* 0011 */
    fB  = vec_mergel(f.simdInternal_, f.simdInternal_); /* 2233 */
#if defined(__GNUC__) && !defined(__ibmxl__) && !defined(__xlC__)
    // gcc-4.9 is missing double-to-float/float-to-double conversions.
    __asm__ ("xvcvspdp %x0,%x1" : "=wd" (d0->simdInternal_) : "wf" (fA));
    __asm__ ("xvcvspdp %x0,%x1" : "=wd" (d1->simdInternal_) : "wf" (fB));
#else
    d0->simdInternal_ = vec_cvf(fA);    /* 01 */
    d1->simdInternal_ = vec_cvf(fB);    /* 23 */
#endif
}

static inline SimdFloat gmx_simdcall
cvtDD2F(SimdDouble d0, SimdDouble d1)
{
    __vector float fA, fB, fC, fD, fE;
#if defined(__GNUC__) && !defined(__ibmxl__) && !defined(__xlC__)
    // gcc-4.9 is missing double-to-float/float-to-double conversions.
    __asm__ ("xvcvdpsp %x0,%x1" : "=wf" (fA) : "wd" (d0.simdInternal_));
    __asm__ ("xvcvdpsp %x0,%x1" : "=wf" (fB) : "wd" (d1.simdInternal_));
#else
    fA = vec_cvf(d0.simdInternal_); /* 0x1x */
    fB = vec_cvf(d1.simdInternal_); /* 2x3x */
#endif
    fC = vec_mergeh(fA, fB);        /* 02xx */
    fD = vec_mergel(fA, fB);        /* 13xx */
    fE = vec_mergeh(fC, fD);        /* 0123 */
    return {
               fE
    };
}

static inline SimdDouble gmx_simdcall
copysign(SimdDouble x, SimdDouble y)
{
#if defined(__GNUC__) && !defined(__ibmxl__) && !defined(__xlC__)
    __vector double res;
    __asm__ ("xvcpsgndp %x0,%x1,%x2" : "=wd" (res) : "wd" (y.simdInternal_), "wd" (x.simdInternal_));
    return {
               res
    };
#else
    return {
               vec_cpsgn(y.simdInternal_, x.simdInternal_)
    };
#endif
}

}      // namespace gmx

#endif // GMX_SIMD_IMPLEMENTATION_IBM_VSX_SIMD_DOUBLE_H