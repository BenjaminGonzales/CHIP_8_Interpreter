//
// Created by Benji on 7/24/2025.
//

#ifndef DISPLAY_H
#define DISPLAY_H

// basic display
typedef struct Display display_t;

// constructor & descructor
display_t *p_display_init(void);
void v_display_destroy(display_t *display);

// functions
int clear_screen(display_t *display);
int draw_internal(display_t *display, int x, int y);
int draw(display_t *display);

#endif //DISPLAY_H

