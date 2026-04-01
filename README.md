@'
# CEL (C Extension Library)

[![Platform](https://img.shields.io/badge/platform-UNIX%20%7C%20Windows-blue)]()
[![License](https://img.shields.io/badge/license-GPL--2.0-green)]()
[![Version](https://img.shields.io/badge/version-1.6.3.83-orange)]()

## 📖 项目简介

**CEL (C Extension Library)** 是一个用 C 语言编写的高性能、跨平台基础扩展库，提供常用的数据结构、系统操作、数据库连接池、加解密、网络通信及协程等功能模块。旨在为 C/C++ 微服务项目提供一套统一、高效、易用的基础工具库。

> **Copyright (C) 2008 - 2017 Hu Jinya <hu_jinya@163.com>**

基于cel开发一个restful微服务示例可参考https://github.com/hujinya/cel-demo

## ✨ 主要特性

| 特性 | 说明 |
|------|------|
| 🖥️ 跨平台 | 支持 UNIX (Linux/macOS) 和 Windows 操作系统 |
| ⚡ 高性能 | 底层 C 语言实现，内存管理优化，执行效率高 |
| 🧩 模块化 | 50+ 功能模块独立，可按需引用 |
| 🔒 安全性 | 提供 AES、DES、MD5、SHA 等加解密支持 |
| 🗄️ 数据库 | MySQL 连接池封装，支持连接复用 |
| 🌐 网络 | HTTP Server/Client、TCP/SSL Socket、事件循环 |
| 🧵 并发 | 线程池、协程、定时器堆/轮 |
| 📝 易集成 | 简单的 make 编译流程，快速接入项目 |

## 📁 目录结构

cel/
├── README.md              # 项目说明文档
├── Makefile               # 顶层编译配置
├── tmpl.mk                # Makefile 模板
├── obj/                   # 编译对象文件目录
├── bin/                   # 编译输出目录
│   └── cel-example        # 测试示例程序
├── include/cel/           # 头文件目录
│   ├── *.h                # 各模块头文件
│   ├── net/               # 网络相关
│   ├── _unix/             # UNIX 平台封装（内部使用）⚠️
│   └── _win/              # Windows 平台封装（内部使用）⚠️
├── src/                   # 源代码目录
│   ├── *.c                # 各模块实现
│   ├── _unix/             # UNIX 平台实现
│   ├── _win/              # Windows 平台实现
│   ├── crypto/            # 加密模块实现
│   ├── net/               # 网络模块实现
│   ├── sql/               # 数据库模块实现
│   └── sys/               # 系统模块实现
└── example/               # 测试示例
    ├── example.c          # 主测试程序
    ├── Makefile           # 示例编译配置
    └── cel_*.c            # 各模块测试代码

> ⚠️ **注意**: `_unix/` 和 `_win/` 目录为内部跨平台封装，**不建议直接引用**，请通过统一接口调用

## 🔧 依赖要求

### 必需依赖

| 依赖项 | 最低版本 | 说明 | 必选 |
|--------|----------|------|------|
| mysql-devel | 8.4+ | 数据库连接支持 | ✅ (使用 sql 模块) |
| openssl-devel | 3.0+ | 加解密功能支持 | ✅ (使用 crypto\net 模块) |

### 依赖库安装

针对linux环境，已经默认放置在源码顶级目录下的openssl\mysqlclient

## 🚀 快速开始

### 编译与安装

1. 进入源码根目录
    cd cel

2. x86_64编译

  make

3. arm64编译

  make ARCH=aarch64

4. 运行测试示例
    ./bin/cel-example

5. （可选）清理编译
    make clean

### 在项目中使用

**方式一：直接引用库**
CEL_PATH = /path/to/cel
CFLAGS += -I$(CEL_PATH)/include
LDFLAGS += -L$(CEL_PATH)/bin -lcel -lmysql -lssl -lcrypto

**方式二：安装到系统**
sudo cp -r include/cel /usr/include/
sudo cp bin/libcel.so /usr/lib/
sudo ldconfig

## 📝 使用示例

### 1. 基础数据结构

```
#include "cel/queue.h"

int main() {
    int i;
    int *buf = (int *)malloc(100000 * sizeof(int));
    CelQueue *queue = cel_queue_new(NULL);
    
    // 初始化数据
    for (i = 0; i < 100000; i++)
        buf[i] = i;
    
    // 从队尾入队
    for (i = 0; i < 100000; i++) {
        cel_queue_push_back(queue, &buf[i]);
    }
    
    // 从队首出队
    for (i = 0; i < 100000; i++) {
        cel_queue_pop_front(queue);
    }
    
    // 从队首入队
    for (i = 0; i < 100000; i++) {
        cel_queue_push_front(queue, &buf[i]);
    }
    
    // 销毁队列
    cel_queue_free(queue);
    free(buf);
    return 0;
}
```

### 2. 哈希表 (Hash Table)

```
#include "cel/hashtable.h"

int main() {
    int i;
    int buf[1000];
    CelHashTable *hashtable = cel_hashtable_new(
        cel_int_hash, cel_int_equal, NULL, NULL
    );
    
    // 插入数据
    for (i = 0; i < 1000; i++) {
        buf[i] = 1000 - i;
        printf("Insert value %d\r\n", buf[i]);
        cel_hashtable_insert(hashtable, &buf[i], NULL);  
    }
    
    // 删除数据
    for (i = 999; i > 0; i--) {
        printf("Remove value %d\r\n", buf[i]);
        cel_hashtable_remove(hashtable, &buf[i]);
    }
    
    // 销毁哈希表
    cel_hashtable_free(hashtable);
    return 0;
}
```

### 3. 日志 (Logging)

```
#include "cel/log.h"
#include "cel/multithread.h"

int main() {
    // 启用多线程支持
    cel_multithread_support();
    
    // 设置日志缓冲区数量
    cel_log_buffer_num_set(64);
    
    // 输出不同级别日志
    cel_log_debug(_T("Debug message"));
    cel_log_info(_T("Info message"));
    cel_log_warn(_T("Warning message"));
    cel_log_error(_T("Error message"));
    
    // 刷新日志缓冲区
    cel_log_flush();
    
    return 0;
}
```

### 4. JSON 处理

```
#include "cel/json.h"
#include "cel/file.h"

int main() {
    CHAR buf[1024] = {'\0'};
    CelJson json;
    size_t len;
    
    // 创建 JSON 对象
    cel_json_init(&json);
    
    // 反序列化（解析 JSON）
    cel_json_deserialize_starts(&json);
    const char *json_str = "{\"key\": \"value\", \"num\": 123}";
    len = strlen(json_str);
    cel_json_deserialize_update(&json, json_str, len);
    cel_json_deserialize_finish(&json);
    
    // 序列化（生成 JSON 字符串）
    cel_json_serialize_starts(&json, 1);
    len = sizeof(buf);
    cel_json_serialize_update(&json, buf, &len);
    cel_json_serialize_finish(&json);
    printf("%s\r\n", buf);
    
    // 销毁 JSON 对象
    cel_json_destroy(&json);
    
    return 0;
}
```

### 5. 数据库连接池

```
#include "cel/sql/sqlconpool.h"
#include "cel/error.h"

int main() {
    CelSqlConPool sqldb_pool;
    CelSqlCon *conn;
    
    // 初始化连接池
    cel_sqlconpool_init(
        &sqldb_pool, 
        CEL_SQLCON_MYSQL,           // 数据库类型
        "192.168.23.151",          // 主机地址
        9076,                       // 端口
        "iam",                      // 数据库名
        "root",                     // 用户名
        "password",                 // 密码
        2,                          // 最小连接数
        64                          // 最大连接数
    );
    
    // 获取连接
    conn = cel_sqlconpool_get(&sqldb_pool);
    if (conn == NULL) {
        puts(cel_geterrstr());
        return -1;
    }
    
    // 执行 SQL 语句...
    // cel_sqlcon_query(conn, "SELECT * FROM users");
    
    // 归还连接到连接池
    cel_sqlconpool_return(&sqldb_pool, conn);
    
    // 销毁连接池
    // cel_sqlconpool_destroy(&sqldb_pool);
    
    return 0;
}
```

### 6. 加解密 (MD5)

```
#include "cel/crypto/md5.h"

int main() {
    unsigned char digest[16];
    const char *data = "abc";
    int i;
    
    // 计算 MD5
    cel_md5((unsigned char*)data, strlen(data), digest);
    
    // 打印结果
    printf("MD5: ");
    for (i = 0; i < 16; i++) {
        printf("%02x", digest[i]);
    }
    printf("\r\n");
    // 输出：900150983cd24fb0d6963f7d28e17f72
    
    return 0;
}
```

### 7. 协程 (Coroutine)

```
#include "cel/coroutine.h"

void coroutine_worker(void *ud) {
    int *data = (int*)ud;
    CelCoroutine co = cel_coroutine_self();
    
    while ((*data)-- >= 0) {
        printf("Coroutine data =%d\n", *data); 
        cel_coroutine_yield(&co);
    }
}

int main() {
    int a = 5;
    CelCoroutine co1, co2;
    
    // 创建协程
    cel_coroutine_create(&co1, NULL, coroutine_worker, &a);
    cel_coroutine_create(&co2, NULL, coroutine_worker, &a);
    
    // 恢复协程执行
    while (cel_coroutine_status(&co1) && cel_coroutine_status(&co2)) {
        printf("\nResume co1\n");
        cel_coroutine_resume(&co1);
        printf("Resume co2\n");
        cel_coroutine_resume(&co2);
    }
    
    return 0;
}
```

### 8. 线程池 (Thread Pool)

```
#include "cel/threadpool.h"
#include "cel/multithread.h"

void thread_worker(void *data, void *user_data) {
_tprintf(_T("Task %d is running by thread [%ld].\r\n"), 
    *((int*)data), cel_thread_getid());
    sleep(1);
}

int main() {
int i, data[10];
CelThreadPool *tp;

// 启用多线程支持
cel_multithread_support();

// 创建线程池
tp = cel_threadpool_new(thread_worker, NULL, 8, FALSE);
cel_threadpool_set_max_unused_threads(4);

// 添加任务
for (i = 0; i < 10; i++) {
    data[i] = i;
    cel_threadpool_push_task(tp, &data[i]);
}

// 等待任务完成
sleep(5);

// 获取线程池状态
_tprintf(_T("Unprocessed tasks: %d, Thread num: %d\r\n"), 
    cel_threadpool_get_unprocessed(tp), 
    cel_threadpool_get_num_threads(tp));

// 销毁线程池
cel_threadpool_free(tp, FALSE, TRUE);

return 0;
}
```



## 🧪 测试

### 运行测试示例
cd cel
./bin/cel-example

### 测试覆盖模块

| 模块 | 测试文件 | 测试内容 |
|------|----------|----------|
| 数据结构 | cel_hashtable.c, cel_queue.c, cel_arraylist.c | 哈希表、队列、数组 |
| 加密解密 | cel_md.c, cel_sha.c, cel_aes.c, cel_des.c | MD5、SHA、AES、DES |
| 数据库 | cel_sqlconpool.c | 连接池、增删改查 |
| 网络 | cel_socket.c, cel_httpserve.c, cel_httpclient.c | Socket、HTTP |
| 系统 | cel_user.c, cel_process.c, cel_file.c | 用户、进程、文件 |
| 并发 | cel_threadpool.c, cel_coroutine.c | 线程池、协程 |
| 定时器 | cel_timerheap.c, cel_timerwheel.c | 定时器堆、定时器轮 |
| 工具 | cel_json.c, cel_jwt.c, cel_log.c | JSON、JWT、日志 |

## ⚠️ 注意事项

| 事项 | 说明 |
|------|------|
| 🔒 跨平台封装 | `_unix/` 和 `_win/` 目录为内部跨平台封装，**不建议直接引用** |
| 🧵 线程安全 | 部分模块非线程安全，多线程环境下请使用锁机制保护 |
| 🧹 资源释放 | 使用数据库连接池、网络服务等模块后，请确保正确释放资源 |
| 📦 依赖管理 | 使用 sql 模块需确保 MySQL 服务可用 |
| 🐛 错误处理 | 所有函数返回值请检查，建议使用 cel_log_error 记录错误 |
| 📝 字符编码 | Windows 平台使用 TCHAR 支持 Unicode，UNIX 平台使用 char |

## 📄 许可证

本项目采用 **GNU General Public License v2.0**

CEL(C Extension Library)
Copyright (C) 2008 - 2025 Hu Jinya <hu_jinya@163.com>

This program is free software; you can redistribute it and/or 
modify it under the terms of the GNU General Public License 
as published by the Free Software Foundation; either version 2 
of the License, or (at your option) any later version.

## 📧 联系方式

| 类型 | 信息 |
|------|------|
| 👤 作者 | Hu Jinya |
| 📧 邮箱 | hu_jinya@163.com |

## 🙏 致谢

感谢以下开源项目的启发：
- MySQL - 数据库支持
- OpenSSL - 加解密支持

---

