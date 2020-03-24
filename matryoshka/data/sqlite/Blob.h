//
// Created by christopher on 13.03.2020.
//

#ifndef MATRYOSHKA_MATRYOSHKA_DATA_SQLITE_BLOB_H_
#define MATRYOSHKA_MATRYOSHKA_DATA_SQLITE_BLOB_H_

namespace matryoshka::data::sqlite {

template<bool HasOwnership>
class Blob {};

template<>
class Blob<false> {
 public:
  constexpr inline Blob(const unsigned char *data, int size) noexcept: data_(data), size_(size) {}

  [[nodiscard]] inline int size() const noexcept {
	return size_;
  }

  [[nodiscard]] inline const unsigned char *data() const noexcept {
	return data_;
  }

  inline explicit operator const unsigned char *() const noexcept {
	return data_;
  }

  inline explicit operator bool() const noexcept {
	return data_ != nullptr;
  }

  template<bool O>
  inline bool operator==(const Blob<O> &rhs) const {
	return size_ == rhs.size() && std::memcmp(data_, rhs.data(), size_) == 0;
  }

  template<bool O>
  inline bool operator!=(const Blob<O> &rhs) const {
	return !(rhs == *this);
  }

 protected:
  const unsigned char *data_;
  int size_;
};

template<>
class Blob<true> {
 public:
  constexpr inline explicit Blob(int size) noexcept: data_(nullptr), size_(size) {}
  constexpr inline Blob(unsigned char *data, int size) noexcept: data_(data), size_(size) {}
  inline explicit Blob(const Blob<false> &shared) noexcept: data_(new unsigned char[shared.size()]),
															size_(shared.size()) {
	if (shared) {
	  std::memcpy(data_, shared.data(), size_);
	}
  }
  inline Blob(Blob &&other) noexcept: data_(other.data_), size_(other.size_) {
	other.data_ = nullptr;
  }

  Blob(Blob const &) = delete;
  Blob &operator=(Blob const &) = delete;
  Blob &operator=(Blob &&other) noexcept {
	size_ = other.size_;
	if (data_ != nullptr) {
	  delete[] data_;
	}
	data_ = other.data_;
	other.data_ = nullptr;
	return *this;
  }

  ~Blob() noexcept {
	delete[] data_;
  }

  [[nodiscard]] inline unsigned char *data() const noexcept {
	return data_;
  }

  [[nodiscard]] inline unsigned char *release() {
	unsigned char *tmp = data_;
	data_ = nullptr;
	return tmp;
  }

  [[nodiscard]] inline int size() const noexcept {
	return size_;
  }

  template<bool O>
  inline bool operator==(const Blob<O> &rhs) const {
	return size_ == rhs.size_ && std::memcmp(data_, rhs.data_, size_) == 0;
  }

  template<bool O>
  inline bool operator!=(const Blob<O> &rhs) const {
	return !(rhs == *this);
  }

  inline explicit operator unsigned char *() const noexcept {
	return data_;
  }

  inline explicit operator bool() const noexcept {
	return data_ != nullptr;
  }

  inline explicit operator Blob<false>() const noexcept {
	return Blob<false>(data_, size_);
  }

  static Blob<true> filled(int num_bytes, unsigned char value = 0) {
	Blob<true> data(new unsigned char[num_bytes], num_bytes);
	std::fill_n(data.data(), num_bytes, value);
	return data;
  }

 protected:
  unsigned char *data_;
  int size_;
};
}

#endif //MATRYOSHKA_MATRYOSHKA_DATA_SQLITE_BLOB_H_
