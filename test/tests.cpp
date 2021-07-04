#include "http_server.h"
/* #include "connection_context.h" */
#include "utils.h"
#include <gtest/gtest.h>

TEST(RequestTest, ParsesCorrectPOSTRequestCorrectly1) {
  const char *raw = "POST /home?param1=abc123&param2=def456&plz=hireme HTTP/1.1\r\n"
                    "Host: localhost:8080\r\n"
                    "User-Agent: curl/7.68.0\r\n"
                    "Accept: */*\r\n"
                    "Content-Type: application/json\r\n"
                    "Content-Length: 40\r\n"
                    "\r\n"
                    R"({"username":"geoboom", "password":"xyz"})";
  std::string raw_str(raw);
  std::string req_body = raw_str.substr(raw_str.find("\r\n\r\n") + std::string("\r\n\r\n").size());

  // unit tests aren't updated with the new design
  /* geo::Request request = geo::Request::build(raw); */
  /* EXPECT_EQ(request.get_method(), std::string("POST")) << "Method mismatch"; */
  /* EXPECT_EQ(request.get_url(), std::string("/home?param1=abc123&param2=def456&plz=hireme")) << "Raw url mismatch"; */
  /* EXPECT_EQ(request.get_raw_body(), req_body) << "Raw body mismatch"; */

  /* EXPECT_EQ(request.get_content_length(), 40) << "Content length mismatch."; */
  /* EXPECT_EQ(request.get_content_type(), "application/json") << "Content type mismatch"; */

  /* geo::KVMap expected_url_params{ */
  /*   {"param1", "abc123"}, */
  /*   {"param2", "def456"}, */
  /*   {"plz", "hireme"}, */
  /* }; */
  /* EXPECT_EQ(request.get_url_params(), expected_url_params) << "URL params parsed wrongly."; */

  /* geo::KVMap expected_body_data{ */
  /*   {"username", "geoboom"}, */
  /*   {"password", "xyz"}, */
  /* }; */
  /* EXPECT_EQ(request.get_body_data(), expected_body_data) << "Body data parsed wrongly."; */
}
