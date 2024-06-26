# HSECloud 

*HSECloud позволяет пользователям загружать и управлять файлами через Telegram. Пользователь может получить уникальный токен, который используется для создания индивидуальной папки для хранения файлов. Для загрузки файлов бот предоставляет ссылку на веб-приложение.*

*Данный репозиторий содержит исходный код данного телеграм бота.*

---

<p align="center">
<a href='https://t.me/hsecloudbot' target='_blank'><img height='10000' style='border:px;height:200px;' src='HSECloud.webp' border='0' alt='Ссылка на нашего Telegram бота' />
<div id="top"></div>
</p>

---
## Процесс подготовки к работе (При учете что операционная система Linux)
#### После клонирования проекта в IDE откройте панель **vcpkg**, после этого внутри данной панели установите все необходимые библиотеки, а именно: ***tgbot-cpp***, ***SQLiteCPP***, ***htpplib***
---

## Основные возможности
- Регистрация пользователей в базе данных.
- Генерация и обновление уникальных токенов для пользователей.
- Создание папок для пользователей по их токену.
- Загрузка и хранение файлов в индивидуальные папки.
- Предоставление ссылки на веб-приложение для управления файлами.
- Передача файлов по прямой ссылке к скачке (полная анонимность)

### Функционал 

### **1. Создание аккаунта внутри бота.**
После ввода команды */start* Вы вводите желаемый логин и пароль, для того чтобы впоследствии вы могли по вводу этих данных использовать личное хранилище с любого аккаунта телеграм, так же бот выдает вам личный токен (для быстрого входа)

### **2. Кнопки в меню бота**
#### Загрузка файла
После того как Вы нажмете данную кнопку, бот перенаправит вас на страницу загрузки файлов, где Вы вводите полученный ранее токен и загружаете необходимые Вам файлы
#### Получить файл
После того как Вы нажмете данную кнопку, бот перенаправит вас на страницу выгрузки файлов файлов, где Вы вводите полученный ранее токен и из списка файлов выгружаете необходимые Вам файлы
#### Сгенерировать новый токен
Бот генерирует Вам новый токен
#### Сменить пользователя 
Бот выходит выходит в главное меню и предлагает варианты входа (По логину/паролю или по токену)
#### Отправить файл без привязки к токену
После того как Вы нажмете данную кнопку, бот перенаправит вас на страницу, где Вы можете прикрепить файл, после чего Вам будет выдана прямая уникальная ссылка на него, которые вы можете отправить любому человеку, и он сможет загрузить этот файл на свое устройство
## Поддержка и обратная связь
  Если у вас есть какие-либо вопросы или замечания, пожалуйста, создайте issue в этом репозитории. Я и моя команда постараюсь ответить в ближайшее время.
<p align="center">
*Приятного использования бота!*
</p>

