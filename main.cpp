/**
 * @file main.cpp
 * @brief Основной файл для приложения Cloud Storage Bot.
 */

#include <tgbot/tgbot.h>
#include <iostream>
#include <memory>
#include <random>
#include <filesystem>
#include <thread>
#include <fstream>
#include <sstream>
#include <httplib.h>

#define SQLITECPP_COMPILE_DLL
#include <SQLiteCpp/SQLiteCpp.h>

namespace fs = std::filesystem;

/**
 * @brief Инициализация базы данных.
 * 
 * Эта функция создаёт SQLite базу данных и таблицу, если она не существует.
 * 
 * @param dbFileName Имя файла базы данных.
 */
void initDatabase(const std::string& dbFileName) {
    try {
        SQLite::Database db(dbFileName, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        db.exec("CREATE TABLE IF NOT EXISTS users ("
                "id INTEGER PRIMARY KEY, "
                "token TEXT);");

        std::cout << "База данных успешно инициализирована." << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка при инициализации базы данных: " << e.what() << std::endl;
    }
}

/**
 * @brief Добавление пользователя в базу данных.
 * 
 * Эта функция вставляет нового пользователя в базу данных с начальным значением NULL для токена.
 * 
 * @param dbFileName Имя файла базы данных.
 * @param userId Идентификатор пользователя.
 */
void addUserToDatabase(const std::string& dbFileName, int64_t userId) {
    try {
        SQLite::Database db(dbFileName, SQLite::OPEN_READWRITE);
        SQLite::Statement query(db, "INSERT OR IGNORE INTO users (id, token) VALUES (?, ?)");
        query.bind(1, userId);
        query.bind(2, nullptr);  // Изначально токен NULL
        query.exec();

        std::cout << "Пользователь успешно добавлен в базу данных." << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка при добавлении пользователя в базу данных: " << e.what() << std::endl;
    }
}

/**
 * @brief Генерация токена.
 * 
 * Эта функция генерирует случайный токен, состоящий из букв и цифр.
 * 
 * @return Сгенерированный токен.
 */
std::string generateToken() {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    const size_t maxIndex = (sizeof(charset) - 1);
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, maxIndex);

    std::string token;
    for (size_t i = 0; i < 18; ++i) {
        token += charset[distribution(generator)];
    }

    return token;
}

/**
 * @brief Обновление токена пользователя.
 * 
 * Эта функция обновляет токен пользователя в базе данных.
 * 
 * @param dbFileName Имя файла базы данных.
 * @param userId Идентификатор пользователя.
 * @param token Новый токен.
 */
void updateUserToken(const std::string& dbFileName, int64_t userId, const std::string& token) {
    try {
        SQLite::Database db(dbFileName, SQLite::OPEN_READWRITE);
        SQLite::Statement query(db, "UPDATE users SET token = ? WHERE id = ?");
        query.bind(1, token);
        query.bind(2, userId);
        query.exec();

        std::cout << "Токен пользователя успешно обновлён." << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка при обновлении токена пользователя: " << e.what() << std::endl;
    }
}

/**
 * @brief Получение токена пользователя.
 * 
 * Эта функция извлекает токен пользователя из базы данных.
 * 
 * @param dbFileName Имя файла базы данных.
 * @param userId Идентификатор пользователя.
 * @return Токен пользователя.
 */
std::string getUserToken(const std::string& dbFileName, int64_t userId) {
    try {
        SQLite::Database db(dbFileName, SQLite::OPEN_READONLY);
        SQLite::Statement query(db, "SELECT token FROM users WHERE id = ?");
        query.bind(1, userId);

        if (query.executeStep()) {
            return query.getColumn(0).getText();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка при получении токена пользователя: " << e.what() << std::endl;
    }
    return "";
}

/**
 * @brief Создание папки для пользователя.
 * 
 * Эта функция создаёт директорию для хранения файлов пользователя.
 * 
 * @param token Токен пользователя.
 */
void createFolderForUser(const std::string& token) {
    fs::create_directories("files/" + token);
    std::cout << "Папка создана для пользователя с токеном: " << token << std::endl;
}

/**
 * @brief Получение списка файлов.
 * 
 * Эта функция возвращает список файлов в указанной директории.
 * 
 * @param folder_path Путь к директории.
 * @return Вектор с именами файлов.
 */
std::vector<std::string> get_files(const std::string& folder_path) {
    std::vector<std::string> file_list;
    for (const auto& entry : fs::directory_iterator(folder_path)) {
        file_list.push_back(entry.path().filename().string());
    }
    return file_list;
}

/**
 * @brief Проверка токена.
 * 
 * Эта функция проверяет существование директории для данного токена.
 * 
 * @param token Токен пользователя.
 * @return true, если директория существует и является директорией.
 * @return false, если директория не существует или не является директорией.
 */
bool validate_token(const std::string& token) {
    std::string token_folder = "files/" + token;
    return fs::exists(token_folder) && fs::is_directory(token_folder);
}

/**
 * @brief Генерация HTML-списка файлов.
 * 
 * Эта функция создаёт HTML-код для отображения списка файлов.
 * 
 * @param files Вектор с именами файлов.
 * @param token Токен пользователя.
 * @return Строка с HTML-кодом списка файлов.
 */
std::string generate_file_list_html(const std::vector<std::string>& files, const std::string& token) {
    std::stringstream ss;
    for (const auto& file : files) {
        ss << "<li><a href=\"/download/" << token << "/" << file << "\">" << file << "</a></li>\n";
    }
    return ss.str();
}

/**
 * @brief Запуск сервера.
 * 
 * Эта функция запускает HTTP-сервер, который обрабатывает запросы на загрузку и скачивание файлов.
 */
void startServer() {
    httplib::Server svr;

    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        std::ifstream file("website.html");
        std::stringstream buffer;
        buffer << file.rdbuf();
        res.set_content(buffer.str(), "text/html");
    });

    svr.Get(R"(/files/(.*))", [](const httplib::Request& req, httplib::Response& res) {
        std::string token = req.matches[1].str();
        if (validate_token(token)) {
            auto files = get_files("files/" + token);
            std::string file_list_html = generate_file_list_html(files, token);
            res.set_content(file_list_html, "text/html");
        }
        else {
            res.set_content("Invalid token.", "text/plain");
        }
    });

    svr.Post(R"(/upload/(.*))", [](const httplib::Request& req, httplib::Response& res) {
        std::string token = req.matches[1].str();
        auto file = req.get_file_value("file");
        std::string file_path = "files/" + token + "/" + file.filename;

        std::ofstream ofs(file_path, std::ios::binary);
        ofs.write(file.content.data(), file.content.size());

        res.set_content("File uploaded successfully.", "text/plain");
    });

    svr.Get(R"(/download/(.*)/(.*))", [](const httplib::Request& req, httplib::Response& res) {
        std::string token = req.matches[1].str();
        std::string file_name = req.matches[2].str();
        std::string file_path = "files/" + token + "/" + file_name;

        std::ifstream file(file_path, std::ios::binary);
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            res.set_content(buffer.str(), "application/octet-stream");
            res.set_header("Content-Disposition", "attachment; filename=\"" + file_name + "\"");
        }
        else {
            res.status = 404;
            res.set_content("File not found.", "text/plain");
        }
    });

    svr.listen("0.0.0.0", 8080);
}

