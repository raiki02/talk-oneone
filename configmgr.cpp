#include "configmgr.h"
#include "dbmanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QHash>
#include <QDebug>

// 默认系统提示词
const QString ConfigMgr::DEFAULT_SYSTEM_PROMPT = R"(你是一个名叫%1的AI助手。请以友好、自然的方式与用户%2对话。
你的性格特点：%3
请用第一人称回答问题，保持对话简洁自然。)";

// 默认AI性格描述
const QString ConfigMgr::DEFAULT_AI_PERSONALITY = R"(温柔体贴、善解人意、偶尔会有些俏皮。喜欢关心对方的日常生活，会在合适的时候给予鼓励和支持。说话时会用一些语气词如"嘛"、"呢"、"呐"来增加亲和力。)";

ConfigMgr &ConfigMgr::instance()
{
    static ConfigMgr instance;
    return instance;
}

ConfigMgr::ConfigMgr()
{
    loadConfig();
}

ConfigMgr::~ConfigMgr()
{
    saveConfig();
}

// 用户信息
QString ConfigMgr::userName() const { return _userName; }
void ConfigMgr::setUserName(const QString &newUserName) {
    if (_userName == newUserName) return;
    _userName = newUserName;
    emit userNameChanged();
}

QString ConfigMgr::userDescription() const { return _userDescription; }
void ConfigMgr::setUserDescription(const QString &newUserDescription) {
    if (_userDescription == newUserDescription) return;
    _userDescription = newUserDescription;
    emit userDescriptionChanged();
}

// AI角色设定
QString ConfigMgr::aiName() const { return _aiName; }
void ConfigMgr::setAiName(const QString &newAiName) {
    if (_aiName == newAiName) return;
    _aiName = newAiName;
    emit aiNameChanged();
}

QString ConfigMgr::aiPersonality() const { return _aiPersonality; }
void ConfigMgr::setAiPersonality(const QString &newAiPersonality) {
    if (_aiPersonality == newAiPersonality) return;
    _aiPersonality = newAiPersonality;
    emit aiPersonalityChanged();
}

QString ConfigMgr::systemPrompt() const { return _systemPrompt; }
void ConfigMgr::setSystemPrompt(const QString &newSystemPrompt) {
    if (_systemPrompt == newSystemPrompt) return;
    _systemPrompt = newSystemPrompt;
    emit systemPromptChanged();
}

// 服务器配置
QString ConfigMgr::llmServerUrl() const { return _llmServerUrl; }
void ConfigMgr::setLlmServerUrl(const QString &newLlmServerUrl) {
    if (_llmServerUrl == newLlmServerUrl) return;
    _llmServerUrl = newLlmServerUrl;
    emit llmServerUrlChanged();
}

QString ConfigMgr::llmModelName() const { return _llmModelName; }
void ConfigMgr::setLlmModelName(const QString &newLlmModelName) {
    if (_llmModelName == newLlmModelName) return;
    _llmModelName = newLlmModelName;
    emit llmModelNameChanged();
}

// ── 讯飞 TTS getter/setter ──

QString ConfigMgr::iflytekAppId() const { return _iflytekAppId; }
void ConfigMgr::setIflytekAppId(const QString &newAppId) {
    if (_iflytekAppId == newAppId) return;
    _iflytekAppId = newAppId;
    emit iflytekAppIdChanged();
}

QString ConfigMgr::iflytekApiKey() const { return _iflytekApiKey; }
void ConfigMgr::setIflytekApiKey(const QString &newApiKey) {
    if (_iflytekApiKey == newApiKey) return;
    _iflytekApiKey = newApiKey;
    emit iflytekApiKeyChanged();
}

QString ConfigMgr::iflytekApiSecret() const { return _iflytekApiSecret; }
void ConfigMgr::setIflytekApiSecret(const QString &newApiSecret) {
    if (_iflytekApiSecret == newApiSecret) return;
    _iflytekApiSecret = newApiSecret;
    emit iflytekApiSecretChanged();
}

QString ConfigMgr::iflytekVoiceName() const { return _iflytekVoiceName; }
void ConfigMgr::setIflytekVoiceName(const QString &newVoiceName) {
    if (_iflytekVoiceName == newVoiceName) return;
    _iflytekVoiceName = newVoiceName;
    emit iflytekVoiceNameChanged();
}

int ConfigMgr::iflytekSpeed() const { return _iflytekSpeed; }
void ConfigMgr::setIflytekSpeed(int newSpeed) {
    if (_iflytekSpeed == newSpeed) return;
    _iflytekSpeed = newSpeed;
    emit iflytekSpeedChanged();
}

int ConfigMgr::iflytekPitch() const { return _iflytekPitch; }
void ConfigMgr::setIflytekPitch(int newPitch) {
    if (_iflytekPitch == newPitch) return;
    _iflytekPitch = newPitch;
    emit iflytekPitchChanged();
}

int ConfigMgr::iflytekVolume() const { return _iflytekVolume; }
void ConfigMgr::setIflytekVolume(int newVolume) {
    if (_iflytekVolume == newVolume) return;
    _iflytekVolume = newVolume;
    emit iflytekVolumeChanged();
}

