@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

rem ── Путь к исполняемому файлу ─────────────────────────────────────────────
set "SCRIPT_DIR=%~dp0"
if "%~1"=="" (
    set "APP=%SCRIPT_DIR%..\build\src\app.exe"
) else (
    set "APP=%~1"
)

if not exist "%APP%" (
    echo Ошибка: исполняемый файл не найден: %APP%
    echo Соберите проект: cmake --build build
    exit /b 1
)

rem ── Временная директория ──────────────────────────────────────────────────
set "TMP=%TEMP%\formatter_tests_%RANDOM%"
mkdir "%TMP%"

rem ── Счётчики ──────────────────────────────────────────────────────────────
set /a PASS=0
set /a FAIL=0
set /a TOTAL=0

goto :main

rem =============================================================================
rem  :run_test id desc width expected_exit expected_first_line
rem  Входной файл: %TMP%\in_<id>.txt  Выходной файл: %TMP%\out_<id>.txt
rem =============================================================================
:run_test
    set "T_ID=%~1"
    set "T_DESC=%~2"
    set "T_WIDTH=%~3"
    set "T_EXP_EXIT=%~4"
    set "T_EXP_LINE=%~5"
    set /a TOTAL+=1

    set "IN_FILE=%TMP%\in_%T_ID%.txt"
    set "OUT_FILE=%TMP%\out_%T_ID%.txt"
    if exist "%OUT_FILE%" del "%OUT_FILE%"

    "%APP%" "%IN_FILE%" "%T_WIDTH%" "%OUT_FILE%" >nul 2>&1
    set "T_ACTUAL_EXIT=%ERRORLEVEL%"

    set "T_ACTUAL_LINE="
    if exist "%OUT_FILE%" (
        set /p T_ACTUAL_LINE=<"%OUT_FILE%"
    )

    if "%T_ACTUAL_EXIT%"=="%T_EXP_EXIT%" (
        if "!T_ACTUAL_LINE!"=="%T_EXP_LINE%" (
            echo   PASS  [%T_ID%] %T_DESC%
            set /a PASS+=1
            goto :eof
        )
    )

    echo   FAIL  [%T_ID%] %T_DESC%
    if not "%T_ACTUAL_EXIT%"=="%T_EXP_EXIT%" (
        echo          exit: ожидался=%T_EXP_EXIT% получен=%T_ACTUAL_EXIT%
    )
    if not "!T_ACTUAL_LINE!"=="%T_EXP_LINE%" (
        echo          первая строка: ожидалась='%T_EXP_LINE%' получена='!T_ACTUAL_LINE!'
    )
    set /a FAIL+=1
    goto :eof

rem =============================================================================
rem  :run_test_exit1 id desc width
rem  Ожидаем код выхода 1 (ошибка), содержимое не проверяем
rem =============================================================================
:run_test_exit1
    set "T_ID=%~1"
    set "T_DESC=%~2"
    set "T_WIDTH=%~3"
    set /a TOTAL+=1

    set "IN_FILE=%TMP%\in_%T_ID%.txt"
    set "OUT_FILE=%TMP%\out_%T_ID%.txt"
    if exist "%OUT_FILE%" del "%OUT_FILE%"

    "%APP%" "%IN_FILE%" "%T_WIDTH%" "%OUT_FILE%" >nul 2>&1
    set "T_ACTUAL_EXIT=%ERRORLEVEL%"

    if "%T_ACTUAL_EXIT%"=="1" (
        echo   PASS  [%T_ID%] %T_DESC%
        set /a PASS+=1
    ) else (
        echo   FAIL  [%T_ID%] %T_DESC%
        echo          exit: ожидался=1 получен=%T_ACTUAL_EXIT%
        set /a FAIL+=1
    )
    goto :eof

rem =============================================================================
rem  :run_test_no_input id desc args...
rem  Запускает app с произвольными аргументами (без заранее созданного файла)
rem  Ожидаем код выхода 1
rem =============================================================================
:run_test_no_input
    set "T_ID=%~1"
    set "T_DESC=%~2"
    set "T_ARGS=%~3"
    set /a TOTAL+=1

    "%APP%" %T_ARGS% >nul 2>&1
    set "T_ACTUAL_EXIT=%ERRORLEVEL%"

    if "%T_ACTUAL_EXIT%"=="1" (
        echo   PASS  [%T_ID%] %T_DESC%
        set /a PASS+=1
    ) else (
        echo   FAIL  [%T_ID%] %T_DESC%
        echo          exit: ожидался=1 получен=%T_ACTUAL_EXIT%
        set /a FAIL+=1
    )
    goto :eof

