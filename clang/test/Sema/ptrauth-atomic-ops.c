// RUN: %clang_cc1 -triple arm64-apple-ios -fsyntax-only -verify -fptrauth-intrinsics -Wno-hip-deprecated-builtins %s
// RUN: %clang_cc1 -triple aarch64-linux-gnu -fsyntax-only -verify -fptrauth-intrinsics -Wno-hip-deprecated-builtins %s

#include <stdatomic.h>

int i;
int *__ptrauth(2, 1, 100) authenticated_ptr = &i;
int *__ptrauth(2, 0, 200) non_addr_discriminatedauthenticated_ptr = &i;
int * wat = &i;
#define ATOMIZE(p) (__typeof__(p) volatile _Atomic *)(long)(&p)

void f() {
  static int j = 1;
  int *raw = &i;
  int *raw2 = &i;
  int *__ptrauth(2, 1, 100) auth_result = &i;
  _Bool res;

  // Operations that do not compute on the value are supported, including with
  // address discrimination.
  __c11_atomic_init(ATOMIZE(authenticated_ptr), raw);
  __c11_atomic_store(ATOMIZE(authenticated_ptr), raw, memory_order_relaxed);
  raw = __c11_atomic_load(ATOMIZE(authenticated_ptr), memory_order_seq_cst);
  raw = __c11_atomic_exchange(ATOMIZE(authenticated_ptr), raw2, memory_order_seq_cst);
  res = __c11_atomic_compare_exchange_strong(ATOMIZE(authenticated_ptr), &raw, raw2, memory_order_seq_cst, memory_order_seq_cst);
  res = __c11_atomic_compare_exchange_weak(ATOMIZE(authenticated_ptr), &raw, raw2, memory_order_seq_cst, memory_order_seq_cst);

  raw = __atomic_load_n(&authenticated_ptr, memory_order_seq_cst);
  __atomic_store_n(&authenticated_ptr, raw, memory_order_seq_cst);
  raw = __atomic_exchange_n(&authenticated_ptr, raw2, memory_order_seq_cst);
  res = __atomic_compare_exchange_n(&authenticated_ptr, &raw, raw2, 0, memory_order_seq_cst, memory_order_seq_cst);
  raw = __scoped_atomic_load_n(&authenticated_ptr, memory_order_seq_cst, __MEMORY_SCOPE_SYSTEM);

  // The by-value forms yield the plain value type.
  _Static_assert(
      __builtin_types_compatible_p(
          __typeof__(__atomic_load_n(&authenticated_ptr,
                                     memory_order_seq_cst)) *,
          int **),
      "");
  _Static_assert(
      __builtin_types_compatible_p(
          __typeof__(__hip_atomic_load(&authenticated_ptr,
                                       memory_order_seq_cst, 1)) *,
          int **),
      "");

  // The generic by-address forms are not supported.
  __atomic_load(&authenticated_ptr, &raw, memory_order_seq_cst);
  // expected-error@-1 {{address argument to generic atomic operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,1,100) *' invalid)}}
  __atomic_store(&authenticated_ptr, &authenticated_ptr, memory_order_seq_cst);
  // expected-error@-1 {{address argument to generic atomic operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,1,100) *' invalid)}}
  __atomic_exchange(&authenticated_ptr, &raw2, &auth_result, memory_order_seq_cst);
  // expected-error@-1 {{address argument to generic atomic operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,1,100) *' invalid)}}
  res = __atomic_compare_exchange(&authenticated_ptr, &raw, &authenticated_ptr, 0, memory_order_seq_cst, memory_order_seq_cst);
  // expected-error@-1 {{address argument to generic atomic operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,1,100) *' invalid)}}
  __scoped_atomic_load(&authenticated_ptr, &raw, memory_order_seq_cst, __MEMORY_SCOPE_SYSTEM);
  // expected-error@-1 {{address argument to generic atomic operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,1,100) *' invalid)}}

  // Arithmetic atomic builtins compute on the signed value.
  __c11_atomic_fetch_add(ATOMIZE(authenticated_ptr), 1, memory_order_seq_cst);
  // expected-error@-1 {{address argument to atomic arithmetic operation must not be a pointer to a '__ptrauth'-qualified type ('volatile __ptrauth(2,1,100) _Atomic(int *) *' invalid)}}
  __atomic_fetch_add(ATOMIZE(authenticated_ptr), 3, memory_order_seq_cst);
  // expected-error@-1 {{address argument to atomic arithmetic operation must not be a pointer to a '__ptrauth'-qualified type ('volatile __ptrauth(2,1,100) _Atomic(int *) *' invalid)}}
  __atomic_fetch_sub(ATOMIZE(authenticated_ptr), 3, memory_order_seq_cst);
  // expected-error@-1 {{address argument to atomic arithmetic operation must not be a pointer to a '__ptrauth'-qualified type ('volatile __ptrauth(2,1,100) _Atomic(int *) *' invalid)}}
  __atomic_fetch_min(ATOMIZE(authenticated_ptr), 3, memory_order_seq_cst);
  // expected-error@-1 {{address argument to atomic arithmetic operation must not be a pointer to a '__ptrauth'-qualified type ('volatile __ptrauth(2,1,100) _Atomic(int *) *' invalid)}}
  __atomic_fetch_max(ATOMIZE(authenticated_ptr), 3, memory_order_seq_cst);
  // expected-error@-1 {{address argument to atomic arithmetic operation must not be a pointer to a '__ptrauth'-qualified type ('volatile __ptrauth(2,1,100) _Atomic(int *) *' invalid)}}
  __c11_atomic_fetch_and(ATOMIZE(authenticated_ptr), 1, memory_order_seq_cst);
  // expected-error@-1 {{address argument to atomic arithmetic operation must not be a pointer to a '__ptrauth'-qualified type ('volatile __ptrauth(2,1,100) _Atomic(int *) *' invalid)}}
  __atomic_fetch_and(ATOMIZE(authenticated_ptr), 3, memory_order_seq_cst);
  // expected-error@-1 {{address argument to atomic arithmetic operation must not be a pointer to a '__ptrauth'-qualified type ('volatile __ptrauth(2,1,100) _Atomic(int *) *' invalid)}}
  __atomic_fetch_or(ATOMIZE(authenticated_ptr), 3, memory_order_seq_cst);
  // expected-error@-1 {{address argument to atomic arithmetic operation must not be a pointer to a '__ptrauth'-qualified type ('volatile __ptrauth(2,1,100) _Atomic(int *) *' invalid)}}
  __atomic_fetch_xor(ATOMIZE(authenticated_ptr), 3, memory_order_seq_cst);
  // expected-error@-1 {{address argument to atomic arithmetic operation must not be a pointer to a '__ptrauth'-qualified type ('volatile __ptrauth(2,1,100) _Atomic(int *) *' invalid)}}
  __atomic_fetch_add(&authenticated_ptr, 1, memory_order_seq_cst);
  // expected-error@-1 {{address argument to atomic arithmetic operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,1,100) *' invalid)}}
  __atomic_fetch_max(&authenticated_ptr, 1, memory_order_seq_cst);
  // expected-error@-1 {{address argument to atomic arithmetic operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,1,100) *' invalid)}}

  // Implicit operations that compute on the value are not supported.
  (*ATOMIZE(authenticated_ptr))++;
  // expected-error@-1 {{atomic increment of '__ptrauth'-qualified type 'volatile __ptrauth(2,1,100) _Atomic(int *)' is not supported}}
  --(*ATOMIZE(authenticated_ptr));
  // expected-error@-1 {{atomic decrement of '__ptrauth'-qualified type 'volatile __ptrauth(2,1,100) _Atomic(int *)' is not supported}}
  (*ATOMIZE(authenticated_ptr)) += 1;
  // expected-error@-1 {{atomic compound assignment of '__ptrauth'-qualified type 'volatile __ptrauth(2,1,100) _Atomic(int *)' is not supported}}

  __c11_atomic_init(ATOMIZE(non_addr_discriminatedauthenticated_ptr), &j);
  __c11_atomic_store(ATOMIZE(non_addr_discriminatedauthenticated_ptr), 0, memory_order_relaxed);
  __c11_atomic_load(ATOMIZE(non_addr_discriminatedauthenticated_ptr), memory_order_seq_cst);
  __atomic_store(&j, ATOMIZE(non_addr_discriminatedauthenticated_ptr), memory_order_release);
  // expected-error@-1 {{incompatible pointer types passing 'volatile __ptrauth(2,0,200) _Atomic(int *) *' to parameter of type 'int *'}}
  __c11_atomic_exchange(ATOMIZE(j), ATOMIZE(non_addr_discriminatedauthenticated_ptr), memory_order_seq_cst);
  // expected-error@-1 {{incompatible pointer to integer conversion passing 'volatile __ptrauth(2,0,200) _Atomic(int *) *' to parameter of type 'typeof (j)' (aka 'int')}}
  __c11_atomic_fetch_add(ATOMIZE(non_addr_discriminatedauthenticated_ptr), ATOMIZE(j), memory_order_seq_cst);
  // expected-error@-1 {{address argument to atomic arithmetic operation must not be a pointer to a '__ptrauth'-qualified type ('volatile __ptrauth(2,0,200) _Atomic(int *) *' invalid)}}
  __c11_atomic_fetch_and(ATOMIZE(j), ATOMIZE(non_addr_discriminatedauthenticated_ptr), memory_order_seq_cst);
  // expected-error@-1 {{incompatible pointer to integer conversion passing 'volatile __ptrauth(2,0,200) _Atomic(int *) *' to parameter of type 'typeof (j)' (aka 'int')}}
  __atomic_fetch_add(&non_addr_discriminatedauthenticated_ptr, 1, memory_order_seq_cst);
  // expected-error@-1 {{address argument to atomic arithmetic operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,0,200) *' invalid)}}

  // __sync builtins are not supported.
  __sync_fetch_and_add(&authenticated_ptr, 1);
  // expected-error@-1 {{address argument to __sync operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,1,100) *' invalid)}}
  __sync_fetch_and_sub(&authenticated_ptr, 1);
  // expected-error@-1 {{address argument to __sync operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,1,100) *' invalid)}}
  __sync_fetch_and_or(&authenticated_ptr, 1);
  // expected-error@-1 {{address argument to __sync operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,1,100) *' invalid)}}
  __sync_fetch_and_and(&authenticated_ptr, 1);
  // expected-error@-1 {{address argument to __sync operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,1,100) *' invalid)}}
  __sync_fetch_and_xor(&authenticated_ptr, 1);
  // expected-error@-1 {{address argument to __sync operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,1,100) *' invalid)}}
  __sync_fetch_and_nand(&authenticated_ptr, 1);
  // expected-error@-1 {{address argument to __sync operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,1,100) *' invalid)}}

  __sync_add_and_fetch(&authenticated_ptr, 1);
  // expected-error@-1 {{address argument to __sync operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,1,100) *' invalid)}}
  __sync_sub_and_fetch(&authenticated_ptr, 1);
  // expected-error@-1 {{address argument to __sync operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,1,100) *' invalid)}}
  __sync_or_and_fetch(&authenticated_ptr, 1);
  // expected-error@-1 {{address argument to __sync operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,1,100) *' invalid)}}
  __sync_and_and_fetch(&authenticated_ptr, 1);
  // expected-error@-1 {{address argument to __sync operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,1,100) *' invalid)}}
  __sync_xor_and_fetch(&authenticated_ptr, 1);
  // expected-error@-1 {{address argument to __sync operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,1,100) *' invalid)}}
  __sync_nand_and_fetch(&authenticated_ptr, 1);
  // expected-error@-1 {{address argument to __sync operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,1,100) *' invalid)}}

  __sync_bool_compare_and_swap(&authenticated_ptr, 1, 0);
  // expected-error@-1 {{address argument to __sync operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,1,100) *' invalid)}}
  __sync_val_compare_and_swap(&authenticated_ptr, 1, 1);
  // expected-error@-1 {{address argument to __sync operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,1,100) *' invalid)}}

  __sync_lock_test_and_set(&authenticated_ptr, 1);
  // expected-error@-1 {{address argument to __sync operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,1,100) *' invalid)}}
  __sync_lock_release(&authenticated_ptr);
  // expected-error@-1 {{address argument to __sync operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,1,100) *' invalid)}}


int i = 0;

  __sync_fetch_and_add(&non_addr_discriminatedauthenticated_ptr, &i);
  // expected-error@-1 {{address argument to __sync operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,0,200) *' invalid)}}
  __sync_fetch_and_sub(&non_addr_discriminatedauthenticated_ptr, &i);
  // expected-error@-1 {{address argument to __sync operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,0,200) *' invalid)}}
  __sync_fetch_and_or(&non_addr_discriminatedauthenticated_ptr, &i);
  // expected-error@-1 {{address argument to __sync operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,0,200) *' invalid)}}
  __sync_fetch_and_and(&non_addr_discriminatedauthenticated_ptr, &i);
  // expected-error@-1 {{address argument to __sync operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,0,200) *' invalid)}}
  __sync_fetch_and_xor(&non_addr_discriminatedauthenticated_ptr, &i);
  // expected-error@-1 {{address argument to __sync operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,0,200) *' invalid)}}

  __sync_add_and_fetch(&non_addr_discriminatedauthenticated_ptr, &i);
  // expected-error@-1 {{address argument to __sync operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,0,200) *' invalid)}}
  __sync_sub_and_fetch(&non_addr_discriminatedauthenticated_ptr, &i);
  // expected-error@-1 {{address argument to __sync operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,0,200) *' invalid)}}
  __sync_or_and_fetch(&non_addr_discriminatedauthenticated_ptr, &i);
  // expected-error@-1 {{address argument to __sync operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,0,200) *' invalid)}}
  __sync_and_and_fetch(&non_addr_discriminatedauthenticated_ptr, &i);
  // expected-error@-1 {{address argument to __sync operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,0,200) *' invalid)}}
  __sync_xor_and_fetch(&non_addr_discriminatedauthenticated_ptr, &i);
  // expected-error@-1 {{address argument to __sync operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,0,200) *' invalid)}}

  __sync_bool_compare_and_swap(&non_addr_discriminatedauthenticated_ptr, &i, &i);
  // expected-error@-1 {{address argument to __sync operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,0,200) *' invalid)}}
  __sync_val_compare_and_swap(&non_addr_discriminatedauthenticated_ptr, &i, &i);
  // expected-error@-1 {{address argument to __sync operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,0,200) *' invalid)}}

  __sync_lock_test_and_set(&non_addr_discriminatedauthenticated_ptr, &i);
  // expected-error@-1 {{address argument to __sync operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,0,200) *' invalid)}}
  __sync_lock_release(&non_addr_discriminatedauthenticated_ptr);
  // expected-error@-1 {{address argument to __sync operation must not be a pointer to a '__ptrauth'-qualified type ('int *__ptrauth(2,0,200) *' invalid)}}
}
