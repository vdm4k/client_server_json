#include <client/client.h>
#include <gtest/gtest.h>
#include <json/json_parser.h>

#include "stream_test.h"

TEST(client_test, send_failed) {
  auto client = std::make_unique<client::client>();
  auto test_manager = std::make_unique<test::test_stream_manager>();
  auto* manager = test_manager.get();
  manager->send_failed = true;
  client->start(std::move(test_manager), "addr", 123, 12);
  while (manager->_activated_endpoints[0] == nullptr)
    ;

  std::vector<std::byte> send_data;
  manager->_activated_endpoints[0]->wait_sended_data(send_data);
  EXPECT_EQ(manager->_activated_endpoints[0]->_state, stream::state::e_failed);
  client->stop();
}

TEST(client_test, send_success) {
  auto client = std::make_unique<client::client>();
  auto test_manager = std::make_unique<test::test_stream_manager>();
  auto* manager = test_manager.get();
  client->start(std::move(test_manager), "addr", 123, 12);
  while (manager->_activated_endpoints[0] == nullptr)
    ;

  std::vector<std::byte> send_data;
  manager->_activated_endpoints[0]->wait_sended_data(send_data);
  client->stop();
  ASSERT_TRUE(send_data.size() > 4);

  json_parser req_parser;
  auto parse_res =
      req_parser.parse_request(send_data.data() + 4, send_data.size() - 4);
  ASSERT_TRUE(parse_res);
  EXPECT_EQ(parse_res->_type, json_parser::request_message_type::e_set);
}

TEST(client_test, receive_set_response) {
  auto client = std::make_unique<client::client>();
  auto test_manager = std::make_unique<test::test_stream_manager>();
  auto* manager = test_manager.get();
  client->start(std::move(test_manager), "addr", 123, 12);
  while (manager->_activated_endpoints[0] == nullptr)
    ;

  std::vector<std::byte> send_data;
  manager->_activated_endpoints[0]->wait_sended_data(send_data);
  ASSERT_TRUE(send_data.size() > 4);

  json_parser req_parser;
  auto parse_res =
      req_parser.parse_request(send_data.data() + 4, send_data.size() - 4);
  ASSERT_TRUE(parse_res);
  EXPECT_EQ(parse_res->_type, json_parser::request_message_type::e_set);

  json_parser resp_generator;
  auto json = resp_generator.generate_set_success_response();
  manager->_activated_endpoints[0]->set_receive_data(json);

  send_data.clear();
  manager->_activated_endpoints[0]->wait_sended_data(send_data);
  ASSERT_TRUE(send_data.size() > 4);
  EXPECT_EQ(client->get_transactions(), 1);
  client->stop();
}

TEST(client_test, receive_set_get_stat_response) {
  auto client = std::make_unique<client::client>();
  auto test_manager = std::make_unique<test::test_stream_manager>();
  auto* manager = test_manager.get();
  client->start(std::move(test_manager), "addr", 123, 12);
  while (manager->_activated_endpoints[0] == nullptr)
    ;

  {
    std::vector<std::byte> send_data;
    manager->_activated_endpoints[0]->wait_sended_data(send_data);
    ASSERT_TRUE(send_data.size() > 4);

    json_parser req_parser;
    auto parse_res =
        req_parser.parse_request(send_data.data() + 4, send_data.size() - 4);
    ASSERT_TRUE(parse_res);
    EXPECT_EQ(parse_res->_type, json_parser::request_message_type::e_set);

    json_parser resp_generator;
    auto json = resp_generator.generate_set_success_response();
    manager->_activated_endpoints[0]->set_receive_data(json);
  }

  {
    std::vector<std::byte> send_data;
    manager->_activated_endpoints[0]->wait_sended_data(send_data);
    ASSERT_TRUE(send_data.size() > 4);

    json_parser req_parser;
    auto parse_res =
        req_parser.parse_request(send_data.data() + 4, send_data.size() - 4);
    ASSERT_TRUE(parse_res);
    EXPECT_EQ(parse_res->_type, json_parser::request_message_type::e_get);

    json_parser resp_generator;
    auto json = resp_generator.generate_get_response("tadam");
    manager->_activated_endpoints[0]->set_receive_data(json);
  }

  {
    std::vector<std::byte> send_data;
    manager->_activated_endpoints[0]->wait_sended_data(send_data);
    ASSERT_TRUE(send_data.size() > 4);

    json_parser req_parser;
    auto parse_res =
        req_parser.parse_request(send_data.data() + 4, send_data.size() - 4);
    ASSERT_TRUE(parse_res);
    EXPECT_EQ(parse_res->_type, json_parser::request_message_type::e_stat);

    json_parser resp_generator;
    auto json = resp_generator.generate_stat_response(2, 0);
    manager->_activated_endpoints[0]->set_receive_data(json);
  }

  client->stop();
  EXPECT_EQ(client->get_transactions(), 3);
}

