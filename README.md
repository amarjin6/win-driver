# Windows driver 💻

## 📕 **Main Purpose** 📕

Develop a driver (virtual device) that monitors the startup of some process X. When this process starts, the driver starts another process Y. As soon as process X terminates for some reason, the driver terminates process Y.

## 📗 **Installation** 📗

* Clone project to your folder
  
        git clone https://github.com/amarjin6/win-driver.git

* Download [**Windows 10 ISO**](https://www.itechtics.com/?dl_id=133)
  
* Create on your VirtualBox **Host** and **Target** machines
  
* Install on **Host** machine [**Visual Studio**](https://download.visualstudio.microsoft.com/download/pr/1206a800-42a6-4dd5-8b7d-27ccca92e823/8958c61ec7143f12d457cc04bf40fdd97f837853da3b66f94276c88374d007fb/vs_Professional.exe)

* Install on both **Host** and **Target** VM [**WDK**]( https://go.microsoft.com/fwlink/?linkid=2085767
)
  
## 📘 **How to run** 📘

* Build cloned project in **Visual Studio** on your **Host** machine
  
* Add `UserMode.exe` file to your **Target** machine and Run
  
* Deploy solution to your **Target** machine with the help of [**Visual Studio tools**](https://learn.microsoft.com/en-us/windows-hardware/drivers/gettingstarted/writing-a-very-small-kmdf--driver)

## 📙 **How to use** 📙

* After driver deployed, Run `UserMode.exe` on **Target Machine**
  
* Check the Logs with [**DebugView**](https://learn.microsoft.com/en-us/sysinternals/downloads/debugview)
  
* If all works correctly, you can add any modifications, whatever you want

# Windows Driver C
