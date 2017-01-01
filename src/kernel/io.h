#ifndef INCLUDE_IO_H
    #define INCLUDE_IO_H

    /** outb:
     *  Sends the given data to the given I/O port. Defined in io.s
     *
     *  @param port The I/O port to send the data to
     *  @param data The data to send to the I/O port
     */
    void write_port(unsigned short port, unsigned char data);

    #endif /* INCLUDE_IO_H */
