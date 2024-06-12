/**
 * @file main.cpp
 * @brief Основной файл для приложения HSECloud Bot.
 *
 * Этот файл содержит реализацию бота для Telegram, который предоставляет
 * функциональность облачного хранилища для пользователей. Бот использует
 * библиотеку TgBot для взаимодействия с API Telegram, SQLiteCpp для работы с
 * базой данных SQLite, а также httplib для запуска HTTP-сервера.
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
#include <ctime>

#define SQLITECPP_COMPILE_DLL
#include <SQLiteCpp/SQLiteCpp.h>

namespace fs = std::filesystem;

/**
 * @brief Логирование сообщений.
 *
 * Эта функция записывает сообщения в файл журнала с отметкой времени.
 *
 * @param message Сообщение для записи в журнал.
 */

void logMessage(const std::string& message) {
    std::ofstream logFile("bot.log", std::ios_base::app);
    std::time_t now = std::time(nullptr);
    logFile << std::ctime(&now) << ": " << message << std::endl;
}

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
        logMessage("Database initialized successfully.");
    }
    catch (const std::exception& e) {
        logMessage("Error initializing database: " + std::string(e.what()));
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
        query.bind(2, nullptr);  // Initially, token is NULL
        query.exec();
        logMessage("User added to database successfully. UserID: " + std::to_string(userId));
    }
    catch (const std::exception& e) {
        logMessage("Error adding user to database: " + std::string(e.what()));
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
    logMessage("Generated token: " + token);
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
        logMessage("User token updated successfully. UserID: " + std::to_string(userId));
    }
    catch (const std::exception& e) {
        logMessage("Error updating user token: " + std::string(e.what()));
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
            std::string token = query.getColumn(0).getText();
            logMessage("Token retrieved for user. UserID: " + std::to_string(userId) + ", Token: " + token);
            return token;
        }
    }
    catch (const std::exception& e) {
        logMessage("Error getting user token: " + std::string(e.what()));
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
    try {
        std::string path = "C:/Users/Public/Music/CloudBot/main/files/" + token;
        fs::create_directories(path);
        logMessage("Folder created for user with token: " + token);
    }
    catch (const std::exception& e) {
        logMessage("Error creating folder for user: " + std::string(e.what()));
    }
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
    logMessage("Files retrieved from folder: " + folder_path);
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
    std::string token_folder = "C:/Users/Public/Music/CloudBot/main/files/" + token;
    bool isValid = fs::exists(token_folder) && fs::is_directory(token_folder);
    logMessage("Token validation: " + token + " is " + (isValid ? "valid" : "invalid"));
    return isValid;
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
    logMessage("HTML file list generated for token: " + token);
    return ss.str();
}

/**
 * @brief Генерация временного токена для отправки.
 *
 * Эта функция генерирует случайный токен фиксированной длины, состоящий из букв и цифр.
 *
 * @param length Длина токена.
 * @return Сгенерированный токен.
 */

std::string generate_send_token(size_t length) {
    const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::default_random_engine rng(std::random_device{}());
    std::uniform_int_distribution<> dist(0, sizeof(charset) - 2);

    std::string token;
    for (size_t i = 0; i < length; ++i) {
        token += charset[dist(rng)];
    }
    return token;
}

/**
 * @brief Обработка загрузки файла.
 *
 * Эта функция обрабатывает запросы на загрузку файлов, создаёт директорию для
 * временного токена и сохраняет загруженные файлы в эту директорию.
 *
 * @param req Запрос.
 * @param res Ответ.
 */


void handle_file_upload(const httplib::Request& req, httplib::Response& res) {
    std::string token = generate_send_token(12);
    std::string dir_path = "hidefiles/" + token;
    fs::create_directories(dir_path);

    std::vector<std::string> filenames;

    for (const auto& file : req.files) {
        auto& file_info = file.second;
        std::string file_path = dir_path + "/" + file_info.filename;
        std::ofstream ofs(file_path, std::ios::binary);
        ofs << file_info.content;
        ofs.close();
        filenames.push_back(file_info.filename);
    }

    std::string link = "http://localhost:8080/sendfile/" + token;
    std::string response = "{\"link\": \"" + link + "\", \"files\": [";
    for (size_t i = 0; i < filenames.size(); ++i) {
        response += "\"" + filenames[i] + "\"";
        if (i < filenames.size() - 1) response += ", ";
    }
    response += "]}";

    res.set_content(response, "application/json");
}

/**
 * @brief Обработка страницы загрузки файлов.
 *
 * Эта функция обрабатывает запросы на загрузку страницы со списком файлов для
 * временного токена.
 *
 * @param req Запрос.
 * @param res Ответ.
 */

