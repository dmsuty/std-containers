#include <iostream>

template<typename T>
class Deque {

private:
  template<bool is_const>
  struct basic_iterator {
    using ValueType = std::conditional_t<is_const, const T, T>;
    using PointerType = ValueType*;
    using DoublePointer = std::conditional_t<is_const, const PointerType*, T**>;

    using difference_type = std::ptrdiff_t;
    using iterator_category = std::random_access_iterator_tag;
    using value_type = typename std::conditional<is_const, const T, T>::type;

    DoublePointer backet_pointer_;
    PointerType array_pointer_;
    size_t index_;

    explicit basic_iterator() = default;

    explicit basic_iterator(DoublePointer backet_pointer, PointerType array_pointer, int index):
      backet_pointer_(backet_pointer), array_pointer_(array_pointer), index_(index) {}

    explicit basic_iterator(DoublePointer backet_pointer):
      backet_pointer_(backet_pointer), array_pointer_(*backet_pointer_), index_(0) {}

    basic_iterator& operator++ () noexcept {
      ++index_;
      if (index_ == kArray_size_) {
        backet_pointer_++;
        array_pointer_ = *backet_pointer_;
        index_ = 0;
      }
      return *this;
    }

    basic_iterator operator++ (int) noexcept {
      basic_iterator result = *this;
      ++(*this);
      return result;
    }

    basic_iterator& operator-- () noexcept {
      if (index_ == 0) {
        backet_pointer_--;
        array_pointer_ = *backet_pointer_;
        index_ = kArray_size_ - 1;
      } else {
        --index_;
      }
      return *this;
    }

    basic_iterator operator-- (int) noexcept {
      basic_iterator result = *this;
      --(*this);
      return result;
    }

    long int operator- (const basic_iterator& other) const noexcept {
      long int backets_between = backet_pointer_ - other.backet_pointer_;
      return backets_between * kArray_size_ + index_ - other.index_;
    }

    basic_iterator& operator+= (int step) noexcept {
      step += index_;
      int backets_step = step / kArray_size_;
      backet_pointer_ += backets_step;
      array_pointer_ = *backet_pointer_;
      index_ = step % kArray_size_;
      return *(this);
    }

    basic_iterator operator+ (int step) const noexcept {
      basic_iterator result = *this;
      result += step;
      return result;
    }

    basic_iterator& operator-= (int step) noexcept {
      step += kArray_size_ - index_ - 1;
      int backets_step = step / kArray_size_;
      backet_pointer_ -= backets_step;
      array_pointer_ = *backet_pointer_;
      index_ = kArray_size_ - 1 - step % kArray_size_;
      return *(this);
    }

    basic_iterator operator- (int step) const noexcept {
      basic_iterator result = *this;
      result -= step;
      return result;
    }

    bool operator< (const basic_iterator& other) const noexcept {
      return *this - other < 0;
    }

    bool operator== (const basic_iterator& other) const noexcept {
      return *this - other == 0;
    }

    bool operator> (const basic_iterator& other) const noexcept {
      return other < *this;
    }

    bool operator!= (const basic_iterator& other) const noexcept {
      return !(*this == other);
    }

    bool operator<= (const basic_iterator& other) const noexcept {
      return !(*this > other);
    }

    bool operator>= (const basic_iterator& other) const noexcept {
      return !(*this < other);
    }

    ValueType& operator* () const noexcept {
      return array_pointer_[index_];
    }

    PointerType operator-> () const noexcept {
      return array_pointer_ + index_;
    }

    void Construct(const T& element) {
      try {
        new(array_pointer_ + index_) T(element);
      } catch (...) {}
    }

    operator basic_iterator<true>() const noexcept {
      return basic_iterator<true>(backet_pointer_, array_pointer_, index_);
    }
  };


public:
  using value_type = T;
  using iterator = basic_iterator<false>;
  using const_iterator = basic_iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
  static const size_t kArray_size_ = 10'000;
  static const size_t kByte_Array_size_ = sizeof(T) * kArray_size_;
  size_t backets_count_;
  T** backets_;
  iterator begin_;
  iterator end_;

  void Expand() {
    try {
      T** new_arrays = new T*[backets_count_ * 3];
    } catch (...) {
      delete[] new_arrays;
    }
    try {
      for (size_t i = 0; i < backets_count_ * 3; ++i) {
        if (i < backets_count_ || (i >= backets_count_ * 2)) {
          new_arrays[i] = reinterpret_cast<T*>(new uint8_t[kByte_Array_size_]);
        } else {
          new_arrays[i] = backets_[i - backets_count_];
        }
      }
      UpdateIteratorAfterExpand(new_arrays, begin_);
      UpdateIteratorAfterExpand(new_arrays, end_);
      delete[] backets_;
      backets_ = new_arrays;
      backets_count_ *= 3;
    } catch (...) {
      for (size_t i = 0; i < backets_count_ * 3; ++i) {
        if (i < backets_count_ || (i >= backets_count_ * 2)) {
          delete[] new_arrays[i];
        }
      }
      delete[] new_arrays;
    }
  }

