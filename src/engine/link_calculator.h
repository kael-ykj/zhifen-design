#ifndef LINK_CALCULATOR_H
#define LINK_CALCULATOR_H

#include <QString>
#include <QList>
#include <QMap>
#include <QPointF>
#include <cmath>

namespace Zhifen {

// 频段定义
enum BandType {
    Band_GSM900 = 0,    // 2G 900MHz
    Band_GSM1800,       // 2G 1800MHz
    Band_WCDMA,         // 3G 2100MHz
    Band_LTE_FDD,       // 4G FDD 1800/2100MHz
    Band_LTE_TDD,       // 4G TDD 2300/2600MHz
    Band_NR_700,        // 5G 700MHz
    Band_NR_2600,       // 5G 2600MHz
    Band_NR_3500,       // 5G 3500MHz
    Band_NR_4900        // 5G 4900MHz
};

// 馈线类型损耗（dB/100m，按频段）
struct FeederLoss {
    QString name;
    QMap<int, qreal> lossPer100m; // band -> dB/100m
};

// 器件计算结果
struct LinkResult {
    QString deviceId;
    QString deviceName;
    QString deviceType;
    qreal inputPower = 0.0;      // 输入功率 dBm
    qreal outputPower = 0.0;     // 输出功率 dBm
    qreal coupledPower = 0.0;    // 耦合端功率 dBm（耦合器）
    qreal loss = 0.0;            // 总损耗 dB
    qreal cableLength = 0.0;     // 前级馈线长度 m
    qreal cableLoss = 0.0;       // 前级馈线损耗 dB
    qreal deviceLoss = 0.0;      // 器件自身损耗 dB
    bool isAntenna = false;
    bool alarm = false;
    QString alarmMessage;
};

// 链路预算报告
struct LinkReport {
    bool success = false;
    QString sourceId;
    qreal sourcePower = 0.0;
    BandType band = Band_LTE_FDD;
    QList<LinkResult> results;
    QList<LinkResult> antennas;
    QList<QString> alarms;
    qreal minAntennaPower = 0.0;
    qreal maxAntennaPower = 0.0;
    qreal avgAntennaPower = 0.0;
    int totalDevices = 0;
    int totalAntennas = 0;

    QString toText() const {
        QString text;
        text += QString("========== 链路预算报告 ==========\n");
        text += QString("信源: %1  发射功率: %2 dBm\n").arg(sourceId).arg(sourcePower, 0, 'f', 2);
        text += QString("频段: %1\n").arg(bandName(band));
        text += QString("器件总数: %1  天线总数: %2\n\n").arg(totalDevices).arg(totalAntennas);
        text += QString("----- 链路明细 -----\n");
        for (const auto &r : results) {
            text += QString("[%1] %2\n").arg(r.deviceType).arg(r.deviceName);
            text += QString("  馈线长度: %1m  馈线损耗: %2dB\n").arg(r.cableLength, 0, 'f', 2).arg(r.cableLoss, 0, 'f', 2);
            text += QString("  器件损耗: %1dB  输入: %2dBm  输出: %3dBm\n")
                .arg(r.deviceLoss, 0, 'f', 2).arg(r.inputPower, 0, 'f', 2).arg(r.outputPower, 0, 'f', 2);
            if (r.alarm) text += QString("  ⚠️ 告警: %1\n").arg(r.alarmMessage);
        }
        text += QString("\n----- 天线口功率 -----\n");
        for (const auto &a : antennas) {
            text += QString("%1: %2 dBm").arg(a.deviceName).arg(a.outputPower, 0, 'f', 2);
            if (a.alarm) text += QString(" ⚠️ %1").arg(a.alarmMessage);
            text += "\n";
        }
        text += QString("\n----- 统计 -----\n");
        text += QString("天线口功率: 最小=%1dBm  最大=%2dBm  平均=%3dBm\n")
            .arg(minAntennaPower, 0, 'f', 2).arg(maxAntennaPower, 0, 'f', 2).arg(avgAntennaPower, 0, 'f', 2);
        if (!alarms.isEmpty()) {
            text += QString("\n----- 告警清单 -----\n");
            for (const auto &a : alarms) text += QString("⚠️ %1\n").arg(a);
        }
        text += QString("==================================\n");
        return text;
    }

    static QString bandName(BandType b) {
        switch (b) {
        case Band_GSM900: return "GSM900 (2G)";
        case Band_GSM1800: return "GSM1800 (2G)";
        case Band_WCDMA: return "WCDMA (3G)";
        case Band_LTE_FDD: return "LTE FDD (4G)";
        case Band_LTE_TDD: return "LTE TDD (4G)";
        case Band_NR_700: return "NR 700MHz (5G)";
        case Band_NR_2600: return "NR 2600MHz (5G)";
        case Band_NR_3500: return "NR 3500MHz (5G)";
        case Band_NR_4900: return "NR 4900MHz (5G)";
        }
        return "Unknown";
    }
};

// 链路预算计算器
class LinkCalculator {
public:
    LinkCalculator() {
        initFeederLossTable();
    }

    void setBand(BandType band) { m_band = band; }
    BandType band() const { return m_band; }

