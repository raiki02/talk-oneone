#include "audiomgr.h"
#include <QMediaDevices>
#include <QAudioDevice>
#include <QDebug>
#include <QBuffer>

AudioMgr::AudioMgr(QObject *parent)
    : QObject{parent}
    ,_audioSink(nullptr)
{
}

AudioMgr::~AudioMgr()
{
    if(_audioSink!=nullptr){
        _audioSink->stop();
        _audioSink->deleteLater();
        _audioSink=nullptr;
    }
}

void AudioMgr::stopAudioOutput()
{
    if(_audioSink!=nullptr){
        _audioSink->stop();
        _audioSink->deleteLater();
        _audioSink=nullptr;
    }
}

void AudioMgr::slot_tts_finished(const QByteArray &data)
{
    _ttsPlaybackFinished = false;

    if (data.isEmpty()) {
        qDebug() << "TTS 数据为空，通知UI恢复";
        emit signal_tts_playback_finished();
        return;
    }

    qDebug() << "TTS 数据大小:" << data.size();

    QBuffer *buffer = new QBuffer;
    buffer->setData(data);
    buffer->open(QIODevice::ReadOnly);

    QAudioFormat format;
    format.setSampleRate(16000);  // 讯飞 TTS 输出 16kHz PCM
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    QAudioDevice info(QMediaDevices::defaultAudioOutput());
    if (!info.isFormatSupported(format)) {
        qWarning() << "Raw audio format not supported by backend, cannot play audio.";
        delete buffer;
        emit signal_tts_playback_finished();
        return;
    }

    if (_audioSink) {
        _audioSink->stop();
        _audioSink->deleteLater();
    }

    _audioSink = new QAudioSink(format, this);
    connect(_audioSink, &QAudioSink::stateChanged,[this, buffer](QtAudio::State newState){
        switch (newState) {
        case QtAudio::IdleState:
            // Finished playing (no more data)
            stopAudioOutput();
            delete buffer;
            if (!_ttsPlaybackFinished) {
                _ttsPlaybackFinished = true;
                emit signal_tts_playback_finished();  // 通知播放完成
            }
            break;

        case QtAudio::StoppedState:
            // Stopped for other reasons (error, interruption, etc.)
            if (_audioSink->error() != QtAudio::NoError) {
                qWarning() << "Audio playback error";
            }
            // 无论如何都要通知 UI 恢复
            if (!_ttsPlaybackFinished) {
                _ttsPlaybackFinished = true;
                emit signal_tts_playback_finished();
            }
            break;

        default:
            break;
        }
    });
    _audioSink->start(buffer);
}
