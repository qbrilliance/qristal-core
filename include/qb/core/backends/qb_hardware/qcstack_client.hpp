// Copyright (c) Quantum Brilliance Pty Ltd
#pragma once

#include "RemoteAccelerator.hpp"
#include <vector>

namespace xacc
{

  /**
   * @brief Qristal implementation of a client for QC Stack Server
   * 
   * Methods supported:
   *    POST
   *    GET
   */
  class QCStackClient : public xacc::Client
  {

    private:

      /// Toggles debug mode
      bool debug_qb_hw_;
    
      /// List of HTTP return codes that the client should interpret as: "continue with polling"
      const std::vector<int> VALID_HTTP_RETURN_CODES_{425};
    
    public:
      
      /// Constructor that inits the parent class (xacc::Client)
      QCStackClient(const bool debug) : xacc::Client(), debug_qb_hw_(debug) {}

      /// Default constructor with no debugging output
      QCStackClient() : QCStackClient(false) {}
    
      /**
       * @brief HTTP POST implementation
       *
       * @param remoteUrl Input the URL where the server is located
       * @param path Input the endpoint path that handles the POST request
       * @param postStr Input body of the request, in JSON format
       * @param headers Input key-value pairs that are used for the HTTP Header
       */
      const std::string post(const std::string &remoteUrl, const std::string &path,
                             const std::string &postStr,
                             std::map<std::string, std::string> headers =
                                 std::map<std::string, std::string>{});
    
      /**
       * @brief HTTP GET implementation
       *
       * @param remoteUrl Input the URL where the QC Stack server is located
       * @param path Input the endpoint path that handles the GET request
       * @param headers Input key-value pairs that are used for the HTTP Header
       * @param extraParams Input key-value pairs that are used for query parameters
       */
      const std::string get(const std::string &remoteUrl, const std::string &path,
                            std::map<std::string, std::string> headers =
                                std::map<std::string, std::string>{},
                            std::map<std::string, std::string> extraParams = {});
  };

}
