/*
    SPDX-FileCopyrightText: 2019-2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_JNIPP_H
#define KANDROIDEXTRAS_JNIPP_H

/** @file jnipp.h
 *  Preprocessor macro implementation details.
 */

///@cond internal

// determine how many elements are in __VA_ARGS__
#define JNI_PP_NARG(...) JNI_PP_NARG_(__VA_ARGS__ __VA_OPT__(, ) JNI_PP_RSEQ_N())
#define JNI_PP_NARG_(...) JNI_PP_ARG_N(__VA_ARGS__)
#define JNI_PP_ARG_N(_1, _2, _3, _4, _5, _6, _7, N, ...) N
#define JNI_PP_RSEQ_N() 7, 6, 5, 4, 3, 2, 1, 0

// preprocessor-level token concat
#define JNI_PP_CONCAT(arg1, arg2) JNI_PP_CONCAT1(arg1, arg2)
#define JNI_PP_CONCAT1(arg1, arg2) JNI_PP_CONCAT2(arg1, arg2)
#define JNI_PP_CONCAT2(arg1, arg2) arg1##arg2

///@endcond

#endif // KANDROIDEXTRAS_JNIPP_H
