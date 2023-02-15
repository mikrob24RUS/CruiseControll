// #include <IRremote.h>

// int IRPin = 12; // порт инфракрасного приёмника
// IRrecv irrecv(IRPin);
// decode_results results;

unsigned long micros_sp = 0;
volatile float sp; // мгновенная скорость
volatile float spAVG = 0; // средняя скорость

float CCSpeedDelta = 0.5; // отклонение от скорости КК для реагирования

int analogPinButtonPanel = 4; // номер аналогового порта сигналов с панели кнопок

int sensorBrPin = 4; // сигнал с педали тормоза
int hallMinPin = 5; // Датчик Холла минимум
int hallMaxPin = 6; // Датчик Холла максимум
int motorPinF = 7; // управление двигателем 1 канал
int motorPinR = 8; // управление двигателем 2 канал
int enabledMotor = 9; // разрешающий сигнал драйверу мотора
int ledHallMaxPin = 10; // светодиод CC_FRW
int buzzerPin = 11; // динамик
int ledHallMinPin = 13; // светодиод CC_REV
int ledCCOnPin = 14; // светодиод CC_On

int acceleratorPin = 5; // сигнал с датчика БДЗ (аналоговый)

boolean ccActive = 0; // активность КК
int timePeriod = 1000;
unsigned long timeMotorStop = 0; // время остановки мотора
int SpeedFail = 0; // счётчик нарушений скорости для безопасности
int SpeedReduction = 0; // счётчик кол-ва последовательного снижения скорости
//int SpeedReduction2 = 0; // счётчик кол-ва последовательного снижения скорости
int SpeedIncrease = 0; // счётчик кол-ва последовательного увеличения скорости
float CCSpeed = 0; // скорость КК
const int CCSpeedMin = 5; // 
const int CCSpeedMax = 5; //
int CurrDZ;
int DZPosMin; // минимальное положение ДЗ ~95
int DZPosMax; // максимальное положение ДЗ ~750
float OldSpeed = 0; // скорость прошлого замера
float SpeedUp = 0; // ускорение
//float SpeedOld = 0; // старая скорость в расчёте ускорения
float CalcSpeedUp = 0; // расчётное ускорение
int CSDK = 15; // коэффициент резкости расчётного ускорения (рекомендуется от 15 до 30, чем больше тем резче разгон)

unsigned long timeOldSpeed = 0; // время замера прошлой скорости

void myTone(int hertz,int longs) {
  unsigned long startTime = millis() + longs;
  float period = 1000000 / hertz;
  while (millis() <= startTime)
  {
    digitalWrite(buzzerPin, HIGH);
    delayMicroseconds(period / 2);
    digitalWrite(buzzerPin, LOW);
    delayMicroseconds(period / 2);
  }
}

void speedometr() { // Определяем текущую скорость
  sp = (float)(1500000.0 / (micros() - micros_sp)); //600000.0 - 6 имп./м  1000000.0 - 10 имп./м и т.д
  micros_sp = micros();
}

void setup() {
  pinMode(motorPinF, OUTPUT);
  digitalWrite(motorPinF, LOW);

  pinMode(motorPinR, OUTPUT);
  digitalWrite(motorPinR, LOW);

  pinMode(enabledMotor, OUTPUT);
  digitalWrite(enabledMotor, HIGH); // подаём разрешающий сигнал на драйвер

  pinMode(hallMinPin, INPUT);
  digitalWrite(hallMinPin, HIGH);

  pinMode(hallMaxPin, INPUT);
  digitalWrite(hallMaxPin, HIGH);

  pinMode(sensorBrPin, INPUT);
  digitalWrite(sensorBrPin, HIGH);

  pinMode(buzzerPin, OUTPUT);
  pinMode(ledCCOnPin, OUTPUT);
  pinMode(ledHallMaxPin, OUTPUT);
  pinMode(ledHallMinPin, OUTPUT);

  digitalWrite(2, HIGH);
  attachInterrupt(0, speedometr, RISING); // Прерывание INT0 на 4 ноге ATMega328. Подаем сигнал с датчика скорости, ограничив амплитуду сигнала до 4,5 В 
                                          // делителем или оптопарой и притянув вход к земле
  //  attachInterrupt(1, multiINTO, LOW); // Прерывание INT0 на 5 ноге ATMega328. Подаем сигнал с датчиков Hall (max, min), break, Sens_A/B (вызывает прерывание, когда на порту LOW)

  analogReference(DEFAULT); // пока считаем, что диапазон измеряемых значений от ДЗ от 0 до +5В, потом возможно придётся разбираться!

//  Serial.begin(9600);

  irrecv.enableIRIn(); // Start the receiver
  spAVG = 20;
}

