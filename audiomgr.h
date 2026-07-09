#ifndef AUDIOMGR_H
#define AUDIOMGR_H

#include <QObject>
#include <QAudioFormat>
#include <QAudioSink>

class AudioMgr : public QObject
{
    Q_OBJECT
public:
    explicit AudioMgr(QObject *parent = nullptr);
    ~AudioMgr();
    void stopAudioOutput();
public slots:
    void slot_tts_finished(const QByteArray &data);
signals:
    void signal_tts_playback_finished();  // TTS播放完成信号
private:
    QAudioSink* _audioSink;
    bool _ttsPlaybackFinished = false;  // 防重复触发 signal_tts_playback_finished
};

#endif // AUDIOMGR_H
