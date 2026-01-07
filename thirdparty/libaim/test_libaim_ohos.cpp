#include "aimsession.h"
#include "flap.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>  // 包含 write 函数声明
#include <string.h>  // 包含 strlen 函数声明

// 简单测试宏
#define TEST(name) void test_##name()
#define RUN_TEST(name) do { printf("Running test_%s...", #name); test_##name(); printf("OK\n"); } while(0)

void terminal_print(const char* msg) {
    if (msg == NULL) return;
    // write(文件描述符, 内容, 长度)：1 是 stdout 的固定文件描述符
    write(1, msg, strlen(msg));
    write(1, "\n", 1);  // 换行，让输出更整洁
}
// 测试 FLAP 基础功能
TEST(flap_basic) {
    FLAP flap;
    flap.resetData().setChannelID(0x02);
    flap.writeByte(0xAB).writeWord(0x1234);
    assert(flap.getDataFieldLength() == 3);  // 1字节 + 2字节 = 3字节

    flap.setSeek(0, FLAP::FLAP_SEEK_START);
    mybyte b;
    word w;
    flap.readByte(b).readWord(w);
    assert(b == 0xAB);
    assert(w == 0x1234);
}

// 测试 AIMSession 配置功能
TEST(aimsession_config) {
    AIMSession session;
    session.setScreenname("test_user").setPassword("test_pwd");
    assert(session.getScreenname() == "test_user");

    int pwd_len;
    const unsigned char* pwd = session.getPassword(pwd_len);
    assert(pwd != nullptr);
    assert(pwd_len > 0);
}

int main() {
    terminal_print("Starting libaim OHOS test...\n");
    fflush(stdout);
    RUN_TEST(flap_basic);
    RUN_TEST(aimsession_config);
    terminal_print("All tests passed!\n");
    fflush(stdout);
    return 0;
}