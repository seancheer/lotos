# 项目目的

Lotos是个人的业余网络编程项目，也是自己将理论付诸于实践的一个过程。初学c的socket编程是在本科三年级，从那时起就梦想写一个HTTP Server，虽然之后陆陆续续也写过几个和网络编程有关的小项目，比如网络扫描器和[Tinyhttpd](https://github.com/EZLippi/Tinyhttpd)，但也仅限于几百行的规模。

2017年花了约两个月断断续续读完了[《Unix/Linux编程实践教程》](https://book.douban.com/subject/1219329/)，可以说这是一本相见恨晚的书，既适合入门，也适合重新梳理知识体系，我觉得自己有能力重新开一个HTTP Server的坑了。

我对Lotos的期望是“能用”，这就要求能够正确处理syscall error、signal以及很多极端情况下的用户请求。更进一步，在能用的基础上争取做到“高性能”，高兴的是，在[Benchmark](./BENCHMARK.md)中，Lotos的性能还是很不错的。

Lotos的开发几乎占用了我一个多月的所有业余时间，私下里认为代码还算工整规范，现在再把开发调试的思考和总结记录成文档，方便与大家交流学习。

#### 开发规范

开发流程比较完整，确定需求 -> 服务器模型选型 -> 数据结构选择 -> 单元测试 -> 开发 -> 集成测试 -> 性能测试。核心的数据结构都配套有单元测试。

#### 数据结构

通过对场景需求和将来扩展性的考量，需要设计合理的、高效的数据结构。Lotos中用了最小堆来淘汰超时过期的HTTP连接，以及使用了HashMap这种KV结构加速对诸如`Mime Type`和 `HTTP Header Handler`的查找。

#### 语言特性

项目中涉及C语言中很多特性，比如条件编译，嵌套宏定义、柔性数组、静态全局变量、函数指针、位运算以及利用内存空间全局区等。

#### 抽象能力

在用户空间进行c编程，由于没有天然的面向对象支持，需要程序员自己封装各种结构体。系统编程中，IO操作基本上都是对文件描述符fd进行操作，在本项目中，一个文件描述符对应着一个TCP连接，把fd封装进`connection_t`作为一个成员变量是一个自然而然的抽象过程。

#### 协议理解

开发HTTP服务器从宏观上来说会对网络协议TCP及其各个状态理解更深，会对HTTP协议主要字段的功能理解更深，会对网络I/O模型认识更深。
