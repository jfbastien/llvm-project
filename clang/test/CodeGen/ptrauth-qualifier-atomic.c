// RUN: %clang_cc1 -triple arm64e-apple-ios -fptrauth-calls -fptrauth-intrinsics -emit-llvm %s -o - | FileCheck %s

typedef int *__ptrauth(2, 1, 100) AuthPtr;
typedef long __ptrauth(2, 0, 200) AuthInt;
typedef void func_t(void);
typedef func_t *__ptrauth(2, 1, 100) AuthFunc;

_Atomic AuthPtr atomic_ptr;
_Atomic AuthPtr atomic_src;
AuthPtr auth_ptr;
_Atomic AuthInt atomic_int;
_Atomic AuthFunc atomic_func;

// CHECK-LABEL: define {{.*}}ptr @load_implicit()
// CHECK:         [[LOAD:%.*]] = load atomic ptr, ptr @atomic_ptr seq_cst, align 8
// CHECK-NEXT:    [[DISC:%.*]] = call i64 @llvm.ptrauth.blend(i64 ptrtoint (ptr @atomic_ptr to i64), i64 100)
// CHECK-NEXT:    [[CMP:%.*]] = icmp ne ptr [[LOAD]], null
// CHECK-NEXT:    br i1 [[CMP]], label %[[NONNULL:.*]], label %[[CONT:.*]]
// CHECK:       [[NONNULL]]:
// CHECK-NEXT:    [[INT:%.*]] = ptrtoint ptr [[LOAD]] to i64
// CHECK-NEXT:    [[AUTH:%.*]] = call i64 @llvm.ptrauth.auth(i64 [[INT]], i32 2, i64 [[DISC]])
// CHECK-NEXT:    [[AUTHED:%.*]] = inttoptr i64 [[AUTH]] to ptr
// CHECK:       [[CONT]]:
// CHECK-NEXT:    [[RESULT:%.*]] = phi ptr [ null, %entry ], [ [[AUTHED]], %[[NONNULL]] ]
// CHECK-NEXT:    ret ptr [[RESULT]]
int *load_implicit(void) { return atomic_ptr; }

// CHECK-LABEL: define {{.*}}void @store_implicit(ptr noundef %p)
// CHECK:         [[VALUE:%.*]] = load ptr, ptr %p.addr, align 8
// CHECK-NEXT:    [[DISC:%.*]] = call i64 @llvm.ptrauth.blend(i64 ptrtoint (ptr @atomic_ptr to i64), i64 100)
// CHECK:         [[INT:%.*]] = ptrtoint ptr [[VALUE]] to i64
// CHECK-NEXT:    [[SIGN:%.*]] = call i64 @llvm.ptrauth.sign(i64 [[INT]], i32 2, i64 [[DISC]])
// CHECK-NEXT:    [[SIGNED:%.*]] = inttoptr i64 [[SIGN]] to ptr
// CHECK:         [[RESULT:%.*]] = phi ptr [ null, %{{.*}} ], [ [[SIGNED]], %{{.*}} ]
// CHECK-NEXT:    store atomic ptr [[RESULT]], ptr @atomic_ptr seq_cst, align 8
void store_implicit(int *p) { atomic_ptr = p; }

// CHECK-LABEL: define {{.*}}void @store_c11(ptr noundef %p)
// CHECK:         [[VALUE:%.*]] = load ptr, ptr %p.addr, align 8
// CHECK-NEXT:    [[DISC:%.*]] = call i64 @llvm.ptrauth.blend(i64 ptrtoint (ptr @atomic_ptr to i64), i64 100)
// CHECK:         [[SIGN:%.*]] = call i64 @llvm.ptrauth.sign(i64 {{.*}}, i32 2, i64 [[DISC]])
// CHECK:         [[SIGNED:%.*]] = phi ptr [ null, %{{.*}} ], [ {{.*}}, %{{.*}} ]
// CHECK-NEXT:    store ptr [[SIGNED]], ptr %.atomictmp, align 8
// CHECK-NEXT:    [[ATOMIC:%.*]] = load ptr, ptr %.atomictmp, align 8
// CHECK-NOT:     call i64 @llvm.ptrauth
// CHECK-NEXT:    store atomic ptr [[ATOMIC]], ptr @atomic_ptr seq_cst, align 8
void store_c11(int *p) {
  __c11_atomic_store(&atomic_ptr, p, __ATOMIC_SEQ_CST);
}

