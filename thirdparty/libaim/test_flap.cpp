#include <gtest/gtest.h>
#include "FLAP.h"
#include <unistd.h>  // 用于 pipe()/read()/write()
#include <cstring>
#include <iostream>

// 测试 Fixture：复用 FLAP 实例和管道模拟环境
class FLAPTest : public ::testing::Test {
protected:
  FLAP flap;
  int pipe_fd[2];  // 管道：pipe_fd[0] 读端，pipe_fd[1] 写端

  void SetUp() override {
    // 创建管道，模拟 FLAP 的文件描述符（fd）
    ASSERT_EQ(pipe(pipe_fd), 0) << "创建管道失败，无法模拟 FLAP 通信";

    // 给 FLAP 设置管道写端（发送数据用），并设置销毁时关闭 fd
    flap.setFileDescriptor(pipe_fd[1], true);

    // 初始化 FLAP 包（设置通道 ID：AIM 协议 SNAC 通信默认 0x02）
    flap.resetData().setChannelID(0x02);
  }

  void TearDown() override {
    // 关闭管道读端（写端由 FLAP 销毁时自动关闭）
    close(pipe_fd[0]);
  }

  // 辅助函数：从管道读端读取 FLAP 包数据（用于验证 send() 效果）
  int readFromPipe(void* buffer, int max_len) {
    return read(pipe_fd[0], buffer, max_len);
  }

  // 辅助函数：验证 FLAP 包头部格式（FLAP 头部固定 6 字节：0x2A + 通道 ID + 序列号 + 数据长度）
  bool verifyFLAPHeader(const mybyte* header, int expected_channel, int expected_seq, int expected_data_len) {
    if (header[0] != 0x2A) return false;  // FLAP 包起始标识固定为 0x2A
    if ((int)header[1] != expected_channel) return false;  // 通道 ID
    if (ntohs(*(word*)(header + 2)) != expected_seq) return false;  // 序列号（网络字节序）
    if (ntohs(*(word*)(header + 4)) != expected_data_len) return false;  // 数据长度（网络字节序）
    return true;
  }
};

// ------------------------------ FLAP 数据包构建与解析测试 ------------------------------
/**
 * 测试 1：写入基础数据（byte/word/dword）+ 读取验证
 * 验证：writeXXX 接口正确写入数据，readXXX 接口正确解析（顺序读写、seek 定位）
 */
TEST_F(FLAPTest, WriteAndRead_BasicTypes_Success) {
  // 1. 写入基础数据（顺序写入）
  mybyte test_byte = 0xAB;
  word test_word = 0x1234;
  dword test_dword = 0x5678ABCD;

  flap.writeByte(test_byte)
      .writeWord(test_word)
      .writeDword(test_dword);

  // 2. 重置读取指针（从起始位置读取）
  flap.setSeek(0, FLAP::FLAP_SEEK_START);

  // 3. 读取数据并验证
  mybyte read_byte;
  word read_word;
  dword read_dword;

  flap.readByte(read_byte)
      .readWord(read_word)
      .readDword(read_dword);

  ASSERT_EQ(read_byte, test_byte) << "字节读取错误";
  ASSERT_EQ(read_word, test_word) << "16位数据读取错误";
  ASSERT_EQ(read_dword, test_dword) << "32位数据读取错误";

  // 4. 测试 seek 和 skipBytes
  flap.setSeek(1, FLAP::FLAP_SEEK_START);  // 定位到第1字节后（跳过 test_byte）
  flap.skipBytes(2);  // 再跳过 test_word（2字节），定位到 test_dword 起始位置
  dword read_dword_2;
  flap.readDword(read_dword_2);
  ASSERT_EQ(read_dword_2, test_dword) << "seek/skip 后读取错误";
}

/**
 * 测试 2：写入 SNAC 头 + 读取验证
 * 验证：writeSNAC() 正确封装 SNAC 头（10字节），readSNAC() 正确解析
 */
