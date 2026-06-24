#include "../include/text_formatter.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>

// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
/*!
 * \brief Функция для парсинга и валидации ширины форматирования строки
 * \param [in] widthArg – строковый аргумент с шириной из командной строки
 * \return int – валидное целочисленное значение ширины строки
 * \throw FormatError – в случае некорректного формата или выхода за границы диапазона [40, 120]
 */
int parseAndValidateWidth(const std::string& widthArg) {
    std::istringstream ss(widthArg);
    int parsedWidth = 0;
    std::string rest;

    if (!(ss >> parsedWidth) || (ss >> rest && !rest.empty())) {
        throw FormatError{ErrorCode::nonIntegerWidth};
    }

    for (char c : widthArg) {
        if (c != '-' && !std::isdigit(static_cast<unsigned char>(c))) {
            throw FormatError{ErrorCode::nonIntegerWidth};
        }
    }

    if (parsedWidth < 40 || parsedWidth > 120) {
        throw FormatError{ErrorCode::invalidLineWidth};
    }

    return parsedWidth;
}

/*!
 * \brief Функция для чтения содержимого входного файла
 * \param [in] inputPath – путь к входному файлу
 * \return std::string – считанный из файла текст
 * \throw FormatError – если файл не найден или оказался пустым
 */
std::string readInputFile(const std::string& inputPath) {
    std::ifstream inputFile(inputPath);
    if (!inputFile.is_open()) {
        throw FormatError{ErrorCode::inputFileNotFound};
    }

    std::ostringstream ss;
    ss << inputFile.rdbuf();
    std::string text = ss.str();
    inputFile.close();

    if (text.empty()) {
        throw FormatError{ErrorCode::emptyInputFile};
    }

    return text;
}

/*!
 * \brief Функция для записи отформатированных строк в выходной файл
 * \param [in] outputPath – путь к выходному файлу
 * \param [in] lines – вектор сформированных строк для записи
 * \throw FormatError – если количество строк превышает 1000 или файл не удалось создать
 */
void writeOutputFile(const std::string& outputPath, const std::vector<std::string>& lines) {
    if (static_cast<int>(lines.size()) > 1000) {
        throw FormatError{ErrorCode::tooManyOutputLines};
    }

    std::ofstream outputFile(outputPath);
    if (!outputFile.is_open()) {
        throw FormatError{ErrorCode::outputFileCreateFail};
    }

    for (const std::string& line : lines) {
        outputFile << line << "\n";
    }
    outputFile.close();
}

// ГЛАВНАЯ ФУНКЦИЯ

int main(int argc, const char* const argv[]) {
    if (argc != 4) {
        std::cerr << "Использование: format <входной_файл> <ширина> <выходной_файл>\n";
        return 1;
    }

    try {
        // Шаг 1. Парсинг и валидация аргумента ширины (передаем argv[2])
        int lineWidth = parseAndValidateWidth(argv[2]);

        // Шаг 2. Чтение входных данных из файла (передаем argv[1])
        std::string text = readInputFile(argv[1]);

        // Шаг 3. Инициализация форматировщика и обработка текста
        TextFormatter formatter(lineWidth);
        FormatError parseResult = formatter.parseWords(text);
        if (parseResult.code != ErrorCode::noError) {
            std::cerr << parseResult.getMessage() << "\n";
            return 1;
        }

        formatter.buildLines();

        // Шаг 4. Запись результатов работы в файл (передаем argv[3])
        writeOutputFile(argv[3], formatter.getOutputLines());

    } catch (const FormatError& err) {
        // Централизованный вывод ошибок валидации и файловой системы
        std::cerr << err.getMessage() << "\n";
        return 1;
    }

    return 0;
}
