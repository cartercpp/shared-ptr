#pragma once

#include <utility>
#include <atomic>
#include <cstddef>

template <typename ValueType>
struct default_deleter
{
	void operator()(ValueType* ptr)
	{
		delete ptr;
	}
};

template <typename ValueType>
struct default_deleter<ValueType[]>
{
	void operator()(ValueType* ptr)
	{
		delete[] ptr;
	}
};

template <typename ValueType, typename DestructorType = default_deleter<ValueType>>
class shared_ptr
{
public:

	// CONSTRUCTORS & STUFF

	shared_ptr(DestructorType destructor = {})
		: m_control{ nullptr }, m_destructor{ destructor }
	{
	}

	shared_ptr(ValueType* ptr, DestructorType destructor = {})
		: m_control{ new ControlBlock{ .m_ptr = ptr, .m_refCount = 1} },
		m_destructor{destructor}
	{
	}

	shared_ptr(const shared_ptr<ValueType, DestructorType>& other)
		: m_control{ other.m_control }, m_destructor{ other.m_destructor }
	{
		if (m_control)
			++m_control->m_refCount;
	}

	shared_ptr(shared_ptr<ValueType, DestructorType>&& other)
		: m_control{ other.m_control }, m_destructor{ std::move(other.m_destructor) }
	{
		other.m_control = nullptr;
	}

	auto& operator=(const shared_ptr<ValueType, DestructorType>& other)
	{
		if (this != &other)
		{
			reset();

			m_control = other.m_control;
			m_destructor = other.m_destructor;

			if (m_control)
				++m_control->m_refCount;
		}

		return *this;
	}

	auto& operator=(shared_ptr<ValueType, DestructorType>&& other)
	{
		if (this != &other)
		{
			reset();

			m_control = other.m_control;
			m_destructor = std::move(other.m_destructor);

			other.m_control = nullptr;
		}

		return *this;
	}
	
	~shared_ptr()
	{
		reset();
	}

	// METHODS & STUFF

	ValueType& operator*()
	{
		return *(m_control->m_ptr);
	}

	const ValueType& operator*() const
	{
		return *(m_control->m_ptr);
	}

	ValueType* operator->()
	{
		return m_control->m_ptr;
	}

	const ValueType* operator->() const
	{
		return m_control->m_ptr;
	}

	ValueType* get()
	{
		return m_control->m_ptr;
	}

	const ValueType* get() const
	{
		return m_control->m_ptr;
	}

	void reset()
	{
		if (m_control)
		{
			if (m_control->m_refCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
			{
				m_destructor(m_control->m_ptr);
				delete m_control;
			}

			m_control = nullptr;
		}
	}

	void swap(shared_ptr<ValueType, DestructorType>& other)
	{
		shared_ptr<ValueType, DestructorType> temp{ std::move(*this) };
		*this = std::move(other);
		other = std::move(temp);
	}
	
	int use_count() const
	{
		return m_control ? m_control->m_refCount.load() : 0;
	}

	bool unique() const noexcept
	{
		return m_control && (m_control->m_refCount.load() == 1);
	}

	operator bool() const noexcept
	{
		return m_control;
	}

private:

	struct ControlBlock
	{
		ValueType* m_ptr;
		std::atomic<int> m_refCount;
	};

	mutable ControlBlock* m_control;
	DestructorType m_destructor;
};

template <typename ValueType, typename DestructorType>
class shared_ptr<ValueType[], DestructorType>
{
public:

	// CONSTRUCTORS & STUFF

	shared_ptr(DestructorType destructor = {})
		: m_control{ nullptr }, m_destructor{ destructor }
	{
	}

	shared_ptr(ValueType* ptr, DestructorType destructor = {})
		: m_control{ new ControlBlock{.m_ptr = ptr, .m_refCount = 1} },
		m_destructor{ destructor }
	{
	}

	shared_ptr(const shared_ptr<ValueType[], DestructorType>& other)
		: m_control{ other.m_control }, m_destructor{ other.m_destructor }
	{
		if (m_control)
			++m_control->m_refCount;
	}

	shared_ptr(shared_ptr<ValueType[], DestructorType>&& other)
		: m_control{ other.m_control }, m_destructor{ std::move(other.m_destructor) }
	{
		other.m_control = nullptr;
	}

	auto& operator=(const shared_ptr<ValueType[], DestructorType>& other)
	{
		if (this != &other)
		{
			reset();

			m_control = other.m_control;
			m_destructor = other.m_destructor;

			if (m_control)
				++m_control->m_refCount;
		}

		return *this;
	}

	auto& operator=(shared_ptr<ValueType[], DestructorType>&& other)
	{
		if (this != &other)
		{
			reset();

			m_control = other.m_control;
			m_destructor = std::move(other.m_destructor);

			other.m_control = nullptr;
		}

		return *this;
	}

	~shared_ptr()
	{
		reset();
	}

	// METHODS & STUFF

	ValueType& operator[](std::size_t index)
	{
		return m_control->m_ptr[index];
	}

	const ValueType& operator[](std::size_t index) const
	{
		return m_control->m_ptr[index];
	}

	ValueType& operator*()
	{
		return *(m_control->m_ptr);
	}

	const ValueType& operator*() const
	{
		return *(m_control->m_ptr);
	}

	ValueType* operator->()
	{
		return m_control->m_ptr;
	}

	const ValueType* operator->() const
	{
		return m_control->m_ptr;
	}

	ValueType* get()
	{
		return m_control->m_ptr;
	}

	const ValueType* get() const
	{
		return m_control->m_ptr;
	}

	void reset()
	{
		if (m_control)
		{
			if (m_control->m_refCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
			{
				m_destructor(m_control->m_ptr);
				delete m_control;
			}

			m_control = nullptr;
		}
	}

	void swap(shared_ptr<ValueType[], DestructorType>& other)
	{
		shared_ptr<ValueType[], DestructorType> temp{std::move(*this)};
		*this = std::move(other);
		other = std::move(temp);
	}

	int use_count() const
	{
		return m_control ? m_control->m_refCount.load() : 0;
	}

	bool unique() const noexcept
	{
		return m_control && (m_control->m_refCount.load() == 1);
	}

	operator bool() const noexcept
	{
		return m_control;
	}

private:

	struct ControlBlock
	{
		ValueType* m_ptr;
		std::atomic<int> m_refCount;
	};

	mutable ControlBlock* m_control;
	DestructorType m_destructor;
};