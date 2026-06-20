#ifndef DMADAPTER_H
#define DMADAPTER_H


#include <casbin/casbin.h>
#include <casbin/persist/adapter.h>
#include "../../data/data.h"  // 您的 DataBaseManager
#include "../../conf/conf.h"
#include <QTimer>
#include <QSqlQuery>
#include <vector>
#include <string>


#include <QFileInfo>
namespace qKratos::Casbin {

class DmAdapter : public casbin::Adapter {
public:
    DmAdapter() = default;

    void LoadPolicy(const std::shared_ptr<casbin::Model>& model) override {
        auto db = DataBaseManager::instance()->db(1);
        if (!db.isOpen()) {
            qCritical() << "Casbin LoadPolicy failed: DB not open";
            return;
        }

        QSqlQuery query(db);
        query.setForwardOnly(true);
        if (!query.exec("SELECT ptype, v0, v1, v2, v3, v4, v5 FROM SSYZC.casbin_rule")) {
            qCritical() << "Casbin SQL error:" << query.lastError().text();
            return;
        }

        while (query.next()) {
            std::string ptype = query.value("ptype").toString().toStdString();
            std::vector<std::string> rule;

            for (int i = 1; i <= 5; ++i) {  // v0 ~ v5
                QString v = query.value(i).toString();
                if (!v.isEmpty()) {
                    rule.emplace_back(v.toStdString());
                }
            }

            if (!rule.empty()) {
                model->AddPolicy(ptype, ptype, rule);
            }
        }
//        qInfo() << "Casbin policy loaded from DM database";
    }

    void SavePolicy(const std::shared_ptr<casbin::Model>& model) override {
        // 生产环境权限只读，由管理员维护
        qWarning() << "Casbin SavePolicy ignored (read-only adapter)";
    }

    void AddPolicy(std::string sec, std::string p_type, std::vector<std::string> rule) override {
        auto db = DataBaseManager::instance()->db(1);
        if (!db.isOpen()) return;

        QSqlQuery query(db);
        query.prepare("INSERT INTO casbin_rule (ptype, v0, v1, v2, v3, v4, v5) VALUES (?, ?, ?, ?, ?, ?, ?)");
        query.addBindValue(QString::fromStdString(p_type));
        for (size_t i = 0; i < 6; ++i) {
            query.addBindValue(i < rule.size() ? QString::fromStdString(rule[i]) : QString());
        }
        query.exec();
    }

    void RemovePolicy(std::string sec, std::string p_type, std::vector<std::string> rule) override {
        auto db = DataBaseManager::instance()->db(1);
        if (!db.isOpen()) return;

        QStringList placeholders;
        for (size_t i = 0; i < rule.size(); ++i) {
            placeholders << QString("v%1 = ?").arg(i);
        }
        QString sql = QString("DELETE FROM casbin_rule WHERE ptype = ? AND %1").arg(placeholders.join(" AND "));
        QSqlQuery query(db);
        query.prepare(sql);
        query.addBindValue(QString::fromStdString(p_type));
        for (const auto& v : rule) {
            query.addBindValue(QString::fromStdString(v));
        }
        query.exec();
    }

    void RemoveFilteredPolicy(std::string sec, std::string ptype, int field_index, std::vector<std::string> field_values) override {
        auto db = DataBaseManager::instance()->db(1);
        if (!db.isOpen()) return;

        QStringList placeholders;
        for (size_t i = 0; i < field_values.size(); ++i) {
            if (field_values[i].empty()) continue;
            placeholders << QString("v%1 = ?").arg(i);
        }
        if (placeholders.isEmpty()) return;

        QString sql = QString("DELETE FROM casbin_rule WHERE ptype = ? AND %1").arg(placeholders.join(" AND "));
        QSqlQuery query(db);
        query.prepare(sql);
        query.addBindValue(QString::fromStdString(ptype));
        for (const auto& v : field_values) {
            if (v.empty()) continue;
            query.addBindValue(QString::fromStdString(v));
        }
        query.exec();
    }

    bool IsFiltered() override { return false; }

    bool IsValid() override { return DataBaseManager::instance()->db(1).isOpen(); }
};

class Casbin {
public:
    static Casbin& instance() {
        static Casbin inst;
        return inst;
    }

    inline bool enforce(const QString& sub, const QString& obj, const QString& act) {
        auto& e = Casbin::instance().enforcer;
        if (!e) return false;

        // 正确方式：用 DataVector（std::vector<std::string>）
        casbin::DataVector params;
        params.push_back(sub.toStdString());
        params.push_back(obj.toStdString());
        params.push_back(act.toStdString());

        return e->Enforce(params);  // 完美匹配！
    }

    std::vector<std::string> getRolesForUser(const QString& userId) {
        return enforcer ? enforcer->GetRolesForUser(userId.toStdString()) : std::vector<std::string>{};
    }

#if 0
private:
    Casbin() {
        if (!DataBaseManager::instance()->db(1).isOpen()) return;

        auto adapter = std::make_shared<DmAdapter>();
        enforcer = std::make_shared<casbin::Enforcer>(Config::instance()->casbinPath().toStdString(), adapter);

        // v1.66.0 没有 enableAutoLoadPolicy，用定时器手动刷新
        QTimer* timer = new QTimer();
        QObject::connect(timer, &QTimer::timeout, [this]() {
            if (enforcer) enforcer->LoadPolicy();
        });
        timer->start(30000);  // 30秒刷新一次
    }

    std::shared_ptr<casbin::Enforcer> enforcer;

#else
    ~Casbin(){
        if(enforcer->IsAutoLoadingRunning())
            enforcer->StopAutoLoadPolicy();
    }
private:
    Casbin() {
        if (!DataBaseManager::instance()->db(1).isOpen()) return;

        auto adapter = std::make_shared<DmAdapter>();
        enforcer =  std::make_shared<casbin::SyncedEnforcer>(Config::instance()->casbinPath().toStdString(), adapter);
        enforcer->StartAutoLoadPolicy(std::chrono::seconds(30)); // 30秒刷新一次
    }

    std::shared_ptr<casbin::SyncedEnforcer> enforcer;
#endif
};

inline bool enforce(const QString& sub, const QString& obj, const QString& act) {
    return Casbin::instance().enforce(sub, obj, act);
}

} // namespace qKratos::Casbin

#endif // DMADAPTER_H
