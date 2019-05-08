// Tencent is pleased to support the open source community by making GAutomator available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.


/*
 * assert.h
 *
 *  Created on: 2012-8-6
 *      Author: yerungui
 */

#ifndef MTAF_COMM_COMM_ASSERT_H_
#define MTAF_COMM_COMM_ASSERT_H_

#include <stdarg.h>
#include <sys/cdefs.h>

#if (!__ISO_C_VISIBLE >= 1999)
#error "C Version < C99"
#endif

#define MTAF_ASSERT(e) ((e) ? (void)0 : __ASSERT(__FILE__, __LINE__, __func__, #e))
#define MTAF_ASSERT2(e, fmt, ...) ((e) ? (void)0 : __ASSERT2(__FILE__, __LINE__, __func__, #e, fmt, ##__VA_ARGS__))
#define MTAF_ASSERTV2(e, fmt, valist) ((e) ? (void)0 : __ASSERTV2(__FILE__, __LINE__, __func__, #e, fmt, valist))

__BEGIN_DECLS
void MTAF_ENABLE_ASSERT();
void MTAF_DISABLE_ASSERT();
int MTAF_IS_ASSERT_ENABLE();

__attribute__((__nonnull__(1, 3, 4))) void __ASSERT(const char *, int, const char *, const char *);
__attribute__((__nonnull__(1, 3, 4, 5))) __attribute__((__format__(printf, 5, 6))) void __ASSERT2(const char *, int, const char *, const char *, const char *, ...);
__attribute__((__nonnull__(1, 3, 4, 5))) __attribute__((__format__(printf, 5, 0))) void __ASSERTV2(const char *, int, const char *, const char *, const char *, va_list);
__END_DECLS

#endif /* MTAF_COMM_COMM_ASSERT_H_ */