// CHECK-LABEL: define {{.*}}void @store_c11_resign()
// CHECK:         [[VALUE:%.*]] = load atomic ptr, ptr @atomic_src seq_cst, align 8
// CHECK-NEXT:    [[SRCDISC:%.*]] = call i64 @llvm.ptrauth.blend(i64 ptrtoint (ptr @atomic_src to i64), i64 100)
// CHECK-NEXT:    [[DSTDISC:%.*]] = call i64 @llvm.ptrauth.blend(i64 ptrtoint (ptr @atomic_ptr to i64), i64 100)
// CHECK:         [[RESIGN:%.*]] = call i64 @llvm.ptrauth.resign(i64 {{.*}}, i32 2, i64 [[SRCDISC]], i32 2, i64 [[DSTDISC]])
// CHECK:         [[RESIGNED:%.*]] = phi ptr [ null, %{{.*}} ], [ {{.*}}, %{{.*}} ]
// CHECK-NEXT:    store ptr [[RESIGNED]], ptr %.atomictmp, align 8
// CHECK-NEXT:    [[ATOMIC:%.*]] = load ptr, ptr %.atomictmp, align 8
// CHECK-NOT:     call i64 @llvm.ptrauth
// CHECK-NEXT:    store atomic ptr [[ATOMIC]], ptr @atomic_ptr seq_cst, align 8
void store_c11_resign(void) {
  __c11_atomic_store(&atomic_ptr, atomic_src, __ATOMIC_SEQ_CST);
}

// CHECK-LABEL: define {{.*}}void @init_local(ptr noundef %p)
// CHECK:         [[VALUE:%.*]] = load ptr, ptr %p.addr, align 8
// CHECK-NEXT:    [[ADDR:%.*]] = ptrtoint ptr %l to i64
// CHECK-NEXT:    [[DISC:%.*]] = call i64 @llvm.ptrauth.blend(i64 [[ADDR]], i64 100)
// CHECK:         [[INT:%.*]] = ptrtoint ptr [[VALUE]] to i64
// CHECK-NEXT:    [[SIGN:%.*]] = call i64 @llvm.ptrauth.sign(i64 [[INT]], i32 2, i64 [[DISC]])
// CHECK-NEXT:    [[SIGNED:%.*]] = inttoptr i64 [[SIGN]] to ptr
// CHECK:         [[RESULT:%.*]] = phi ptr [ null, %{{.*}} ], [ [[SIGNED]], %{{.*}} ]
// CHECK-NEXT:    store ptr [[RESULT]], ptr %l, align 8
void init_local(int *p) { AuthPtr _Atomic l = p; }

// CHECK-LABEL: define {{.*}}void @copy_implicit()
// CHECK:         [[LOAD:%.*]] = load atomic ptr, ptr @atomic_src seq_cst, align 8
// CHECK:         [[SRCDISC:%.*]] = call i64 @llvm.ptrauth.blend(i64 ptrtoint (ptr @atomic_src to i64), i64 100)
// CHECK:         [[DSTDISC:%.*]] = call i64 @llvm.ptrauth.blend(i64 ptrtoint (ptr @atomic_ptr to i64), i64 100)
// CHECK:         call i64 @llvm.ptrauth.resign(i64 {{.*}}, i32 2, i64 [[SRCDISC]], i32 2, i64 [[DSTDISC]])
// CHECK-NOT:     call i64 @llvm.ptrauth.auth
// CHECK-NOT:     call i64 @llvm.ptrauth.sign
// CHECK:         store atomic ptr {{.*}}, ptr @atomic_ptr seq_cst, align 8
// CHECK:         ret void
void copy_implicit(void) { atomic_ptr = atomic_src; }

// CHECK-LABEL: define {{.*}}i64 @load_int()
// CHECK:         [[LOAD:%.*]] = load atomic i64, ptr @atomic_int seq_cst, align 8
// CHECK-NEXT:    [[CMP:%.*]] = icmp ne i64 [[LOAD]], 0
// CHECK:         [[AUTH:%.*]] = call i64 @llvm.ptrauth.auth(i64 [[LOAD]], i32 2, i64 200)
// CHECK:         [[RESULT:%.*]] = phi i64 [ 0, %entry ], [ [[AUTH]], %{{.*}} ]
// CHECK-NEXT:    ret i64 [[RESULT]]
long load_int(void) { return atomic_int; }

