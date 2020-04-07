/*
Copyright © 2020 Dmytro Korniienko (kDn)
JeeUI2 lib used under MIT License Copyright (c) 2019 Marsel Akhkamov

    This file is part of FireLamp_JeeUI.

    FireLamp_JeeUI is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FireLamp_JeeUI is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FireLamp_JeeUI.  If not, see <https://www.gnu.org/licenses/>.

  (Этот файл — часть FireLamp_JeeUI.

   FireLamp_JeeUI - свободная программа: вы можете перераспространять ее и/или
   изменять ее на условиях Стандартной общественной лицензии GNU в том виде,
   в каком она была опубликована Фондом свободного программного обеспечения;
   либо версии 3 лицензии, либо (по вашему выбору) любой более поздней
   версии.

   FireLamp_JeeUI распространяется в надежде, что она будет полезной,
   но БЕЗО ВСЯКИХ ГАРАНТИЙ; даже без неявной гарантии ТОВАРНОГО ВИДА
   или ПРИГОДНОСТИ ДЛЯ ОПРЕДЕЛЕННЫХ ЦЕЛЕЙ. Подробнее см. в Стандартной
   общественной лицензии GNU.

   Вы должны были получить копию Стандартной общественной лицензии GNU
   вместе с этой программой. Если это не так, см.
   <https://www.gnu.org/licenses/>.)
*/

#include "main.h"
#include "effects.h"

class INTRFACE_GLOBALS{
public:
#pragma pack(push,1)
 struct {
    bool isSetup:1;
    bool isTmSetup:1;
    bool isAddSetup:1;
    bool isEdEvent:1;
 };
 #pragma pack(pop)
 uint8_t addSList = 1;
 EFFECT *prevEffect = nullptr;
 INTRFACE_GLOBALS() {   
    isSetup = false;
    isTmSetup = false;
    isAddSetup = false;
    isEdEvent = false;
}
};

INTRFACE_GLOBALS iGLOBAL; // объект глобальных переменных интерфейса

void bSetCloseCallback()
{
    iGLOBAL.isAddSetup = false;
    jee.var("isAddSetup", "false");
    jee._refresh = true;
}

void bDelEventCallback(bool);
void bAddEventCallback();

void bOwrEventCallback()
{
    bDelEventCallback(false);
    bAddEventCallback();
}

void event_worker(const EVENT *event) // обработка эвентов лампы
{
    LOG.printf_P(PSTR("%s - %s\n"), ((EVENT *)event)->getName().c_str(), myLamp.timeProcessor.getFormattedShortTime().c_str());

    String filename;
    String tmpStr = jee.param(F("txtColor"));
    tmpStr.replace(F("#"),F("0x"));
    CRGB::HTMLColorCode color = (CRGB::HTMLColorCode)strtol(tmpStr.c_str(),NULL,0);

    switch (event->event)
    {
    case EVENT_TYPE::ON :
        myLamp.setOnOff(true);
        jee.var(F("ONflag"), (myLamp.isLampOn()?F("true"):F("false")));
        break;
    case EVENT_TYPE::OFF :
        myLamp.setOnOff(false);
        jee.var(F("ONflag"), (myLamp.isLampOn()?F("true"):F("false")));
        break;
    case EVENT_TYPE::ALARM :
        break;
    case EVENT_TYPE::DEMO_ON :
        if(myLamp.getMode()!=MODE_DEMO || !myLamp.isLampOn())
            myLamp.startDemoMode();
        break;
    case EVENT_TYPE::LAMP_CONFIG_LOAD :
        filename=String(F("/glb/"));
        filename.concat(event->message);
        if(event->message)
            jee.load(filename.c_str());
        break;
    case EVENT_TYPE::EFF_CONFIG_LOAD :
        filename=String(F("/cfg/"));
        filename.concat(event->message);
        if(event->message)
            myLamp.effects.loadConfig(filename.c_str());    
        break;
    case EVENT_TYPE::EVENTS_CONFIG_LOAD :
        filename=String(F("/evn/"));
        filename.concat(event->message);
        if(event->message)
            myLamp.events.loadConfig(filename.c_str());
        break;
    case EVENT_TYPE::SEND_TEXT :
        if(event->message==nullptr)
            break;
        {
            String toPrint(event->message);
            toPrint.replace(F("%TM"),myLamp.timeProcessor.getFormattedShortTime());

            if(!myLamp.isLampOn()){
                myLamp.disableEffectsUntilText();
                myLamp.setOffAfterText();
                myLamp.setOnOff(true);
                myLamp.sendStringToLamp(toPrint.c_str(),color);
            } else {
                if(event->message) myLamp.sendStringToLamp(toPrint.c_str(),color);
            }
        }
        return;
        break;

    default:
        break;
    }
    if(event->message) myLamp.sendStringToLamp(event->message,color);
    jee._refresh = true;
}

