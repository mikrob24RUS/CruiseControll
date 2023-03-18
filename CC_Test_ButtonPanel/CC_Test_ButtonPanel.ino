int analogPinButtonPanel = 3; // номер аналогового порта сигналов с панели кнопок, например порт A3 (на Ардуино) - 3
int analogPinLastValue = 0; // Последнее значение с кнопок
int analogPinValue = 0; // подтверждённое значение с кнопок (может и не нужно будет)
unsigned long nextWaitPress = 0; // следующее нажатие не раньше
int beetwenMinDelayMs = 250; // промежуток между последовательными нажатиями в милисек
int calcSlidePressDelay = 1000; // промежуток после которого фиксируем кол-во последовательных нажатий
int countSlidePress = 0; // кол-во последовательных нажатий в установленных интервалах
int btn1Value=253; // включает КК с текущей скоростью или выключает КК
int btn2Value=509; // скорость + (при включенном КК, увеличивает заданную скорость ~ на 2 км/ч, 
                  //при выключенном — включает КК с ранее установленной, к примеру ехали по трассе с КК на 90 км/ч, 
                  //отключили по той или иной причине (помеха, перекрёсток, развязка и т.п.) и нажав на эту кнопку 
                  //снова включается КК с заранее заданной скоростью 90 км/ч)
int btn3Value=768; // скорость — (при включенном КК снижает установленную скорость на 2 км/ч)
int btn4Value=1022; // кнопка для скоростей 60/90/110 (соответственно одно нажатие — 60, два нажатия — 90, три нажатия — 110)
int btnDeltaValue=30; // дельта, в пределах которой будет "подтягиваться" значение до ближайшей кнопки

void setup() {
  Serial.begin(9600);
}

void loop() {  
   int value = analogRead(analogPinButtonPanel);
   if ((value > analogPinLastValue + btnDeltaValue ||  value < analogPinLastValue - btnDeltaValue) && (millis() > nextWaitPress))
   {
    if (value > 10) {
      nextWaitPress = millis() + beetwenMinDelayMs;
      analogPinLastValue = value;
      // проверяем не та ли кнопка с которой допустимо несколько последовательных нажатий
      if (analogPinLastValue < btn4Value + btnDeltaValue && analogPinLastValue > btn4Value - btnDeltaValue) {
        countSlidePress++;
        if (countSlidePress > 3) { countSlidePress = 3; }
      } else { analogPinValue = analogPinLastValue; }

      if (countSlidePress == 0)
        Serial.println(value);
    }
    else { analogPinLastValue = 0; }
   }
   
   if (countSlidePress > 0 && millis() > nextWaitPress + calcSlidePressDelay) {
    Serial.println(countSlidePress);
    countSlidePress = 0;
   }
}