int main() {
    std::thread serverThread(startServer);

    TgBot::Bot bot("YOUR_BOT_TOKEN");

    initDatabase("cloud_storage.db");

    bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) {
        addUserToDatabase("cloud_storage.db", message->chat->id);

        std::string token = getUserToken("cloud_storage.db", message->chat->id);
        std::string responseMessage = "Добро пожаловать в Cloud Storage Bot! Здесь вы можете загружать и управлять вашими файлами.";
        if (!token.empty()) {
            responseMessage += "\n\nВаш текущий токен: " + token;
        }
        else {
            responseMessage += "\n\nУ вас пока нет токена. Пожалуйста, сгенерируйте его.";
        }

        TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup);
        std::vector<TgBot::InlineKeyboardButton::Ptr> row;

        TgBot::InlineKeyboardButton::Ptr tokenButton(new TgBot::InlineKeyboardButton);
        tokenButton->text = "Токен";
        tokenButton->callbackData = "token";
        row.push_back(tokenButton);

        TgBot::InlineKeyboardButton::Ptr uploadButton(new TgBot::InlineKeyboardButton);
        uploadButton->text = "Загрузка";
        uploadButton->callbackData = "upload";
        row.push_back(uploadButton);

        keyboard->inlineKeyboard.push_back(row);

        bot.getApi().sendMessage(message->chat->id, responseMessage, false, 0, keyboard);
    });

    bot.getEvents().onCallbackQuery([&bot](TgBot::CallbackQuery::Ptr query) {
        if (query->data == "token") {
            TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup);
            std::vector<TgBot::InlineKeyboardButton::Ptr> rowYesNo;

            TgBot::InlineKeyboardButton::Ptr yesButton(new TgBot::InlineKeyboardButton);
            yesButton->text = "Да";
            yesButton->callbackData = "confirm_yes";
            rowYesNo.push_back(yesButton);

            TgBot::InlineKeyboardButton::Ptr noButton(new TgBot::InlineKeyboardButton);
            noButton->text = "Нет";
            noButton->callbackData = "confirm_no";
            rowYesNo.push_back(noButton);

            keyboard->inlineKeyboard.push_back(rowYesNo);

            bot.getApi().sendMessage(query->message->chat->id, "Вы уверены, что хотите сгенерировать новый токен?", false, 0, keyboard);
        }
        else if (query->data == "confirm_yes") {
            std::string newToken = generateToken();
            updateUserToken("cloud_storage.db", query->message->chat->id, newToken);
            bot.getApi().sendMessage(query->message->chat->id, "Ваш новый токен: " + newToken);
        }
        else if (query->data == "confirm_no") {
            bot.getApi().sendMessage(query->message->chat->id, "Генерация токена отменена. Возвращение в меню.");
        }
        else if (query->data == "upload") {
            std::string token = getUserToken("cloud_storage.db", query->message->chat->id);
            if (token.empty()) {
                bot.getApi().sendMessage(query->message->chat->id, "У вас пока нет токена. Пожалуйста, сгенерируйте его.");
            }
            else {
                createFolderForUser(token);
                bot.getApi().sendMessage(query->message->chat->id, "Папка создана для вашего токена. Пожалуйста, посетите http://localhost:8080 для загрузки ваших файлов.");
            }
        }
        bot.getApi().answerCallbackQuery(query->id);
    });

    TgBot::TgLongPoll longPoll(bot);

    std::cout << "Бот запущен..." << std::endl;
    while (true) {
        try {
            longPoll.start();
        }
        catch (TgBot::TgException& e) {
            std::cerr << "Ошибка: " << e.what() << std::endl;
        }
    }

    serverThread.join();
    return 0;
}
