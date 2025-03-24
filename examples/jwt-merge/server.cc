// jwt_server.cpp
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>
#include <jwt-cpp/jwt.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

const std::string SECRET_KEY = "YZWVYYRazBwRHF9/iD4rfgJn/yLoISV6bKiRs/XA1EM=";
const int EXPIRATION_SEC = 5;
const int PORT_NUM = 36010;

std::string get_cookie_expire_time() {
  std::time_t now = std::chrono::system_clock::to_time_t(
      std::chrono::system_clock::now() + std::chrono::seconds(EXPIRATION_SEC));
  char buffer[100];
  std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT",
                std::gmtime(&now));
  return std::string(buffer);
}

// 生成 JWT
std::string generate_jwt() {
  return jwt::create()
      .set_issuer("auth0")
      .set_type("JWS")
      .set_issued_at(std::chrono::system_clock::now())
      .set_expires_at(std::chrono::system_clock::now() +
                      std::chrono::seconds(EXPIRATION_SEC))
      .sign(jwt::algorithm::hs256{SECRET_KEY});
}

// 验证 JWT
bool validate_jwt(const std::string &token) {
  try {
    auto decoded = jwt::decode(token);
    jwt::verify()
        .allow_algorithm(jwt::algorithm::hs256{SECRET_KEY})
        .with_issuer("auth0")
        .verify(decoded);
    return std::chrono::system_clock::now() < decoded.get_expires_at();
  } catch (...) {
    return false;
  }
}

// 创建JSON响应
std::string create_json_response(const std::string &key,
                                 const std::string &value) {
  rapidjson::StringBuffer s;
  rapidjson::Writer<rapidjson::StringBuffer> writer(s);

  writer.StartObject();
  writer.Key(key.c_str());
  writer.String(value.c_str());
  writer.EndObject();

  return s.GetString();
}

// 处理请求
void handle_request(const http::request<http::string_body> &req,
                    http::response<http::string_body> &res) {
  res.set(http::field::server, "JWT Server");
  res.set(http::field::content_type, "application/json");
  res.set(http::field::access_control_allow_origin, "*");

  if (req.method() != http::verb::post) {
    res.result(http::status::method_not_allowed);
    res.body() = create_json_response("error", "Method Not Allowed");
    return;
  }

  if (req.target() == "/login") {
    std::cout << "login" << std::endl;
    std::string token = generate_jwt();

    // 设置 Cookie
    std::string cookie = "jwt_token=" + token +
                         "; "
                         "Expires=" +
                         get_cookie_expire_time() +
                         "; "
                         "Max-Age=" +
                         std::to_string(EXPIRATION_SEC) +
                         "; "
                         "Path=/; "
                         "HttpOnly; "
                         "SameSite=Lax"; // 生产环境应加 Secure
    std::cout << "token = " << token << std::endl;
    res.set(http::field::set_cookie, cookie);
    res.body() = create_json_response("status", "login_success");
    res.result(http::status::ok);
  } else if (req.target() == "/check") {
    std::cout << "check" << std::endl;
    // 从 Cookie 获取 token
    std::string token;
    if (auto cookie_hdr = req.find(http::field::cookie);
        cookie_hdr != req.end()) {
      std::string cookies = std::string(cookie_hdr->value());
      size_t pos = cookies.find("jwt_token=");
      if (pos != std::string::npos) {
        size_t end = cookies.find(';', pos);
        token = cookies.substr(pos + 10, end - pos - 10);
      }
    }
    std::cout << "token = " << token << std::endl;

    if (token.empty()) {
      res.result(http::status::unauthorized);
      res.body() = create_json_response("error", "Missing token");
      return;
    }

    res.body() =
        create_json_response("valid", validate_jwt(token) ? "true" : "false");
    res.result(http::status::ok);
  } else {
    res.result(http::status::not_found);
    res.body() = create_json_response("error", "Endpoint Not Found");
  }
}

int main() {
  try {
    net::io_context ioc{1};
    tcp::acceptor acceptor{ioc, {tcp::v4(), PORT_NUM}};
    std::cout << "Server started on port " << PORT_NUM << std::endl;
    for (;;) {
      tcp::socket socket{ioc};
      acceptor.accept(socket);

      beast::flat_buffer buffer;
      http::request<http::string_body> req;
      http::read(socket, buffer, req);

      http::response<http::string_body> res;
      handle_request(req, res);

      res.prepare_payload();
      http::write(socket, res);
      socket.shutdown(tcp::socket::shutdown_send);
    }
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}