TEST_F(FLAPTest, WriteAndRead_SNAC_Success) {
  // 1. 构建 SNAC 数据
  FLAP::SNAC test_snac = {
    .familyID = 0x0001,    // 通用 SNAC 家族
    .subTypeID = 0x0002,   // 子类型
    .flags1 = 0x00,
    .flags2 = 0x01,
    .requestID = 0x12345678
  };

  // 2. 写入 SNAC 头
  flap.writeSNAC(test_snac);

  // 3. 重置读取指针并读取
  flap.setSeek(0, FLAP::FLAP_SEEK_START);
  FLAP::SNAC read_snac;
  flap.readSNAC(read_snac);

  // 4. 验证 SNAC 字段一致
  ASSERT_EQ(read_snac.familyID, test_snac.familyID) << "SNAC familyID 错误";
  ASSERT_EQ(read_snac.subTypeID, test_snac.subTypeID) << "SNAC subTypeID 错误";
  ASSERT_EQ(read_snac.flags1, test_snac.flags1) << "SNAC flags1 错误";
  ASSERT_EQ(read_snac.flags2, test_snac.flags2) << "SNAC flags2 错误";
  ASSERT_EQ(read_snac.requestID, test_snac.requestID) << "SNAC requestID 错误";
}

/**
 * 测试 3：写入 TLV 数据 + 读取验证（字符串/word/自定义数据）
 * 验证：writeTLV() 三种重载正确封装，readTLV() 正确解析
 */
TEST_F(FLAPTest, WriteAndRead_TLV_Success) {
  // 1. 写入三种 TLV
  word tlv_type1 = 0x0001;
  std::string tlv_data1 = "test_tlv_string";  // 字符串 TLV

  word tlv_type2 = 0x0002;
  word tlv_data2 = 0x9ABC;  // word 类型 TLV

  word tlv_type3 = 0x0003;
  mybyte tlv_data3[] = {0xDE, 0xF0, 0x12};  // 自定义字节数组 TLV
  int tlv_len3 = sizeof(tlv_data3);

  flap.writeTLV(tlv_type1, tlv_data1)
      .writeTLV(tlv_type2, tlv_data2)
      .writeTLV(tlv_type3, tlv_data3, tlv_len3);

  // 2. 重置读取指针，依次读取 TLV
  flap.setSeek(0, FLAP::FLAP_SEEK_START);

  // 读取 TLV1（字符串）
  word read_type1;
  std::string read_data1;
  flap.readTLV(read_type1, read_data1);
  ASSERT_EQ(read_type1, tlv_type1) << "TLV1 类型错误";
  ASSERT_EQ(read_data1, tlv_data1) << "TLV1 数据错误";

  // 读取 TLV2（word）
  word read_type2, read_data2;
  flap.readTLV(read_type2, read_data2);
  ASSERT_EQ(read_type2, tlv_type2) << "TLV2 类型错误";
  ASSERT_EQ(read_data2, tlv_data2) << "TLV2 数据错误";

  // 读取 TLV3（自定义字节数组）
  word read_type3;
  mybyte read_data3[10];
  int read_len3;
  flap.readTLV(read_type3, read_data3, sizeof(read_data3), read_len3);
  ASSERT_EQ(read_type3, tlv_type3) << "TLV3 类型错误";
  ASSERT_EQ(read_len3, tlv_len3) << "TLV3 长度错误";
  ASSERT_EQ(memcmp(read_data3, tlv_data3, tlv_len3), 0) << "TLV3 数据错误";
}

// ------------------------------ FLAP 收发逻辑测试（模拟管道） ------------------------------
/**
 * 测试 4：send() 发送 FLAP 包 + receive() 接收（管道模拟）
 * 验证：FLAP 包通过管道正确传输，接收端能解析出完整数据（头部 + 数据）
 */
