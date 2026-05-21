/**
 * @file Tests_TextFormatter.cpp
 * @brief Модульные тесты класса TextFormatter
 *        на основе Google Test.
 */

#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "../include/text_formatter.h"

// ═══════════════════════════════════════════════════════════════════════════
//  ТАБЛИЦА 1 — TextFormatter::parseWords
// ═══════════════════════════════════════════════════════════════════════════

// Тест 1: Корректный текст без знаков препинания
TEST(ParseWordsTest, SimpleTextWithoutPunctuation)
{
    TextFormatter f(20);
    auto err = f.parseWords("Привет мир");
    EXPECT_EQ(err.code, ErrorCode::noError);
    ASSERT_EQ(f.words.size(), 2u);
    EXPECT_EQ(f.words[0], "Привет");
    EXPECT_EQ(f.words[1], "мир");
}

// Тест 2: Знак препинания стоит отдельно — приклеивается к предыдущему слову
TEST(ParseWordsTest, PunctuationSeparateAttachesToPreviousWord)
{
    TextFormatter f(20);
    auto err = f.parseWords("слово . следующее");
    EXPECT_EQ(err.code, ErrorCode::noError);
    ASSERT_EQ(f.words.size(), 2u);
    EXPECT_EQ(f.words[0], "слово.");
    EXPECT_EQ(f.words[1], "следующее");
}

// Тест 3: Знак препинания вплотную к слову
TEST(ParseWordsTest, PunctuationAdjacentToWord)
{
    TextFormatter f(20);
    auto err = f.parseWords("слово, следующее");
    EXPECT_EQ(err.code, ErrorCode::noError);
    ASSERT_EQ(f.words.size(), 2u);
    EXPECT_EQ(f.words[0], "слово,");
    EXPECT_EQ(f.words[1], "следующее");
}

// Тест 4: Пустой входной текст
TEST(ParseWordsTest, EmptyInput)
{
    TextFormatter f(20);
    auto err = f.parseWords("");
    EXPECT_EQ(err.code, ErrorCode::noError);
    EXPECT_TRUE(f.words.empty());
}

// Тест 5: Недопустимый символ @
TEST(ParseWordsTest, InvalidCharacterAtSign)
{
    TextFormatter f(20);
    auto err = f.parseWords("Привет @мир");
    EXPECT_EQ(err.code, ErrorCode::invalidCharacter);
    EXPECT_EQ(err.lineNumber, 1);
    EXPECT_EQ(err.charPosition, 8);
    EXPECT_EQ(err.invalidChar, '@');
}

// Тест 6: Недопустимый символ не в первой строке
TEST(ParseWordsTest, InvalidCharacterInSecondLine)
{
    TextFormatter f(20);
    auto err = f.parseWords("Строка один\nСтрока #два");
    EXPECT_EQ(err.code, ErrorCode::invalidCharacter);
    EXPECT_EQ(err.lineNumber, 2);
    EXPECT_EQ(err.charPosition, 8);
    EXPECT_EQ(err.invalidChar, '#');
}

// Тест 7: Строка входного файла превышает 1024 символа
TEST(ParseWordsTest, LineTooLong)
{
    TextFormatter f(50);
    std::string longLine(1025, 'a');
    auto err = f.parseWords(longLine);
    EXPECT_EQ(err.code, ErrorCode::inputLineTooLong);
}

// Тест 8: Слово длиннее заданной ширины строки
TEST(ParseWordsTest, WordTooLong)
{
    TextFormatter f(10);
    auto err = f.parseWords("ОченьДлинноеСловоДляТеста");
    EXPECT_EQ(err.code, ErrorCode::wordTooLong);
}

// Тест 9: Несколько пробелов подряд — не создают пустых слов
TEST(ParseWordsTest, MultipleSpacesNoEmptyWords)
{
    TextFormatter f(20);
    auto err = f.parseWords("раз   два");
    EXPECT_EQ(err.code, ErrorCode::noError);
    ASSERT_EQ(f.words.size(), 2u);
    EXPECT_EQ(f.words[0], "раз");
    EXPECT_EQ(f.words[1], "два");
}

