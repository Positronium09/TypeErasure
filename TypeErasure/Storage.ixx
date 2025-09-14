export module TypeErasure:Storage;

import std;

export namespace TypeErasure
{
	constexpr auto BUFFER_SIZE = sizeof(void*) * 2;

	template <std::size_t Size>
	concept SmallSize = Size <= BUFFER_SIZE;

	template <typename T>
	concept SmallObject = SmallSize<sizeof(T)> && SmallSize<alignof(T)>;

	template <std::size_t Size>
	struct Storage;

	template <std::size_t Size> requires (!SmallSize<Size>)
	struct Storage<Size>
	{
		using Buffer = std::array<std::byte, Size>;

		constexpr Storage() noexcept = default;
		constexpr Storage(const Storage& other) noexcept
		{
			std::memmove(ptr->data(), other.ptr->data(), Size);
		}
		constexpr auto operator=(const Storage& other) noexcept -> Storage&
		{
			if (this != &other)
			{
				std::memmove(ptr->data(), other.ptr->data(), Size);
			}
			return *this;
		}
		constexpr Storage(Storage&& other) noexcept = default;
		constexpr auto operator=(Storage&& other) noexcept -> Storage & = default;

		auto Allocate() noexcept
		{
			ptr = std::make_unique<Buffer>();
		}
		auto Deallocate() noexcept
		{
			ptr.reset();
		}
		auto Reset() noexcept
		{
			Deallocate();
		}
		auto IsAllocated() const noexcept
		{
			return ptr != nullptr;
		}

		auto GetBuffer() const noexcept
		{
			return std::span<const std::byte, Size>{ *ptr };
		}
		auto GetBuffer() noexcept
		{
			return std::span<std::byte, Size>{ *ptr };
		}
		auto GetPtr() const noexcept
		{
			return static_cast<void*>(*ptr);
		}

		static constexpr auto GetSize() noexcept
		{
			return Size;
		}

		private:
		std::unique_ptr<Buffer> ptr = std::make_unique<Buffer>();
	};

	template <std::size_t Size> requires SmallSize<Size>
	struct Storage<Size>
	{
		using Buffer = std::array<std::byte, Size>;
		constexpr Storage() noexcept = default;
		constexpr Storage(const Storage& other) noexcept
		{
			std::memmove(buffer.data(), other.buffer.data(), Size);
		}
		constexpr auto operator=(const Storage& other) noexcept -> Storage&
		{
			if (this != &other)
			{
				std::memmove(buffer.data(), other.buffer.data(), Size);
			}
			return *this;
		}
		constexpr Storage(Storage&& other) noexcept = default;
		constexpr auto operator=(Storage&& other) noexcept -> Storage& = default;

		auto Allocate() noexcept
		{
			std::memset(buffer.data(), 0, Size);
		}
		auto Deallocate() noexcept
		{
			std::memset(buffer.data(), 0, Size);
		}
		auto Reset() noexcept
		{
			Deallocate();
		}
		static auto IsAllocated() noexcept
		{
			return true;
		}

		auto GetBuffer() const noexcept
		{
			return std::span<const std::byte, Size>{ buffer };
		}
		auto GetBuffer() noexcept
		{
			return std::span<std::byte, Size>{ buffer };
		}
		auto GetPtr() const noexcept
		{
			return static_cast<void*>(buffer.data());
		}

		static constexpr auto GetSize() noexcept
		{
			return Size;
		}

		private:
		Buffer buffer{ };
	};

	template <typename T>
	struct TypedStorage : Storage<sizeof(T)>
	{
		auto GetObjectPtr() noexcept
		{
			return std::bit_cast<const T*>(this->GetPtr());
		}
		auto GetObjectPtr() const noexcept
		{
			return std::bit_cast<T*>(this->GetPtr());
		}

		auto GetObject() noexcept
		{
			return *GetObjectPtr();
		}
		auto GetObject() const noexcept
		{
			return *GetObjectPtr();
		}
	};
}
