Kako dodati Etherhet/Lcd okolje ()Ethernet example)
_____ v example projektu__________________
- delete readonly property
- dodaj  path na ..\..\..\..\Libraries\CMSIS\Device\ST\STM32F4xx\Include (kiks v projektu...??)
- rename main v nekaj drugega
- set output na lib
- od/zakomentiraj USE_DHCP in USE_LCD
- v ethernet examplu zamenjaa target na 205 in prevedes


USE_STDPERIPH_DRIVER,STM32F2XX,USE_USB_OTG_FS,__PFM6__,HSE_VALUE=25000000
USE_STDPERIPH_DRIVER,STM32F2XX,USE_USB_OTG_FS,__DISCO__,HSE_VALUE=8000000

14.2.2016

FIL iz stdin,stdout v IO !!!
ungetch, ungets
batch, ungets na startu...
_io_init v initVCP
+/-L lcd output 