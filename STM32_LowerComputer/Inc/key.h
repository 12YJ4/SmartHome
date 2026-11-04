#ifndef KEY_H
#define KEY_H

#define KEY_NONE 0
#define KEY_DOWN 1
#define KEY_LEFT 2
#define KEY_UP 3
#define KEY_SEL 4

void KeyInit(void);
unsigned int GetKey(void);

#endif
