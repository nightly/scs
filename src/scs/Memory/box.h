#pragma once

#include <memory>

namespace scs {

	/*
	 * @brief Heap allocates the T, but has value semantics, i.e. "value_ptr"
	 */
	template <typename T>
	class Box {
	public:
		using element_type = T;
	private:
		std::unique_ptr<T> ptr_;
	public:
		Box(T&& obj) : ptr_(std::make_unique<T>(std::move(obj))) {}
		Box(const T& obj) : ptr_(std::make_unique<T>(obj)) {}
		explicit Box(T* e) noexcept : ptr_{e} {}

		Box(const Box& other) : Box(*other.ptr_) {} // Copies T
		Box& operator=(const Box& other) {
			*ptr_ = *other.ptr_;
			return *this;
		}
		Box(Box&& other) noexcept = default;
		Box& operator=(Box&& other) noexcept = default;
		~Box() = default;

		T& operator*() { return *ptr_; }
		const T& operator*() const { return *ptr_; }

		T* operator->() { return ptr_.get(); }
		const T* operator->() const { return ptr_.get(); }

		bool operator==(const Box& other) const {
			return *ptr_ == *other.ptr_;
		}
		
		bool operator!=(const Box& other) const {
			return *ptr_ != *other.ptr_;
		}
	};


}