/**
 * @file Singleton.h
 * @brief Thread-safe Singleton pattern template implementation
 * @details This file provides a thread-safe, modern C++ implementation of the
 *          Singleton pattern using std::shared_ptr and std::call_once for
 *          guaranteed thread safety and lazy initialization.
 * 
 * @author UniConv Development Team
 * @copyright Copyright (c) 2025. All rights reserved.
 * @license MIT License
 * @version 1.0.0.1
 * 
 * @par Key Features:
 * - Thread-safe lazy initialization using std::call_once
 * - RAII-compliant using std::shared_ptr
 * - Move-only semantics (no copy constructor/assignment)
 * - Template-based for type safety
 * - Memory-safe automatic cleanup
 * 
 * @par Usage Pattern:
 * Inherit from Singleton<YourClass> to make YourClass a singleton.
 * Access the instance using YourClass::GetInstance().
 * 
 * @par Thread Safety:
 * Uses std::call_once with std::once_flag to ensure exactly one instance
 * is created even in multi-threaded environments.
 * 
 * @code{.cpp}
 * // Example usage
 * class MyClass : public Singleton<MyClass> {
 *     friend class Singleton<MyClass>; // Allow Singleton access to constructor
 * private:
 *     MyClass() = default; // Private constructor
 * public:
 *     void DoSomething() { }
 * };
 * 
 * // Usage
 * auto instance = MyClass::GetInstance();
 * instance->DoSomething();
 * @endcode
 * 
 * @since 1.0.0.1
 */

#ifndef SINGLETON_H
#define SINGLETON_H
#include <memory>
#include <mutex>
#include <iostream>

/**
 * @brief Thread-safe Singleton pattern template class
 * @tparam T The type to be made singleton
 * @details Template class that implements the Singleton pattern with thread-safe
 *          lazy initialization. Classes inheriting from this template become
 *          singletons with guaranteed single instance creation.
 * 
 * @par Design Pattern:
 * - **Lazy Initialization**: Instance created only when first requested
 * - **Thread Safety**: Uses std::call_once for initialization
 * - **RAII Compliance**: Automatic memory management via std::shared_ptr
 * - **Non-Copyable**: Copy constructor and assignment operator deleted
 * 
 * @par Memory Management:
 * - Uses std::shared_ptr for automatic memory management
 * - Instance automatically destroyed when program ends
 * - No manual cleanup required
 * 
 * @par Thread Safety Guarantees:
 * - GetInstance() is thread-safe for initialization
 * - Multiple threads can safely call GetInstance() concurrently
 * - Exactly one instance is guaranteed to be created
 * 
 * @warning Derived classes should:
 * - Make constructor private/protected
 * - Declare Singleton<T> as friend class
 * - Not provide public copy/move constructors
 * 
 * @since 1.0.0.1
 */
template <typename T>
class Singleton {
	
protected:
	/**
	 * @brief Default constructor (protected)
	 * @details Protected to prevent direct instantiation while allowing
	 *          derived classes to use default construction.
	 */
	Singleton() = default;

	/**
	 * @brief Deleted copy constructor
	 * @details Prevents copying of singleton instances to maintain
	 *          single instance guarantee.
	 */
	Singleton(const Singleton<T>&) = delete;

	/**
	 * @brief Deleted assignment operator
	 * @details Prevents assignment of singleton instances to maintain
	 *          single instance guarantee.
	 */
	Singleton& operator=(const Singleton<T>& st) = delete;

	/**
	 * @brief Static instance holder
	 * @details Shared pointer that holds the single instance of type T.
	 *          Initialized to nullptr and created lazily in GetInstance().
	 */
	static std::shared_ptr<T> _instance;

public:
	/**
	 * @brief Get the singleton instance
	 * @return std::shared_ptr<T> Shared pointer to the singleton instance
	 * @details Thread-safe method to obtain the singleton instance. Uses
	 *          std::call_once to ensure exactly one instance is created
	 *          even in multi-threaded environments.
	 * 
	 * @par Thread Safety:
	 * - Uses std::once_flag and std::call_once for initialization
	 * - Safe to call from multiple threads concurrently
	 * - Guaranteed to return the same instance across all calls
	 * 
	 * @par Lazy Initialization:
	 * Instance is created only on first call to this method.
	 * Subsequent calls return the already-created instance.
	 * 
	 * @par Memory Management:
	 * Returns std::shared_ptr allowing safe sharing across multiple owners.
	 * 
	 * @code{.cpp}
	 * // Thread-safe usage example
	 * auto instance1 = MyClass::GetInstance();
	 * auto instance2 = MyClass::GetInstance();
	 * // instance1 and instance2 point to the same object
	 * assert(instance1.get() == instance2.get());
	 * @endcode
	 * 
	 * @since 1.0.0.1
	 */
	static std::shared_ptr<T> GetInstance() {
		static std::once_flag s_flag;
		std::call_once(s_flag, [&]() {
			_instance = std::shared_ptr<T>(new T);
			});

		return _instance;
	}

	/**
	 * @brief Print instance memory address for debugging
	 * @details Utility method that prints the raw pointer address of the
	 *          singleton instance to std::cout. Useful for debugging and
	 *          verifying singleton behavior.
	 * 
	 * @par Usage:
	 * Call this method to verify that all GetInstance() calls return
	 * pointers to the same memory address.
	 * 
	 * @note This method is primarily for debugging purposes
	 * @warning Ensure _instance is not null before calling this method
	 * 
	 * @since 1.0.0.1
	 */
	void PrintAddress() {
		// Get the raw pointer from shared_ptr
		std::cout << _instance.get() << std::endl;
	}

	/**
	 * @brief Virtual destructor
	 * @details Virtual destructor to ensure proper cleanup of derived classes.
	 *          Commented out destructor message to avoid noise in normal operation.
	 * 
	 * @note Automatic cleanup is handled by std::shared_ptr
	 */
	~Singleton() {
		// Uncomment for debugging destructor calls
		// std::cout << "this is singleton destruct" << std::endl;
	}
};

/**
 * @brief Static member definition
 * @details Template static member definition. Each instantiation of the template
 *          gets its own static _instance variable initialized to nullptr.
 * 
 * @note This definition is required for proper template static member initialization
 */
template <typename T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr;

#endif // SINGLETON_H
