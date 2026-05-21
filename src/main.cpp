#include "../include/text_formatter.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>

int main(int argc, char* argv[]) {
    // 1. Проверяем количество аргументов
    if (argc != 4) {
        std::cerr << "Использование: format <входной_файл> <ширина> <выходной_файл>\n";
        return 1;
    }

    const std::string inputPath  = argv[1];
    const std::string widthArg   = argv[2];
    const std::string outputPath = argv[3];

    // 2. Читаем ширину строки
    int lineWidth = 0;
    {
        // 2.1 Проверяем, что это целое число
        std::istringstream ss(widthArg);
        int parsedWidth = 0;
        std::string rest;
        if (!(ss >> parsedWidth) || (ss >> rest && !rest.empty())) {
            FormatError err;
            err.code = ErrorCode::nonIntegerWidth;
            std::cerr << err.getMessage() << "\n";
            return 1;
        }
        // Доп. проверка: только цифры (допускаем знак минуса для диагностики)
        for (char c : widthArg) {
            if (c != '-' && !std::isdigit(static_cast<unsigned char>(c))) {
                FormatError err2;
                err2.code = ErrorCode::nonIntegerWidth;
                std::cerr << err2.getMessage() << "\n";
                return 1;
            }
        }
        lineWidth = parsedWidth;
    }
    // 2.2 Проверяем диапазон
    if (lineWidth < 40 || lineWidth > 120) {
        FormatError err;
        err.code = ErrorCode::invalidLineWidth;
        std::cerr << err.getMessage() << "\n";
        return 1;
    }

    // 3. Открываем входной файл
    std::ifstream inputFile(inputPath);
    if (!inputFile.is_open()) {
        FormatError err;
        err.code = ErrorCode::inputFileNotFound;
        std::cerr << err.getMessage() << "\n";
        return 1;
    }

    // 3.2 Проверяем, не пуст ли файл
    std::string text;
    {
        std::ostringstream ss;
        ss << inputFile.rdbuf();
        text = ss.str();
    }
    inputFile.close();

    if (text.empty()) {
        FormatError err;
        err.code = ErrorCode::emptyInputFile;
        std::cerr << err.getMessage() << "\n";
        return 1;
    }

    // 5. Создаём объект форматировщика
    TextFormatter formatter(lineWidth);

    // 6. Разбираем текст на слова
    FormatError parseResult = formatter.parseWords(text);
    if (parseResult.code != ErrorCode::noError) {
        std::cerr << parseResult.getMessage() << "\n";
        return 1;
    }

    // 7. Формируем строки
    formatter.buildLines();

    // 7.1 Проверяем количество строк
    if (static_cast<int>(formatter.getOutputLines().size()) > 1000) {
        FormatError err;
        err.code = ErrorCode::tooManyOutputLines;
        std::cerr << err.getMessage() << "\n";
        return 1;
    }

    // 8. Открываем выходной файл
    std::ofstream outputFile(outputPath);
    if (!outputFile.is_open()) {
        FormatError err;
        err.code = ErrorCode::outputFileCreateFail;
        std::cerr << err.getMessage() << "\n";
        return 1;
    }

    // 9. Записываем строки
    for (const std::string& line : formatter.getOutputLines()) {
        outputFile << line << "\n";
    }

    // 10. Закрываем файлы
    outputFile.close();

    return 0;
}
