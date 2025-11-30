#ifndef DMADAPTER_H
#define DMADAPTER_H


#include <casbin/casbin.h>
#include <casbin/persist/adapter.h>
#include "../../data/data.h"  // 您的 DataBaseManager
#include <QSqlQuery>
#include <vector>
#include <string>

namespace qratos::rbac {

class DmAdapter : public casbin::Adapter {
public:
    DmAdapter() = default;

    // 匹配您贴的 adapter.h 接口
    void LoadPolicy(const std::shared_ptr<casbin::Model>& model) override {
        auto db = DataBaseManager::instance()->data().db;
        if (!db.isOpen()) {
            qCritical() << "Casbin LoadPolicy failed: DB not open";
            return;
        }

        QSqlQuery query(db);
        query.setForwardOnly(true);
        if (!query.exec("SELECT ptype, v0, v1, v2, v3, v4, v5 FROM casbin_rule")) {
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
                model->addPolicy(ptype, ptype, rule);
            }
        }
        qInfo() << "Casbin policy loaded from DM database";
    }

    void SavePolicy(const std::shared_ptr<casbin::Model>& model) override {
        // 生产环境权限只读，由管理员维护
        qWarning() << "Casbin SavePolicy ignored (read-only adapter)";
    }

    void AddPolicy(std::string sec, std::string p_type, std::vector<std::string> rule) override {
        auto db = DataBaseManager::instance()->data().db;
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
        auto db = DataBaseManager::instance()->data().db;
        if (!db.isOpen()) return;

        QStringList conditions;
        for (size_t i = 0; i < rule.size(); ++i) {
            conditions << QString("v%1 = '%2'").arg(i).arg(QString::fromStdString(rule[i]));
        }
        QString sql = QString("DELETE FROM casbin_rule WHERE ptype = '%1' AND %2").arg(QString::fromStdString(p_type)).arg(conditions.join(" AND "));
        QSqlQuery query(db);
        query.exec(sql);
    }

    void RemoveFilteredPolicy(std::string sec, std::string ptype, int field_index, std::vector<std::string> field_values) override {
        auto db = DataBaseManager::instance()->data().db;
        if (!db.isOpen()) return;

        QStringList conditions;
        for (size_t i = 0; i < field_values.size(); ++i) {
            if (field_values[i].empty()) continue;
            conditions << QString("v%1 = '%2'").arg(i).arg(QString::fromStdString(field_values[i]));
        }
        QString sql = QString("DELETE FROM casbin_rule WHERE ptype = '%1' AND %2").arg(QString::fromStdString(ptype)).arg(conditions.join(" AND "));
        QSqlQuery query(db);
        query.exec(sql);
    }

    bool IsFiltered() override { return false; }

    bool IsValid() override { return DataBaseManager::instance()->data().db.isOpen(); }
};

class Casbin {
public:
    static Casbin& instance() {
        static Casbin inst;
        return inst;
    }

    bool enforce(const QString& sub, const QString& obj, const QString& act) {
            if (!enforcer) return false;
//            std::vector<std::string> params{sub.toStdString(), obj.toStdString(), act.toStdString()};
            std::vector<std::string> params = {
                        sub.toStdString(),
                        obj.toStdString(),
                        act.toStdString()
                    };
            return enforcer->EnforceWithMatcher("", params); // 正确调用！
        }

        std::vector<std::string> getRolesForUser(const QString& userId) {
            return enforcer ? enforcer->GetRolesForUser(userId.toStdString()) : std::vector<std::string>{};
        }

    private:
        Casbin() {
            if (!DataBaseManager::instance()->connect()) return;
            auto adapter = std::make_shared<DmAdapter>();
            enforcer = std::make_shared<casbin::Enforcer>("config/rbac_model.conf", adapter);
            enforcer->enableAutoLoadPolicy(30);
        }

        std::shared_ptr<casbin::Enforcer> enforcer;
    };

inline bool enforce(const QString& sub, const QString& obj, const QString& act) {
    return Casbin::instance().enforce(sub, obj, act);
}

} // namespace qratos::rbac

#endif // DMADAPTER_H