    // 设置功率阈值（dBm），低于此值告警
    void setMinAntennaPower(qreal v) { m_minAntennaPower = v; }
    void setMaxAntennaPower(qreal v) { m_maxAntennaPower = v; }

    // 获取馈线损耗（dB/m）
    qreal feederLoss(const QString &feederType) const {
        auto it = m_feederTable.find(feederType);
        if (it != m_feederTable.end()) {
            auto lit = it->lossPer100m.find(m_band);
            if (lit != it->lossPer100m.end()) return lit.value() / 100.0;
        }
        return 0.07; // 默认 7dB/100m
    }

    // 计算功率经过损耗后的值
    static qreal applyLoss(qreal powerDbm, qreal lossDb) {
        return powerDbm - lossDb;
    }

    // 功分器分配损耗
    static qreal splitterLoss(int ways) {
        return 10.0 * log10(ways);
    }

    // 生成示例报告（用于演示）
    LinkReport generateDemoReport() {
        LinkReport report;
        report.success = true;
        report.sourceId = "RRU-001";
        report.sourcePower = 43.0; // 20W
        report.band = m_band;
        report.totalDevices = 6;
        report.totalAntennas = 4;

        // RRU -> 1/2馈线(10m) -> 二功分
        LinkResult r1;
        r1.deviceId = "SPL-001"; r1.deviceName = "二功分器"; r1.deviceType = "功分器";
        r1.cableLength = 10.0; r1.cableLoss = 10.0 * feederLoss("1/2");
        r1.inputPower = applyLoss(report.sourcePower, r1.cableLoss);
        r1.deviceLoss = splitterLoss(2) + 0.1; // 分配损耗+插入损耗
        r1.outputPower = applyLoss(r1.inputPower, r1.deviceLoss);
        report.results.append(r1);

        // 支路1: 功分 -> 1/2馈线(5m) -> 10dB耦合器
        LinkResult r2;
        r2.deviceId = "CPL-001"; r2.deviceName = "10dB耦合器"; r2.deviceType = "耦合器";
        r2.cableLength = 5.0; r2.cableLoss = 5.0 * feederLoss("1/2");
        r2.inputPower = applyLoss(r1.outputPower, r2.cableLoss);
        r2.deviceLoss = 0.5; // 插入损耗
        r2.outputPower = applyLoss(r2.inputPower, r2.deviceLoss); // 直通端
        r2.coupledPower = applyLoss(r2.inputPower, 10.0); // 耦合端
        report.results.append(r2);

        // 耦合端 -> 天线1
        LinkResult a1;
        a1.deviceId = "ANT-001"; a1.deviceName = "全向吸顶天线1"; a1.deviceType = "天线";
        a1.isAntenna = true;
        a1.cableLength = 3.0; a1.cableLoss = 3.0 * feederLoss("1/2");
        a1.inputPower = applyLoss(r2.coupledPower, a1.cableLoss);
        a1.outputPower = a1.inputPower;
        a1.deviceLoss = 0;
        if (a1.outputPower < m_minAntennaPower) {
            a1.alarm = true; a1.alarmMessage = "天线口功率低于阈值";
            report.alarms.append(QString("%1 功率 %2dBm 低于阈值 %3dBm")
                .arg(a1.deviceName).arg(a1.outputPower, 0, 'f', 2).arg(m_minAntennaPower));
        }
        report.results.append(a1); report.antennas.append(a1);

        // 直通端 -> 1/2馈线(8m) -> 天线2
        LinkResult a2;
        a2.deviceId = "ANT-002"; a2.deviceName = "全向吸顶天线2"; a2.deviceType = "天线";
        a2.isAntenna = true;
        a2.cableLength = 8.0; a2.cableLoss = 8.0 * feederLoss("1/2");
        a2.inputPower = applyLoss(r2.outputPower, a2.cableLoss);
        a2.outputPower = a2.inputPower;
        if (a2.outputPower < m_minAntennaPower) {
            a2.alarm = true; a2.alarmMessage = "天线口功率低于阈值";
            report.alarms.append(QString("%1 功率 %2dBm 低于阈值 %3dBm")
                .arg(a2.deviceName).arg(a2.outputPower, 0, 'f', 2).arg(m_minAntennaPower));
        }
        report.results.append(a2); report.antennas.append(a2);

        // 支路2: 功分 -> 1/2馈线(12m) -> 三功分
        LinkResult r3;
        r3.deviceId = "SPL-002"; r3.deviceName = "三功分器"; r3.deviceType = "功分器";
        r3.cableLength = 12.0; r3.cableLoss = 12.0 * feederLoss("1/2");
        r3.inputPower = applyLoss(r1.outputPower, r3.cableLoss);
        r3.deviceLoss = splitterLoss(3) + 0.15;
        r3.outputPower = applyLoss(r3.inputPower, r3.deviceLoss);
        report.results.append(r3);

        // 三功分 -> 天线3
        LinkResult a3;
        a3.deviceId = "ANT-003"; a3.deviceName = "定向壁挂天线1"; a3.deviceType = "天线";
        a3.isAntenna = true;
        a3.cableLength = 4.0; a3.cableLoss = 4.0 * feederLoss("1/2");
        a3.inputPower = applyLoss(r3.outputPower, a3.cableLoss);
        a3.outputPower = a3.inputPower;
        if (a3.outputPower < m_minAntennaPower) {
            a3.alarm = true; a3.alarmMessage = "天线口功率低于阈值";
            report.alarms.append(QString("%1 功率 %2dBm 低于阈值 %3dBm")
                .arg(a3.deviceName).arg(a3.outputPower, 0, 'f', 2).arg(m_minAntennaPower));
        }
        report.results.append(a3); report.antennas.append(a3);

        // 三功分 -> 天线4
        LinkResult a4;
        a4.deviceId = "ANT-004"; a4.deviceName = "全向吸顶天线3"; a4.deviceType = "天线";
        a4.isAntenna = true;
        a4.cableLength = 6.0; a4.cableLoss = 6.0 * feederLoss("1/2");
        a4.inputPower = applyLoss(r3.outputPower, a4.cableLoss);
        a4.outputPower = a4.inputPower;
        if (a4.outputPower < m_minAntennaPower) {
            a4.alarm = true; a4.alarmMessage = "天线口功率低于阈值";
            report.alarms.append(QString("%1 功率 %2dBm 低于阈值 %3dBm")
                .arg(a4.deviceName).arg(a4.outputPower, 0, 'f', 2).arg(m_minAntennaPower));
        }
        report.results.append(a4); report.antennas.append(a4);

        // 统计
        if (!report.antennas.isEmpty()) {
            qreal sum = 0;
            report.minAntennaPower = 1e9; report.maxAntennaPower = -1e9;
            for (const auto &a : report.antennas) {
                sum += a.outputPower;
                report.minAntennaPower = qMin(report.minAntennaPower, a.outputPower);
                report.maxAntennaPower = qMax(report.maxAntennaPower, a.outputPower);
            }
            report.avgAntennaPower = sum / report.antennas.size();
        }

        return report;
    }

private:
    BandType m_band = Band_LTE_FDD;
    qreal m_minAntennaPower = 10.0;  // dBm
    qreal m_maxAntennaPower = 20.0;  // dBm
    QMap<QString, FeederLoss> m_feederTable;

