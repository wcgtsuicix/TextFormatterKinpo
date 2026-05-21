#include "text_formatter.h"

#include <sstream>
#include <cctype>
#include <numeric>

// ─────────────────────────── FormatError ────────────────────────────────────

std::string FormatError::getMessage() const {
    switch (code) {
        case ErrorCode::noError:
            return "Успех.";
        case ErrorCode::inputFileNotFound:
            return "Неверно указан файл с входными данными. "
                   "Возможно, файл не существует.";
        case ErrorCode::outputFileCreateFail:
            return "Неверно указан файл для выходных данных. "
                   "Возможно, указанного расположения не существует "
                   "или нет прав на запись.";
        case ErrorCode::invalidLineWidth:
            return "Ширина строки должна быть в диапазоне 40–120 символов.";
        case ErrorCode::emptyInputFile:
            return "Входной файл пуст. Добавьте текст для форматирования.";
        case ErrorCode::invalidCharacter:
            return std::string("Обнаружен недопустимый символ '") +
                   invalidChar + "' (" +
                   std::to_string(lineNumber) + " строка " +
                   std::to_string(charPosition) + " символ).";
        case ErrorCode::wordTooLong:
            return "Обнаружено слово длиннее максимальной ширины строки.";
        case ErrorCode::nonIntegerWidth:
            return "Ширина строки должна быть целым числом.";
        case ErrorCode::inputLineTooLong:
            return "Обнаружено превышение допустимой длины строки "
                   "во входном файле (1024 символа).";
        case ErrorCode::tooManyOutputLines:
            return "Обнаружено превышение допустимого количества строк.";
        default:
            return "Неизвестная ошибка.";
    }
}

// ─────────────────────────── TextFormatter ──────────────────────────────────

TextFormatter::TextFormatter(int width) : lineWidth(width) {}

// ── UTF-8 утилиты ────────────────────────────────────────────────────────────

/**
 * Длина строки в кодовых точках Unicode (не в байтах).
 * UTF-8: продолжающие байты имеют вид 10xxxxxx (0x80–0xBF), их пропускаем.
 */
static int utf8Len(const std::string& s) {
    int count = 0;
    for (unsigned char c : s) {
        if ((c & 0xC0) != 0x80) ++count; // не продолжающий байт
    }
    return count;
}

/**
 * Разбивает строку UTF-8 на кодовые точки (каждая точка — строка из 1–4 байт).
 */
static std::vector<std::string> utf8Codepoints(const std::string& s) {
    std::vector<std::string> result;
    size_t i = 0;
    while (i < s.size()) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        int bytes = 1;
        if      ((c & 0x80) == 0)    bytes = 1;
        else if ((c & 0xE0) == 0xC0) bytes = 2;
        else if ((c & 0xF0) == 0xE0) bytes = 3;
        else if ((c & 0xF8) == 0xF0) bytes = 4;
        result.push_back(s.substr(i, bytes));
        i += bytes;
    }
    return result;
}

// ── Допустимые ASCII-символы для знаков препинания ───────────────────────────
// Разрешены: . , ! ? ; : - ( ) [ ] { } " ' / \\ + = < > и т.п.
// Запрещены: @ # $ % ^ & * ` ~ | и прочие «технические» символы.
static const std::string ALLOWED_PUNCT = ".,!?;:-()'\"[]{}/<>+=-\\_";

static bool isAllowedAsciiPunct(char c) {
    return ALLOWED_PUNCT.find(c) != std::string::npos;
}

/**
 * Проверяет, допустим ли символ (задаётся стартовым байтом UTF-8 кодовой точки).
 * Допустимы: пробел, табуляция, цифры ASCII, буквы ASCII, кириллица (0x80+),
 * разрешённые знаки препинания ASCII.
 */
static bool isAllowedCodepoint(const std::string& cp) {
    if (cp.empty()) return false;
    unsigned char first = static_cast<unsigned char>(cp[0]);

    // Многобайтовые символы (кириллица и пр.) — разрешены
    if (first >= 0x80) return true;

    char c = cp[0];
    if (c == ' ' || c == '\t')    return true;
    if (std::isdigit((unsigned char)c)) return true;
    if (std::isalpha((unsigned char)c)) return true;
    if (isAllowedAsciiPunct(c))   return true;

    return false;
}

/**
 * Проверяет, является ли UTF-8 токен «чистым» знаком препинания
 * (состоит только из разрешённых ASCII-знаков препинания, без букв/цифр).
 */
static bool isPunctToken(const std::string& token) {
    if (token.empty()) return false;
    auto cps = utf8Codepoints(token);
    for (const auto& cp : cps) {
        unsigned char first = static_cast<unsigned char>(cp[0]);
        if (first >= 0x80) return false;       // кириллица — не пунктуация
        char c = cp[0];
        if (std::isalnum((unsigned char)c)) return false;
        if (!isAllowedAsciiPunct(c))        return false;
    }
    return true;
}