void motorMode(int mode) { // управляем приводом. задаём направление вращения или выключаем
  // останавливаем мотор
  digitalWrite(motorPinF, LOW);
  digitalWrite(motorPinR, LOW);
  switch (mode) {
    case 0: // остановить мотор
      // мотор остановлен на входе
      break;
    case 1:  // крутим вперёд
      if (digitalRead(hallMaxPin) != 0) {
        digitalWrite(motorPinF, LOW);
        digitalWrite(motorPinR, HIGH);
      }
      break;
    case 2:  // крутим назад
      if (digitalRead(hallMinPin) != 0) {
        digitalWrite(motorPinF, HIGH);
        digitalWrite(motorPinR, LOW);
      }
      break;
  }
}

void buzzGo(int mode) {
  switch (mode) {
    case 1: // получен известный код IR приёмником
        myTone(1500, 100);      
      break;
    case 2:
        myTone(1500, 100);      
      break;
    case 5: // включаем КК
      break;  
    case 9: // левый код IR приёмника
      break;
    case 10: // отключение КК
      break;  
  }
}

int ReadDZ_AVG(){
  int i;
  int sval = 0;
 
  for (i = 0; i < 100; i++){
    sval = sval + analogRead(acceleratorPin);
  }
 
  sval = int(sval / 100);    // среднее
  return sval;
}

float ReadSP_AVG() {
  float ssp;
  int i = 0;
  float sval = 0;
  unsigned long timeEnd;

  timeEnd = millis() + 100;

  while (millis() < timeEnd) {
    ssp = sp;
    if (ssp < 200) {
      sval = sval + ssp;
      i = i + 1;
    }
  }
 
  sval = sval / i;    // среднее
  return sval;
}

void ccOFF() {  // отключаем КК, сбрасываем газ
  motorMode(0);
  ccActive = false;  // снимаем статус активности КК
  timeMotorStop = millis() + 10000;

  motorMode(2);
  while (digitalRead(hallMinPin) != 0 && millis() < timeMotorStop) { //пока ДЗ не встанет в минимум вращаем в обратку привод или временное ограничение 5 секунд
  //  delay(3);
  }
  motorMode(0);
//  buzzGo(10);
}

