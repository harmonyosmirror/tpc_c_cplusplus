#include <gtest/gtest.h>
#include "aimession.h"
#include "aimlistener.h"
#include <thread>
#include <chrono>
#include <cstring>

// 自定义监听器：用于捕获 AIMSession 分发的事件（如接收消息、好友事件）
class TestAIMListener : public AIMListener {
public:
  // 存储接收的消息（用于验证）
  std::string received_msg;
  std::string received_from;  // 消息发送方
  bool buddy_added = false;   // 好友添加成功标记

  // 重写 AIMListener 接口（根据实际 aimlistener.h 定义调整，此处假设核心回调）
  void onInstantMessageReceived(const std::string& from, const std::string& msg) override {
    received_from = from;
    received_msg = msg;
  }

  void onBuddyAdded(const std::string& buddyName) override {
    buddy_added = true;
  }

  void onConnectionFailed(const std::string& reason) override {
    connection_failed_reason = reason;
  }

  std::string connection_failed_reason;
};

// 测试 Fixture：复用 AIMSession 实例和测试环境
class AIMSessionTest : public ::testing::Test {
protected:
  AIMSession session;
  TestAIMListener test_listener;

  // 测试前初始化：设置服务器、添加监听器
  void SetUp() override {
    // 1. 设置 AIM 登录服务器（默认 AOL 服务器或本地模拟服务器）
    session.setServerHostname("login.oscar.aol.com")  // 或本地服务器 "127.0.0.1"
           .setServerPort(5190)                      // AIM 协议默认端口
           .addAIMListener(test_listener);           // 添加自定义监听器，捕获事件

    // 2. 测试账号（替换为你的真实测试账号）
    test_screenname = "test_sender_001";
    test_password = "TestPass123!";
    test_target_buddy = "test_receiver_002";  // 另一个测试账号（接收方）
  }

  // 测试后清理：断开连接、移除监听器
  void TearDown() override {
    // 若连接未断开，尝试注销（AIMSession 无 logout()，需依赖 nextEvent() 处理断开）
    session.removeAIMListener(test_listener);
  }

  // 测试辅助函数：等待事件触发（避免网络延迟导致断言失败）
  template <typename Predicate>
  bool waitFor(Predicate pred, int timeout_ms = 5000) {
    auto start = std::chrono::steady_clock::now();
    while (!pred()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start
      ).count();
      if (elapsed > timeout_ms) return false;  // 超时失败
    }
    return true;
  }

  // 测试数据
  std::string test_screenname;
  std::string test_password;
  std::string test_target_buddy;
};

// ------------------------------ 核心功能测试 ------------------------------
/**
 * 测试 1：正常登录 + 连接成功
 * 验证：setScreenname/setPassword 生效，connect() 返回 true，连接状态正常
 */
TEST_F(AIMSessionTest, Connect_Success_WithValidCredentials) {
  // 1. 设置账号密码
  session.setScreenname(test_screenname)
         .setPassword(test_password);

  // 2. 执行连接
  bool connect_result = session.connect();
  ASSERT_TRUE(connect_result) << "连接失败，可能是服务器不可用或账号密码错误";

  // 3. 验证 FLAP 层连接（FLAP 是底层通信对象，通过 getFlap() 检查）
  FLAP& flap = session.getFlap();
  // 假设 FLAP 有 isConnected() 接口（若没有，可通过 nextEvent() 间接验证）
  ASSERT_TRUE(flap.isConnected()) << "FLAP 层连接失败，上层连接状态异常";

  // 4. 发送客户端就绪（AIM 协议要求连接后发送）
  bool ready_result = session.sendClientReady();
  ASSERT_TRUE(ready_result) << "发送客户端就绪失败";
}

/**
 * 测试 2：发送即时消息 + 接收消息（端到端测试）
 * 验证：sendInstantMessage() 成功，接收方监听器能捕获到消息
 */