// Тест 10: Текст из одного слова
TEST(ParseWordsTest, SingleWord)
{
    TextFormatter f(20);
    auto err = f.parseWords("Привет");
    EXPECT_EQ(err.code, ErrorCode::noError);
    ASSERT_EQ(f.words.size(), 1u);
    EXPECT_EQ(f.words[0], "Привет");
}

// Тест 11: Слово ровно равно ширине строки — не ошибка
TEST(ParseWordsTest, WordExactlyLineWidth)
{
    TextFormatter f(5);
    auto err = f.parseWords("Abcde");
    EXPECT_EQ(err.code, ErrorCode::noError);
    ASSERT_EQ(f.words.size(), 1u);
}

// Тест 12: Слово на 1 символ длиннее ширины — ошибка
TEST(ParseWordsTest, WordOneCharLongerThanWidth)
{
    TextFormatter f(5);
    auto err = f.parseWords("Abcdef");
    EXPECT_EQ(err.code, ErrorCode::wordTooLong);
}

// Тест 13: Только пробелы и табуляции — пустой список слов
TEST(ParseWordsTest, OnlyWhitespace)
{
    TextFormatter f(20);
    auto err = f.parseWords("   \t   ");
    EXPECT_EQ(err.code, ErrorCode::noError);
    EXPECT_TRUE(f.words.empty());
}

// Тест 14: Несколько знаков препинания подряд
TEST(ParseWordsTest, MultiplePunctuationMarks)
{
    TextFormatter f(20);
    auto err = f.parseWords("слово . , следующее");
    EXPECT_EQ(err.code, ErrorCode::noError);
    ASSERT_EQ(f.words.size(), 2u);
    EXPECT_EQ(f.words[0], "слово.,");
    EXPECT_EQ(f.words[1], "следующее");
}

// Тест 15: Строка ровно 1024 символа — допустима
TEST(ParseWordsTest, LineExactly1024Chars)
{
    TextFormatter f(50);
    std::string line;
    for (int i = 0; i < 341; ++i) line += "ab ";
    line += "x";
    auto err = f.parseWords(line);
    EXPECT_EQ(err.code, ErrorCode::noError);
}

// Тест 16: Табуляция как разделитель
TEST(ParseWordsTest, TabAsDelimiter)
{
    TextFormatter f(20);
    auto err = f.parseWords("раз\tдва\tтри");
    EXPECT_EQ(err.code, ErrorCode::noError);
    ASSERT_EQ(f.words.size(), 3u);
    EXPECT_EQ(f.words[0], "раз");
    EXPECT_EQ(f.words[1], "два");
    EXPECT_EQ(f.words[2], "три");
}

// Тест 17: Недопустимый символ в начале первого слова
TEST(ParseWordsTest, InvalidCharacterAtStart)
{
    TextFormatter f(20);
    auto err = f.parseWords("@начало текста");
    EXPECT_EQ(err.code, ErrorCode::invalidCharacter);
    EXPECT_EQ(err.lineNumber, 1);
    EXPECT_EQ(err.charPosition, 1);
    EXPECT_EQ(err.invalidChar, '@');
}

// Тест 18: Недопустимый символ в конце последней строки
TEST(ParseWordsTest, InvalidCharacterAtEnd)
{
    TextFormatter f(20);
    auto err = f.parseWords("нормальный текст$");
    EXPECT_EQ(err.code, ErrorCode::invalidCharacter);
    EXPECT_EQ(err.lineNumber, 1);
    EXPECT_EQ(err.charPosition, 17);
    EXPECT_EQ(err.invalidChar, '$');
}

// Тест 19: Несколько абзацев (пустые строки между ними)
TEST(ParseWordsTest, MultipleParagraphsWithEmptyLines)
{
    TextFormatter f(20);
    auto err = f.parseWords("раз два\n\nтри четыре");
    EXPECT_EQ(err.code, ErrorCode::noError);
    ASSERT_EQ(f.words.size(), 4u);
    EXPECT_EQ(f.words[0], "раз");
    EXPECT_EQ(f.words[1], "два");
    EXPECT_EQ(f.words[2], "три");
    EXPECT_EQ(f.words[3], "четыре");
}

