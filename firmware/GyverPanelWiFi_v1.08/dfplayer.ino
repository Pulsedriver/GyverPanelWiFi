void InitializeDfPlayer1() {
#if (USE_MP3 == 1)
#if defined(ESP8266)
//mp3Serial.begin(9600);                           // Используйте этот вариант, если у вас библиотека ядра ESP8266 версии 2.5.2
//mp3Serial.begin(9600, SRX, STX);                 // Используйте этот вариант, если у вас библиотека ядра ESP8266 версии 2.6.0
  mp3Serial.begin(9600, SWSERIAL_8N1, SRX, STX);   // Используйте этот вариант, если у вас библиотека ядра ESP8266 версии 2.6.1
#endif
#if defined(ESP32)
//mp3Serial.begin(9600, SRX, STX);                 // Используйте этот вариант, если у вас библиотека EspSoftwareSerial v5.4
  mp3Serial.begin(9600, SWSERIAL_8N1, SRX, STX);   // Используйте этот вариант, если у вас библиотека EspSoftwareSerial v6.0
#endif

  dfPlayer.begin(mp3Serial, false, true);
  dfPlayer.setTimeOut(1000);
  dfPlayer.EQ(DFPLAYER_EQ_NORMAL);
  dfPlayer.volume(1);
#endif  
}

void InitializeDfPlayer2() {    
#if (USE_MP3 == 1)
  Serial.print(F("\nИнициализация MP3 плеера."));
  refreshDfPlayerFiles();    
  Serial.println(String(F("Звуков будильника найдено: ")) + String(alarmSoundsCount));
  Serial.println(String(F("Звуков рассвета найдено: ")) + String(dawnSoundsCount));
  set_isDfPlayerOk(alarmSoundsCount + dawnSoundsCount > 0);
#else  
  set_isDfPlayerOk(false);
#endif  
}

#if (USE_MP3 == 1)
void printDetail(uint8_t type, int value){
  switch (type) {
    case TimeOut:
      //Serial.println(F("Таймаут!"));
      break;
    case WrongStack:
      //Serial.println(F("Ошибка стека!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Карта вставлена."));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Карта удалена."));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Карта готова."));
      break;
    case DFPlayerUSBInserted:
      Serial.println(F("Подключен USB."));
      break;
    case DFPlayerUSBRemoved:
      Serial.println(F("Отключен USB."));
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Номер: "));
      Serial.print(value);
      Serial.println(F(". Завершено."));
      if (!(isAlarming || isPlayAlarmSound) && soundFolder == 0 && soundFile == 0) {
        dfPlayer.stop();
      }
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Нет карты"));
          break;
        case Sleeping:
          Serial.println(F("Ожидание..."));
          break;
        case SerialWrongStack:
          Serial.println(F("Неверные данные"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Ошибка контрольной суммы"));
          break;
        case FileIndexOut:
          Serial.println(F("Неверный индекс файла"));
          break;
        case FileMismatch:
          Serial.println(F("Файл не найден"));
          break;
        case Advertise:
          Serial.println(F("Реклама"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

void refreshDfPlayerFiles() {
  // Чтение почему-то не всегда работает, иногда возвращает 0 или число от какого-то предыдущего запроса
  // Для того, чтобы наверняка считать значение - первое прочитанное игнорируем, потом читаем несколько раз до повторения.

  // Папка с файлами для будильника
  int cnt = 0, val = 0, new_val = 0; 
  do {
    val = dfPlayer.readFileCountsInFolder(1);     delay(10);
    new_val = dfPlayer.readFileCountsInFolder(1); delay(10);    
    if (val == new_val && val != 0) break;
    cnt++;
    delay(100);
    Serial.print(F("."));
  } while ((val == 0 || new_val == 0 || val != new_val) && cnt < 5);
  alarmSoundsCount = val < 0 ? 0 : val;
  
  // Папка с файлами для рассвета
  cnt = 0, val = 0, new_val = 0; 
  do {
    val = dfPlayer.readFileCountsInFolder(2);     delay(10);
    new_val = dfPlayer.readFileCountsInFolder(2); delay(10);     
    if (val == new_val && val != 0) break;
    cnt++;
    delay(100);
    Serial.print(F("."));
  } while ((val == 0 || new_val == 0 || val != new_val) && cnt < 5);    
  dawnSoundsCount = val < 0 ? 0 : val;
  Serial.println();  
}
#endif

void PlayAlarmSound() {
  
  if (!isDfPlayerOk) return;

  #if (USE_MP3 == 1)  
  int8_t sound = alarmSound;
  // Звук будильника - случайный?
  if (sound == 0) {
    sound = random8(1, alarmSoundsCount);     // -1 - нет звука; 0 - случайный; 1..alarmSoundsCount - звук
  }
  // Установлен корректный звук?
  if (sound > 0) {
    dfPlayer.stop();
    delay(100);                              // Без этих задержек между вызовами функция dfPlayer приложение крашится.
    dfPlayer.volume(constrain(maxAlarmVolume,1,30));
    delay(100);
    dfPlayer.playFolder(1, sound);
    delay(100);
    dfPlayer.enableLoop();
    delay(100);    
    alarmSoundTimer.setInterval(alarmDuration * 60L * 1000L);
    alarmSoundTimer.reset();
    set_isPlayAlarmSound(true);
  } else {
    // Звука будильника нет - плавно выключить звук рассвета
    StopSound(1000);
  }
  #endif
}

void PlayDawnSound() {
  if (!isDfPlayerOk) return;

  #if (USE_MP3 == 1)
  // Звук рассвета отключен?
  int8_t sound = dawnSound;
  // Звук рассвета - случайный?
  if (sound == 0) {
    sound = random8(1, dawnSoundsCount);     // -1 - нет звука; 0 - случайный; 1..alarmSoundsCount - звук
  }
  // Установлен корректный звук?
  if (sound > 0) {
    dfPlayer.stop();
    delay(100);                             // Без этих задержек между вызовами функция dfPlayer приложение крашится.
    dfPlayer.volume(1);
    delay(100);
    dfPlayer.playFolder(2, sound);
    delay(100);
    dfPlayer.enableLoop();
    delay(100);
    // Установить время приращения громкости звука - от 1 до maxAlarmVolume за время продолжительности рассвета realDawnDuration
    fadeSoundDirection = 1;   
    fadeSoundStepCounter = maxAlarmVolume;
    if (fadeSoundStepCounter <= 0) fadeSoundStepCounter = 1;
    fadeSoundTimer.setInterval(realDawnDuration * 60L * 1000L / fadeSoundStepCounter);
    alarmSoundTimer.setInterval(4294967295);
  } else {
    StopSound(1000);
  }
  #endif
}

void StopSound(int duration) {

  if (!isDfPlayerOk) return;

  #if (USE_MP3 == 1)
  
  set_isPlayAlarmSound(false);

  if (duration <= 0) {
    dfPlayer.stop();
    delay(100);
    dfPlayer.volume(0);
    return;
  }
  
  // Чтение текущего уровня звука часто глючит и возвращает 0. Тогда использовать maxAlarmVolume
  fadeSoundStepCounter = dfPlayer.readVolume();
  if (fadeSoundStepCounter <= 0) fadeSoundStepCounter = maxAlarmVolume;
  if (fadeSoundStepCounter <= 0) fadeSoundStepCounter = 1;
    
  fadeSoundDirection = -1;     
  fadeSoundTimer.setInterval(duration / fadeSoundStepCounter);  
  
  #endif
}
