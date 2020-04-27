//
// Created by christopher on 13.03.2020.
//

#ifndef MATRYOSHKA_MATRYOSHKA_DATA_SQLITE_BLOB_H_
#define MATRYOSHKA_MATRYOSHKA_DATA_SQLITE_BLOB_H_

#include <cassert>
#include <fstream>

namespace matryoshka::data::sqlite {

class BlobBase {
 public:
  [[nodiscard]] virtual const unsigned char *Data() const noexcept = 0;

  [[nodiscard]] inline int Size() const noexcept {
	return size_;
  }

  inline explicit operator bool() const noexcept {
	return this->Data() != nullptr && size_ > 0;
  }

  inline explicit operator const unsigned char *() const noexcept {
	return this->Data();
  }

  [[nodiscard]] const unsigned char &operator[](int index) const noexcept {
	return this->Data()[index];
  }

  [[nodiscard]] inline bool has_equal_content(const BlobBase *rhs) const noexcept {
	return size_ == rhs->Size() && std::memcmp(this->Data(), rhs->Data(), size_) == 0;
  }

  bool Save(std::string_view path, bool append = false) const {
	std::ofstream output(path.data(),
						 std::ifstream::out | std::ifstream::binary
							 | (append ? std::ifstream::app : std::ifstream::trunc));
	if (output) {
	  output.write(reinterpret_cast<const char *>(this->Data()), size_);
	  if (output) {
		return true;
	  }
	}
	return false;
  }

 protected:
  constexpr explicit BlobBase(int size) noexcept: size_(size) {}
  int size_;
};

template<bool HasOwnership>
class Blob {};

template<>
class Blob<false> : public BlobBase {
 public:
  constexpr inline Blob(const unsigned char *data, int size) noexcept: BlobBase(size), data_(data) {}

  [[nodiscard]] inline const unsigned char *Data() const noexcept final {
	return data_;
  }

 protected:
  const unsigned char *data_;
};

template<>
class Blob<true> : public BlobBase {
 public:
  constexpr explicit Blob() noexcept: BlobBase(0), data_(nullptr) {}
  inline explicit Blob(int size) : BlobBase(size), data_(new unsigned char[size]) {}
  constexpr Blob(unsigned char *data, int size) noexcept: BlobBase(size), data_(data) {}
  inline explicit Blob(const Blob<false> &shared) : BlobBase(shared.Size()), data_(new unsigned char[shared.Size()]) {
	if (data_ && shared) {
	  std::memcpy(data_, shared.Data(), size_);
	}
  }
  explicit Blob(std::string_view path, int maximal_size = -1) : BlobBase(0), data_(nullptr) {
	std::ifstream file(path.data(), std::ifstream::in | std::ifstream::binary);
	if (file) {
	  // Get file length
	  file.seekg(0, std::ifstream::end);
	  int length = file.tellg();
	  file.seekg(0, std::ifstream::beg);

	  // Enforce maximal size
	  if (maximal_size >= 0 && length >= maximal_size) {
		return;
	  }

	  // Read the data
	  data_ = new unsigned char[length];
	  file.read(reinterpret_cast<char *>(data_), length);
	  size_ = file.gcount();
	}
  }
  inline Blob(Blob &&other) noexcept: BlobBase(other.size_), data_(other.data_) {
	other.data_ = nullptr;
  }

  Blob(Blob const &) = delete;
  Blob &operator=(Blob const &) = delete;
  Blob &operator=(Blob &&other) noexcept {
	size_ = other.size_;
	delete[] data_;

	data_ = other.data_;
	other.data_ = nullptr;
	return *this;
  }

  ~Blob() noexcept {
	delete[] data_;
  }

  [[nodiscard]] Blob<true> Copy() const {
	Blob<true> result(size_);
	if (result) {
	  std::memcpy(result.data_, data_, size_);
	}
	return result;
  }

  [[nodiscard]] inline Blob<false> Part(int length, int onset = 0) {
	assert(onset + length <= size_);
	return Blob<false>(&data_[onset], length);
  }

  [[nodiscard]] inline const unsigned char *Data() const noexcept final {
	return data_;
  }

  [[nodiscard]] inline unsigned char *Data() noexcept {
	return data_;
  }

  inline explicit operator unsigned char *() const noexcept {
	return data_;
  }

  [[nodiscard]] inline unsigned char *Release() {
	unsigned char *tmp = data_;
	data_ = nullptr;
	return tmp;
  }

  [[nodiscard]] unsigned char &operator[](int index) noexcept {
	return data_[index];
  }

  inline explicit operator Blob<false>() const noexcept {
	return Blob<false>(data_, size_);
  }

  static Blob<true> Filled(int num_bytes, unsigned char value = 0) {
	Blob<true> data(new unsigned char[num_bytes], num_bytes);
	std::fill_n(data.Data(), num_bytes, value);
	return data;
  }

 protected:
  unsigned char *data_;
};

// All the allowed comparisons
static inline bool operator==(const Blob<false> &lhs, const Blob<false> &rhs) noexcept {
  return lhs.has_equal_content(&rhs);
}
static inline bool operator!=(const Blob<false> &lhs, const Blob<false> &rhs) noexcept {
  return !(lhs == rhs);
}
static inline bool operator==(const Blob<false> &lhs, const Blob<true> &rhs) noexcept {
  return lhs.has_equal_content(&rhs);
}
static inline bool operator!=(const Blob<false> &lhs, const Blob<true> &rhs) noexcept {
  return !(lhs == rhs);
}
static inline bool operator==(const Blob<true> &lhs, const Blob<true> &rhs) noexcept {
  return lhs.has_equal_content(&rhs);
}
static inline bool operator!=(const Blob<true> &lhs, const Blob<true> &rhs) noexcept {
  return !(lhs == rhs);
}
static inline bool operator==(const Blob<true> &lhs, const Blob<false> &rhs) noexcept {
  return lhs.has_equal_content(&rhs);
}
static inline bool operator!=(const Blob<true> &lhs, const Blob<false> &rhs) noexcept {
  return !(lhs == rhs);
}
}

#endif //MATRYOSHKA_MATRYOSHKA_DATA_SQLITE_BLOB_H_
