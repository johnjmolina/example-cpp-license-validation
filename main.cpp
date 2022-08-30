#include <iostream>
#include <string>
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/base_uri.h>
#include <openssl/sha.h>
#include <IOKit/IOKitLib.h>

// Verify a license key using the validate-key action
pplx::task<web::http::http_response> validate_license_key(web::http::client::http_client client, const std::string fingerprint,
                                                          const std::string license_key)
{
  web::http::http_request req;

  web::json::value meta, scope;
  scope["fingerprint"] = web::json::value::string(fingerprint);
  meta["key"] = web::json::value::string(license_key);
  meta["scope"] = scope;

  web::json::value body;
  body["meta"] = meta;

  req.headers().add("Content-Type", "application/vnd.api+json");
  req.headers().add("Accept", "application/json");

  req.set_request_uri(web::uri("/licenses/actions/validate-key"));
  req.set_method(web::http::methods::POST);
  req.set_body(body.serialize());

  return client.request(req);
}

web::json::value readlicensefile(std::string const &filepath)
{
  web::json::value license;
  try
  {
    std::ifstream fp(filepath);
    std::stringstream str;
    str << fp.rdbuf();
    fp.close();
    license = web::json::value::parse(str);
  }
  catch (web::json::json_exception &e)
  {
    std::cerr << "Error reading license file " << filepath << std::endl;
    exit(-1);
  }
  return license;
}

std::string tohex(std::string &src)
{
  std::stringstream ss;
  for (unsigned int i = 0; i < src.size(); i++)
    ss << std::setfill('0') << std::setw(2) << std::hex << (0xff & (unsigned int)src[i]);
  return ss.str();
}
std::string tohex(const char *src, const int size)
{
  std::stringstream ss;
  for (unsigned int i = 0; i < size; i++)
    ss << std::setfill('0') << std::setw(2) << std::hex << (0xff & (unsigned int)src[i]);
  return ss.str();
}
std::string anonymize(const std::string msg)
{
  char hash[SHA256_DIGEST_LENGTH];
  SHA256((unsigned char *)msg.c_str(), msg.size(), (unsigned char *)hash);
  return tohex(hash, SHA256_DIGEST_LENGTH);
}
std::string getuuid()
{
  const size_t bufSize = 512u;
  char buffer[bufSize];
  io_registry_entry_t ioRegistryRoot = IORegistryEntryFromPath(kIOMainPortDefault, "IOService:/");
  CFStringRef uuidCf = (CFStringRef)IORegistryEntryCreateCFProperty(ioRegistryRoot, CFSTR(kIOPlatformUUIDKey), kCFAllocatorDefault, 0);
  IOObjectRelease(ioRegistryRoot);
  CFStringGetCString(uuidCf, buffer, bufSize, kCFStringEncodingUTF8);
  CFRelease(uuidCf);
  return std::string(buffer);
}

int main(int argc, char *argv[])
{
  if (argc == 1)
  {
    std::cerr << "[ERROR] "
              << "No license key file specified"
              << std::endl;

    exit(1);
  }

  const std::string account_id = getenv("KEYGEN_ACCOUNT_ID");
  std::string fingerprint = anonymize(getuuid());

  web::json::value license = readlicensefile(argv[1]);
  web::http::client::http_client client(web::uri("https://api.keygen.sh/v1/accounts/" + account_id));
  validate_license_key(client, fingerprint, license["KEY"].as_string())
      .then([](web::http::http_response res)
            {
      auto result = res.extract_json().get();
      if (result.has_field("errors"))
      {
        auto errors = result.at("errors").as_array();
        auto err = errors[0];

        std::cerr << "[ERROR] "
             << "API request failed: "
             << "status=" << res.status_code() << " "
             << "title='" << err.at("title").as_string() << "' "
             << "detail='" << err.at("detail").as_string() << "'"
             << std::endl;

        exit(1);
      }

      auto data = result.at("data");
      auto meta = result.at("meta");

      if (meta.at("valid").as_bool())
      {
        std::cout << "# [OK] "
             << "License key is valid: "
             << "code=" << meta.at("constant").as_string() << std::endl 
             << "# id=" << data.at("id").as_string()
             << std::endl;
      }
      else
      {
        std::cerr << "[ERROR] "
             << "License key is not valid: "
             << "code=" << meta.at("constant").as_string()
             << std::endl;
      } })
      .wait();
}