rem =============================================================================
rem  :write_ps id "строка1\nстрока2\n..."
rem  Записывает многострочный файл через PowerShell
rem =============================================================================
:write_ps
    set "WP_ID=%~1"
    set "WP_CONTENT=%~2"
    powershell -NoProfile -Command ^
        "[System.IO.File]::WriteAllText('%TMP%\in_%WP_ID%.txt', \"%WP_CONTENT%\".Replace('\n',\"`n\"), [System.Text.Encoding]::UTF8)"
    goto :eof

rem =============================================================================
:main
rem =============================================================================

echo.
echo ════════════════════════════════════════════════════
echo   Блок 1: Корректное форматирование — ширина 40
echo ════════════════════════════════════════════════════

call :write_ps 1 "Привет"
call :run_test 1 "Одно слово — без переноса" 40 0 "Привет"

call :write_ps 2 "Раз два три"
call :run_test 2 "Три коротких слова в одну строку" 40 0 "Раз два три"

call :write_ps 3 "один два три четыре пять шесть семь восемь"
call :run_test 3 "Длинный текст — первая строка по ширине 40" 40 0 "один  два три четыре пять шесть семь"

call :write_ps 4 "слово."
call :run_test 4 "Слово с точкой — пунктуация приклеена" 40 0 "слово."

call :write_ps 5 "слово , следующее"
call :run_test 5 "Запятая стоит отдельно — приклеивается к предыдущему слову" 40 0 "слово, следующее"

call :write_ps 6 "первое второе"
call :run_test 6 "Два слова — выравнивание по ширине 40 (isLast=true, без растяжки)" 40 0 "первое второе"

call :write_ps 7 "а б"
call :run_test 7 "Два однобуквенных слова — вывод без растяжки" 40 0 "а б"

call :write_ps 8 "Hello world"
call :run_test 8 "ASCII: два слова — без растяжки" 40 0 "Hello world"

echo.
echo ════════════════════════════════════════════════════
echo   Блок 2: Корректное форматирование — ширина 60
echo ════════════════════════════════════════════════════

call :write_ps 10 "Это простой текст для проверки форматирования по ширине шестьдесят символов."
call :run_test 10 "Абзац — ширина 60, первая строка" 60 0 "Это  простой  текст для проверки форматирования по ширине"

call :write_ps 11 "один два три четыре пять шесть семь восемь девять десять одиннадцать двенадцать"
call :run_test 11 "Много слов — ширина 60, первая строка" 60 0 "один  два три четыре пять шесть семь восемь девять десять"

call :write_ps 12 "Hello, world! This is a test of the text formatter."
call :run_test 12 "ASCII с пунктуацией — ширина 60, первая строка" 60 0 "Hello,  world!  This  is  a  test  of  the  text formatter."

call :write_ps 13 "слово"
call :run_test 13 "Одно слово — любая ширина, вывод без пробелов" 60 0 "слово"

call :write_ps 14 "первое второе третье"
call :run_test 14 "Три слова — одна строка, isLast=true, без растяжки" 60 0 "первое второе третье"

echo.
echo ════════════════════════════════════════════════════
echo   Блок 3: Корректное форматирование — ширина 80 и 120
echo ════════════════════════════════════════════════════

call :write_ps 20 "Программа форматирует текст"
call :run_test 20 "Ширина 80 — короткий текст без переноса" 80 0 "Программа форматирует текст"

call :write_ps 21 "один два три"
call :run_test 21 "Ширина 120 — три слова без переноса" 120 0 "один два три"

call :write_ps 22 "a b"
call :run_test 22 "Ширина 120 — два однобуквенных слова" 120 0 "a b"

echo.
echo ════════════════════════════════════════════════════
echo   Блок 4: Ошибки аргументов командной строки (код выхода 1)
echo ════════════════════════════════════════════════════

rem Тесты без входного файла — передаём неверное число аргументов напрямую
set /a TOTAL+=1
"%APP%" >nul 2>&1
if "%ERRORLEVEL%"=="1" (
    echo   PASS  [30] Нет аргументов
    set /a PASS+=1
) else (
    echo   FAIL  [30] Нет аргументов — ожидался exit=1, получен=%ERRORLEVEL%
    set /a FAIL+=1
)

