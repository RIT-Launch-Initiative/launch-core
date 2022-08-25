This is where platform independent peripherals can be defined.

They can implement or use top-level devices, but should have not be platform specific.

ex)
- you can implement an XBee peripheral that owns a StreamDevice (which is actually a UART)
- you can implement an XBee peripheral that IS a stream devices

Both of those implementations are valid as it's still not platform dependent and
can be ported to anything, as long as the peripheral is present.

This is also a place to put a peripheral that can be used to implement a top level
device. For example, a W5500 ethernet chip can use a platform independent "RegisterDevice"
to execute SPI commands, but the W5500 will ultimately implement a socket device.
