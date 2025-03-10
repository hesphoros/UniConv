# string_view

创建string_view时候不存在任何动态分配内存，不存在字符串的遍历

- **一直以来，对于C字符串而言，如果需要获取它的长度，至少需要strlen之类的函数。但是我们似乎忽略了一点，那就是，如果是静态字符串，编译器其实是知道它的长度的，也就是，静态字符串的长度可以在编译期间确定，那就可以减少了很多问题。**

**题外话**：编译期确定字符串长度、对象大小，这种并不是什么奇技淫巧，因为早在`operator new`运算符重载的时候，就有一个size_t参数，这个就是编译器传入的对象大小，而std::string_view，则是在编译期间传入字符串的指针和长度，构建对象。但是，std::string和std::string_view这两个类同时提供了`只带字符串指针`和`同时带字符串指针和字符串长度`两个版本的构造函数，默认的情况下，`std::string str = "this is a static string!"`会调用`basic_string( const CharT* s,const Allocator& alloc = Allocator() )`构造，但是`std::string_view sv = "this is a static string!"`会调用带长度的`basic_string_view(const _CharT* __str, size_type __len) noexcept`版本，

std::string_view是指一种轻量级的字符串视图类，它是C++17标准引入的一个重要特性。在原文中，std::string_view sv = "this is a static string"; 这一行代码展示了如何创建一个std::string_view对象，并将其初始化为一个字符串字面量。

#### 字符串视图

C++17中我们可以使用std::string_view来获取一个字符串的视图，字符串视图并不真正的创建或者拷贝字符串，而只是拥有一个字符串的查看功能。std::string_view比std::string的性能要高很多，因为每个std::string都独自拥有一份字符串的拷贝，而std::string_view只是记录了自己对应的字符串的指针和偏移位置。当我们在只是查看字符串的函数中可以直接使用std::string_view来代替

具体来说，std::string_view有以下几个关键特点：

- 可以避免不必要的拷贝

#### 轻量级

std::string_view本身非常轻巧，因为它只包含一个指向字符串数据的指针和一个表示字符串长度的整数值。这使得它在内存占用方面远小于std::string。

它不分配或管理内存，因此开销非常小

#### 非拥有式

不同于std::string，std::string_view不会管理或拥有底层字符串的内存。它只是一个观察者，可以用来检查和操作现有字符串数据，但不会自行负责内存的分配或释放。



#### 零拷贝

因为std::string_view不持有自己的副本，所以在调用它时不会涉及任何内存复制，从而提高了性能，尤其是在需要多次访问大型字符串数据时。

#### 只读性

std::string_view是只读的，意味着它只能用于查看和操作字符串，而不能对其进行修改。这增加了安全性，防止意外的数据改变

## 使用场景

- 只读访问字符串数据。
- 避免不必要的字符串拷贝。
- 提高函数接口的通用性。

1. **替代 const char\***：
   - 当你需要一个只读的 C 风格字符串时，可以使用 std::string_view 替代 const char*。
2. **替代 const std::string&**：
   - 当你需要一个只读的 std::string 引用时，可以使用 std::string_view 替代 const std::string&。这样可以避免不必要的临时对象创建。
3. **统一接口**：
   - 使用 std::string_view 可以使函数能够接受多种类型的字符串输入（如 const char*, 字符串字面量, std::string 等），而不需要为每种情况单独写不同的重载版本。