// CHECK-LABEL: define {{.*}}void @store_int(i64 noundef %v)
// CHECK:         [[VALUE:%.*]] = load i64, ptr %v.addr, align 8
// CHECK-NEXT:    [[CMP:%.*]] = icmp ne i64 [[VALUE]], 0
// CHECK:         [[SIGN:%.*]] = call i64 @llvm.ptrauth.sign(i64 [[VALUE]], i32 2, i64 200)
// CHECK:         [[RESULT:%.*]] = phi i64 [ 0, %entry ], [ [[SIGN]], %{{.*}} ]
// CHECK-NEXT:    store atomic i64 [[RESULT]], ptr @atomic_int seq_cst, align 8
void store_int(long v) { atomic_int = v; }

// CHECK-LABEL: define {{.*}}ptr @exchange_c11(ptr noundef %p)
// CHECK:         [[VALUE:%.*]] = load ptr, ptr %p.addr, align 8
// CHECK-NEXT:    [[DISC:%.*]] = call i64 @llvm.ptrauth.blend(i64 ptrtoint (ptr @atomic_ptr to i64), i64 100)
// CHECK:         [[SIGN:%.*]] = call i64 @llvm.ptrauth.sign(i64 {{.*}}, i32 2, i64 [[DISC]])
// CHECK:         [[SIGNED:%.*]] = phi ptr [ null, %{{.*}} ], [ {{.*}}, %{{.*}} ]
// CHECK-NEXT:    store ptr [[SIGNED]], ptr %.atomictmp, align 8
// CHECK-NEXT:    [[VAL:%.*]] = load ptr, ptr %.atomictmp, align 8
// CHECK-NEXT:    [[OLD:%.*]] = atomicrmw xchg ptr @atomic_ptr, ptr [[VAL]] seq_cst, align 8
// CHECK-NEXT:    [[OLDDISC:%.*]] = call i64 @llvm.ptrauth.blend(i64 ptrtoint (ptr @atomic_ptr to i64), i64 100)
// CHECK:         [[OLDAUTH:%.*]] = call i64 @llvm.ptrauth.auth(i64 {{.*}}, i32 2, i64 [[OLDDISC]])
// CHECK:         [[RESULT:%.*]] = phi ptr [ null, %{{.*}} ], [ {{.*}}, %{{.*}} ]
// CHECK-NEXT:    store ptr [[RESULT]], ptr %atomic-temp, align 8
int *exchange_c11(int *p) {
  return __c11_atomic_exchange(&atomic_ptr, p, __ATOMIC_SEQ_CST);
}