void bEditEventCallback()
{
    EVENT *next = myLamp.events.getNextEvent(nullptr);
    int index = jee.param(F("evSelList")).toInt();
    int i = 0;
    while (next!=nullptr)
    {
        if(i==index) break;
        i++;
        next = myLamp.events.getNextEvent(next);
    }
    jee.var(F("isEnabled"),(next->isEnabled?F("true"):F("false")));
    jee.var(F("d1"),(next->d1?F("true"):F("false")));
    jee.var(F("d2"),(next->d2?F("true"):F("false")));
    jee.var(F("d3"),(next->d3?F("true"):F("false")));
    jee.var(F("d4"),(next->d4?F("true"):F("false")));
    jee.var(F("d5"),(next->d5?F("true"):F("false")));
    jee.var(F("d6"),(next->d6?F("true"):F("false")));
    jee.var(F("d7"),(next->d7?F("true"):F("false")));
    jee.var(F("evList"),String(next->event));
    jee.var(F("repeat"),String(next->repeat));
    jee.var(F("msg"),String(next->message));
    jee.var(F("tmEvent"), next->getDateTime());
    iGLOBAL.isEdEvent = true;
    jee.var(F("isEdEvent"),F("true"));
    jee._refresh = true;
}

void bDelEventCallback(bool isRefresh)
{
    EVENT *next = myLamp.events.getNextEvent(nullptr);
    int index = jee.param(F("evSelList")).toInt();
    int i = 0;
    while (next!=nullptr)
    {
        if(i==index) break;
        i++;
        next = myLamp.events.getNextEvent(next);
    }
    if(next!=nullptr)
        myLamp.events.delEvent(*next);
    myLamp.events.saveConfig();
    jee._refresh = isRefresh;
}

void bDelEventCallback()
{
    bDelEventCallback(true);
}

void bAddEventCallback()
{
    EVENT event;

    event.isEnabled=(jee.param(F("isEnabled"))==F("true"));
    event.d1=(jee.param(F("d1"))==F("true"));
    event.d2=(jee.param(F("d2"))==F("true"));
    event.d3=(jee.param(F("d3"))==F("true"));
    event.d4=(jee.param(F("d4"))==F("true"));
    event.d5=(jee.param(F("d5"))==F("true"));
    event.d6=(jee.param(F("d6"))==F("true"));
    event.d7=(jee.param(F("d7"))==F("true"));
    event.event=(EVENT_TYPE)jee.param(F("evList")).toInt();
    event.repeat=jee.param(F("repeat")).toInt();
    String tmEvent = jee.param(F("tmEvent"));
    time_t unixtime;
    tmElements_t tm;
    // Serial.println(tmEvent);
    // Serial.println(tmEvent.substring(0,4).c_str());
    tm.Year=atoi(tmEvent.substring(0,4).c_str())-1970;
    tm.Month=atoi(tmEvent.substring(5,7).c_str());
    tm.Day=atoi(tmEvent.substring(8,10).c_str());
    tm.Hour=atoi(tmEvent.substring(11,13).c_str());
    tm.Minute=atoi(tmEvent.substring(14,16).c_str());
    tm.Second=0;

    Serial.printf_P(PSTR("%d %d %d %d %d\n"), tm.Year, tm.Month, tm.Day, tm.Hour, tm.Minute);

    unixtime = makeTime(tm);
    event.unixtime = unixtime;
    String tmpMsg(jee.param(F("msg")));
    event.message = (char*)(tmpMsg.c_str());
    myLamp.events.addEvent(event);
    myLamp.events.saveConfig();
    iGLOBAL.isEdEvent = false;
    jee.var(F("isEdEvent"),F("false"));
    jee._refresh = true;
}

