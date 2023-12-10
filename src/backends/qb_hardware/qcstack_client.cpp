// Copyright (c) Quantum Brilliance Pty Ltd

// Qristal 
#include "qb/core/backends/qb_hardware/qcstack_client.hpp"

// XACC
#include "xacc.hpp"

// CPR
#include <cpr/cpr.h>

// JSON
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace xacc
{

  const std::string xacc::QCStackClient::post(const std::string &remoteUrl,
                                 const std::string &path,
                                 const std::string &postStr,
                                 std::map<std::string, std::string> headers)
  {
    if (debug_qb_hw_) std::cout << "* [DEBUG]: xacc::QCStackClient::post" << postStr << std::endl;
    
    if (headers.empty())
    {
      headers.insert(std::make_pair("Content-type", "application/json"));
      headers.insert(std::make_pair("Connection", "keep-alive"));
      headers.insert(std::make_pair("Accept", "*/*"));
    }
  
    cpr::Header cprHeaders;
    for (auto &kv : headers) cprHeaders.insert({kv.first, kv.second});
  
    auto r = cpr::Post(cpr::Url{remoteUrl + path}, cpr::Body(postStr), cprHeaders,
                       cpr::VerifySsl(false));
    if (r.status_code == 500) xacc::info("* Error: QB hardware process failure");
    if (r.status_code == 404) xacc::info("* Error: QB hardware received an invalid command");
    if (r.status_code != 200) throw std::runtime_error("HTTP POST Error - status code " +
     std::to_string(r.status_code) + ": " + r.error.message + ": " + r.text);
  
    return r.text;
  }
  
  const std::string xacc::QCStackClient::get(const std::string &remoteUrl, 
                                             const std::string &path,
                                             std::map<std::string, std::string> headers,
                                             std::map<std::string, std::string> extraParams)
  {
    if (debug_qb_hw_) std::cout << "* [DEBUG]: xacc::QCStackClient::get" << std::endl;

    if (headers.empty())
    {
      headers.insert(std::make_pair("Content-type", "application/json"));
      headers.insert(std::make_pair("Connection", "keep-alive"));
      headers.insert(std::make_pair("Accept", "*/*"));
    }
  
    cpr::Header cprHeaders;
    for (auto &kv : headers) cprHeaders.insert({kv.first, kv.second});
  
    cpr::Parameters cprParams;
    for (auto &kv : extraParams) cprParams.AddParameter({kv.first, kv.second});
  
    auto r = cpr::Get(cpr::Url{remoteUrl + path}, cprHeaders, cprParams,
                      cpr::VerifySsl(false));
    if (debug_qb_hw_) std::cout << "* [DEBUG]: r.status_code: " << r.status_code << "\n";
    if (r.status_code == 500) xacc::info("* Error: QB hardware process failure");
    if (r.status_code == 404) xacc::info("* Error: QB hardware received an invalid command");
    if (r.status_code != 200)
    {
      if (std::find(VALID_HTTP_RETURN_CODES_.begin(), VALID_HTTP_RETURN_CODES_.end(), r.status_code) != VALID_HTTP_RETURN_CODES_.end()) {
          json gr;
          gr["status_code"] = r.status_code;
          std::cout << "* [Debug]: r.status_code: " << r.status_code << "\n";
          return gr.dump();
      } else {
        throw std::runtime_error("HTTP GET Error - status code " +
                                 std::to_string(r.status_code) + ": " +
                                 r.error.message + ": " + r.text);
      }
    }
    
    return r.text;
  }

}