void handle_file_download_page(const httplib::Request& req, httplib::Response& res) {
    std::string token = req.matches[1].str();
    std::string dir_path = "hidefiles/" + token;

    if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
        res.set_content("Files not found", "text/plain");
        res.status = 404;
        return;
    }

    std::string html = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>Download Files</title><style>body { font-family: Arial, sans-serif; background-color: #f4f4f4; margin: 0; padding: 0; display: flex; justify-content: center; align-items: center; height: 100vh; } .container { background: #fff; padding: 20px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); border-radius: 8px; text-align: center; width: 80%; max-width: 600px; } ul { list-style-type: none; padding: 0; } li { margin: 10px 0; background: #e9ecef; padding: 10px; border-radius: 4px; } a { text-decoration: none; color: #007BFF; } a:hover { text-decoration: underline; }</style></head><body><div class=\"container\"><h1>Download Files</h1><ul>";

    for (const auto& entry : fs::directory_iterator(dir_path)) {
        std::string filename = entry.path().filename().string();
        std::string file_link = "/sendfile/" + token + "/" + filename;
        html += "<li><a href=\"" + file_link + "\">" + filename + "</a></li>";
    }

    html += "</ul></div></body></html>";

    res.set_content(html, "text/html");
}

/**
 * @brief Обработка скачивания файлов.
 *
 * Эта функция обрабатывает запросы на скачивание файлов для временного токена.
 *
 * @param req Запрос.
 * @param res Ответ.
 */

void handle_file_download(const httplib::Request& req, httplib::Response& res) {
    std::string token = req.matches[1].str();
    std::string filename = req.matches[2].str();
    std::string file_path = "hidefiles/" + token + "/" + filename;

    if (fs::exists(file_path) && fs::is_regular_file(file_path)) {
        std::ifstream ifs(file_path, std::ios::binary);
        std::ostringstream oss;
        oss << ifs.rdbuf();
        res.set_content(oss.str(), "application/octet-stream");
        res.set_header("Content-Disposition", "attachment; filename=" + filename);
    }
    else {
        res.set_content("File not found", "text/plain");
        res.status = 404;
    }
}

/**
 * @brief Запуск HTTP-сервера.
 *
 * Эта функция запускает HTTP-сервер для обработки различных запросов, включая
 * загрузку и скачивание файлов, а также отображение страниц.
 */