// ═══════════════════════════════════════════════════════════════════════════
//  ТАБЛИЦА 2 — TextFormatter::buildLines
// ═══════════════════════════════════════════════════════════════════════════

// Вспомогательная функция: инициализирует words и запускает buildLines
static std::vector<std::string> runBuildLines(const std::vector<std::string>& words,
                                              int width) {
    TextFormatter f(width);
    f.words = words;
    f.buildLines();
    return f.getOutputLines();
}

// Тест 1: Все слова в одну строку
TEST(BuildLinesTest, AllWordsInOneLine)
{
    auto out = runBuildLines({"раз", "два"}, 10);
    ASSERT_EQ(out.size(), 1u);
    EXPECT_EQ(out[0], "раз два");
}

// Тест 2: Слова распределяются по нескольким строкам
TEST(BuildLinesTest, MultipleLines)
{
    auto out = runBuildLines({"раз","два","три","четыре","пять"}, 12);
    ASSERT_EQ(out.size(), 2u);
    EXPECT_EQ(out[1], "четыре пять");
}

// Тест 3: Одно слово точно равно ширине
TEST(BuildLinesTest, SingleWordExactWidth)
{
    auto out = runBuildLines({"ровно"}, 5);
    ASSERT_EQ(out.size(), 1u);
    EXPECT_EQ(out[0], "ровно");
}

// Тест 4: Каждое слово в отдельной строке
TEST(BuildLinesTest, EachWordOnSeparateLine)
{
    auto out = runBuildLines({"длинное", "слово"}, 8);
    ASSERT_EQ(out.size(), 2u);
    EXPECT_EQ(out[0], "длинное");
    EXPECT_EQ(out[1], "слово");
}

// Тест 5: Пустой список слов
TEST(BuildLinesTest, EmptyWordList)
{
    auto out = runBuildLines({}, 40);
    EXPECT_TRUE(out.empty());
}

// Тест 6: Одно слово в списке
TEST(BuildLinesTest, SingleWordInList)
{
    auto out = runBuildLines({"одиночка"}, 40);
    ASSERT_EQ(out.size(), 1u);
    EXPECT_EQ(out[0], "одиночка");
}

// Тест 7: Сумма длин + пробелы = lineWidth точно
TEST(BuildLinesTest, WordsExactlyFit)
{
    auto out = runBuildLines({"аб", "вг", "де"}, 8);
    ASSERT_EQ(out.size(), 1u);
    EXPECT_EQ(out[0], "аб вг де");
}

// Тест 8: Следующее слово не влезает ровно на 1 символ
TEST(BuildLinesTest, NextWordDoesNotFit)
{
    auto out = runBuildLines({"аб", "вг", "де", "жз"}, 8);
    ASSERT_EQ(out.size(), 2u);
    EXPECT_EQ(out[1], "жз");
}

// Тест 9: Каждое слово = lineWidth — каждое в отдельной строке
TEST(BuildLinesTest, EachWordEqualsLineWidth)
{
    auto out = runBuildLines({"ааааа", "ббббб", "ввввв"}, 5);
    ASSERT_EQ(out.size(), 3u);
    EXPECT_EQ(out[0], "ааааа");
    EXPECT_EQ(out[1], "ббббб");
    EXPECT_EQ(out[2], "ввввв");
}

// Тест 10: Много коротких слов — все в одну строку
TEST(BuildLinesTest, ManyShortWordsOneLine)
{
    auto out = runBuildLines({"а","б","в","г","д","е","ж","з","и","й"}, 20);
    ASSERT_EQ(out.size(), 1u);
    EXPECT_EQ(out[0], "а б в г д е ж з и й");
}

// Тест 11: Длинный текст — несколько строк, последняя неполная
TEST(BuildLinesTest, LongTextMultipleLines)
{
    auto out = runBuildLines(
        {"один","два","три","четыре","пять","шесть","семь"}, 11);
    ASSERT_EQ(out.size(), 4u);
    EXPECT_EQ(out[3], "семь");
}

