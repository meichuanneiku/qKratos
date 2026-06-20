# qKratos

qKratos 是一个基于 Qt 框架实现的 HTTP 服务器，灵感来自 Go 语言的 Kratos 微服务框架。

## 功能特性

- **HTTP 服务器**: 基于 Qt HTTPServer 模块构建
- **用户管理 API**: 提供用户的创建、查询、删除等 RESTful 接口
- **JWT 认证**: 支持基于 JWT Token 的用户身份验证
- **权限控制**: 集成 Casbin 实现基于角色的访问控制 (RBAC)
- **多数据库支持**: 
  - MySQL
  - 达梦数据库 (DM8)
  - Redis

## 技术栈

- Qt 5.12+
- C++17
- Casbin (权限控制)
- JWT-CPP (JWT 认证)

## 项目结构

```
qKratos/
├── api/                    # API 路由定义
│   └── user/v1/           # 用户模块 API
├── cmd/                    # 应用入口
│   └── server/            # HTTP 服务器入口
├── configs/                # 配置文件
│   ├── config.json        # 主配置文件
│   └── authz/             # 权限配置
│       └── casbin_model.conf
├── internal/              # 内部实现
│   ├── biz/              # 业务逻辑层
│   │   └── user/         # 用户业务逻辑
│   ├── conf/             # 配置管理
│   ├── data/             # 数据访问层
│   │   ├── data.h        # 数据库连接管理
│   │   └── user/         # 用户数据仓库
│   ├── pkg/              # 公共组件
│   │   ├── casbin/       # Casbin 适配器
│   │   ├── error/        # 错误码定义
│   │   ├── jwt/          # JWT 实现
│   │   ├── middleware/   # 中间件
│   │   ├── permission/   # 权限控制
│   │   └── response/     # 响应封装
│   ├── server/           # HTTP 服务器实现
│   └── service/          # 服务层
│       └── user/         # 用户服务
└── third_party/          # 第三方依赖
    ├── casbin-cpp/       # Casbin C++ 版本
    └── jwt-cpp/          # JWT C++ 库
```

## API 端点

| 方法 | 路径 | 描述 |
|------|------|------|
| POST | `/api/v1/users` | 创建用户 |
| GET | `/api/v1/users/<id>` | 获取指定用户 |
| GET | `/api/v1/user?id=` | 根据 ID 查询用户 |
| POST | `/api/v1/login` | 用户登录 |

## 配置文件

`configs/config.json` 包含以下配置项:

- **server.http**: HTTP 服务器配置 (地址、端口、超时)
- **server.ws**: WebSocket 服务器配置
- **data.mysql**: MySQL 数据库连接
- **data.dmsql**: 达梦数据库连接
- **data.redis**: Redis 连接
- **auth**: JWT 配置 (密钥、过期时间)
- **casbin**: 权限配置文件路径

## 构建

项目使用 Qt Creator 或 qmake 构建:

```bash
qmake qKratos.pro
make
```

## 运行

```bash
./qKratos-HttpServer -c configs/config.json
```

默认监听地址: `0.0.0.0:8080`

## 依赖

- Qt 5.12+
- OpenSSL (Windows 平台)
- MySQL Client / 达梦数据库客户端

## 许可证

MIT License