#pragma once

#include <QObject>
#include <QSocketNotifier>

class PtyTerminal : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString textBuffer READ textBuffer NOTIFY textBufferChanged)

public:
    explicit PtyTerminal(QObject *parent = nullptr);
    ~PtyTerminal();

    QString textBuffer() const { return m_textBuffer; }

    Q_INVOKABLE void sendKey(const QString &str);
    Q_INVOKABLE void clear();

signals:
    void textBufferChanged();

private slots:
    void onPtyReadyRead();

private:
    int m_masterFd = -1;
    pid_t m_childPid = -1;
    QSocketNotifier *m_notifier = nullptr;
    QString m_textBuffer;

    QString cleanAnsi(const QString &input);
};