#include <tgbot/tgbot.h>
#include <iostream>
#include <memory>
#include <random>
#include <filesystem>

#define SQLITECPP_COMPILE_DLL
#include <SQLiteCpp/SQLiteCpp.h>

namespace fs = std::filesystem;

void initDatabase(const std::string& dbFileName) {
    try {
        SQLite::Database db(dbFileName, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        db.exec("CREATE TABLE IF NOT EXISTS users ("
            "id INTEGER PRIMARY KEY, "
            "token TEXT);");

        std::cout << "Database initialized successfully." << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error initializing database: " << e.what() << std::endl;
    }
}

void addUserToDatabase(const std::string& dbFileName, int64_t userId) {
    try {
        SQLite::Database db(dbFileName, SQLite::OPEN_READWRITE);
        SQLite::Statement query(db, "INSERT OR IGNORE INTO users (id, token) VALUES (?, ?)");
        query.bind(1, userId);
        query.bind(2, nullptr);  // Initially, token is NULL
        query.exec();

        std::cout << "User added to database successfully." << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error adding user to database: " << e.what() << std::endl;
    }
}

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

void updateUserToken(const std::string& dbFileName, int64_t userId, const std::string& token) {
    try {
        SQLite::Database db(dbFileName, SQLite::OPEN_READWRITE);
        SQLite::Statement query(db, "UPDATE users SET token = ? WHERE id = ?");
        query.bind(1, token);
        query.bind(2, userId);
        query.exec();

        std::cout << "User token updated successfully." << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error updating user token: " << e.what() << std::endl;
    }
}

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
        std::cerr << "Error getting user token: " << e.what() << std::endl;
    }
    return "";
}

void createFolderForUser(const std::string& token) {
    fs::create_directories("files/" + token);
    std::cout << "Folder created for user with token: " << token << std::endl;
}

int main() {
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

        keyboard->inlineKeyboard.push_back(row);

        bot.getApi().sendMessage(message->chat->id, responseMessage, false, 0, keyboard);
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
        }
        else if (query->data == "confirm_yes") {
            std::string newToken = generateToken();
            updateUserToken("cloud_storage.db", query->message->chat->id, newToken);
            bot.getApi().sendMessage(query->message->chat->id, "Your new token is: " + newToken);
        }
        else if (query->data == "confirm_no") {
            bot.getApi().sendMessage(query->message->chat->id, "Token generation cancelled. Returning to menu.");
        }
        else if (query->data == "upload") {
            std::string token = getUserToken("cloud_storage.db", query->message->chat->id);
            if (token.empty()) {
                bot.getApi().sendMessage(query->message->chat->id, "You do not have a token yet. Please generate one.");
            }
            else {
                createFolderForUser(token);
                bot.getApi().sendMessage(query->message->chat->id, "Folder created for your token. Please upload your file.");
            }
        }
        bot.getApi().answerCallbackQuery(query->id);
        });

    TgBot::TgLongPoll longPoll(bot);

    std::cout << "Bot started..." << std::endl;
    while (true) {
        try {
            longPoll.start();
        }
        catch (TgBot::TgException& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    return 0;
}
