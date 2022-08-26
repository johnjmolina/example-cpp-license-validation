#include <iostream>
#include <string>
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/base_uri.h>

using namespace std;
using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace web::json;
using namespace utility;

// Verify a license key using the validate-key action
pplx::task<http_response> validate_license_key(http_client client, const string activation_token,
                                               const string license_key)
{
  http_request req;

  value meta;
  meta["key"] = value::string(license_key);
  //meta["scope"]["fingerprint"] = ;

  value body;
  body["meta"] = meta;


  req.headers().add("Content-Type", "application/vnd.api+json");
  req.headers().add("Accept", "application/json");

  req.set_request_uri(uri("/licenses/actions/validate-key"));
  req.set_method(methods::POST);
  req.set_body(body.serialize());

  return client.request(req);
}

json readlicensefile(string const &filepath){
  value license;
  try{
    ifstream fp(filepath);
    stringstream str;
    str << fp.rdbuf();
    fp.close();
    license = value::parse(str);
  }catch(json_exception &e){
    cerr << "Error reading license file " << filepath << endl;
    exit(-1);
  }
  return license
}

int main(int argc, char* argv[])
{
  if (argc == 1)
  {
    cerr << "[ERROR] "
         << "No license key file specified"
         << endl;

    exit(1);
  }

  const string account_id = getenv("KEYGEN_ACCOUNT_ID");
  cerr << "reading license file" << endl; 
  json license = readlicensefile(argv[1]);
  cerr << "key = " << license_key << endl;
  http_client client(uri("https://api.keygen.sh/v1/accounts/" + account_id));

  validate_license_key(client, license_key)
    .then([](http_response res)
    {
      auto json = res.extract_json().get();
      if (json.has_field("errors"))
      {
        auto errors = json.at("errors").as_array();
        auto err = errors[0];

        cerr << "[ERROR] "
             << "API request failed: "
             << "status=" << res.status_code() << " "
             << "title='" << err.at("title").as_string() << "' "
             << "detail='" << err.at("detail").as_string() << "'"
             << endl;

        exit(1);
      }

      auto data = json.at("data");
      auto meta = json.at("meta");

      cout << meta.serialize() << endl;      
      if (meta.at("valid").as_bool())
      {
        cout << "[OK] "
             << "License key is valid: "
          //<< "code=" << meta.at("code").as_string() << endl;/*<< " "
             << "id=" << data.at("id").as_string()
             << endl;
      }
      else
      {
        cerr << "[ERROR] "
             << "License key is not valid: "
             << "code=" << meta.at("code").as_string()
             << endl;
      }
    })
    .wait();
}