// CHECK-LABEL: define {{.*}}i1 @cmpxchg_c11(ptr noundef %p)
// CHECK:         store ptr {{.*}}, ptr %expected, align 8
// CHECK-NEXT:    [[DESIRED:%.*]] = load ptr, ptr %p.addr, align 8
// CHECK-NEXT:    [[DESDISC:%.*]] = call i64 @llvm.ptrauth.blend(i64 ptrtoint (ptr @atomic_ptr to i64), i64 100)
// CHECK:         [[DESSIGN:%.*]] = call i64 @llvm.ptrauth.sign(i64 {{.*}}, i32 2, i64 [[DESDISC]])
// CHECK:         [[DESSIGNED:%.*]] = phi ptr [ null, %{{.*}} ], [ {{.*}}, %{{.*}} ]
// CHECK-NEXT:    store ptr [[DESSIGNED]], ptr %.atomictmp, align 8
// CHECK-NEXT:    [[EXPECTED:%.*]] = load ptr, ptr %expected, align 8
// CHECK-NEXT:    [[DES:%.*]] = load ptr, ptr %.atomictmp, align 8
// CHECK-NEXT:    [[EXPDISC:%.*]] = call i64 @llvm.ptrauth.blend(i64 ptrtoint (ptr @atomic_ptr to i64), i64 100)
// CHECK:         [[EXPSIGN:%.*]] = call i64 @llvm.ptrauth.sign(i64 {{.*}}, i32 2, i64 [[EXPDISC]])
// CHECK:         [[EXP:%.*]] = phi ptr [ null, %{{.*}} ], [ {{.*}}, %{{.*}} ]
// CHECK-NEXT:    [[PAIR:%.*]] = cmpxchg ptr @atomic_ptr, ptr [[EXP]], ptr [[DES]] seq_cst seq_cst, align 8
// CHECK-NEXT:    [[PREV:%.*]] = extractvalue { ptr, i1 } [[PAIR]], 0
// CHECK-NEXT:    [[SUCCESS:%.*]] = extractvalue { ptr, i1 } [[PAIR]], 1
// CHECK-NEXT:    br i1 [[SUCCESS]], label %cmpxchg.continue, label %cmpxchg.store_expected
// CHECK:       cmpxchg.store_expected:
// CHECK-NEXT:    [[OLDDISC:%.*]] = call i64 @llvm.ptrauth.blend(i64 ptrtoint (ptr @atomic_ptr to i64), i64 100)
// CHECK:         [[OLDAUTH:%.*]] = call i64 @llvm.ptrauth.auth(i64 {{.*}}, i32 2, i64 [[OLDDISC]])
// CHECK:         [[OLD:%.*]] = phi ptr [ null, %{{.*}} ], [ {{.*}}, %{{.*}} ]
// CHECK-NEXT:    store ptr [[OLD]], ptr %expected, align 8
// CHECK:       cmpxchg.continue:
// CHECK-NOT:     call i64 @llvm.ptrauth.auth
// CHECK:         ret i1
_Bool cmpxchg_c11(int *p) {
  int *expected = p;
  return __c11_atomic_compare_exchange_strong(&atomic_ptr, &expected, p,
                                              __ATOMIC_SEQ_CST,
                                              __ATOMIC_SEQ_CST);
}

// CHECK-LABEL: define {{.*}}ptr @load_n()
// CHECK:         [[LOAD:%.*]] = load atomic ptr, ptr @auth_ptr seq_cst, align 8
// CHECK-NEXT:    [[DISC:%.*]] = call i64 @llvm.ptrauth.blend(i64 ptrtoint (ptr @auth_ptr to i64), i64 100)
// CHECK:         [[INT:%.*]] = ptrtoint ptr [[LOAD]] to i64
// CHECK-NEXT:    [[AUTH:%.*]] = call i64 @llvm.ptrauth.auth(i64 [[INT]], i32 2, i64 [[DISC]])
int *load_n(void) { return __atomic_load_n(&auth_ptr, __ATOMIC_SEQ_CST); }

// CHECK-LABEL: define {{.*}}void @init_c11(ptr noundef %p)
// CHECK:         [[VALUE:%.*]] = load ptr, ptr %p.addr, align 8
// CHECK-NEXT:    [[DISC:%.*]] = call i64 @llvm.ptrauth.blend(i64 ptrtoint (ptr @atomic_ptr to i64), i64 100)
// CHECK:         [[VALUEINT:%.*]] = ptrtoint ptr [[VALUE]] to i64
// CHECK-NEXT:    [[SIGN:%.*]] = call i64 @llvm.ptrauth.sign(i64 [[VALUEINT]], i32 2, i64 [[DISC]])
// CHECK:         [[SIGNED:%.*]] = phi ptr [ null, %{{.*}} ], [ {{.*}}, %{{.*}} ]
// CHECK-NEXT:    store ptr [[SIGNED]], ptr @atomic_ptr, align 8
void init_c11(int *p) { __c11_atomic_init(&atomic_ptr, p); }

// CHECK-LABEL: define {{.*}}ptr @load_func()
// CHECK:         [[LOAD:%.*]] = load atomic ptr, ptr @atomic_func seq_cst, align 8
// CHECK-NEXT:    [[DISC:%.*]] = call i64 @llvm.ptrauth.blend(i64 ptrtoint (ptr @atomic_func to i64), i64 100)
// CHECK:         [[INT:%.*]] = ptrtoint ptr [[LOAD]] to i64
// CHECK-NEXT:    {{.*}} = call i64 @llvm.ptrauth.resign(i64 [[INT]], i32 2, i64 [[DISC]], i32 0, i64 0)
func_t *load_func(void) { return atomic_func; }