  void UpdateIteratorAfterExpand(T** new_arrays, iterator& iter) {
    int backets_number = iter.backet_pointer_ - backets_;
    iter.backet_pointer_ = new_arrays + (backets_number + backets_count_);
    iter.array_pointer_ = *(iter.backet_pointer_);
  }

  iterator ToIter(int i) {
    return begin_ + i;
  }

  bool FreePlaceBack() {
    return !(end_.backet_pointer_ == backets_ + backets_count_ - 1 && end_.index_ == kArray_size_ - 1);
  }

  bool FreePlaceFront() {
    return !(begin_.backet_pointer_ == backets_ && begin_.index_ == 0);
  }

  void swap(Deque& other) {
    std::swap(backets_, other.backets_);
    std::swap(backets_count_, other.backets_count_);
    std::swap(begin_, other.begin_);
    std::swap(end_, other.end_);
  }

  void clear() {
    while (size()) {
      this->pop_back();
    }
  }

public:
  Deque(): backets_count_(1), backets_(new T*[1]) {
    try {
      backets_[0] = reinterpret_cast<T*>(new uint8_t[kByte_Array_size_]);
      begin_ = iterator(backets_) + kArray_size_ / 2;
      end_ = begin_;
    } catch (...) {
      delete[] backets_;
      throw;
    }
  }

  Deque(int new_size): Deque() {
    for (int i = 0; i < new_size; ++i) {
      (*this).push_back(T());
    }
  }

  Deque(int new_size, const T& element): Deque() {
    for (int i = 0; i < new_size; ++i) {
      (*this).push_back(element);
    }
  }

  Deque(const Deque& other):
      backets_count_(other.backets_count_), backets_(new T*[backets_count_]) {
    for (size_t i = 0; i < backets_count_; ++i) {
      backets_[i] = reinterpret_cast<T*>(new uint8_t[kByte_Array_size_]);
    }
    begin_ = iterator(backets_) + (other.begin_ - iterator(other.backets_));
    end_ = begin_ + other.size();
    iterator curr_iter = begin_;
    iterator other_iter = other.begin_;
    while (curr_iter != end_) {
      curr_iter.Construct(*other_iter);
      ++curr_iter;
      ++other_iter;
    }
  }

  ~Deque() {
    for (size_t i = 0; i < backets_count_; ++i) {
      delete[] backets_[i];
    }
    delete[] backets_;
  }

  Deque& operator=(const Deque &other) {
    Deque copy(other);
    swap(copy);
    return *this;
  }

  size_t size() const noexcept {
    return end_ - begin_;
  }

  bool empty() const noexcept {
    return size() == 0;
  }

  T& operator[] (size_t i) noexcept {
    return *(ToIter(i));
  }

  const T& operator[] (size_t i) const noexcept {
    return *(ToIter(i));
  }

  T& at(size_t i) {
    if (i >= size()) {
      throw std::out_of_range("Out of range ;(");
    }
    return (*this)[i];
  }

  const T& at(size_t i) const {
    if (i >= size()) {
      throw std::out_of_range("Out of range ;(");
    }
    return (*this)[i];
  }

  void push_back(const T& element) {
    if (!FreePlaceBack()) {
      Expand();
    }
    end_.Construct(element);
    ++end_;
  }

  void push_front(const T& element) {
    if (!FreePlaceFront()) {
      Expand();
    }
    --begin_;
    begin_.Construct(element);
  }

  void pop_back() {
    if (size() == 0) {
      throw;
    }
    --end_;
  }

  void pop_front() {
    if (size() == 0) {
      throw;
    }
    ++begin_;
  }

  void erase(iterator iter) {
    if (size() == 0) {
      throw;
    }
    for (iterator curr_iter = iter; curr_iter + 1 != end_; curr_iter++) {
      std::swap(*(curr_iter), *(curr_iter + 1));
    }
    this->pop_back();
  }

  iterator insert(iterator iter, const T& element) {
    int shift = iter - begin_;
    try {
      this->push_back(element);
    } catch (...) {
      throw;
    }
    for (iterator curr_iter = end_ - 1; curr_iter - begin_ != shift; curr_iter--) {
      std::swap(*(curr_iter), *(curr_iter - 1));
    }
    return begin_ + shift;
  }

  iterator begin() noexcept {
    return begin_;
  }

  iterator end() noexcept {
    return end_;
  }

  const_iterator begin() const noexcept {
    return cbegin();
  }

  const_iterator end() const noexcept {
    return cend();
  }

  reverse_iterator rbegin() noexcept {
    return reverse_iterator(end_);
  }

  reverse_iterator rend() noexcept {
    return reverse_iterator(begin_);
  }

  const_iterator cbegin() const  noexcept {
    return const_iterator(begin_);
  }

  const_iterator cend() const  noexcept {
    return const_iterator(end_);
  }

  const_reverse_iterator crbegin() const noexcept {
    return std::make_reverse_iterator(cbegin());
  }

  const_reverse_iterator crend() const noexcept {
    return std::make_reverse_iterator(cend());
  }
};