    void initFeederLossTable() {
        // 1/2馈线
        FeederLoss f1; f1.name = "1/2";
        f1.lossPer100m = {{Band_GSM900, 6.9}, {Band_GSM1800, 9.5}, {Band_WCDMA, 10.5},
                          {Band_LTE_FDD, 11.0}, {Band_LTE_TDD, 12.0}, {Band_NR_700, 6.0},
                          {Band_NR_2600, 12.5}, {Band_NR_3500, 15.0}, {Band_NR_4900, 18.0}};
        m_feederTable["1/2"] = f1;

        // 7/8馈线
        FeederLoss f2; f2.name = "7/8";
        f2.lossPer100m = {{Band_GSM900, 3.9}, {Band_GSM1800, 5.4}, {Band_WCDMA, 5.9},
                          {Band_LTE_FDD, 6.2}, {Band_LTE_TDD, 6.8}, {Band_NR_700, 3.4},
                          {Band_NR_2600, 7.1}, {Band_NR_3500, 8.5}, {Band_NR_4900, 10.0}};
        m_feederTable["7/8"] = f2;

        // 1-5/8馈线
        FeederLoss f3; f3.name = "1-5/8";
        f3.lossPer100m = {{Band_GSM900, 2.4}, {Band_GSM1800, 3.3}, {Band_WCDMA, 3.6},
                          {Band_LTE_FDD, 3.8}, {Band_LTE_TDD, 4.1}, {Band_NR_700, 2.1},
                          {Band_NR_2600, 4.3}, {Band_NR_3500, 5.2}, {Band_NR_4900, 6.2}};
        m_feederTable["1-5/8"] = f3;

        // 5D-FB
        FeederLoss f4; f4.name = "5D-FB";
        f4.lossPer100m = {{Band_GSM900, 11.0}, {Band_GSM1800, 15.0}, {Band_WCDMA, 16.5},
                          {Band_LTE_FDD, 17.0}, {Band_LTE_TDD, 18.5}, {Band_NR_700, 9.5},
                          {Band_NR_2600, 19.0}, {Band_NR_3500, 22.0}, {Band_NR_4900, 26.0}};
        m_feederTable["5D-FB"] = f4;

        // 8D-FB
        FeederLoss f5; f5.name = "8D-FB";
        f5.lossPer100m = {{Band_GSM900, 7.5}, {Band_GSM1800, 10.5}, {Band_WCDMA, 11.5},
                          {Band_LTE_FDD, 12.0}, {Band_LTE_TDD, 13.0}, {Band_NR_700, 6.5},
                          {Band_NR_2600, 13.5}, {Band_NR_3500, 16.0}, {Band_NR_4900, 19.0}};
        m_feederTable["8D-FB"] = f5;
    }
};

} // namespace Zhifen

#endif // LINK_CALCULATOR_H
