#ifndef __KEY_BOARD_H__

#define __KEY_BOARD_H__

/*shift键的按下和释放*/
#define press_left_shift                0x2a
#define press_right_shift              0x36
#define release_left_shift             0xaa
#define release_right_shift           0xb6

/*alt键的按下和释放*/
#define press_alt                           0x38
#define release_alt                        0xb8

/*ctrl键的按下和释放*/
#define press_ctrl                          0x1d
#define release_ctrl                       0x9d

/*Caps Lock*/
#define press_Caps                        0x3a
#define release_Caps                     0xba

/*Num Lock*/
#define press_Num                         0x45
#define release_Num                      0xc5

#define SHIFT_MODE          ( 1 << 0 )
#define ALT_MODE                        (1 << 1 )
#define CTRL_MODE                       ( 1 << 2 )
#define CAPS_MODE                      (1 << 3 )
#define NUM_MODE                        (1 << 4 )

#endif
