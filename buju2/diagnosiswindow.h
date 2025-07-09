#ifndef DIAGNOSISWINDOW_H
#define DIAGNOSISWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QScrollArea>
#include <QVector>
#include <QHBoxLayout>

class DiagnosisWindow : public QWidget {
    Q_OBJECT
public:
    struct MotorUI {
        QLabel *imageLabel;
        QLabel *typeLabel;
        QLabel *statusLabel;
        QLabel *reasonLabel;
        QLabel *cnnStatusLabel;    // 新增：CNN预测结果标签
        QLabel *confidenceLabel;    // 新增：预测置信度标签
        QHBoxLayout *layout;
    };

    explicit DiagnosisWindow(int motorCount = 8,
                             const QVector<QString>& motorTypes = {"直流电机1", "直流电机2", "FOC电机1", "FOC电机2"},
                             const QVector<QString>& statusOptions = {"良好", "警告", "故障"},
                             const QVector<QString>& reasonOptions = {"电流正常", "电流过大", "振动正常", "振动异常"},
                             QWidget *parent = nullptr);

    // 更新后的电机状态更新函数，新增CNN预测结果和置信度参数
    void updateMotorStatus(int motorIndex,
                           const QString& status,
                           const QString& reason,
                           const QString& cnnStatus = "未预测",  // 默认值
                           double confidence = 0.0);            // 默认值

private:
    QVector<MotorUI> motorUIs;
    int motorCount;
    QVector<QString> motorTypes;
    QVector<QString> statusOptions;
    QVector<QString> reasonOptions;
    QScrollArea *scrollArea;

    void setupUI();
    void createMotorRow(int index);

public slots:
    // 保持原有槽函数声明不变
    void handleMotorStatusUpdate(int motorIndex, const QString& status, const QString& reason);
};

#endif // DIAGNOSISWINDOW_H
