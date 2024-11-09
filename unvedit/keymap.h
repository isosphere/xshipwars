#ifndef KEYMAP_H
#define KEYMAP_H

/*
 *      Keymap codes:
 *
 *      Identifies the keymap code which corresponds to its index
 *      number in the keymap structure.
 */
#define KM_VIEW_MOVE            0       /* Move selected object. */
#define KM_VIEW_TRANSLATE       1
#define KM_VIEW_ZOOM            2

static char *keymap_name[] = {
        "ViewMove",
        "ViewTranslate",
        "ViewZoom"
};
 
#define TOTAL_KEYMAPS   (sizeof(keymap_name) / sizeof(char *))
        
   
/* Keymap structure. */
typedef struct {
        
        keycode_t keycode;
        
} keymap_struct;
extern keymap_struct keymap[TOTAL_KEYMAPS];


#endif	/* KEYMAP_H */
