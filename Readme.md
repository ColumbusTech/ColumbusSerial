# cuserial

One-header C library for serial port IO.

The master is hosted on github: https://github.com/ColumbusUtrigas/cuserial

## Using
All you need is including **cuserial.h** in your project! Library does not require any compilation flags, other libraries or tons of code.

### Examples
```c
struct cuserial_t serial;
enum cuserial_status_t status;
memset(&serial, 0, sizeof(serial));

serial.baudrate = CUSERIAL_BAUDRATE_9600;
serial.timeout = 1000;
strcpy(serial.port, "COM5"); // or /dev/ttyS5 on Linux

char buf;

cuserial_connect(&serial, &status);
cuserial_write(&serial, "1", 1);
cuserial_read(&serial, &buf, 1);
cuserial_disconnect(&serial);

printf("%c\n", buf);
```

## Documentation
To generate documentation of llibrary use
```
doxygen
```
This command will generate documentation in docs

Or you can visit Github Wiki of library: TODO

## Contributing
TODO