TEST_F(FLAPTest, SendAndReceive_FullPacket_Success) {
  // 1. 构建完整 FLAP 包（头部 + SNAC 数据 + TLV 数据）
  FLAP::SNAC test_snac = {0x0001, 0x0002, 0x00, 0x01, 0x12345678};
  word tlv_type = 0x0001;
  std::string tlv_data = "flap_test_data";

  flap.resetData()
      .setChannelID(0x02)  // SNAC 通信通道
      .writeSNAC(test_snac)
      .writeTLV(tlv_type, tlv_data);

  // 2. 发送 FLAP 包（写入管道写端）
  bool send_result = flap.send();
  ASSERT_TRUE(send_result) << "FLAP 包发送失败";

  // 3. 从管道读端读取数据（验证发送的包格式）
  mybyte recv_buffer[1024] = {0};
  int recv_len = readFromPipe(recv_buffer, sizeof(recv_buffer));
  ASSERT_GT(recv_len, 6) << "接收数据过短（小于 FLAP 头部长度 6 字节）";

  // 4. 验证 FLAP 头部
  bool header_valid = verifyFLAPHeader(
    recv_buffer,
    0x02,                  // 预期通道 ID
    0,                     // 首次发送序列号为 0
    flap.getDataFieldLength()  // 预期数据长度（FLAP 内部数据长度）
  );
  ASSERT_TRUE(header_valid) << "FLAP 头部格式错误";

  // 5. 用另一个 FLAP 实例解析接收的数据
  FLAP recv_flap;
  // 将接收的数据写入 recv_flap 的缓冲区（模拟 receive() 接收）
  recv_flap.resetData()
           .writeData(recv_buffer + 6, recv_len - 6);  // 跳过头部，写入数据部分

  // 6. 解析数据并验证
  recv_flap.setSeek(0, FLAP::FLAP_SEEK_START);
  FLAP::SNAC read_snac;
  recv_flap.readSNAC(read_snac);
  ASSERT_EQ(read_snac.requestID, test_snac.requestID) << "接收的 SNAC 数据错误";

  word read_tlv_type;
  std::string read_tlv_data;
  recv_flap.readTLV(read_tlv_type, read_tlv_data);
  ASSERT_EQ(read_tlv_data, tlv_data) << "接收的 TLV 数据错误";
}

// ------------------------------ FLAP 异常场景测试 ------------------------------
/**
 * 测试 5：读取超出缓冲区范围（抛出异常）
 * 验证：FLAP 正确处理越界读取，抛出 std::out_of_range
 */
TEST_F(FLAPTest, Read_OutOfRange_ThrowsException) {
  // 写入 2 字节数据
  flap.writeByte(0x01).writeByte(0x02);
  flap.setSeek(0, FLAP::FLAP_SEEK_START);

  // 尝试读取 3 字节（超出缓冲区长度 2）
  mybyte buffer[3];
  EXPECT_THROW(flap.readData(buffer, 3), std::out_of_range) << "越界读取未抛出异常";
}

/**
 * 测试 6：resetData() 重置缓冲区
 * 验证：resetData() 后缓冲区长度为 0，指针重置
 */
TEST_F(FLAPTest, ResetData_ClearsBuffer) {
  // 先写入数据
  flap.writeWord(0x1234);
  ASSERT_EQ(flap.getDataFieldLength(), 2) << "写入数据后缓冲区长度错误";

  // 重置
  flap.resetData();
  ASSERT_EQ(flap.getDataFieldLength(), 0) << "resetData() 未清空缓冲区长度";

  // 尝试读取（应抛出异常）
  word read_word;
  EXPECT_THROW(flap.readWord(read_word), std::out_of_range) << "resetData() 后仍可读取数据";
}

// ------------------------------ 运算符重载测试 ------------------------------
/**
 * 测试 7：operator<< 输出 FLAP 包（调试功能验证）
 * 验证：输出流重载能正常打印 FLAP 包（无崩溃，格式正确）
 */
TEST_F(FLAPTest, OperatorOutput_PrintsSuccess) {
  // 构建测试包
  flap.writeSNAC(0x0001, 0x0002, 0x00, 0x01, 0x12345678)
      .writeTLV(0x0001, "test_output");

  // 打印（无崩溃即通过，若需验证输出内容可重定向流）
  std::cout << "FLAP 包输出测试：" << flap << std::endl;
  SUCCEED() << "operator<< 输出正常";
}