#include "io.h"
#include "ports.h"

    /* The I/O ports */
    #define FB_COMMAND_PORT         0x3D4
    #define FB_DATA_PORT            0x3D5

    /* The I/O port commands */
    #define FB_HIGH_BYTE_COMMAND    14
    #define FB_LOW_BYTE_COMMAND     15

    /** fb_move_cursor:
     *  Moves the cursor of the framebuffer to the given position
     *
     *  @param pos The new position of the cursor
     */
    void fb_move_cursor(unsigned short pos)
    {
        outb (FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
        outb (FB_DATA_PORT,    ((pos >> 8) & 0x00FF));
        outb (FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
        outb (FB_DATA_PORT,    pos & 0x00FF);
    }
