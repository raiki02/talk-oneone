-- init.sql - Talk-Oneone database schema for MySQL 8.4

CREATE DATABASE IF NOT EXISTS talkoneone
    CHARACTER SET utf8mb4
    COLLATE utf8mb4_unicode_ci;

USE talkoneone;

-- Application configuration (replaces QSettings INI file)
CREATE TABLE IF NOT EXISTS config (
    config_key   VARCHAR(128) PRIMARY KEY,
    config_value TEXT         NOT NULL,
    updated_at   TIMESTAMP    NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Conversation metadata (replaces conversations.json)
CREATE TABLE IF NOT EXISTS conversations (
    id          CHAR(36)     PRIMARY KEY,
    title       VARCHAR(255) NOT NULL DEFAULT '新对话',
    created_at  TIMESTAMP    NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at  TIMESTAMP    NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_updated_at (updated_at DESC)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Chat messages (replaces individual <uuid>.json files)
CREATE TABLE IF NOT EXISTS messages (
    id              BIGINT       AUTO_INCREMENT PRIMARY KEY,
    conversation_id CHAR(36)     NOT NULL,
    role            VARCHAR(32)  NOT NULL,
    content         TEXT         NOT NULL,
    created_at      TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP(3),
    FOREIGN KEY (conversation_id) REFERENCES conversations(id) ON DELETE CASCADE,
    INDEX idx_conversation_time (conversation_id, created_at ASC)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Seed default config values
INSERT INTO config (config_key, config_value) VALUES
    ('User/userName',        '用户'),
    ('User/userDescription', ''),
    ('AI/aiName',            '雅萱'),
    ('AI/aiPersonality',     ''),
    ('AI/systemPrompt',      ''),
    ('Server/llmServerUrl',  'http://10.242.115.43:11434/api/chat'),
    ('Server/llmModelName',  'qwen3.5:0.8b'),
    ('iFlytek/appId',        ''),
    ('iFlytek/apiKey',       ''),
    ('iFlytek/apiSecret',    ''),
    ('iFlytek/voiceName',    'xiaoyan'),
    ('iFlytek/speed',        '50'),
    ('iFlytek/pitch',        '50'),
    ('iFlytek/volume',       '50')
ON DUPLICATE KEY UPDATE config_key = config_key;