TEST(client_test, send_1_transaction) {
  auto client = std::make_unique<client::client>();
  auto test_manager = std::make_unique<test::test_stream_manager>();
  auto* manager = test_manager.get();
  client->start(std::move(test_manager), "addr", 123, 1);
  while (manager->_activated_endpoints[0] == nullptr)
    ;

  {
    std::vector<std::byte> send_data;
    manager->_activated_endpoints[0]->wait_sended_data(send_data);
    ASSERT_TRUE(send_data.size() > 4);

    json_parser req_parser;
    auto parse_res =
        req_parser.parse_request(send_data.data() + 4, send_data.size() - 4);
    ASSERT_TRUE(parse_res);
    EXPECT_EQ(parse_res->_type, json_parser::request_message_type::e_set);

    json_parser resp_generator;
    auto json = resp_generator.generate_set_success_response();
    manager->_activated_endpoints[0]->set_receive_data(json);
  }
  while (client->is_active())
    ;
  client->stop();  // join
  EXPECT_EQ(client->get_transactions(), 1);
}

TEST(client_test, receive_set_response_in_2_parts) {
  auto client = std::make_unique<client::client>();
  auto test_manager = std::make_unique<test::test_stream_manager>();
  auto* manager = test_manager.get();
  client->start(std::move(test_manager), "addr", 123, 12);
  while (manager->_activated_endpoints[0] == nullptr)
    ;

  std::vector<std::byte> send_data;
  manager->_activated_endpoints[0]->wait_sended_data(send_data);
  ASSERT_TRUE(send_data.size() > 4);

  json_parser req_parser;
  auto parse_res =
      req_parser.parse_request(send_data.data() + 4, send_data.size() - 4);
  ASSERT_TRUE(parse_res);
  EXPECT_EQ(parse_res->_type, json_parser::request_message_type::e_set);

  json_parser resp_generator;
  auto json = resp_generator.generate_set_success_response();
  manager->_activated_endpoints[0]->set_receive_data(json, 2);
  while (!manager->_activated_endpoints[0]->_data_received)
    ;

  send_data.clear();
  manager->_activated_endpoints[0]->wait_sended_data(send_data);
  ASSERT_TRUE(send_data.size() > 4);
  client->stop();
  EXPECT_EQ(client->get_transactions(), 1);
}

TEST(client_test, receive_2_set_response_in_one_message) {
  auto client = std::make_unique<client::client>();
  auto test_manager = std::make_unique<test::test_stream_manager>();
  auto* manager = test_manager.get();
  client->start(std::move(test_manager), "addr", 123, 12);
  while (manager->_activated_endpoints[0] == nullptr)
    ;

  std::vector<std::byte> send_data;
  manager->_activated_endpoints[0]->wait_sended_data(send_data);
  ASSERT_TRUE(send_data.size() > 4);

  json_parser req_parser;
  auto parse_res =
      req_parser.parse_request(send_data.data() + 4, send_data.size() - 4);
  ASSERT_TRUE(parse_res);
  EXPECT_EQ(parse_res->_type, json_parser::request_message_type::e_set);

  json_parser resp_generator;
  auto json = resp_generator.generate_set_success_response();
  manager->_activated_endpoints[0]->set_receive_data(json, 1, 2);
  while (!manager->_activated_endpoints[0]->_data_received)
    ;

  send_data.clear();
  manager->_activated_endpoints[0]->wait_sended_data(send_data);
  ASSERT_TRUE(send_data.size() > 4);
  client->stop();
  EXPECT_EQ(client->get_transactions(), 2);
}

TEST(client_test, receive_3_set_response_in_2_parts) {
  auto client = std::make_unique<client::client>();
  auto test_manager = std::make_unique<test::test_stream_manager>();
  auto* manager = test_manager.get();
  client->start(std::move(test_manager), "addr", 123, 12);
  while (manager->_activated_endpoints[0] == nullptr)
    ;

  std::vector<std::byte> send_data;
  manager->_activated_endpoints[0]->wait_sended_data(send_data);
  ASSERT_TRUE(send_data.size() > 4);

  json_parser req_parser;
  auto parse_res =
      req_parser.parse_request(send_data.data() + 4, send_data.size() - 4);
  ASSERT_TRUE(parse_res);
  EXPECT_EQ(parse_res->_type, json_parser::request_message_type::e_set);

  json_parser resp_generator;
  auto json = resp_generator.generate_set_success_response();
  manager->_activated_endpoints[0]->set_receive_data(json, 2, 3);
  while (!manager->_activated_endpoints[0]->_data_received)
    ;

  send_data.clear();
  manager->_activated_endpoints[0]->wait_sended_data(send_data);
  ASSERT_TRUE(send_data.size() > 4);
  client->stop();
  EXPECT_EQ(client->get_transactions(), 3);
}
