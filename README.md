# CPP 实用工具类合集


## print

---
> 万能 `print`


## benchmark

---
> 测试代码运行时间

```c++
#include "benchmark.h"
 
int main() {
    TICK(test_name)
    // your test code
    TOCK(test_name)
    return 0;
}
```

## cppdemangle

---
> 输出任何类型的的字符串

```c++
std::basic_string<.....> ....
```