void setDZ(int tmpDZ) { // Устанавливаем ДЗ в конкретное положение
  timeMotorStop = millis() + 2000;
  if (ReadDZ_AVG() < tmpDZ){
    while (digitalRead(hallMinPin) == 0 && millis() < timeMotorStop){ // если привод в минимуме вытягиваем провисание
      motorMode(1);
      delay(16);
      motorMode(0);  // пауза для плавности восстановления дросселя
      delay(16); // задержка для плавности восстановления дросселя
    // контроль вмешательства человека в управление дросселем
      if ((digitalRead(hallMinPin) == 0 && digitalRead(motorPinF) == 1 && digitalRead(motorPinR) == 0) || (digitalRead(hallMaxPin) == 0 && digitalRead(motorPinF) == 0 && digitalRead(motorPinR) == 1)) {
        motorMode(0);
        return;
      }
      if (digitalRead(sensorBrPin) == 0) { //нажата педаль тормоза отключаем КК
        ccOFF();
        return;
      }      
    }
    timeMotorStop = millis() + 2000;
    while (digitalRead(hallMaxPin) != 0 && ReadDZ_AVG() < tmpDZ && millis() < timeMotorStop) { // временное ограничение 5 секунд
//      Serial.println(ReadDZ_AVG());
      motorMode(1);
      delay(8);
      motorMode(0);  // пауза для плавности восстановления дросселя
      delay(55); // задержка для плавности восстановления дросселя
    // контроль вмешательства человека в управление дросселем
      if ((digitalRead(hallMinPin) == 0 && digitalRead(motorPinF) == 1 && digitalRead(motorPinR) == 0) || (digitalRead(hallMaxPin) == 0 && digitalRead(motorPinF) == 0 && digitalRead(motorPinR) == 1)) {
        motorMode(0);
        return;
      }
      if (digitalRead(sensorBrPin) == 0) { //нажата педаль тормоза отключаем КК
        ccOFF();
        return;
      }
    }  
  }
  else
  {
    timeMotorStop = millis() + 2000;
    while (digitalRead(hallMinPin) != 0 && ReadDZ_AVG() > tmpDZ && millis() < timeMotorStop) { // временное ограничение 5 секунд
      motorMode(2);
      delay(5);
      motorMode(0);  // пауза для плавности
      delay(55); // задержка для плавности
      // контроль вмешательства человека в управление дросселем
      if ((digitalRead(hallMinPin) == 0 && digitalRead(motorPinF) == 1 && digitalRead(motorPinR) == 0) || (digitalRead(hallMaxPin) == 0 && digitalRead(motorPinF) == 0 && digitalRead(motorPinR) == 1)) {
        motorMode(0);
//        buzzGo(9);
        return;
      }
      if (digitalRead(sensorBrPin) == 0) { //нажата педаль тормоза отключаем КК
        ccOFF();
        return;
      }        
    }  
  }
  motorMode(0);
}

void ccON() {
  int ssp;
  ssp = sp;
  if (ssp > 20 && ssp < 130) {
    SpeedReduction = 0;
    OldSpeed = ssp; // запоминаем текущ. скорость как старую
    timeOldSpeed = millis();
    spAVG = ssp;
  
//    SpeedOld = ssp;
    
    // управляем ledCCOnPin
    digitalWrite(ledCCOnPin, HIGH);

    setDZ(CurrDZ + 5);
    ccActive = true;
    buzzGo(5);
  }
  else
    buzzGo(10);
}