TEST_F(AIMSessionTest, SendInstantMessage_Success_And_Receive) {
  // 1. 先登录成功
  session.setScreenname(test_screenname)
         .setPassword(test_password);
  ASSERT_TRUE(session.connect()) << "登录失败，无法测试消息发送";
  ASSERT_TRUE(session.sendClientReady()) << "客户端就绪失败";

  // 2. 构造测试消息（带时间戳，避免重复）
  std::string test_msg = "[Test] Hello from AIMSession! " + std::to_string(time(nullptr));

  // 3. 发送消息给目标账号
  bool send_result = session.sendInstantMessage(test_target_buddy, test_msg);
  ASSERT_TRUE(send_result) << "消息发送失败";

  // 4. 等待接收消息（通过监听器捕获，最多等 5 秒）
  bool msg_received = waitFor([this, &test_msg]() {
    return test_listener.received_msg == test_msg && test_listener.received_from == test_target_buddy;
  });

  // 5. 断言：消息接收成功且内容一致
  ASSERT_TRUE(msg_received) << "未接收到消息（可能接收方离线），预期：" << test_msg 
                            << "，实际接收：" << test_listener.received_msg;
}

/**
 * 测试 3：添加好友成功
 * 验证：sendAddBuddy() 生效，监听器捕获 onBuddyAdded 事件
 */
TEST_F(AIMSessionTest, SendAddBuddy_Success) {
  // 1. 登录成功
  session.setScreenname(test_screenname)
         .setPassword(test_password);
  ASSERT_TRUE(session.connect()) << "登录失败，无法测试添加好友";
  ASSERT_TRUE(session.sendClientReady()) << "客户端就绪失败";

  // 2. 添加测试好友（目标账号）
  bool add_result = session.sendAddBuddy(test_target_buddy);
  ASSERT_TRUE(add_result) << "发送添加好友请求失败";

  // 3. 等待好友添加事件（最多等 3 秒）
  bool buddy_added = waitFor([this]() {
    return test_listener.buddy_added;
  });

  ASSERT_TRUE(buddy_added) << "好友添加请求未被确认（可能目标账号未同意）";
}

// ------------------------------ 异常场景测试 ------------------------------
/**
 * 测试 4：无效账号密码登录失败
 * 验证：错误凭据下 connect() 返回 false，监听器捕获连接失败原因
 */
TEST_F(AIMSessionTest, Connect_Failure_WithInvalidCredentials) {
  // 1. 设置错误账号密码
  session.setScreenname("invalid_user_123")
         .setPassword("wrong_password_456");

  // 2. 执行连接（预期失败）
  bool connect_result = session.connect();
  ASSERT_FALSE(connect_result) << "无效账号登录成功，不符合预期";

  // 3. 验证监听器捕获到失败原因
  bool failure_caught = waitFor([this]() {
    return !test_listener.connection_failed_reason.empty();
  });
  ASSERT_TRUE(failure_caught) << "未捕获到连接失败原因";
  std::cout << "连接失败原因：" << test_listener.connection_failed_reason << std::endl;
}

/**
 * 测试 5：未登录时发送消息失败
 * 验证：未调用 connect() 前，sendInstantMessage() 返回 false
 */
TEST_F(AIMSessionTest, SendInstantMessage_Failure_WithoutLogin) {
  // 未设置账号密码，未登录
  bool send_result = session.sendInstantMessage(test_target_buddy, "Test msg");
  ASSERT_FALSE(send_result) << "未登录时发送消息成功，不符合接口设计预期";
}

/**
 * 测试 6：超长消息发送（边界测试）
 * 验证：协议允许的最大长度消息（假设 AIM 最大 4096 字节）能发送成功
 */
TEST_F(AIMSessionTest, SendInstantMessage_LongContent_Success) {
  // 1. 登录成功
  session.setScreenname(test_screenname)
         .setPassword(test_password);
  ASSERT_TRUE(session.connect()) << "登录失败";
  ASSERT_TRUE(session.sendClientReady()) << "客户端就绪失败";

  // 2. 构造 4096 字节的超长消息（AIM 协议常见最大长度）
  std::string long_msg(4096, 'a');  // 4096 个 'a'

  // 3. 发送超长消息
  bool send_result = session.sendInstantMessage(test_target_buddy, long_msg);
  ASSERT_TRUE(send_result) << "超长消息发送失败（可能超出协议限制）";

  // 4. 验证接收长度一致（可选）
  bool msg_received = waitFor([this, &long_msg]() {
    return test_listener.received_msg.size() == long_msg.size();
  });
  ASSERT_TRUE(msg_received) << "超长消息接收长度不一致";
}