FormatError TextFormatter::parseWords(const std::string& text) {
    words.clear();

    // Разбиваем текст на строки
    std::vector<std::string> lines;
    {
        std::istringstream ss(text);
        std::string line;
        while (std::getline(ss, line)) {
            // Убираем '\r' (Windows CRLF)
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            lines.push_back(line);
        }
    }

    for (int lineIdx = 0; lineIdx < static_cast<int>(lines.size()); ++lineIdx) {
        const std::string& line = lines[lineIdx];

        // 2.1 Проверяем длину строки в символах (кодовых точках)
        int lineCharLen = utf8Len(line);
        if (lineCharLen > 1024) {
            FormatError err;
            err.code = ErrorCode::inputLineTooLong;
            return err;
        }

        // 2.2 Проверяем каждый символ (кодовую точку)
        auto codepoints = utf8Codepoints(line);
        for (int cpIdx = 0; cpIdx < static_cast<int>(codepoints.size()); ++cpIdx) {
            const std::string& cp = codepoints[cpIdx];
            if (!isAllowedCodepoint(cp)) {
                FormatError err;
                err.code         = ErrorCode::invalidCharacter;
                err.lineNumber   = lineIdx + 1;
                err.charPosition = cpIdx + 1;
                // Сохраняем первый байт (для ASCII-символов это весь символ)
                err.invalidChar  = cp[0];
                return err;
            }
        }

        // 2.3 Разбиваем строку на токены по пробелам и табуляции
        std::vector<std::string> tokens;
        {
            std::istringstream ss(line);
            std::string tok;
            while (ss >> tok) {
                tokens.push_back(tok);
            }
        }

        // 2.4 Обрабатываем токены
        for (const std::string& tok : tokens) {
            if (tok.empty()) continue;

            if (isPunctToken(tok)) {
                // 2.4.1 Знак препинания — приклеиваем к последнему слову
                if (!words.empty()) {
                    words.back() += tok;
                } else {
                    words.push_back(tok);
                }
            } else {
                // Длину считаем в символах (кодовых точках)
                int wordCharLen = utf8Len(tok);
                // 2.4.2 Проверяем длину слова
                if (wordCharLen > lineWidth) {
                    FormatError err;
                    err.code = ErrorCode::wordTooLong;
                    return err;
                }
                // 2.4.3 Добавляем как новое слово
                words.push_back(tok);
            }
        }
    }

    // Проверяем слова после приклеивания знаков препинания
    for (const std::string& w : words) {
        if (utf8Len(w) > lineWidth) {
            FormatError err;
            err.code = ErrorCode::wordTooLong;
            return err;
        }
    }

    FormatError ok;
    ok.code = ErrorCode::noError;
    return ok;
}

void TextFormatter::buildLines() {
    outputLines.clear();
    if (words.empty()) return;

    std::vector<std::string> currentLine;
    int currentLen = 0; // суммарная длина в символах (кодовых точках) + пробелы

    for (const std::string& word : words) {
        int wordLen = utf8Len(word);

        if (currentLine.empty()) {
            // 2.1 Первое слово в строке
            currentLine.push_back(word);
            currentLen = wordLen;
        } else {
            // 2.2 Проверяем, помещается ли слово с пробелом перед ним
            if (currentLen + 1 + wordLen <= lineWidth) {
                currentLine.push_back(word);
                currentLen += 1 + wordLen;
            } else {
                // 2.3 Не помещается — завершаем текущую строку
                outputLines.push_back(justifyLine(currentLine, false));
                currentLine.clear();
                currentLine.push_back(word);
                currentLen = wordLen;
            }
        }
    }

    // 3. Последняя строка (isLast = true)
    if (!currentLine.empty()) {
        outputLines.push_back(justifyLine(currentLine, true));
    }
}

std::string TextFormatter::justifyLine(const std::vector<std::string>& lineWords,
                                       bool isLast) {
    if (lineWords.empty()) return "";

    // 1. Одно слово или последняя строка — без выравнивания
    if (lineWords.size() == 1 || isLast) {
        std::string result;
        for (size_t i = 0; i < lineWords.size(); ++i) {
            if (i > 0) result += ' ';
            result += lineWords[i];
        }
        return result;
    }

    // 2. Суммарная длина слов в символах (кодовых точках)
    int totalChars = 0;
    for (const auto& w : lineWords)
        totalChars += utf8Len(w);

    // 3. Общее количество пробелов
    int totalSpaces = lineWidth - totalChars;

    // 4. Количество промежутков
    int gaps = static_cast<int>(lineWords.size()) - 1;

    // 5. Базовое количество пробелов
    int base  = totalSpaces / gaps;
    // 6. Остаток — первые «extra» промежутков получают +1
    int extra = totalSpaces % gaps;

    // 7. Сборка строки
    std::string result;
    for (int i = 0; i < static_cast<int>(lineWords.size()); ++i) {
        result += lineWords[i];
        if (i < gaps) {
            int spaces = base + (i < extra ? 1 : 0);
            result += std::string(spaces, ' ');
        }
    }

    return result;
}

std::vector<std::string> TextFormatter::getOutputLines() const {
    return outputLines;
}
