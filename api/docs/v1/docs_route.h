#pragma once

#include <QHttpServer>
#include <QJsonObject>

inline QJsonObject generateOpenApiSpec()
{
    QJsonObject spec;

    spec["openapi"] = "3.0.3";
    spec["info"] = QJsonObject{
        {"title", "qKratos API"},
        {"version", "1.0.0"},
        {"description", "Qt 实现的 Kratos 风格 HTTP 服务器"}
    };
    spec["servers"] = QJsonArray{
        QJsonObject{{"url", "http://localhost:8080"}, {"description", "Local dev"}}
    };

    QJsonObject paths;

    // POST /api/v1/login
    {
        QJsonObject post;
        post["tags"] = QJsonArray{"用户认证"};
        post["summary"] = "用户登录";
        post["requestBody"] = QJsonObject{
            {"required", true},
            {"content", QJsonObject{
                {"application/json", QJsonObject{
                    {"schema", QJsonObject{
                        {"type", "object"},
                        {"properties", QJsonObject{
                            {"username", QJsonObject{{"type", "string"}}},
                            {"password", QJsonObject{{"type", "string"}}}
                        }},
                        {"required", QJsonArray{"username", "password"}}
                    }}
                }}
            }}
        };
        QJsonObject response200;
        response200["description"] = "登录成功，返回 token";
        QJsonObject responses;
        responses["200"] = response200;
        post["responses"] = responses;
        paths["/api/v1/login"] = QJsonObject{{"post", post}};
    }

    // GET /api/v1/users
    {
        QJsonObject get;
        get["tags"] = QJsonArray{"用户管理"};
        get["summary"] = "获取用户列表";
        get["parameters"] = QJsonArray{
            QJsonObject{
                {"name", "page"},
                {"in", "query"},
                {"required", false},
                {"schema", QJsonObject{{"type", "integer"}, {"default", 0}}}
            },
            QJsonObject{
                {"name", "systemid"},
                {"in", "header"},
                {"required", true},
                {"schema", QJsonObject{{"type", "integer"}}}
            }
        };
        QJsonObject responses;
        responses["200"] = QJsonObject{{"description", "用户列表"}};
        get["responses"] = responses;
        paths["/api/v1/users"] = QJsonObject{{"get", get}};
    }

    // POST /api/v1/users
    {
        QJsonObject post;
        post["tags"] = QJsonArray{"用户管理"};
        post["summary"] = "创建用户";
        post["requestBody"] = QJsonObject{
            {"required", true},
            {"content", QJsonObject{
                {"application/json", QJsonObject{
                    {"schema", QJsonObject{
                        {"type", "object"},
                        {"properties", QJsonObject{
                            {"name", QJsonObject{{"type", "string"}}}
                        }},
                        {"required", QJsonArray{"name"}}
                    }}
                }}
            }}
        };
        QJsonObject responses;
        responses["200"] = QJsonObject{{"description", "创建成功"}};
        post["responses"] = responses;
        paths["/api/v1/users"].toObject()["post"] = post;
    }

    // GET /api/v1/users/{id}
    {
        QJsonObject get;
        get["tags"] = QJsonArray{"用户管理"};
        get["summary"] = "根据 ID 查询用户";
        get["parameters"] = QJsonArray{
            QJsonObject{
                {"name", "id"},
                {"in", "path"},
                {"required", true},
                {"schema", QJsonObject{{"type", "integer"}}}
            }
        };
        QJsonObject responses;
        responses["200"] = QJsonObject{{"description", "用户信息"}};
        get["responses"] = responses;
        paths["/api/v1/users/{id}"] = QJsonObject{{"get", get}};
    }

    // PUT /api/v1/users/{id}
    {
        QJsonObject put;
        put["tags"] = QJsonArray{"用户管理"};
        put["summary"] = "更新用户";
        put["parameters"] = QJsonArray{
            QJsonObject{
                {"name", "id"},
                {"in", "path"},
                {"required", true},
                {"schema", QJsonObject{{"type", "integer"}}}
            }
        };
        QJsonObject responses;
        responses["200"] = QJsonObject{{"description", "更新成功"}};
        put["responses"] = responses;
        paths["/api/v1/users/{id}"].toObject()["put"] = put;
    }

    spec["paths"] = paths;

    QJsonObject components;
    QJsonObject securitySchemes;
    securitySchemes["BearerAuth"] = QJsonObject{
        {"type", "http"},
        {"scheme", "bearer"},
        {"bearerFormat", "JWT"}
    };
    components["securitySchemes"] = securitySchemes;
    spec["components"] = components;
    spec["security"] = QJsonArray{
        QJsonObject{{"BearerAuth", QJsonArray{}}}
    };

    return spec;
}

inline void RegisterDocsRoutes(QHttpServer& server)
{
    server.route("/api/v1/docs.json", QHttpServerRequest::Method::Get,
                 [](const QHttpServerRequest&) {
        QJsonDocument doc(generateOpenApiSpec());
        return QHttpServerResponse("application/json; charset=utf-8",
                                    doc.toJson(QJsonDocument::Compact));
    });

    server.route("/api/v1/docs", QHttpServerRequest::Method::Get,
                 [](const QHttpServerRequest&) {
        const char* html = R"(
<!DOCTYPE html>
<html>
<head>
  <title>qKratos API Docs</title>
  <meta charset="utf-8"/>
  <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/swagger-ui-dist@5/swagger-ui.css">
</head>
<body>
  <div id="swagger-ui"></div>
  <script src="https://cdn.jsdelivr.net/npm/swagger-ui-dist@5/swagger-ui-bundle.js"></script>
  <script>SwaggerUIBundle({ url: '/api/v1/docs.json', dom_id: '#swagger-ui' });</script>
</body>
</html>
)";
        return QHttpServerResponse("text/html; charset=utf-8", QByteArray(html));
    });
}
