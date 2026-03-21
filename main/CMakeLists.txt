# See the build system documentation in IDF programming guide
# for more information about component CMakeLists.txt files.

idf_component_register(
    SRCS 
        "main.c"
        "WIFI.c"
        "CloudService.c"
        "RTUProcess.c"
        "Pump.c"
        "Heater.c"
        "TempratureSensor.c"
        "PressureSensor.c"
        "LevelSensor.c"
        "BoilerUnit.c"
    INCLUDE_DIRS 
        "."
)
