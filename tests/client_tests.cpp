#include <client/client.h>
#include <gtest/gtest.h>
#include <json/json_parser.h>

#include "stream_test.h"

TEST(client_test, send_failed) {
  auto clnt = std::make_unique<client::client>();
  auto test_mngr = std::make_unique<test::test_stream_manager>();
  auto* mngr = test_mngr.get();
  mngr->send_failed = true;
  clnt->start(std::move(test_mngr), "addr", 123, 12);
  while (mngr->_activated_endpoints[0] == nullptr)
    ;

  std::vector<std::byte> send_data;
  mngr->_activated_endpoints[0]->wait_sended_data(send_data);
  EXPECT_EQ(mngr->_activated_endpoints[0]->_state, stream::state::e_failed);
  clnt->stop();
}

TEST(client_test, send_success) {
  auto clnt = std::make_unique<client::client>();
  auto test_mngr = std::make_unique<test::test_stream_manager>();
  auto* mngr = test_mngr.get();
  clnt->start(std::move(test_mngr), "addr", 123, 12);
  while (mngr->_activated_endpoints[0] == nullptr)
    ;

  std::vector<std::byte> send_data;
  mngr->_activated_endpoints[0]->wait_sended_data(send_data);
  clnt->stop();
  ASSERT_TRUE(send_data.size() > 4);

  json_parser req_parser;
  auto parse_res =
      req_parser.parse_request(send_data.data() + 4, send_data.size() - 4);
  ASSERT_TRUE(parse_res);
  EXPECT_EQ(parse_res->_type, json_parser::request_message_type::e_set);
}

TEST(client_test, receive_set_response) {
  auto clnt = std::make_unique<client::client>();
  auto test_mngr = std::make_unique<test::test_stream_manager>();
  auto* mngr = test_mngr.get();
  clnt->start(std::move(test_mngr), "addr", 123, 12);
  while (mngr->_activated_endpoints[0] == nullptr)
    ;

  std::vector<std::byte> send_data;
  mngr->_activated_endpoints[0]->wait_sended_data(send_data);
  ASSERT_TRUE(send_data.size() > 4);

  json_parser req_parser;
  auto parse_res =
      req_parser.parse_request(send_data.data() + 4, send_data.size() - 4);
  ASSERT_TRUE(parse_res);
  EXPECT_EQ(parse_res->_type, json_parser::request_message_type::e_set);

  json_parser resp_generator;
  auto json = resp_generator.generate_get_response("tadam");
  mngr->_activated_endpoints[0]->set_receive_data(json);

  send_data.clear();
  mngr->_activated_endpoints[0]->wait_sended_data(send_data);
  ASSERT_TRUE(send_data.size() > 4);
  clnt->stop();
}

TEST(client_test, receive_half_set_response) {
  auto clnt = std::make_unique<client::client>();
  auto test_mngr = std::make_unique<test::test_stream_manager>();
  auto* mngr = test_mngr.get();
  clnt->start(std::move(test_mngr), "addr", 123, 12);
  while (mngr->_activated_endpoints[0] == nullptr)
    ;

  std::vector<std::byte> send_data;
  mngr->_activated_endpoints[0]->wait_sended_data(send_data);
  ASSERT_TRUE(send_data.size() > 4);

  json_parser req_parser;
  auto parse_res =
      req_parser.parse_request(send_data.data() + 4, send_data.size() - 4);
  ASSERT_TRUE(parse_res);
  EXPECT_EQ(parse_res->_type, json_parser::request_message_type::e_set);

  json_parser resp_generator;
  auto json = resp_generator.generate_get_response("tadam");
  mngr->_activated_endpoints[0]->set_receive_data(json, 2);
  while (!mngr->_activated_endpoints[0]->_data_received)
    ;

  send_data.clear();
  mngr->_activated_endpoints[0]->wait_sended_data(send_data);
  ASSERT_TRUE(send_data.size() > 4);
  clnt->stop();
}

TEST(client_test, receive_2_set_response) {
  auto clnt = std::make_unique<client::client>();
  auto test_mngr = std::make_unique<test::test_stream_manager>();
  auto* mngr = test_mngr.get();
  clnt->start(std::move(test_mngr), "addr", 123, 12);
  while (mngr->_activated_endpoints[0] == nullptr)
    ;

  std::vector<std::byte> send_data;
  mngr->_activated_endpoints[0]->wait_sended_data(send_data);
  ASSERT_TRUE(send_data.size() > 4);

  json_parser req_parser;
  auto parse_res =
      req_parser.parse_request(send_data.data() + 4, send_data.size() - 4);
  ASSERT_TRUE(parse_res);
  EXPECT_EQ(parse_res->_type, json_parser::request_message_type::e_set);

  json_parser resp_generator;
  auto json = resp_generator.generate_get_response("tadam");
  mngr->_activated_endpoints[0]->set_receive_data(json, 1, 2);
  while (!mngr->_activated_endpoints[0]->_data_received)
    ;

  send_data.clear();
  mngr->_activated_endpoints[0]->wait_sended_data(send_data);
  ASSERT_TRUE(send_data.size() > 4);
  clnt->stop();
}
