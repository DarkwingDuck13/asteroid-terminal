#pragma once

#include <QObject>
#include <QSocketNotifier>
#include <QTimer>

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
    void onKbdReadyRead();
    void tryOpenKeyboard();

private:
    int m_masterFd = -1;
    pid_t m_childPid = -1;
    QSocketNotifier *m_ptyNotifier = nullptr;
    QString m_textBuffer;

    // Direct hardware evdev input
    int m_kbdFd = -1;
    QSocketNotifier *m_kbdNotifier = nullptr;
    QTimer *m_kbdWatchTimer = nullptr;
    bool m_shiftPressed = false;
    bool m_ctrlPressed = false;

    void handleKeyEvent(int code, int value);
    QString cleanAnsi(const QString &input);
};
