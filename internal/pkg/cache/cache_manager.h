#pragma once

#include <memory>
#include "cache.h"
#include "redis.h"

namespace qKratos::Cache {

class CacheManager {
public:
    static CacheManager& instance() {
        static CacheManager inst;
        return inst;
    }

    bool init(const QString& host, quint16 port, const QString& password, int db) {
        m_cache = std::make_unique<RedisCache>(host, port, password, db);
        if (m_cache->connect()) {
            return true;
        }
        qWarning() << "Cache not available, continuing without cache";
        m_cache.reset();
        return false;
    }

    Cache* cache() const { return m_cache.get(); }
    bool isEnabled() const { return m_cache != nullptr; }

private:
    CacheManager() = default;
    std::unique_ptr<RedisCache> m_cache;
};

} // namespace qKratos::Cache