QString ConfigMgr::buildFullSystemPrompt() const
{
    if (!_systemPrompt.isEmpty()) {
        return _systemPrompt;
    }
    return DEFAULT_SYSTEM_PROMPT.arg(_aiName, _userName, _aiPersonality);
}

void ConfigMgr::saveConfig()
{
    if (!DbManager::instance().isConnected()) {
        qWarning() << "ConfigMgr::saveConfig: DB not connected, config not persisted.";
        return;
    }

    QSqlDatabase db = DbManager::instance().database();
    QSqlQuery query(db);

    db.transaction();

    auto upsert = [&](const QString& key, const QString& value) {
        query.prepare("INSERT INTO config (config_key, config_value) VALUES (?, ?) "
                      "ON DUPLICATE KEY UPDATE config_value = VALUES(config_value)");
        query.addBindValue(key);
        query.addBindValue(value);
        if (!query.exec()) {
            qWarning() << "ConfigMgr::saveConfig failed for" << key << ":" << query.lastError().text();
        }
    };

    upsert("User/userName",        _userName);
    upsert("User/userDescription", _userDescription);
    upsert("AI/aiName",            _aiName);
    upsert("AI/aiPersonality",     _aiPersonality);
    upsert("AI/systemPrompt",      _systemPrompt);
    upsert("Server/llmServerUrl",  _llmServerUrl);
    upsert("Server/llmModelName",  _llmModelName);
    upsert("iFlytek/appId",        _iflytekAppId);
    upsert("iFlytek/apiKey",       _iflytekApiKey);
    upsert("iFlytek/apiSecret",    _iflytekApiSecret);
    upsert("iFlytek/voiceName",    _iflytekVoiceName);
    upsert("iFlytek/speed",        QString::number(_iflytekSpeed));
    upsert("iFlytek/pitch",        QString::number(_iflytekPitch));
    upsert("iFlytek/volume",       QString::number(_iflytekVolume));

    db.commit();
}

void ConfigMgr::loadConfig()
{
    if (!DbManager::instance().isConnected()) {
        // DB not available: initialize with defaults
        resetToDefault();
        return;
    }

    QSqlDatabase db = DbManager::instance().database();
    QSqlQuery query(db);
    query.prepare("SELECT config_key, config_value FROM config");

    QHash<QString, QString> configMap;
    if (query.exec()) {
        while (query.next()) {
            configMap.insert(query.value(0).toString(), query.value(1).toString());
        }
    }

    _userName        = configMap.value("User/userName",        DEFAULT_USER_NAME);
    _userDescription = configMap.value("User/userDescription", "");
    _aiName          = configMap.value("AI/aiName",            DEFAULT_AI_NAME);
    _aiPersonality   = configMap.value("AI/aiPersonality",     DEFAULT_AI_PERSONALITY);
    _systemPrompt    = configMap.value("AI/systemPrompt",      "");
    _llmServerUrl    = configMap.value("Server/llmServerUrl",  DEFAULT_LLM_URL);
    _llmModelName    = configMap.value("Server/llmModelName",  DEFAULT_MODEL);
    _iflytekAppId    = configMap.value("iFlytek/appId",        "");
    _iflytekApiKey   = configMap.value("iFlytek/apiKey",       "");
    _iflytekApiSecret= configMap.value("iFlytek/apiSecret",    "");
    _iflytekVoiceName= configMap.value("iFlytek/voiceName",    DEFAULT_IFLYTEK_VOICE);
    _iflytekSpeed    = configMap.value("iFlytek/speed",        QString::number(DEFAULT_IFLYTEK_SPEED)).toInt();
    _iflytekPitch    = configMap.value("iFlytek/pitch",        QString::number(DEFAULT_IFLYTEK_PITCH)).toInt();
    _iflytekVolume   = configMap.value("iFlytek/volume",       QString::number(DEFAULT_IFLYTEK_VOLUME)).toInt();
}

void ConfigMgr::resetToDefault()
{
    _userName = DEFAULT_USER_NAME;
    _userDescription = "";
    _aiName = DEFAULT_AI_NAME;
    _aiPersonality = DEFAULT_AI_PERSONALITY;
    _systemPrompt = "";
    _llmServerUrl = DEFAULT_LLM_URL;
    _llmModelName = DEFAULT_MODEL;

    _iflytekAppId = "";
    _iflytekApiKey = "";
    _iflytekApiSecret = "";
    _iflytekVoiceName = DEFAULT_IFLYTEK_VOICE;
    _iflytekSpeed = DEFAULT_IFLYTEK_SPEED;
    _iflytekPitch = DEFAULT_IFLYTEK_PITCH;
    _iflytekVolume = DEFAULT_IFLYTEK_VOLUME;

    emit userNameChanged();
    emit userDescriptionChanged();
    emit aiNameChanged();
    emit aiPersonalityChanged();
    emit systemPromptChanged();
    emit llmServerUrlChanged();
    emit llmModelNameChanged();

    emit iflytekAppIdChanged();
    emit iflytekApiKeyChanged();
    emit iflytekApiSecretChanged();
    emit iflytekVoiceNameChanged();
    emit iflytekSpeedChanged();
    emit iflytekPitchChanged();
    emit iflytekVolumeChanged();

    // DB-connected path: persist the defaults
    saveConfig();
}