void bOTACallback()
{
    myLamp.startOTA();
}

void bRefreshCallback()
{
    jee._refresh = true;
}

void bFDelCallback()
{
    // Удаление конфигурации из ФС
    String filename = String(F("/cfg/"))+jee.param(F("fileName"));
    if(filename.length()>4)
        if(SPIFFS.begin()){
            SPIFFS.remove(filename);
        }
    filename = String(F("/glb/"))+jee.param(F("fileName"));
    if(filename.length()>4)
        if(SPIFFS.begin()){
            SPIFFS.remove(filename);
        }
    filename = String(F("/evn/"))+jee.param(F("fileName"));
    if(filename.length()>4)
        if(SPIFFS.begin()){
            SPIFFS.remove(filename);
        }
    iGLOBAL.isAddSetup = false;
    jee.var(F("isAddSetup"), F("false"));
    jee.var(F("fileName"),F(""));
    jee._refresh = true;
}

void bFLoadCallback()
{
    // Загрузка сохраненной конфигурации эффектов вместо текущей
    String fn = jee.param(F("fileList"));
    myLamp.effects.loadConfig(fn.c_str());
    jee.var(F("fileName"),fn);
    jee._refresh = true;
}

void bFSaveCallback()
{
    // Сохранение текущей конфигурации эффектов в ФС
    String filename = String(F("/cfg/"))+jee.param(F("fileName"));
    if(filename.length()>4)
        if(SPIFFS.begin()){
            // if(!SPIFFS.exists(F("/cfg")))
            //     SPIFFS.mkdir(F("/cfg"));
            myLamp.effects.saveConfig(filename.c_str());
            filename = String(F("/glb/"))+jee.param(F("fileName"));
            jee.save(filename.c_str());
            filename = String(F("/evn/"))+jee.param(F("fileName"));
            myLamp.events.saveConfig(filename.c_str());
        }
    iGLOBAL.isAddSetup = false;
    jee.var(F("isAddSetup"), F("false"));
    jee._refresh = true;
}

void bTxtSendCallback()
{
    String tmpStr = jee.param(F("txtColor"));
    tmpStr.replace(F("#"),F("0x"));
    //LOG.printf("%s %d\n", tmpStr.c_str(), strtol(tmpStr.c_str(),NULL,0));
    myLamp.sendStringToLamp(jee.param(F("msg")).c_str(), (CRGB::HTMLColorCode)strtol(tmpStr.c_str(),NULL,0)); // вывести текст на лампу
}

void bTmSubmCallback()
{
#ifdef LAMP_DEBUG
    LOG.println(F("bTmSubmCallback pressed"));
#endif
    myLamp.timeProcessor.setTimezone(jee.param(F("timezone")).c_str());
    myLamp.timeProcessor.setTime(jee.param(F("time")).c_str());

    if(myLamp.timeProcessor.getIsSyncOnline()){
        myLamp.refreshTimeManual(); // принудительное обновление времени
    }
    iGLOBAL.isTmSetup = false;
    jee.var(F("isTmSetup"), F("false"));
    myLamp.sendStringToLamp(myLamp.timeProcessor.getFormattedShortTime().c_str(), CRGB::Green); // вывести время на лампу
    jee._refresh = true;
}

void bMQTTformCallback()
{
    jee.save();
    ESP.restart();
}

void bDemoCallback()
{
    if(myLamp.getMode()!=MODE_DEMO)
        myLamp.startDemoMode();
    else
        myLamp.startNormalMode();

    // // Отладка
    // myLamp.sendStringToLamp(WiFi.localIP().toString().c_str(), CRGB::Blue);
    // myLamp.sendStringToLamp(WiFi.localIP().toString().c_str(), CRGB::Green);
    // myLamp.sendStringToLamp(WiFi.localIP().toString().c_str(), CRGB::Red);

    jee._refresh = true;
}

