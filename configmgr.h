#ifndef CONFIGMGR_H
#define CONFIGMGR_H

#include <QObject>

class ConfigMgr : public QObject
{
    Q_OBJECT
    // 用户信息
    Q_PROPERTY(QString userName READ userName WRITE setUserName NOTIFY userNameChanged FINAL)
    Q_PROPERTY(QString userDescription READ userDescription WRITE setUserDescription NOTIFY userDescriptionChanged FINAL)

    // AI角色设定
    Q_PROPERTY(QString aiName READ aiName WRITE setAiName NOTIFY aiNameChanged FINAL)
    Q_PROPERTY(QString aiPersonality READ aiPersonality WRITE setAiPersonality NOTIFY aiPersonalityChanged FINAL)
    Q_PROPERTY(QString systemPrompt READ systemPrompt WRITE setSystemPrompt NOTIFY systemPromptChanged FINAL)

    // 服务器配置
    Q_PROPERTY(QString llmServerUrl READ llmServerUrl WRITE setLlmServerUrl NOTIFY llmServerUrlChanged FINAL)
    Q_PROPERTY(QString llmModelName READ llmModelName WRITE setLlmModelName NOTIFY llmModelNameChanged FINAL)

    // 讯飞 TTS 设置
    Q_PROPERTY(QString iflytekAppId READ iflytekAppId WRITE setIflytekAppId NOTIFY iflytekAppIdChanged FINAL)
    Q_PROPERTY(QString iflytekApiKey READ iflytekApiKey WRITE setIflytekApiKey NOTIFY iflytekApiKeyChanged FINAL)
    Q_PROPERTY(QString iflytekApiSecret READ iflytekApiSecret WRITE setIflytekApiSecret NOTIFY iflytekApiSecretChanged FINAL)
    Q_PROPERTY(QString iflytekVoiceName READ iflytekVoiceName WRITE setIflytekVoiceName NOTIFY iflytekVoiceNameChanged FINAL)
    Q_PROPERTY(int iflytekSpeed READ iflytekSpeed WRITE setIflytekSpeed NOTIFY iflytekSpeedChanged FINAL)
    Q_PROPERTY(int iflytekPitch READ iflytekPitch WRITE setIflytekPitch NOTIFY iflytekPitchChanged FINAL)
    Q_PROPERTY(int iflytekVolume READ iflytekVolume WRITE setIflytekVolume NOTIFY iflytekVolumeChanged FINAL)

public:
    static ConfigMgr& instance();

    // 用户信息
    QString userName() const;
    void setUserName(const QString &newUserName);
    QString userDescription() const;
    void setUserDescription(const QString &newUserDescription);

    // AI角色设定
    QString aiName() const;
    void setAiName(const QString &newAiName);
    QString aiPersonality() const;
    void setAiPersonality(const QString &newAiPersonality);
    QString systemPrompt() const;
    void setSystemPrompt(const QString &newSystemPrompt);

    // 服务器配置
    QString llmServerUrl() const;
    void setLlmServerUrl(const QString &newLlmServerUrl);
    QString llmModelName() const;
    void setLlmModelName(const QString &newLlmModelName);

    // 讯飞 TTS
    QString iflytekAppId() const;
    void setIflytekAppId(const QString &newAppId);
    QString iflytekApiKey() const;
    void setIflytekApiKey(const QString &newApiKey);
    QString iflytekApiSecret() const;
    void setIflytekApiSecret(const QString &newApiSecret);
    QString iflytekVoiceName() const;
    void setIflytekVoiceName(const QString &newVoiceName);
    int iflytekSpeed() const;
    void setIflytekSpeed(int newSpeed);
    int iflytekPitch() const;
    void setIflytekPitch(int newPitch);
    int iflytekVolume() const;
    void setIflytekVolume(int newVolume);

    // 构建完整的系统提示词
    Q_INVOKABLE QString buildFullSystemPrompt() const;

    // 保存和加载配置
    Q_INVOKABLE void saveConfig();
    Q_INVOKABLE void loadConfig();
    Q_INVOKABLE void resetToDefault();

private:
    ConfigMgr();
    ~ConfigMgr();
    ConfigMgr(const ConfigMgr&) = delete;
    ConfigMgr& operator=(const ConfigMgr&) = delete;

    // 用户信息
    QString _userName;
    QString _userDescription;

    // AI角色设定
    QString _aiName;
    QString _aiPersonality;
    QString _systemPrompt;

    // 服务器配置
    QString _llmServerUrl;
    QString _llmModelName;

    // 讯飞 TTS 设置
    QString _iflytekAppId;
    QString _iflytekApiKey;
    QString _iflytekApiSecret;
    QString _iflytekVoiceName;  // vcn: 发音人名称
    int _iflytekSpeed;          // 0-100
    int _iflytekPitch;          // 0-100
    int _iflytekVolume;         // 0-100

    // 默认值
    static constexpr const char* DEFAULT_USER_NAME = "用户";
    static constexpr const char* DEFAULT_AI_NAME = "雅萱";
    static constexpr const char* DEFAULT_LLM_URL = "http://10.242.115.43:11434/api/chat";
    static constexpr const char* DEFAULT_MODEL = "qwen3.5:0.8b";

    // 讯飞 TTS 默认值
    static constexpr const char* DEFAULT_IFLYTEK_VOICE = "xiaoyan";
    static constexpr int DEFAULT_IFLYTEK_SPEED = 50;
    static constexpr int DEFAULT_IFLYTEK_PITCH = 50;
    static constexpr int DEFAULT_IFLYTEK_VOLUME = 50;

    static const QString DEFAULT_SYSTEM_PROMPT;
    static const QString DEFAULT_AI_PERSONALITY;

signals:
    void userNameChanged();
    void userDescriptionChanged();
    void aiNameChanged();
    void aiPersonalityChanged();
    void systemPromptChanged();
    void llmServerUrlChanged();
    void llmModelNameChanged();

    // 讯飞 TTS 信号
    void iflytekAppIdChanged();
    void iflytekApiKeyChanged();
    void iflytekApiSecretChanged();
    void iflytekVoiceNameChanged();
    void iflytekSpeedChanged();
    void iflytekPitchChanged();
    void iflytekVolumeChanged();
};

#endif // CONFIGMGR_H
