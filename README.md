# AsioProject

## 目前已做：
- [基础socket通信API](https://github.com/AhDaii/AsioProject/tree/master/PreExam)
- [基础同步客户端](https://github.com/AhDaii/AsioProject/tree/master/SyncClient)
- [收发分离的同步客户端(用于测试粘包)](https://github.com/AhDaii/AsioProject/tree/master/SyncClient_v2)
- [基础同步服务端](https://github.com/AhDaii/AsioProject/tree/master/SyncServer)
- [异步通信API](https://github.com/AhDaii/AsioProject/tree/master/AsycApi)
- [基础Echo异步服务端](https://github.com/AhDaii/AsioProject/commit/d8b406fcbcf388e520f64234128ba3522068bd28)
  - [增加伪闭包](https://github.com/AhDaii/AsioProject/commit/7822cf6d300ee66ef40b7c61f1660b0cc96414cd)
  - [增加发送队列实现全双工通信](https://github.com/AhDaii/AsioProject/commit/f929bcc68d734e9ce2ea147da977fe39aa7c271b)
  - [解决粘包问题](https://github.com/AhDaii/AsioProject/commit/f37f9c308d5750b7442e26d497435813693abff2)
  - [增加字节序处理和发送队列大小控制](https://github.com/AhDaii/AsioProject/commit/f7ff805f64b71ce945f0069521b01665bf6a15f8)
- Protobuf传输数据
  - [服务端](https://github.com/AhDaii/AsioProject/tree/master/AsycServer_Protobuf)
  - [客户端](https://github.com/AhDaii/AsioProject/tree/master/SyncClient_Protobuf)
- JsonCPP传输数据
  - [服务端](https://github.com/AhDaii/AsioProject/tree/master/AsycServer_JsonCPP)
  - [客户端](https://github.com/AhDaii/AsioProject/tree/master/SyncClient_JsonCPP)
- [简易方法解决粘包问题](https://github.com/AhDaii/AsioProject/tree/master/AsycServer_v2)
- 消息格式更改为标准的TLV格式
  - [服务端](https://github.com/AhDaii/AsioProject/tree/master/AsycServer_Logic)
  - [客户端](https://github.com/AhDaii/AsioProject/tree/master/SyncClient_JsonCPP_v2)
- [服务端添加逻辑层](https://github.com/AhDaii/AsioProject/tree/master/AsycServer_Logic)
- 实现并行处理
  - 方法1: [多个线程，每个线程管理一个io_context](https://github.com/AhDaii/AsioProject/tree/master/AsycServer_IOServicePool)