void jeebuttonshandle()
{
    static unsigned long timer;
    jee.btnCallback(F("bTmSubm"), bTmSubmCallback);
    jee.btnCallback(F("bMQTTform"), bMQTTformCallback); // MQTT form button
    jee.btnCallback(F("bDemo"), bDemoCallback);
    jee.btnCallback(F("bTxtSend"), bTxtSendCallback);
    jee.btnCallback(F("bFLoad"), bFLoadCallback);
    jee.btnCallback(F("bFSave"), bFSaveCallback);
    jee.btnCallback(F("bFDel"), bFDelCallback);
    jee.btnCallback(F("bRefresh"), bRefreshCallback);
    jee.btnCallback(F("bOTA"), bOTACallback);
    jee.btnCallback(F("bAddEvent"), bAddEventCallback);
    jee.btnCallback(F("bDelEvent"), bDelEventCallback);
    jee.btnCallback(F("bEditEvent"), bEditEventCallback);
    jee.btnCallback(F("bOwrEvent"), bOwrEventCallback);
    jee.btnCallback(F("bSetClose"), bSetCloseCallback);

    //публикация изменяющихся значений
    if (timer + 5*1000 > millis())
        return;
    timer = millis();
    jee.var(F("pTime"),myLamp.timeProcessor.getFormattedShortTime(), true); // обновить опубликованное значение
}

void create_parameters(){
#ifdef LAMP_DEBUG
    LOG.println(F("Создание дефолтных параметров"));
#endif
    // создаем дефолтные параметры для нашего проекта
    jee.var_create(F("wifi"), F("STA")); // режим работы WiFi по умолчанию ("STA" или "AP")  (параметр в энергонезависимой памяти)
    jee.var_create(F("ssid"), F("")); // имя точки доступа к которой подключаемся (параметр в энергонезависимой памяти)
    jee.var_create(F("pass"), F("")); // пароль точки доступа к которой подключаемся (параметр в энергонезависимой памяти)

    // параметры подключения к MQTT
    jee.var_create(F("mqtt_int"), F("30")); // интервал отправки данных по MQTT в секундах (параметр в энергонезависимой памяти)
    
    jee.var_create(F("effList"), F("1"));
    jee.var_create(F("isSetup"), F("false"));

    jee.var_create(F("bright"), F("127"));
    jee.var_create(F("speed"), F("127"));
    jee.var_create(F("scale"), F("127"));
    jee.var_create(F("canBeSelected"), F("true"));
    jee.var_create(F("isFavorite"), F("true"));

    jee.var_create(F("ONflag"), F("true"));
    jee.var_create(F("MIRR_H"), F("false"));
    jee.var_create(F("MIRR_V"), F("false"));
    jee.var_create(F("msg"), F(""));
    jee.var_create(F("txtColor"), F("#ffffff"));
    jee.var_create(F("txtSpeed"), F("100"));
    jee.var_create(F("txtOf"), F("0"));
    jee.var_create(F("perTime"), F("1"));

    jee.var_create(F("GlobBRI"), F("127"));

    jee.var_create(F("time"), F("00:00"));
    jee.var_create(F("timezone"), F(""));
    jee.var_create(F("tm_offs"), F("0"));
    jee.var_create(F("isTmSetup"), F("false"));
    jee.var_create(F("isTmSync"), F("true"));

    jee.var_create(F("isGLBbr"),F("false"));

    jee.var_create(F("isAddSetup"), F("false"));
    jee.var_create(F("fileName"), F("cfg1"));

    jee.var_create(F("ny_period"), F("0"));
    jee.var_create(F("ny_unix"), F("1609459200"));

    jee.var_create(F("addSList"),F("1"));

}

