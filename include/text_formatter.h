#pragma once
#include <string>
#include <vector>

/**
 * @brief Коды ошибок программы форматирования текста.
 */
enum class ErrorCode {
    noError,              ///< Ошибок нет
    inputFileNotFound,    ///< Входной файл не найден
    outputFileCreateFail, ///< Не удалось создать выходной файл
    invalidLineWidth,     ///< Ширина строки вне диапазона 40–120
    emptyInputFile,       ///< Входной файл пуст
    invalidCharacter,     ///< Недопустимый символ во входном тексте
    wordTooLong,          ///< Слово длиннее заданной ширины строки
    nonIntegerWidth,      ///< Ширина строки не является целым числом
    inputLineTooLong,     ///< Строка входного файла длиннее 1024 символов
    tooManyOutputLines    ///< Строк в результате больше 1000
};

/**
 * @brief Структура для передачи информации об ошибке.
 */
struct FormatError {
    ErrorCode code       = ErrorCode::noError; ///< Код ошибки
    int       lineNumber = 0;                  ///< Номер строки входного файла
    int       charPosition = 0;                ///< Позиция символа в строке
    char      invalidChar  = '\0';             ///< Недопустимый символ

    /**
     * @brief Составляет текстовое сообщение об ошибке.
     * @return Строка с описанием ошибки для пользователя.
     */
    std::string getMessage() const;
};

/**
 * @brief Класс форматирования текста по заданной ширине с выравниванием.
 */
class TextFormatter {
public:
    /**
     * @brief Конструктор. Запоминает заданную ширину строки.
     * @param width Ширина строки вывода в символах.
     */
    explicit TextFormatter(int width);

    /**
     * @brief Читает исходный текст, проверяет символы и заполняет список words.
     * @param text Строка с исходным текстом из входного файла.
     * @return FormatError (code == noError при успехе).
     */
    FormatError parseWords(const std::string& text);

    /**
     * @brief Перебирает слова и собирает из них строки нужной ширины.
     *        Заполняет outputLines.
     */
    void buildLines();

    /**
     * @brief Выравнивает строку по ширине, равномерно распределяя пробелы.
     * @param lineWords Вектор слов текущей строки.
     * @param isLast    true, если строка последняя в абзаце.
     * @return Выровненная строка нужной ширины.
     */
    std::string justifyLine(const std::vector<std::string>& lineWords, bool isLast);

    /**
     * @brief Возвращает готовый список строк результата.
     * @return Вектор отформатированных строк.
     */
    std::vector<std::string> getOutputLines() const;

    // Поля (доступны для тестирования через friend или публично)
    int lineWidth;                   ///< Ширина строки вывода
    std::vector<std::string> words;  ///< Слова, извлечённые из входного текста
    std::vector<std::string> outputLines; ///< Готовые отформатированные строки
};
