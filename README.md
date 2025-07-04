# Dota-2-Buyback-Timers
Custom keypad with ESP32S3 to track buyback status (8-minute cooldown) of 5 heroes on the enemy team. Using ESP-IDF, FreeRTOS, and LVGL. 

## Problems & Solutions
In this project, I used an ESP NANO by Arduino, which has a UBLOX chip that contains an ESP32S3. When I created this project, I wanted a responsive and easy-to-read display, so I decided to use a 3.5-inch, 320x480 pixels display from Waveshare that uses an ST7796S Display Driver. I worked on a previous project that I abandoned, which was a fitness tracker that involved using a small round display (by Waveshare). This display used a GC9A01 display driver. I bring this up because I was able to get the display to work and run a few LVGL examples on it, so I used it as a reference for when I went to implement LVGL into this project.

To cut down on time for the display portion of that project, I looked at some initialization code for that display driver from an Arduino library. When I wrote initialization code for the ST7796S driver, it didn't seem to work at all. I spent a few weeks looking at an Arduino TFT library as I was able to get the display to work on an Arduino UNO, but not when I tried to translate it over to an ESP32S3. As a last-ditch effort, shot in the dark, I decided to copy and paste the initialization code from the GC9A01 display driver from the other project I mentioned, into this project when I was working on the display section, and for some reason it just worked (???). I'm not sure if the display driver they listed was incorrect or what, but that was a major roadblock in this project, getting the display to initialize. I originally thought that maybe my SPI code was incorrect, but I learned that it was not the case. Instead, using the display initialization code from a previous project was the solution. 

The next roadblock I had was creating a custom LVGL screen to suit my use case. My experience with LVGL at the start of this project was just getting it working by understanding how it requires custom functions to be written that communicate with the hardware (display) being used, and then once those functions are added, you can start running their widget examples. I looked at the tabview example since it seemed the most fitting for my purpose on the type of data I wanted to display. I initially tried to create my own timers using the ESP32S3 timer API but this was resulting in the watch dog timer tripping because it was conflicting with the way that the LVGL tick handler works. So after looking through the LVGL documentation I saw that it included its own timers that can be called and appeared to be the proper way of updating the display and its widgets as you can update text by simply pulling in variables that are updated elsewhere such as in a task that handles a specific key being pressed. 

## Summary
This project is still ongoing. A few changes need to be done such as the direction of the ESP Nano on the board (USB-C is in the way of two keys currently). The project is built using a custom PCB that allows 11 keys that the user can press to configure the device to start various timers related to the game Dota 2. In the game Dota 2 both teams can use gold acquired during the match to "buyback" which allows their character to respawn instantly (if already dead). When this is used, that player is unable to "buyback" again until 8 minutes have passed in-game. Since the game can be very fast-paced, it can be difficult to keep track of the cooldown of each player on the enemy team who has used their "buyback." The purpose of this project is to create a device that can easily keep track of up to 5 individual, 8-minute timers. There is functionality that allows the user to pause the timer, since in-game, either team can pause the game. And just incase there's further complication, while the in-game timer is paused on the device, the user can also increment or decrement the in-game timer while updating any active "buyback" timers. The YouTube short link demonstrates this device in action: https://www.youtube.com/shorts/Tls8hikkFVM

## Keys
The keys will be referred as shown:

```
(K1) (K2) (K3) (K4) (K5)
(K6) (K7) (K8) (K9) (K10)
          (K11)

K10 - Officially starts/creates the in-game timer
K6 - Deletes the in-game timer
K8 - Pauses/Resumes the in-game timer
K7 - Decrements the in-game timer
K9 - Increments the in-game timer
K1 - Starts the 8 minute timer for enemy player #1
K2 - Starts the 8 minute timer for enemy player #2
K3 - Starts the 8 minute timer for enemy player #3
K4 - Starts the 8 minute timer for enemy player #4
K5 - Starts the 8 minute timer for enemy player #5
K11 - Toggles between the three tabs (Only using tab #3 for now)
```



The main functionality of this project is completed. 

PCB Model:
![PCB Timer](https://github.com/user-attachments/assets/d78664d9-8c9e-4968-bb70-12cdcbcb1291)

Below are images of the display: 
![20250629_022920](https://github.com/user-attachments/assets/be1462a4-fd5f-4fdf-97cb-de46bb5e961d)
![image](https://github.com/user-attachments/assets/c861952d-e247-4f1a-9362-aaf1363e0960)
