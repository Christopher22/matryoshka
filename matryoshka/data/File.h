//
// Created by christopher on 19.03.2020.
//

#ifndef MATRYOSHKA_MATRYOSHKA_DATA_FILE_H_
#define MATRYOSHKA_MATRYOSHKA_DATA_FILE_H_

#include "FileSystemObject.h"

namespace matryoshka::data {
class File : public FileSystemObject<1> {
 public:
  constexpr explicit File(FileSystemObject::HandleType handle) noexcept: FileSystemObject(handle) {}

  inline File(File &&other) noexcept: File(other.Handle()) {
	other.Invalidate();
  }

  File &operator=(File &&other) noexcept {
	id_ = other.id_;
	other.Invalidate();
	return *this;
  }
};
}

#endif //MATRYOSHKA_MATRYOSHKA_DATA_FILE_H_
