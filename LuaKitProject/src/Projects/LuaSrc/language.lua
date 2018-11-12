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
-- };

local languagesTable = {
    "en",
    "ch",
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
}

local languageCode = lua.language.getLanguageType()

return function (word)
    local t = require(languagesTable[languageCode+1])
    if t then
        return t[word]
    else
        return ""
    end
end