set /a TOTAL+=1
"%APP%" only_one_arg >nul 2>&1
if "%ERRORLEVEL%"=="1" (
    echo   PASS  [31] Один аргумент
    set /a PASS+=1
) else (
    echo   FAIL  [31] Один аргумент — ожидался exit=1, получен=%ERRORLEVEL%
    set /a FAIL+=1
)

set /a TOTAL+=1
"%APP%" arg1 arg2 >nul 2>&1
if "%ERRORLEVEL%"=="1" (
    echo   PASS  [32] Два аргумента
    set /a PASS+=1
) else (
    echo   FAIL  [32] Два аргумента — ожидался exit=1, получен=%ERRORLEVEL%
    set /a FAIL+=1
)

set /a TOTAL+=1
"%APP%" arg1 arg2 arg3 arg4 >nul 2>&1
if "%ERRORLEVEL%"=="1" (
    echo   PASS  [33] Пять аргументов (слишком много)
    set /a PASS+=1
) else (
    echo   FAIL  [33] Пять аргументов — ожидался exit=1, получен=%ERRORLEVEL%
    set /a FAIL+=1
)

echo.
echo ════════════════════════════════════════════════════
echo   Блок 5: Ошибки ширины строки (код выхода 1)
echo ════════════════════════════════════════════════════

call :write_ps 40 "текст для теста"
call :run_test_exit1 40 "Ширина 39 (меньше минимума)" 39

call :write_ps 41 "текст для теста"
call :run_test_exit1 41 "Ширина 121 (больше максимума)" 121

call :write_ps 42 "текст для теста"
call :run_test_exit1 42 "Ширина 0" 0

call :write_ps 43 "текст для теста"
call :run_test_exit1 43 "Ширина -1 (отрицательная)" -1

call :write_ps 44 "текст для теста"
call :run_test_exit1 44 "Ширина — не число (abc)" abc

call :write_ps 45 "текст для теста"
call :run_test_exit1 45 "Ширина — дробное число (60.5)" 60.5

call :write_ps 46 "текст для теста"
call :run_test_exit1 46 "Ширина — число с плюсом (+60)" +60

call :write_ps 47 "текст для теста"
call :run_test_exit1 47 "Ширина — пустая строка" ""

rem Граничные значения должны ПРОХОДИТЬ
call :write_ps 48 "минимальная ширина"
call :run_test 48 "Ширина 40 (минимум допустимый) — успех" 40 0 "минимальная ширина"

call :write_ps 49 "максимальная ширина"
call :run_test 49 "Ширина 120 (максимум допустимый) — успех" 120 0 "максимальная ширина"

echo.
echo ════════════════════════════════════════════════════
echo   Блок 6: Ошибки входного файла
echo ════════════════════════════════════════════════════

rem Несуществующий файл — создавать не надо
set /a TOTAL+=1
"%APP%" "%TMP%\nonexistent_file.txt" 60 "%TMP%\out_50.txt" >nul 2>&1
if "%ERRORLEVEL%"=="1" (
    echo   PASS  [50] Входной файл не найден
    set /a PASS+=1
) else (
    echo   FAIL  [50] Входной файл не найден — ожидался exit=1, получен=%ERRORLEVEL%
    set /a FAIL+=1
)

call :write_ps 51 ""
call :run_test_exit1 51 "Пустой входной файл" 60

call :write_ps 52 "обычный текст @недопустимый символ"
call :run_test_exit1 52 "Недопустимый символ @ в тексте" 60

call :write_ps 53 "текст #символ решётки"
call :run_test_exit1 53 "Недопустимый символ # в тексте" 60

call :write_ps 54 "текст $символ доллара"
call :run_test_exit1 54 "Недопустимый символ $ в тексте" 60

call :write_ps 55 "текст ^символ крышечка"
call :run_test_exit1 55 "Недопустимый символ ^ в тексте" 60

call :write_ps 56 "текст &амперсанд"
call :run_test_exit1 56 "Недопустимый символ & в тексте" 60

call :write_ps 57 "текст ~тильда"
call :run_test_exit1 57 "Недопустимый символ ~ в тексте" 60

call :write_ps 58 "текст `обратная кавычка"
call :run_test_exit1 58 "Недопустимый символ ` в тексте" 60

call :write_ps 59 "текст |вертикальная черта"
call :run_test_exit1 59 "Недопустимый символ | в тексте" 60

echo.
echo ════════════════════════════════════════════════════
echo   Блок 7: Слишком длинное слово и строка
echo ════════════════════════════════════════════════════

