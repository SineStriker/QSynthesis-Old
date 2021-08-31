#include "Variables.h"
#include "QUtauStrings.h"

QString Version = "0.1.65";
QString TempDirName = "QSyntehsis_Temporary";

QString AppPath;

QString MainTitle;
QString ErrorTitle;
QString UntitledFileName;
QString UntitledPrefix;
QString DeletedPrefix;

PluginsCollect *plugins = nullptr;
VoiceCollect *voice = nullptr;
NormalFileCollect *themes = nullptr;
NormalFileCollect *languages = nullptr;

QTranslator *translator = nullptr;

QString openFilterString;
QString saveFilterString;
QString importFilterString;
QString appendFilterString;
QString exportSelectionFilterString;
QString exportTrackFilterString;
QString exportOtoIniFilterString;
QString imageFilterString;
QString audioFilterString;
QString toolsFilterString;

QString tempDirectoryName;

QString voiceProfile() {
    return AppPath + Slash + DIR_NAME_VOICE;
}

QString pluginsProfile() {
    return AppPath + Slash + DIR_NAME_PLUGIN;
}

QString configProfile() {
    return AppPath + Slash + DIR_NAME_CONFIG;
}
QString themesProfile() {
    return configProfile() + Slash + DIR_NAME_CONFIG_THEMES;
}

QString languagesProfile() {
    return configProfile() + Slash + DIR_NAME_CONFIG_LANGUAGES;
}

QString settingConifgPath() {
    return configProfile() + Slash + FILE_NAME_SETTING_JSON;
}

QString keyboardConfigPath() {
    return configProfile() + Slash + FILE_NAME_KEYBOARD_JSON;
}

QString settingIniPath() {
    return AppPath + Slash + FILE_NAME_SETTING_INI;
}