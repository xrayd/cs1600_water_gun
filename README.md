# cs1600_water_gun
Code governing our water gun for the CS1600 final project! 

# Requirements
C++ code requires Adafruit Sleepy Dog library and Blynk.

# Directories and Files
* `deprecated_site`
  * Contains all Python files used to create a website so we can control our gun from the web.
  * Ultimately could not use because when deployed to Streamlit, Cloudflare blocked our connection
  * Also, site only shows changes locally, not on the entire site when you make a new GET request
* `watergun`
  * Arduino Project containing all necessary files for running the gun
  * `watergun.ino`
    * Methods for handling connection to Blynk (IoT software) such that we can send/receive values with an iPhone!
    * Able to receive data for when to shoot the gun, then act on it to actually shoot the gun
    * Contains all methods interfacing between code and the physical world
    * Handles plate rotation
    * Handles physical water gun shooting
    * Handles resetting the system to original state if inactive for 30 seconds
    * Watchdog timers to securely pull/unpull the trigger