call :write_ps 60 "ОченьДлинноеСловоКотороеНеВлезетВСтрокуШириной40символов"
call :run_test_exit1 60 "Слово длиннее ширины 40" 40

call :write_ps 61 "VeryLongWordThatWillNotFitIntoLineWidthOfFortyChars1234567"
call :run_test_exit1 61 "ASCII слово длиннее ширины 40" 40

rem Строка ровно 1024 символа — должна проходить
powershell -NoProfile -Command ^
    "[System.IO.File]::WriteAllText('%TMP%\in_62.txt', 'а' * 1024, [System.Text.Encoding]::UTF8)"
call :run_test 62 "Строка ровно 1024 символа — успех" 40 1 ""

rem Строка 1025 символов — должна завершаться с ошибкой
powershell -NoProfile -Command ^
    "[System.IO.File]::WriteAllText('%TMP%\in_63.txt', 'а' * 1025, [System.Text.Encoding]::UTF8)"
call :run_test_exit1 63 "Строка 1025 символов — ошибка inputLineTooLong" 40

echo.
echo ════════════════════════════════════════════════════
echo   Блок 8: Ошибка выходного файла
echo ════════════════════════════════════════════════════

call :write_ps 70 "нормальный текст"
set /a TOTAL+=1
set "OUT_FILE=%TMP%\out_70.txt"
if exist "%OUT_FILE%" del "%OUT_FILE%"
"%APP%" "%TMP%\in_70.txt" 60 "%TMP%\несуществующий_каталог\out.txt" >nul 2>&1
if "%ERRORLEVEL%"=="1" (
    echo   PASS  [70] Выходной файл в несуществующем каталоге
    set /a PASS+=1
) else (
    echo   FAIL  [70] Выходной файл в несуществующем каталоге — ожидался exit=1, получен=%ERRORLEVEL%
    set /a FAIL+=1
)

echo.
echo ════════════════════════════════════════════════════
echo   Блок 9: Пунктуация — корректная обработка
echo ════════════════════════════════════════════════════

call :write_ps 80 "Привет , мир !"
call :run_test 80 "Запятая и восклицательный стоят отдельно — приклеиваются" 40 0 "Привет, мир!"

call :write_ps 81 "слово."
call :run_test 81 "Точка вплотную к слову — не отделяется" 40 0 "слово."

call :write_ps 82 "текст ; продолжение"
call :run_test 82 "Точка с запятой отдельно — приклеивается" 40 0 "текст; продолжение"

call :write_ps 83 "вопрос ?"
call :run_test 83 "Знак вопроса отдельно — приклеивается" 40 0 "вопрос?"

call :write_ps 84 "слово : другое"
call :run_test 84 "Двоеточие отдельно — приклеивается" 40 0 "слово: другое"

echo.
echo ════════════════════════════════════════════════════
echo   Блок 10: Разрешённые спецсимволы
echo ════════════════════════════════════════════════════

call :write_ps 90 "цена 100 рублей"
call :run_test 90 "Цифры в тексте — допустимы" 40 0 "цена 100 рублей"

call :write_ps 91 "(скобки) допустимы"
call :run_test 91 "Скобки в тексте — допустимы" 40 0 "(скобки) допустимы"

call :write_ps 92 "дефис-слово допустимо"
call :run_test 92 "Дефис внутри слова — допустим" 40 0 "дефис-слово допустимо"

call :write_ps 93 "ширина 40-120 символов"
call :run_test 93 "Дефис с цифрами — допустим" 40 0 "ширина 40-120 символов"

echo.
echo ════════════════════════════════════════════════════
echo   Блок 11: Смешанный кириллица + ASCII
echo ════════════════════════════════════════════════════

call :write_ps 100 "Hello мир"
call :run_test 100 "ASCII + кириллица в одной строке" 40 0 "Hello мир"

call :write_ps 101 "version 2.0 — финальная"
call :run_test 101 "Версия с дефисом и точкой" 40 0 "version 2.0 — финальная"

call :write_ps 102 "C++ это язык программирования"
call :run_test 102 "Знак + в тексте — допустим" 40 0 "C++ это язык программирования"

echo.
echo ════════════════════════════════════════════════════
echo   Итог
echo ════════════════════════════════════════════════════

rmdir /s /q "%TMP%"

echo   Всего: %TOTAL%  ^|  Пройдено: %PASS%  ^|  Провалено: %FAIL%
echo.

if %FAIL% gtr 0 exit /b 1
exit /b 0
