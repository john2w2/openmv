// Include MicroPython API.
#include "py/runtime.h"
#include "py/obj.h"
#include <stdint.h>
#include <stddef.h>
 
// We can write a function which will be called from Python as -function_name()- not sure if we still need this
// if we have the core functions/parameters define in the wrap function call

uint8_t* BEM(int width, int height, int** events, int event_size, uint8_t* output_buffer) {
    // Step 1: Allocate temporary signed integer array for counting
    int* temp_bem = fb_alloc(width * height * sizeof(int), FB_ALLOC_NO_HINT);
    memset(temp_bem, 0, width * height * sizeof(int));
    
    // Step 2: Accumulate ON/OFF events
    for (int i = 0; i < event_size; i++) {
        int x = events[i][5];
        int y = events[i][4];
        int type = events[i][0];
        
        if (x >= 0 && x < width && y >= 0 && y < height) {
            int idx = y * width + x;
            
            if (type == 0) {
                temp_bem[idx]--;  // Decrement for negative/OFF events
            } else if (type == 1) {
                temp_bem[idx]++;  // Increment for positive/ON events
            }
        }
    }
    
    // Step 3: Convert to binary (0 or 1) - your exact logic
    uint8_t* bem = output_buffer ? output_buffer : fb_alloc(width * height, FB_ALLOC_NO_HINT);
    for (int i = 0; i < width * height; i++) {
        bem[i] = (temp_bem[i] != 0) ? 1 : 0;
    }
    // check output_buffer = NULL?

    // Step 4: Free temporary buffer
    fb_free();  // Frees the most recent allocation (temp_bem)
    
    return bem;
}

// This is the function which will be called from Python as bem.BEM(width, height, events, len(events))
static mp_obj_t py_bem(mp_obj_t width_obj, mp_obj_t height_obj,
                        mp_obj_t events_obj, mp_obj_t event_size_obj){
    int width = mp_obj_get_int(width_obj);
    int height = mp_obj_get_int(height_obj);
    int event_size = mp_obj_get_int(event_size_obj);

    // have a pointer to locate
    mp_obj_t *event_list;
    mp_obj_get_array_fixed_n(events_obj, event_size, &event_list);

    int **events = m_new(int*, event_size);
    for (int i = 0; i < event_size, i++){ // start looping through all the event 
        mp_obj_t *event_data
        mp_obj_get_array_fixed_n(event_list[i], 6, &event_data);
        events[i] = m_new(int, 6); // update the events

        for (int j = 0; j < 6; j++);{
            events[i][j] = mp_obj_get_int(event_data[j]);
        }
    }

    uint8_t *output = BEM(width, height, events, event_size, NULL);

    return mp_obj_new_bytearray_by_ref(width * height, output);
}

static MP_DEFINE_CONST_FUN_OBJ_4(py_bem_obj, py_bem);

static const mp_rom_map_elem_t bem_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_bem) },
    { MP_ROM_QSTR(MP_QSTR_BEM), MP_ROM_PTR(&py_bem_obj) },
};
static MP_DEFINE_CONST_DICT(bem_globals, bem_globals_table);

// Define module object.
const mp_obj_module_t bem_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *) &bem_globals,
};

// Register the module to make it available in Python
MP_REGISTER_MODULE(MP_QSTR_bem, bem_module);