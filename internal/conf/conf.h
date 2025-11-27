#ifndef CONF_H
#define CONF_H


#include <QString>

struct ServerConfig {
    QString addr = "0.0.0.0";
    int     port = 8080;
    int     timeoutSec = 30;
};

class Config
{
public:
    static Config& instance() {
        static Config c;
        return c;
    }

    bool load(const QString& inputPath = "config/bootstrap.yaml");
    const ServerConfig& server() const { return server_; }

private:
    Config() = default;
    ServerConfig server_;
};
#endif // CONF_H
