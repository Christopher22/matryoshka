//
// Created by christopher on 28.04.2020.
//

#include "Server.h"

using namespace matryoshka::data;

namespace matryoshka::server {

Server::Server(data::FileSystem &&file_system) : file_system_(std::move(file_system)) {

}

restinio::request_handling_status_t Server::operator()(restinio::request_handle_t req) {
  const restinio::http_method_id_t method = req->header().method();
  if (method == restinio::http_method_get() || method == restinio::http_method_head()) {
	return this->handle_query(std::move(req));
  }

  return req->create_response(restinio::status_not_implemented()).done();
}

restinio::request_handling_status_t Server::handle_query(restinio::request_handle_t req) {
  // Parse the path and try to open the file
  const Path path(req->header().request_target());
  auto file_container = file_system_.Open(path);

  File *file;
  if ((file = std::get_if<File>(&file_container)) != nullptr) {
	// Query the file size
	const int file_size = file_system_.Size(*file);
	const bool header_only = req->header().method() == restinio::http_method_head();

	// Prepare the response
	auto response = req->create_response<restinio::user_controlled_output_t>(restinio::status_ok());
	response.append_header(restinio::http_field::server, "Matryoshka")
		.append_header_date_field()
		.set_content_length(file_size);

	// Send the data in GET, but not HEAD request
	if (!header_only) {
	  response.flush(); // Send header
	  // Write the data chunkwise
	  file_system_.Read(*file, 0, file_size, [&](FileSystem::Chunk &&blob) {
		response.append_body(std::forward<FileSystem::Chunk>(blob));
		response.flush();
		return true;
	  });
	}

	return response.done();
  } else {
	return req->create_response(restinio::status_not_found())
		.append_header(restinio::http_field::server, "Matryoshka")
		.append_header_date_field()
		.done();
  }
}

}