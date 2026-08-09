# 防御机制分析：为什么堆喷和同对象重用彻底失败？

## 1. Seccomp 系统级拦截
在 Android 15 中，尝试调用 `msgsnd`、`io_uring_setup`、`signalfd` 等系统调用时，直接触发 `Unknown signal 31` (SIGSYS)。这意味着系统在用户态就切断了堆喷原语的入口。

## 2. RANDOM_KMALLOC_CACHES
Kernel 5.15 默认开启了此机制。内核将相同大小（如 192 字节）的内存块，随机分配到物理上完全隔离的多个缓存池中。这使得任何基于公共缓存的堆喷（`pipe`、`timerfd`）命中率无限趋近于 0。

## 3. 专用 Slab 缓存防重用
即使在触发 UAF 后，利用 `FUTEX_WAIT_REQUEUE_PI` 尝试重新分配 `futex_q`（同对象重用），内核依然会拒绝将物理内存原地归还。导致漏洞触发后的悬垂指针始终指向被隔离的空内存，无法被利用。
