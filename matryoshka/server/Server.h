//
// Created by christopher on 28.04.2020.
//

#ifndef MATRYOSHKA_MATRYOSHKA_SERVER_SERVER_H_
#define MATRYOSHKA_MATRYOSHKA_SERVER_SERVER_H_

#include "../data/FileSystem.h"

#include <restinio/all.hpp>

namespace matryoshka::server {
class Server {
 public:
  explicit Server(matryoshka::data::FileSystem &&file_system);
  restinio::request_handling_status_t operator()(restinio::request_handle_t req);

 protected:
  restinio::request_handling_status_t handle_query(restinio::request_handle_t req);

 private:
  matryoshka::data::FileSystem file_system_;
};
}

#endif //MATRYOSHKA_MATRYOSHKA_SERVER_SERVER_H_
