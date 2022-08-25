This is where devices should be implemented for specific platforms.

Devices should all implement a top level device type (stream, register, block, socket),
The end application should be platform independent and can use these devices as if
they were a top-level device, regardless of actual implementation.
