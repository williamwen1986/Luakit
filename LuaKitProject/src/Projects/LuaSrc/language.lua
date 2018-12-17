-- enum LanguageType {
--     ENGLISH,
--     CHINESE,
--     CHINESE_TW,
--     JAPANESE,
--     FRENCH,
--     ITALIAN,
--     GERMAN,
--     SPANISH,
--     DUTCH,
--     RUSSIAN,
--     HUNGARIAN,
--     PORTUGUESE,
--     KOREAN,
--     ARABIC,
--     NORWEGIAN,
--     POLISH,
--     TURKISH,
--     UKRAINIAN,
--     ROMANIAN,
--     BULGARIAN,
--     BELARUSIAN,
--     PORTUGUESE_BRASIL,
--     BURMESE,
--     INDONESIAN,
--     HINDI,
--     MALAY,
-- };

local languagesTable = {
    "en",
    "zh",
    "chtw",
    "ja",
    "fr",
    "it",
    "de",
    "es",
    "nl",
    "ru",
    "hu",
    "pt",
    "ko",
    "ar",
    "nb",
    "pl",
    "tr",
    "uk",
    "ro",
    "bg",
    "be",
    "pt-BR",
    "my",
    "id",
    "hi",
    "ms",
    "th",
}

local languageCode = lua_language.getLanguageType()

return function (word)
    local t = require("language.lan")
    local lan = languagesTable[languageCode+1]
    if t[lan] and t[lan][word] then
        return t[lan][word]
    elseif t['en'] and t['en'][word] then
        return t['en'][word]
    else
        return ''
    end
end