void startServer() {
    httplib::Server svr;

    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        std::ifstream file("index.html");
        std::stringstream buffer;
        buffer << file.rdbuf();
        res.set_content(buffer.str(), "text/html");
        logMessage("Served index.html");
        });

    svr.Get(R"(/files/(.*))", [](const httplib::Request& req, httplib::Response& res) {
        std::string token = req.matches[1].str();
        if (validate_token(token)) {
            auto files = get_files("C:/Users/Public/Music/CloudBot/main/files/" + token);
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
        std::string file_path = "C:/Users/Public/Music/CloudBot/main/files/" + token + "/" + file.filename;

        std::ofstream ofs(file_path, std::ios::binary);
        ofs.write(file.content.data(), file.content.size());
        res.set_content("File uploaded successfully.", "text/plain");
        logMessage("File uploaded for token: " + token + ", File: " + file.filename);
        });

    svr.Get(R"(/download/(.*)/(.*))", [](const httplib::Request& req, httplib::Response& res) {
        std::string token = req.matches[1].str();
        std::string file_name = req.matches[2].str();
        std::string file_path = "C:/Users/Public/Music/CloudBot/main/files/" + token + "/" + file_name;

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

    svr.Get("/sendfile", [](const httplib::Request&, httplib::Response& res) {
        std::ifstream ifs("sendfiles.html");
        std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
        res.set_content(content, "text/html");
        });

    svr.Post("/upload", handle_file_upload);
    svr.Get(R"(/sendfile/([0-9A-Za-z]+))", handle_file_download_page);
    svr.Get(R"(/sendfile/([0-9A-Za-z]+)/(.+))", handle_file_download);

    logMessage("Server started on port 8080");
    svr.listen("0.0.0.0", 8080);
}

/**
 * @brief Основная функция приложения.
 *
 * Эта функция инициализирует базу данных, настраивает бота Telegram, запускает сервер
 * и обрабатывает команды и события от пользователей.
 *
 * @return Код завершения программы.
 */

int main() {
    std::thread serverThread(startServer);

    TgBot::Bot bot("7241180998:AAEBDjKwo4gRCZZYgWkNhZkKSWrEKitsHDg");

    initDatabase("cloud_storage.db");

    bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) {
        addUserToDatabase("cloud_storage.db", message->chat->id);

        std::string token = getUserToken("cloud_storage.db", message->chat->id);
        std::string responseMessage = "Welcome to Cloud Storage Bot! Here you can upload and manage your files.";
        if (!token.empty()) {
            responseMessage += "\n\nYour current token: " + token;
        }
        else {
            responseMessage += "\n\nYou do not have a token yet. Please generate one.";
        }

        TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup);
        std::vector<TgBot::InlineKeyboardButton::Ptr> row;

        TgBot::InlineKeyboardButton::Ptr tokenButton(new TgBot::InlineKeyboardButton);
        tokenButton->text = "Token";
        tokenButton->callbackData = "token";
        row.push_back(tokenButton);

        TgBot::InlineKeyboardButton::Ptr uploadButton(new TgBot::InlineKeyboardButton);
        uploadButton->text = "Upload";
        uploadButton->callbackData = "upload";
        row.push_back(uploadButton);

        TgBot::InlineKeyboardButton::Ptr sendButton(new TgBot::InlineKeyboardButton);
        sendButton->text = "Send";
        sendButton->callbackData = "send";
        row.push_back(sendButton);

        keyboard->inlineKeyboard.push_back(row);

        bot.getApi().sendMessage(message->chat->id, responseMessage, false, 0, keyboard);
        logMessage("Sent welcome message to user. UserID: " + std::to_string(message->chat->id));
        });

    bot.getEvents().onCallbackQuery([&bot](TgBot::CallbackQuery::Ptr query) {
        if (query->data == "token") {
            TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup);
            std::vector<TgBot::InlineKeyboardButton::Ptr> rowYesNo;

            TgBot::InlineKeyboardButton::Ptr yesButton(new TgBot::InlineKeyboardButton);
            yesButton->text = "Yes";
            yesButton->callbackData = "confirm_yes";
            rowYesNo.push_back(yesButton);

            TgBot::InlineKeyboardButton::Ptr noButton(new TgBot::InlineKeyboardButton);
            noButton->text = "No";
            noButton->callbackData = "confirm_no";
            rowYesNo.push_back(noButton);

            keyboard->inlineKeyboard.push_back(rowYesNo);

            bot.getApi().sendMessage(query->message->chat->id, "Are you sure you want to generate a new token?", false, 0, keyboard);
            logMessage("Sent token generation confirmation to user. UserID: " + std::to_string(query->message->chat->id));
        }
        else if (query->data == "confirm_yes") {
            std::string newToken = generateToken();
            updateUserToken("cloud_storage.db", query->message->chat->id, newToken);
            bot.getApi().sendMessage(query->message->chat->id, "Your new token is: " + newToken);
            logMessage("Generated new token for user. UserID: " + std::to_string(query->message->chat->id));
        }
        else if (query->data == "confirm_no") {
            bot.getApi().sendMessage(query->message->chat->id, "Token generation cancelled. Returning to menu.");
            logMessage("Token generation cancelled by user. UserID: " + std::to_string(query->message->chat->id));
        }
        else if (query->data == "upload") {
            std::string token = getUserToken("cloud_storage.db", query->message->chat->id);
            if (token.empty()) {
                bot.getApi().sendMessage(query->message->chat->id, "You do not have a token yet. Please generate one.");
                logMessage("User attempted to upload without token. UserID: " + std::to_string(query->message->chat->id));
            }
            else {
                createFolderForUser(token);
                bot.getApi().sendMessage(query->message->chat->id, "Folder created for your token.");

                TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup);
                std::vector<TgBot::InlineKeyboardButton::Ptr> row;

                TgBot::InlineKeyboardButton::Ptr webAppButton(new TgBot::InlineKeyboardButton);
                webAppButton->text = "Open WebApp";
                webAppButton->url = "https://235f-62-217-184-150.ngrok-free.app";
                row.push_back(webAppButton);

                keyboard->inlineKeyboard.push_back(row);

                bot.getApi().sendMessage(query->message->chat->id, "Click the button below to open the web application.", false, 0, keyboard);
                logMessage("Sent web app link to user. UserID: " + std::to_string(query->message->chat->id));
            }
        }
        else if (query->data == "send") {
            TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup);
            std::vector<TgBot::InlineKeyboardButton::Ptr> row;

            TgBot::InlineKeyboardButton::Ptr webAppButton(new TgBot::InlineKeyboardButton);
            webAppButton->text = "Open WebApp";
            webAppButton->url = "https://235f-62-217-184-150.ngrok-free.app/sendfile";
            row.push_back(webAppButton);

            keyboard->inlineKeyboard.push_back(row);

            bot.getApi().sendMessage(query->message->chat->id, "Click the button below to open the web application.", false, 0, keyboard);
            logMessage("Sent web app link to user. UserID: " + std::to_string(query->message->chat->id));
        }
        bot.getApi().answerCallbackQuery(query->id);
        });

    TgBot::TgLongPoll longPoll(bot);

    logMessage("Bot started...");
    while (true) {
        try {
            longPoll.start();
        }
        catch (TgBot::TgException& e) {
            logMessage("Error: " + std::string(e.what()));
        }
    }

    serverThread.join();
    return 0;
}
