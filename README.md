# DS_deyellower
Blast your Nintendo ~3~DS(i) screens with white for hours and pray that you don't destroy it.  
Inspired by (and above line stolen from) [3Deyellower](https://github.com/ManiacOfGitHub/3Deyellower)


# Features
This application allows to set timers for an initial screen on phase (default 4 hours), a screen off phase after the last to let the screen rest and hopefully reduce the chances of damage (default 10 minutes), and how many times to repeat the whole process (default 12 times).  
It's also possible to have only one screen on\* when the process if running.  
The console will turn off automatically after the last screen on phase is finished.  
Sleep mode will be disabled once the process is started, allowing to close the lid without interrupting it.  

\*in reality, the other screen will be on, but have it's backlight turned off, as it doesn't seem to be possible to completely turn off only one screen; in the screen off phase both screens will actually be off though.


## Experimental features
This application features a white screen mode, and an experimental mode with cycling colors that I came up with after some observations I made, which I'll copy-paste here:
> After noticing my DSi's screens started to yellow from the corners and sides, I tried using leaving it on for 2.5 days on a white screen.  
> While that did help a bit, I was starting to get worried about leaving the console on at max brightness, so I stopped; it was also apparently reyellowing a bit after leaving it off for the night.  
> But then the next day, after using the console for an afternoon to play games, I noticed that the yellowing was noticeably reduced.  
>Because of this, I started thinking that changing colors would be more effective compared to a static white screen.  

This application also allows to set any backlight level (including turning off the backlights completely) instead of only the maximum.  
As part of this, it will try to detect if it's running on a DSi or a DS; avoid running the application in DS mode on a DSi or it will misidentify which console it's running on (but even if that happens, the maximum and off backlight levels will work as expected).

Both of these features are experimental, as the effectiveness of the cycling colors and/or lower backlight levels in different circumstances have to be tested and validated.

# Usage
For nativating the menus, follow the on-screen instructions.  
Once the process is started, hold dpad up to temporarly show a blue screen at maximum brightness, right for yellow, down for white, and left for black; hold A to print the remaining time and number of repetitions.  
When the screens or backlight are off, the screens/backlight will temporarly turn back on when a button is being held, but please avoid doing it repeatedly and/or rapidly during the screen off phase as it's probably not the best for the screens and nullifies the whole purpose of letting the screens rest.  
It is possible to disable the screen off phase by setting it's duration to 0, but **this is not generally recommended especially at higher backlight levels**.

# Epilepsy warning
The cycling colors mode features rapidly fading colors.
Please take the necessary precautions, or avoid using the mode yourself, if you're sensitive to rapidly flashing/fading lights.  

# Disclaimer
There's still a lot of unknown things regarding yellow NDS(i) screens, the causes, and the processes to potentially revert it.  
This program, and any other method of deyellowing screens by leaving them on for a long time, are not guaranteed to work for everyone (or to be completely effective), especially for more serious cases.  
Leaving the console on for a prolonged period of time, especially at maximum brightness, can and has been known to kill screens, and will potentially reduce the lifespan of them.  
It is not guaranteed that the screen of phases are enough to prevent damage to the screens, and the ideal durations of them is so far unknown.  
Leaving the console on and plugged to a charger for a prolonged period of time will also definitely shorten the battery's lifetime and it's capacity to hold a charge.  
Understand and accepts these risks before trying to deyellow screens, with this application or in general.  
**I'm not responsible for damaged or broken displays, batteries, or other components caused by leaving this application running for long periods of time.**  

# Test mode
A test mode is present that allows manually change the backlight level and turn the screens on or off.  
**This test mode is not meant as a replacement for the normal deyellowing process. Avoid rapidly and/or repeatedly turning the screens on and off, and leaving them on for long periods of time. I'm not responsible for damage caused by the misuse of this mode.**  
To access this mode, press L+R+left+Y+Select on the main menu.  
Hold L and press up to turn the top screen backlight on, or down to turn it off; hold R for the bottom screen instead (calls [`powerOn`/`powerOff`](https://blocksds.skylyrac.net/libnds/system_8h.html#a033c884f09d843121781af8bc212f8ce) with argument [`PM_BACKLIGHT_TOP`/`PM_BACKLIGHT_BOTTOM`](https://blocksds.skylyrac.net/libnds/system_8h.html#a033c884f09d843121781af8bc212f8ce)).  
Hold Y and press up to turn on the screens, or down to turn them off (calls [`powerOn`/`powerOff`](https://blocksds.skylyrac.net/libnds/system_8h.html#a033c884f09d843121781af8bc212f8ce) with argument [`POWER_LCD`](https://blocksds.skylyrac.net/libnds/system_8h.html#a033c884f09d843121781af8bc212f8ce)).  
Hold X and press up to set backlight level to 0, right for 1, down for 2, left for 3, A for 4, B for 5 (calls [`systemSetBacklightLevel`]([`PM_BACKLIGHT_TOP`/`PM_BACKLIGHT_BOTTOM`](https://blocksds.skylyrac.net/libnds/system_8h.html#a033c884f09d843121781af8bc212f8ce)) with the corresponding argument).