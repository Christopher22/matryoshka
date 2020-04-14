//
// Created by christopher on 19.03.2020.
//

#ifndef MATRYOSHKA_MATRYOSHKA_DATA_FOLDER_H_
#define MATRYOSHKA_MATRYOSHKA_DATA_FOLDER_H_

#include "FileSystemObject.h"

namespace matryoshka::data {
class Folder : public FileSystemObject<0> {
  constexpr explicit Folder(FileSystemObject::HandleType handle) noexcept: FileSystemObject(handle) {}
};
}

#endif //MATRYOSHKA_MATRYOSHKA_DATA_FOLDER_H_