// Тест 12: Два слова, сумма длин = lineWidth, но нужен пробел, не влезают
TEST(BuildLinesTest, TwoWordsNeedSpaceButNotFit)
{
    auto out = runBuildLines({"привет", "мир"}, 9);
    ASSERT_EQ(out.size(), 2u);
    EXPECT_EQ(out[0], "привет");
    EXPECT_EQ(out[1], "мир");
}

// Тест 13: Слово со знаком препинания — граница строки смещается
TEST(BuildLinesTest, WordWithPunctuationAffectsBoundary)
{
    auto out = runBuildLines({"Привет,", "мир", "всем"}, 12);
    ASSERT_EQ(out.size(), 2u);
    EXPECT_EQ(out[1], "всем");
}

// Тест 14: Одно длинное слово = lineWidth
TEST(BuildLinesTest, OneLongWordEqualsLineWidth)
{
    auto out = runBuildLines({"аааааааааа"}, 10);
    ASSERT_EQ(out.size(), 1u);
    EXPECT_EQ(out[0], "аааааааааа");
}

// Тест 15: Чередование длинных и коротких слов
TEST(BuildLinesTest, AlternatingLongAndShortWords)
{
    auto out = runBuildLines({"длинноеслово", "а", "длинноеслово", "б"}, 14);
    ASSERT_EQ(out.size(), 2u);
    EXPECT_EQ(out[1], "длинноеслово б");
}

// Тест 16: Каждое слово = lineWidth = 3, все отдельно
TEST(BuildLinesTest, AllWordsSeparateLineWidth3)
{
    auto out = runBuildLines({"раз", "два", "три"}, 3);
    ASSERT_EQ(out.size(), 3u);
    EXPECT_EQ(out[0], "раз");
    EXPECT_EQ(out[1], "два");
    EXPECT_EQ(out[2], "три");
}

// Тест 17: Последняя строка — жадная упаковка
TEST(BuildLinesTest, LastLineGreedyPack)
{
    auto out = runBuildLines({"аб","вг","де","жз","ий"}, 8);
    ASSERT_EQ(out.size(), 2u);
    EXPECT_EQ(out[1], "жз ий");
}

// ═══════════════════════════════════════════════════════════════════════════
//  ТАБЛИЦА 3 — TextFormatter::justifyLine
// ═══════════════════════════════════════════════════════════════════════════

// Вспомогательная функция для тестирования justifyLine
static std::string runJustify(const std::vector<std::string>& words,
                              int width, bool isLast) {
    TextFormatter f(width);
    return f.justifyLine(words, isLast);
}

// Вспомогательная функция для проверки длины строки в кодовых точках
static int utf8Length(const std::string& s) {
    int count = 0;
    for (unsigned char c : s)
        if ((c & 0xC0) != 0x80) ++count;
    return count;
}

// Тест 1: Равномерное распределение, остаток != 0
TEST(JustifyLineTest, UnevenDistribution)
{
    auto res = runJustify({"Привет","мир","всем"}, 20, false);
    EXPECT_EQ(utf8Length(res), 20);
    EXPECT_EQ(res, "Привет    мир   всем");
}

// Тест 2: Пробелы делятся поровну (остаток = 0)
TEST(JustifyLineTest, EvenDistribution)
{
    auto res = runJustify({"Раз","два","три"}, 15, false);
    EXPECT_EQ(utf8Length(res), 15);
    EXPECT_EQ(res, "Раз   два   три");
}

// Тест 3: Первый промежуток +1 (extra=1)
TEST(JustifyLineTest, FirstGapGetsExtra)
{
    auto res = runJustify({"один","два","три"}, 15, false);
    EXPECT_EQ(utf8Length(res), 15);
    EXPECT_EQ(res, "один   два  три");
}

// Тест 4: Последняя строка — без выравнивания
TEST(JustifyLineTest, LastLineNoJustify)
{
    auto res = runJustify({"Конец","абзаца"}, 20, true);
    EXPECT_EQ(res, "Конец абзаца");
}

// Тест 5: Одно слово — промежутков нет
TEST(JustifyLineTest, SingleWordNoGaps)
{
    auto res = runJustify({"Слово"}, 20, false);
    EXPECT_EQ(res, "Слово");
}

