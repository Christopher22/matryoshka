/*
This file is part of Matryoshka.
Copyright (C) 2020 Christopher Gundler <christopher@gundler.de>
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef MATRYOSHKA_MATRYOSHKA_DATA_FOLDER_H_
#define MATRYOSHKA_MATRYOSHKA_DATA_FOLDER_H_

#include "FileSystemObject.h"

namespace matryoshka::data {
class Folder : public FileSystemObject<0> {
  constexpr explicit Folder(FileSystemObject::HandleType handle) noexcept: FileSystemObject(handle) {}
};
}

#endif //MATRYOSHKA_MATRYOSHKA_DATA_FOLDER_H_
