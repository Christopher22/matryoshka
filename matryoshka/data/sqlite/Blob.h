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

  [[nodiscard]] inline int Size() const noexcept {
	return size_;
  }

  [[nodiscard]] inline const unsigned char *Data() const noexcept {
	return data_;
  }

  inline explicit operator const unsigned char *() const noexcept {
	return data_;
  }

  inline explicit operator bool() const noexcept {
	return data_ != nullptr;
  }

  template<bool O>
  inline bool operator==(const Blob<O> &rhs) const noexcept {
	return size_ == rhs.Size() && std::memcmp(data_, rhs.Data(), size_) == 0;
  }

  template<bool O>
  inline bool operator!=(const Blob<O> &rhs) const noexcept {
	return !(rhs == *this);
  }

 protected:
  const unsigned char *data_;
  int size_;
};

template<>
class Blob<true> {
 public:
  constexpr explicit Blob() noexcept: data_(nullptr), size_(0) {}
  inline explicit Blob(int size) : data_(new unsigned char[size]), size_(size) {}
  constexpr Blob(unsigned char *data, int size) noexcept: data_(data), size_(size) {}
  inline explicit Blob(const Blob<false> &shared) : data_(new unsigned char[shared.Size()]),
													size_(shared.Size()) {
	if (shared) {
	  std::memcpy(data_, shared.Data(), size_);
	}
  }
  inline Blob(Blob &&other) noexcept: data_(other.data_), size_(other.size_) {
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

  [[nodiscard]] inline unsigned char *Data() const noexcept {
	return data_;
  }

  [[nodiscard]] inline unsigned char *Data() noexcept {
	return data_;
  }

  [[nodiscard]] inline unsigned char *Release() {
	unsigned char *tmp = data_;
	data_ = nullptr;
	return tmp;
  }

  [[nodiscard]] inline int Size() const noexcept {
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

  static Blob<true> Filled(int num_bytes, unsigned char value = 0) {
	Blob<true> data(new unsigned char[num_bytes], num_bytes);
	std::fill_n(data.Data(), num_bytes, value);
	return data;
  }

 protected:
  unsigned char *data_;
  int size_;
};
}

#endif //MATRYOSHKA_MATRYOSHKA_DATA_SQLITE_BLOB_H_
