STM32 BootLoader, document by http://www.st.com/st-web-ui/static/active/en/resource/technical/document/application_note/CD00167594.pdf

The instructions are detailed more fully here:
http://leaflabs.com/docs/bootloader.html#flashing-a-custom-bootloader 

Board: MiCOKit-3288 by: http://mico.io
  CPU: stm32f411 

Base on mbed(CMSIS) 20150707

Complier: gcc-arm-none-eabi-4_9-2015q2, information from espruino
Debug SWD: stlink-v2

CMD:

```
#make
#make flash
#cutecom 
```

serial: 115200,8N1
LED is GPIOB-12
default USART: usart2


```
tools/stm32loader.py
```


 
good luck.