void loop() {  
  spAVG = ReadSP_AVG();

  CurrDZ = ReadDZ_AVG();
  if (DZPosMin < 20) {
    DZPosMin = CurrDZ;
  }

  if ((millis() > 900) && (millis() < 1100)) {
    buzzGo(1);
  }

  if (irrecv.decode(&results)) {
  // 0xFD40BF - CH+      *
  // 0xFD807F - CH-      *
  // 0xFD20DF - EQ       *
  // 0xFD50AF - >>|      *
  // 0xFD906F - |<<      *
  // 0xFDA05F - V-
  // 0xFD609F - V+
  // 0xFD00FF - >/||
  // 0xFDE21D - call off
  // 0xFDA25D - call on
  // 0xFD9867 - power  *
    switch (results.value) {
      case 0xFD9867: // кнопка калибровки // 0xFD9867 - power
        buzzGo(1);
        ccOFF();
        break;

      case 0xFD20DF: // включаем/выключаем круиз // 0xFD20DF - EQ
        buzzGo(1);
        if (!ccActive) {
          ccON();
          CCSpeed = spAVG;
        }
        else {
          ccOFF();
        }
        break;

      case 0xFD50AF: // Увеличить скорость КК на 4 // 0xFD50AF - >>|
        buzzGo(1);
        if (ccActive) {
          CCSpeed = CCSpeed + 4;
          if (spAVG + 4 < CCSpeed)
          {
            setDZ(CurrDZ + int(spAVG/4));
          }
        }
        else {
          ccON();
        }
        break;

      case 0xFD906F: // Уменьшить скорость КК на 4 // 0xFD906F - |<<
        buzzGo(1);
        if (ccActive) {
          CCSpeed = CCSpeed - 4;
          if (spAVG - 4 > CCSpeed)
          {
            setDZ(CurrDZ - int(spAVG/4));
          }
        }
        break;

      case 0xFD807F: // Увеличить скорость КК на 10 // 0xFD807F - CH-
        buzzGo(1);
        if (ccActive) {
          CCSpeed = CCSpeed + 10;
          if (spAVG + 10 < CCSpeed)
          {
            setDZ(CurrDZ + int(spAVG/2));
          }
        }
        break;

      case 0xFD40BF: // Уменьшить скорость КК на 10 // 0xFD40BF - CH+
        buzzGo(1);
        if (ccActive) {
          CCSpeed = CCSpeed - 10;
          if (spAVG - 10 > CCSpeed)
          {
            setDZ(CurrDZ - int(spAVG/2));
          }
        }
        break;
      //////////////////////////////////////////
      case 0xFDA05F: //  - V- включить КК и скорость 40
        buzzGo(1);
        if (!ccActive) {
          ccON();
        }
        CCSpeed = 42;
        break;

      case 0xFD609F: //  - V+ включить КК и скорость 60
        buzzGo(1);
        if (!ccActive) {
          ccON();
        }
        CCSpeed = 64;
        break;

      case 0xFD00FF: //  - >/|| включить КК и скорость 90
        buzzGo(1);
        if (!ccActive) {
          ccON();
        }
        CCSpeed = 96;
        break;

      case 0xFDA25D: // "call on" просто прибавить дроссель даже без КК
        buzzGo(1);
        setDZ(CurrDZ + 8);
        ccActive = false;
        break;

      case 0xFDE21D: // "call off" просто убавить дроссель даже без КК
        buzzGo(1);
        setDZ(CurrDZ - 8);
        ccActive = false;
        break;
        
        //  default:
        //  buzzGo(9);
    }
    delay(50);
    irrecv.resume();// Receive the next value
  }

  if ((ccActive || digitalRead(hallMinPin) != 0) && digitalRead(sensorBrPin) == 0) { //нажата педаль тормоза отключаем КК
    ccOFF();
  }

  if (ccActive) {
    if ((spAVG < 20) || (spAVG > 130)) {// если скорость за пределами 20..130 отключаем КК
      SpeedFail = SpeedFail + 1;
      if (SpeedFail > 5)
      {
        ccOFF();
      }
    }
    else
    {
      SpeedFail = 0;
    }
  }

  if (ccActive) {
    if (spAVG < CCSpeed - CCSpeedMin) {// скорость сильно ниже заданной
      digitalWrite(ledHallMinPin, HIGH);
      digitalWrite(ledCCOnPin, LOW);
      digitalWrite(ledHallMaxPin, LOW);      
    }     
    else
    if (spAVG > CCSpeed + CCSpeedMax){// скорость сильно выше заданной
      digitalWrite(ledHallMinPin, LOW);
      digitalWrite(ledCCOnPin, LOW);
      digitalWrite(ledHallMaxPin, HIGH);
    }
    else
    {
      digitalWrite(ledHallMinPin, LOW);
      digitalWrite(ledCCOnPin, HIGH);
      digitalWrite(ledHallMaxPin, LOW);
    }
  }
  else
  {
    digitalWrite(ledHallMinPin, LOW);
    digitalWrite(ledCCOnPin, LOW);
    digitalWrite(ledHallMaxPin, LOW);
  }

  if (ccActive)
  {// проверяем включен ли КК

    if ((spAVG < CCSpeed - CCSpeedMin || spAVG > CCSpeed + CCSpeedMax) && millis() >= timeOldSpeed + timePeriod)
      {// скорость за пределами нормы и настало время для след.прохода
      
      // расчёт ускорения
      SpeedUp = (spAVG - OldSpeed)/((millis() - timeOldSpeed)/1000);

      // вычисляем рачётное ускорение    
      CalcSpeedUp = ((CCSpeed - spAVG) / (CCSpeed / CSDK));
//      CalcSpeedUp = ((CCSpeed - spAVG) / (CCSpeed / CSDK)) * 1000 / (millis() - timeOldSpeed);
  
      if (spAVG < CCSpeed - CCSpeedMin) {// скорость ниже заданной
        if (SpeedUp > 0) {// если скорость в верном направлении
          if (SpeedUp < CalcSpeedUp - CalcSpeedUp/5) { // если ускорение не достаточное
            setDZ(CurrDZ + int((CalcSpeedUp - SpeedUp) * 15));      
//            setDZ(CurrDZ + int((CCSpeed - spAVG)/2 + spAVG/8));      
          }// *если ускорение не достаточное
          else 
          if (SpeedUp > CalcSpeedUp + CalcSpeedUp/5 && SpeedUp > 1) { // если ускорение черезмерное
            setDZ(CurrDZ - 6);                      
//            setDZ(CurrDZ - 4 - int(abs(SpeedUp)));                      
          }// *если ускорение черезмерное      
        }// *если скорость в верном направлении
        else
        {// если происходит снижение скорости
          setDZ(CurrDZ + int(spAVG * 3));
        }// *если происходит снижение скорости
      }// *скорость сильно ниже заданной
      else
      if (spAVG > CCSpeed + CCSpeedMax){// скорость сильно выше заданной
        if (spAVG > CCSpeed + 10) {// если скорость критически высока снижаем до минимума
          setDZ(DZPosMin);
        }// *если скорость критически высока снижаем до минимума
        else
        {// скорость сильно выше заданной, но не выше критической, продолжаем анализировать
          if (spAVG < OldSpeed) {// если уже происходит снижение скорости
            if ((spAVG < CCSpeed + CCSpeedMax + 1) && ((digitalRead(hallMinPin) == 0) || (OldSpeed - spAVG > 2))) { // снижая скорость приближаемся к норме
              setDZ(CurrDZ + int(spAVG/4));
            } // *снижая скорость приближаемся к норме
          }// *если уже происходит снижение скорости
          else
          if (spAVG > OldSpeed) { // если происходит увеличение скорости
            setDZ(CurrDZ - 4 - int(14 * abs(SpeedUp)));
          } // *если происходит увеличение скорости
        }// *скорость сильно выше заданной, но не выше критической, продолжаем анализировать     
      }// *скорость сильно выше заданной
      // запоминаем скорость как старую
      OldSpeed = ReadSP_AVG();
      timeOldSpeed = millis();      
    } // *скорость за пределами нормы и настало время для след.прохода
    else // скорость в пределах нормы
    if ((spAVG > CCSpeed - CCSpeedMin && spAVG < CCSpeed + CCSpeedMax) && (spAVG < OldSpeed - CCSpeedDelta || spAVG > OldSpeed + CCSpeedDelta))
    {// скорость отклонилась на x км/ч
      if (spAVG < OldSpeed) {// снизилась
        if (spAVG > CCSpeed) {
          SpeedReduction = 1;
        }
        else
        {
          SpeedReduction = int(CCSpeed - spAVG) + 1;
        }

        if (spAVG < CCSpeed && SpeedUp < -0.3) {
          SpeedReduction = SpeedReduction + 1;
        }
        // расчёт ускорения
        SpeedUp = (spAVG - OldSpeed)/((millis() - timeOldSpeed)/1000);
        if (spAVG < CCSpeed && SpeedUp < -0.3) {
          SpeedReduction = SpeedReduction + 1;
        }

        int frw;
        frw = int(spAVG / 8);
//          frw = int(spAVG * abs(SpeedUp) * 2.5);
        if (frw < 4) frw = 4;
        frw = frw  * SpeedReduction;
        setDZ(CurrDZ + frw);

/*        Serial.print(CCSpeed);
        Serial.print("-");
        Serial.print(spAVG);
        Serial.print("-");
        Serial.print(frw);
        Serial.print("-");
        Serial.println(SpeedUp);*/
      }// *снизилась
      if (spAVG > OldSpeed) {// увеличилась
        if (spAVG < CCSpeed) {
          SpeedIncrease = 1;
        }
        else
        {
          SpeedIncrease = int(spAVG - CCSpeed) + 1;
        }
        int rev;
        rev = 6;
//          rev = int(spAVG * abs(SpeedUp) / 4);
        if (rev < 6 ) { rev = 6; }
        rev = rev  * SpeedIncrease;
        setDZ(CurrDZ - rev);
      }// *увеличилась     
  
//        delay(500);
      // запоминаем скорость как старую
      OldSpeed = ReadSP_AVG();
      timeOldSpeed = millis();
      }// *скорость отклонилась на 1 км/ч
    // *скорость в пределах нормы              
  }// *проверяем включен ли КК и настало ли время для след.прохода
}