// ------------------------------ 配置参数测试 ------------------------------
/**
 * 测试 7：参数设置有效性（setter + getter）
 * 验证：set 方法设置后，get 方法能正确获取
 */
TEST_F(AIMSessionTest, SetterAndGetter_WorkCorrectly) {
  // 1. 测试服务器地址/端口设置
  std::string test_host = "test-server.example.com";
  int test_port = 12345;
  session.setServerHostname(test_host).setServerPort(test_port);
  ASSERT_EQ(session.getServerHostname(), test_host) << "服务器地址设置失败";
  ASSERT_EQ(session.getServerPort(), test_port) << "服务器端口设置失败";

  // 2. 测试用户名设置
  std::string test_sn = "test_user";
  session.setScreenname(test_sn);
  ASSERT_EQ(session.getScreenname(), test_sn) << "用户名设置失败";

  // 3. 测试密码设置（加密后存储，验证长度和非空）
  std::string test_pwd = "test123";
  session.setPassword(test_pwd);
  int pwd_len = 0;
  const unsigned char* encrypted_pwd = session.getPassword(pwd_len);
  ASSERT_NE(encrypted_pwd, nullptr) << "密码加密后为空";
  ASSERT_GT(pwd_len, 0) << "加密密码长度为 0，不符合预期";
  ASSERT_NE(memcmp(encrypted_pwd, test_pwd.c_str(), test_pwd.size()), 0) << "密码未加密，直接存储明文";
}

/**
 * 测试：AIMSession 发送消息时，FLAP 包格式正确
 * 验证：AIMSession.sendInstantMessage() 会通过 FLAP 构建正确的 SNAC/FLAP 包
 */
TEST_F(AIMSessionTest, SendInstantMessage_FlapPacket_FormatCorrect) {
  // 1. 登录成功
  session.setScreenname(test_screenname)
         .setPassword(test_password);
  ASSERT_TRUE(session.connect()) << "登录失败";
  ASSERT_TRUE(session.sendClientReady()) << "客户端就绪失败";

  // 2. 发送测试消息
  std::string test_msg = "FLAP 包格式测试";
  session.sendInstantMessage(test_target_buddy, test_msg);

  // 3. 获取 AIMSession 内部的 FLAP 实例，验证包数据
  FLAP& flap = session.getFlap();
  int data_len = flap.getDataFieldLength();
  ASSERT_GT(data_len, 0) << "FLAP 缓冲区无数据，消息未写入";

  // 4. 解析 FLAP 包中的 SNAC（消息发送对应的 SNAC 家族：0x0004 是消息家族）
  flap.setSeek(0, FLAP::FLAP_SEEK_START);
  FLAP::SNAC snac;
  flap.readSNAC(snac);
  ASSERT_EQ(snac.familyID, 0x0004) << "消息发送的 SNAC 家族错误（预期 0x0004）";
  ASSERT_EQ(snac.subTypeID, 0x0006) << "消息发送的 SNAC 子类型错误（AIM 协议：0x0006 是发送即时消息）";

  // 5. 验证 FLAP 通道 ID（SNAC 通信默认 0x02）
  ASSERT_EQ(flap.getChannelID(), 0x02) << "FLAP 通道 ID 错误（预期 0x02）";
}

/**
 * 测试：AIMSession 连接失败时，FLAP 层连接状态异常
 */
TEST_F(AIMSessionTest, Connect_Failure_FlapDisconnected) {
  // 设置错误服务器地址
  session.setServerHostname("invalid-server.example.com")
         .setServerPort(9999)  // 无效端口
         .setScreenname("invalid_user")
         .setPassword("wrong_pwd");

  // 连接失败
  bool connect_result = session.connect();
  ASSERT_FALSE(connect_result) << "无效配置连接成功，不符合预期";

  // 验证 FLAP 层未连接（通过 FLAP 的 fd 状态判断）
  FLAP& flap = session.getFlap();
  // 尝试通过 FLAP 发送数据（应失败）
  flap.resetData().writeByte(0x01);
  bool send_result = flap.send();
  ASSERT_FALSE(send_result) << "FLAP 层未连接但发送成功，状态异常";
}