// Тест 6: Два слова — один промежуток получает все пробелы
TEST(JustifyLineTest, TwoWordsSingleGapGetsAllSpaces)
{
    auto res = runJustify({"Привет","мир"}, 15, false);
    EXPECT_EQ(utf8Length(res), 15);
    EXPECT_EQ(res, "Привет      мир");
}

// Тест 7: Два слова, isLast=true — один пробел
TEST(JustifyLineTest, TwoWordsLastLineOneSpace)
{
    auto res = runJustify({"раз","два"}, 40, true);
    EXPECT_EQ(res, "раз два");
}

// Тест 8: Три слова с extra=1
TEST(JustifyLineTest, ThreeWordsWithExtra)
{
    auto res = runJustify({"а","б","в"}, 10, false);
    EXPECT_EQ(utf8Length(res), 10);
    EXPECT_EQ(res, "а    б   в");
}

// Тест 9: Четыре слова, extra=0
TEST(JustifyLineTest, FourWordsNoExtra)
{
    auto res = runJustify({"раз","два","три","та"}, 20, false);
    EXPECT_EQ(utf8Length(res), 20);
    EXPECT_EQ(res, "раз   два   три   та");
}

// Тест 10: Четыре слова, все промежутки по 3
TEST(JustifyLineTest, FourWordsAllGapsThree)
{
    auto res = runJustify({"а","б","в","г"}, 13, false);
    EXPECT_EQ(utf8Length(res), 13);
    EXPECT_EQ(res, "а   б   в   г");
}

// Тест 11: Слово со знаком препинания
TEST(JustifyLineTest, WordWithPunctuation)
{
    auto res = runJustify({"Привет,","мир"}, 15, false);
    EXPECT_EQ(utf8Length(res), 15);
    EXPECT_EQ(res, "Привет,     мир");
}

// Тест 12: Ровно 1 пробел между словами
TEST(JustifyLineTest, ExactlyOneSpaceBetweenWords)
{
    auto res = runJustify({"абв","где"}, 7, false);
    EXPECT_EQ(utf8Length(res), 7);
    EXPECT_EQ(res, "абв где");
}

// Тест 13: Одно слово = ширине строки
TEST(JustifyLineTest, SingleWordEqualsWidth)
{
    auto res = runJustify({"абвгдеж"}, 7, false);
    EXPECT_EQ(res, "абвгдеж");
}

// Тест 14: isLast=false, одно слово
TEST(JustifyLineTest, SingleWordNotLastLine)
{
    auto res = runJustify({"единственное"}, 40, false);
    EXPECT_EQ(res, "единственное");
}

// Тест 15: Пять слов, extra=3
TEST(JustifyLineTest, FiveWordsExtraThree)
{
    auto res = runJustify({"а","б","в","г","д"}, 20, false);
    EXPECT_EQ(utf8Length(res), 20);
    EXPECT_EQ(res, "а    б    в    г   д");
}

// Тест 16: Слова разной длины, extra=0
TEST(JustifyLineTest, DifferentLengthWordsNoExtra)
{
    auto res = runJustify({"один","два","три","четыре"}, 25, false);
    EXPECT_EQ(utf8Length(res), 25);
    EXPECT_EQ(res, "один   два   три   четыре");
}

// Тест 17: Два слова, isLast=false, lineWidth=40 — 38 пробелов между ними
TEST(JustifyLineTest, TwoWordsManySpaces)
{
    auto res = runJustify({"а","б"}, 40, false);
    EXPECT_EQ(utf8Length(res), 40);
    
    // Проверяем количество пробелов между словами
    std::string aBytes = "а";
    std::string bBytes = "б";
    size_t aEnd = aBytes.size();
    size_t bStart = res.size() - bBytes.size();
    int spaces = static_cast<int>(bStart - aEnd);
    EXPECT_EQ(spaces, 38);
}

// Тест 18: isLast=true — один пробел, нет выравнивания
TEST(JustifyLineTest, LastLineWithPunctuation)
{
    auto res = runJustify({"Конец,","всё"}, 30, true);
    EXPECT_EQ(res, "Конец, всё");
}

// ═══════════════════════════════════════════════════════════════════════════
//  main
// ═══════════════════════════════════════════════════════════════════════════

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}