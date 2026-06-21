# Wrapper component to expose the local arduino component under the
# arduino-esp32 name expected by Adafruit_BusIO.
idf_component_register(
    REQUIRES arduino
)