void interface(){ // функция в которой мф формируем веб интерфейс
    if(!jee.isLoading()){
#ifdef LAMP_DEBUG
        LOG.println(F("Внимание: Создание интерфейса! Такие вызовы должны быть минимизированы."));
#endif
        jee.app(F(("Огненная лампа"))); // название программы (отображается в веб интерфейсе)

        // создаем меню
        jee.menu(F("Эффекты"));
        jee.menu(F("Лампа"));
        jee.menu(F("Настройки"));
        // создаем контент для каждого пункта меню

        jee.page(); // разделитель между страницами
        // Страница "Управление эффектами"

        EFFECT enEff; enEff.setNone();
        jee.checkbox(F("ONflag"),F("Включение&nbspлампы"));
        //char nameBuffer[64]; // Exception (3) при обращении к PROGMEM, поэтому ход конем - копируем во временный буффер
        
        if(!iGLOBAL.isAddSetup){
            do {
                enEff = *myLamp.effects.enumNextEffect(&enEff);
                if(enEff.eff_nb!=EFF_NONE && (enEff.canBeSelected || iGLOBAL.isSetup)){
                    //strncpy_P(nameBuffer, enEff.eff_name, sizeof(nameBuffer)-1);
                    //jee.option(String((int)enEff.eff_nb), nameBuffer);
                    jee.option(String((int)enEff.eff_nb), FPSTR(enEff.eff_name));
                }
            } while((enEff.eff_nb!=EFF_NONE));
            jee.select(F("effList"), F("Эффект"));
        } else {
            jee.option(jee.param(F("effList")), F("Список эффектов отключен, выйдите из режима настройки!"));
            jee.select(F("effList"), F("Эффект"));
            jee.button(F("bSetClose"), F("gray"), F("Выйти из настроек"));
        }
        jee.range(F("bright"),1,255,1,F("Яркость"));
        jee.range(F("speed"),1,255,1,F("Скорость"));
        jee.range(F("scale"),1,255,1,F("Масштаб"));

        //jee.button(F("btn1"),F("gray"),F("<"), 1);
        if(myLamp.getMode()==MODE_DEMO)
            jee.button(F("bDemo"),F("green"),F("DEMO -> OFF"));
        else
            jee.button(F("bDemo"),F("gray"),F("DEMO -> ON"));
        //jee.button(F("btn3"),F("gray"),F(">"), 3);

        if(iGLOBAL.isSetup){
            jee.checkbox(F("canBeSelected"),F("В&nbspсписке&nbspвыбора"));
            jee.checkbox(F("isFavorite"),F("В&nbspсписке&nbspдемо"));  
        }
        jee.checkbox(F("isSetup"),F("Конфигурирование"));

        jee.page(); // разделитель между страницами
        //Страница "Управление лампой"
        if(iGLOBAL.isTmSetup){
            jee.time(F("time"),F("Время"));
            jee.number(F("tm_offs"), F("Смещение времени в секундах для NTP"));
            jee.text(F("timezone"),F("Часовой пояс (http://worldtimeapi.org/api/timezone/)"));
            jee.checkbox(F("isTmSync"),F("Включить&nbspсинхронизацию"));
            jee.button(F("bTmSubm"),F("gray"),F("Сохранить"));
        } else {
            jee.pub(F("pTime"),F("Текущее время на ESP"),F("--:--"));
            jee.var(F("pTime"),myLamp.timeProcessor.getFormattedShortTime()); // обновить опубликованное значение
            jee.text(F("msg"),F("Текст для вывода на матрицу"));
            jee.color(F("txtColor"), F("Цвет сообщения"));
            jee.button(F("bTxtSend"),F("gray"),F("Отправить"));
        }
        jee.checkbox(F("isTmSetup"),F("Настройка&nbspвремени"));

        jee.page(); // разделитель между страницами
        // Страница "Настройки соединения"
        // if(!jee.connected || jee.param(F("wifi"))==F("AP")){
        //     jee.formWifi(); // форма настроек Wi-Fi
        //     jee.formMqtt(); // форма настроек MQTT
        // }
        jee.checkbox(F("isAddSetup"),F("Расширенные&nbspнастройки"));
        if(iGLOBAL.isAddSetup){
            jee.option(F("1"), F("Конфигурации"));
            jee.option(F("2"), F("Время/Текст"));
            jee.option(F("3"), F("События"));
            jee.option(F("4"), F("Wifi & MQTT"));
            jee.option(F("99"), F("Другое"));
            jee.select(F("addSList"), F("Группа настроек"));
            switch (iGLOBAL.addSList)
            {
            case 1:
                jee.text(F("fileName"),F("Конфигурация"));
                jee.button(F("bFSave"),F("green"),F("Записать в ФС"));
                jee.button(F("bFDel"),F("red"),F("Удалить из ФС"));
                break;
            case 2:
                jee.number(F("ny_period"), F("Период вывода новогоднего поздравления в минутах (0 - не выводить)"));
                jee.number(F("ny_unix"), F("UNIX дата/время нового года"));
                jee.range(F("txtSpeed"),30,150,10,F("Задержка прокрутки текста"));
                jee.range(F("txtOf"),-1,10,1,F("Смещение вывода текста"));
                jee.option(String(PERIODICTIME::PT_NOT_SHOW), F("Не выводить"));
                jee.option(String(PERIODICTIME::PT_EVERY_60), F("Каждый час"));
                jee.option(String(PERIODICTIME::PT_EVERY_30), F("Каждые полчаса"));
                jee.option(String(PERIODICTIME::PT_EVERY_15), F("Каждые 15 минут"));
                jee.option(String(PERIODICTIME::PT_EVERY_10), F("Каждые 10 минут"));
                jee.option(String(PERIODICTIME::PT_EVERY_5), F("Каждые 5 минут"));
                jee.option(String(PERIODICTIME::PT_EVERY_1), F("Каждую минуту"));
                jee.select(F("perTime"), F("Периодический вывод времени"));
                break;       
            case 3:
                jee.checkbox(F("isEdEvent"),F("Новое&nbspсобытие"));
                if(jee.param(F("isEdEvent"))==F("true")){ // редактируем параметры событий
                    jee.option(String(EVENT_TYPE::ON), F("Включить лампу"));
                    jee.option(String(EVENT_TYPE::OFF), F("Выключить лампу"));
                    jee.option(String(EVENT_TYPE::DEMO_ON), F("Включить DEMO"));
                    jee.option(String(EVENT_TYPE::ALARM), F("Будильник"));
                    jee.option(String(EVENT_TYPE::LAMP_CONFIG_LOAD), F("Загрузка конф. лампы"));
                    jee.option(String(EVENT_TYPE::EFF_CONFIG_LOAD), F("Загрузка конф. эффектов"));
                    jee.option(String(EVENT_TYPE::EVENTS_CONFIG_LOAD), F("Загрузка конф. событий"));
                    jee.option(String(EVENT_TYPE::SEND_TEXT), F("Вывести текст"));
                    jee.select(F("evList"), F("Тип события"));
                    jee.checkbox(F("isEnabled"),F("Разрешено"));
                    jee.datetime(F("tmEvent"),F("Дата/время события"));
                    jee.number(F("repeat"),F("Повтор, мин"));
                    jee.text(F("msg"),F("Текст для вывода на матрицу"));
                    jee.checkbox(F("d1"),F("Понедельник"));
                    jee.checkbox(F("d2"),F("Вторник"));
                    jee.checkbox(F("d3"),F("Среда"));
                    jee.checkbox(F("d4"),F("Четверг"));
                    jee.checkbox(F("d5"),F("Пятница"));
                    jee.checkbox(F("d6"),F("Суббота"));
                    jee.checkbox(F("d7"),F("Воскресенье"));
                    jee.button(F("bOwrEvent"),F("grey"),F("Обновить событие"));
                    jee.button(F("bAddEvent"),F("green"),F("Добавить событие"));
                } else {
                    EVENT *next = myLamp.events.getNextEvent(nullptr);
                    int i = 0;
                    while (next!=nullptr)
                    {
                        jee.option(String(i), next->getName());
                        i++;
                        next = myLamp.events.getNextEvent(next);
                    }
                    jee.select(F("evSelList"), F("Событие"));
                    jee.button(F("bEditEvent"),F("green"),F("Редактировать событие"));
                    jee.button(F("bDelEvent"),F("red"),F("Удалить событие"));
                }
                break;      
            case 4:
                jee.formWifi(); // форма настроек Wi-Fi
                jee.formMqtt(); // форма настроек MQTT            
                break;       
            case 5:
                break;
            case 99:
                jee.number(F("mqtt_int"), F("Интервал mqtt сек."));
                jee.checkbox(F("isGLBbr"),F("Глобальная&nbspяркость"));
                jee.checkbox(F("MIRR_H"),F("Отзеркаливание&nbspH"));
                jee.checkbox(F("MIRR_V"),F("Отзеркаливание&nbspV")); 
                jee.button(F("bOTA"),(myLamp.getMode()==MODE_OTA?F("grey"):F("blue")),F("Обновление по ОТА-PIO"));   
                break;      
            default:
                break;
            }
        } else {
            if(SPIFFS.begin()){
#ifdef ESP32
                File root = SPIFFS.open("/cfg");
                File file = root.openNextFile();
#else
                Dir dir = SPIFFS.openDir(F("/cfg"));
#endif
                String fn;
#ifdef ESP32
                while (file) {
                    fn=file.name();
                    if(!file.isDirectory()){
#else                    
                while (dir.next()) {
                    fn=dir.fileName();
#endif

                        fn.replace(F("/cfg/"),F(""));
                        //LOG.println(fn);
                        jee.option(fn, fn);
#ifdef ESP32
                        file = root.openNextFile();
                    }
#endif
                }
            }
            String cfg(F("Конфигурации")); cfg+=" ("; cfg+=jee.param(F("fileList")); cfg+=")";
            jee.select(F("fileList"), cfg);

            jee.button(F("bFLoad"),F("gray"),F("Считать с ФС"));
        }
        jee.page(); // разделитель между страницами
    } else {
#ifdef LAMP_DEBUG
        LOG.println(F("Внимание: Загрузка минимального интерфейса, т.к. обнаружен вызов index.htm"));
#endif
        jee.app(F(("Огненная лампа"))); // название программы (отображается в веб интерфейсе)

        // создаем меню
        jee.menu(F("Эффекты"));

        jee.page(); // разделитель между страницами
        jee.button(F("bRefresh"),F("gray"),F("Обновить интерфейс"));
        jee.page(); // разделитель между страницами
    }
}

void update(){ // функция выполняется после ввода данных в веб интерфейсе. получение параметров из веб интерфейса в переменные
#ifdef LAMP_DEBUG
    LOG.println(F("In update..."));
#endif
    // получаем данные в переменную в ОЗУ для дальнейшей работы
    bool isRefresh = jee._refresh;
    EFFECT *curEff = myLamp.effects.getEffectBy((EFF_ENUM)jee.param(F("effList")).toInt());
    mqtt_int = jee.param(F("mqtt_int")).toInt();
    bool isGlobalBrightness = jee.param(F("isGLBbr"))==F("true");
    myLamp.setIsGlobalBrightness(isGlobalBrightness);

    if(iGLOBAL.isEdEvent!=(jee.param(F("isEdEvent"))==F("true"))){
        iGLOBAL.isEdEvent = !iGLOBAL.isEdEvent;
        isRefresh = true;
    }
    
    if(iGLOBAL.isTmSetup!=(jee.param(F("isTmSetup"))==F("true"))){
        iGLOBAL.isTmSetup = !iGLOBAL.isTmSetup;
        isRefresh = true;
    }

    if(iGLOBAL.isAddSetup!=(jee.param(F("isAddSetup"))==F("true"))){
        iGLOBAL.isAddSetup = !iGLOBAL.isAddSetup;
        isRefresh = true;
    }

    if((jee.param(F("isSetup"))==F("true"))!=iGLOBAL.isSetup){
        iGLOBAL.isSetup = !iGLOBAL.isSetup;
        if(iGLOBAL.prevEffect!=nullptr)
            isRefresh = true;
    }

    myLamp.effects.moveBy(curEff->eff_nb);
    myLamp.restartDemoTimer();

    if(curEff->eff_nb!=EFF_NONE){
        if((curEff!=iGLOBAL.prevEffect  || isRefresh) && iGLOBAL.prevEffect!=nullptr){
            jee.var(F("isFavorite"), (curEff->isFavorite?F("true"):F("false")));
            jee.var(F("canBeSelected"), (curEff->canBeSelected?F("true"):F("false")));
            jee.var(F("bright"),String(myLamp.getLampBrightness()));
            jee.var(F("speed"),String(curEff->speed));
            jee.var(F("scale"),String(curEff->scale));
            jee.var(F("ONflag"), (myLamp.isLampOn()?F("true"):F("false")));

            isRefresh = true;
            myLamp.startFader(true);
        } else {
            curEff->isFavorite = (jee.param(F("isFavorite"))==F("true"));
            curEff->canBeSelected = (jee.param(F("canBeSelected"))==F("true"));
            myLamp.setLampBrightness(jee.param(F("bright")).toInt());
            curEff->speed = jee.param(F("speed")).toInt();
            curEff->scale = jee.param(F("scale")).toInt();
            
            myLamp.setLoading(true); // перерисовать эффект

            if(iGLOBAL.prevEffect!=nullptr){
                if(!myLamp.effects.autoSaveConfig()){ // отложенная запись, не чаще чем однократно в 30 секунд 
                    myLamp.ConfigSaveSetup(60*1000); //через минуту сработает еще попытка записи и так до успеха
                }
            }
        }
    }

    iGLOBAL.prevEffect = curEff;

    uint8_t cur_addSList = jee.param(F("addSList")).toInt();
    if(iGLOBAL.addSList!=cur_addSList){
        iGLOBAL.addSList = cur_addSList;
        isRefresh = true;
    }

    myLamp.setTextMovingSpeed(jee.param(F("txtSpeed")).toInt());
    myLamp.setTextOffset(jee.param(F("txtOf")).toInt());
    myLamp.setPeriodicTimePrint((PERIODICTIME)jee.param(F("perTime")).toInt());

    myLamp.setMIRR_H(jee.param(F("MIRR_H"))==F("true"));
    myLamp.setMIRR_V(jee.param(F("MIRR_V"))==F("true"));
    myLamp.setOnOff(jee.param(F("ONflag"))==F("true"));
    myLamp.timeProcessor.SetOffset(jee.param(F("tm_offs")).toInt());
    myLamp.setNYUnixTime(jee.param(F("ny_unix")).toInt());
    myLamp.setNYMessageTimer(jee.param(F("ny_period")).toInt());


    if(myLamp.getMode() == MODE_DEMO || isGlobalBrightness)
        jee.var(F("GlobBRI"), String(myLamp.getLampBrightness()));
    myLamp.timeProcessor.setIsSyncOnline(jee.param(F("isTmSync"))==F("true"));
    //jee.param(F("effList"))=String(0);
    jee.var(F("pTime"),myLamp.timeProcessor.getFormattedShortTime()); // обновить опубликованное значение

    jee.setDelayedSave(30000); // отложенное сохранение конфига, раз в 30 секунд относительно последнего изменения
    jee._refresh = isRefresh; // устанавливать в самом конце!
}

void updateParm() // передача параметров в UI после нажатия сенсорной или мех. кнопки
{
#ifdef LAMP_DEBUG
    LOG.println(F("Обновляем параметры после нажатия кнопки..."));
#endif
    EFFECT *curEff = myLamp.effects.getCurrent();

    if(myLamp.getMode() == MODE_DEMO || myLamp.IsGlobalBrightness())
        jee.var(F("GlobBRI"), String(myLamp.getLampBrightness()));
    else
        myLamp.setGlobalBrightness(jee.param(F("GlobBRI")).toInt());

    jee.var(F("bright"),String(myLamp.getLampBrightness()));
    jee.var(F("speed"),String(curEff->speed));
    jee.var(F("scale"),String(curEff->scale));
    jee.var(F("effList"),String(curEff->eff_nb));
    jee.var(F("ONflag"), (myLamp.isLampOn()?F("true"):F("false")));

    myLamp.setLoading(); // обновить эффект
    iGLOBAL.prevEffect = curEff; // обновить указатель на предыдущий эффект
    if(myLamp.getMode()!=MODE_DEMO)
        jee.save(); // Cохранить конфиг
    jee._refresh = true;
}