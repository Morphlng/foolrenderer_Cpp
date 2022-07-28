#pragma once

#include "rvector.h"
#include <cstdint>

#define MAX_FLOAT_VARIABLES 2
#define MAX_VECTOR2_VARIABLES 2
#define MAX_VECTOR3_VARIABLES 5
#define MAX_VECTOR4_VARIABLES 2

/*
	\brief Structure used to pass data between shaders in different stages.

	The vertex shader stores floating-point-based data into this structure and
	automatically interpolated on the surface of the triangle before the
	fragment shader is executed. The interpolation result can be used in the
	fragment shader.

	IMPORTANT: Do not directly access the members of the structure in the
	shader. Instead, use shader_context_*() functions.
*/
struct ShaderContext {
    // Array to store various types of variables.
    float float_variables[MAX_FLOAT_VARIABLES];
    vec2 vec2_variables[MAX_VECTOR2_VARIABLES];
    vec3 vec3_variables[MAX_VECTOR3_VARIABLES];
    vec4 vec4_variables[MAX_VECTOR4_VARIABLES];

    // Record whether the variable at each index is used.
    bool float_allocations[MAX_FLOAT_VARIABLES];
    bool vec2_allocations[MAX_VECTOR2_VARIABLES];
    bool vec3_allocations[MAX_VECTOR3_VARIABLES];
    bool vec4_allocations[MAX_VECTOR4_VARIABLES];

    // Queue of indexes of variables that have been used.
    int8_t float_index_queue[MAX_FLOAT_VARIABLES];
    int8_t vec2_index_queue[MAX_VECTOR2_VARIABLES];
    int8_t vec3_index_queue[MAX_VECTOR3_VARIABLES];
    int8_t vec4_index_queue[MAX_VECTOR4_VARIABLES];

    // Number of variables used.
    int8_t float_variable_count;
    int8_t vec2_variable_count;
    int8_t vec3_variable_count;
    int8_t vec4_variable_count;

    /*
        \brief Removes all data from the shader context and also serve as an
            initialization function.

        It should not be and is not necessary to use this function in the shader.
        
        \param context The shader context object.
    */
    void clear() {
        for (int8_t i = 0; i < MAX_FLOAT_VARIABLES; i++) {
            float_allocations[i] = false;
        }
        for (int8_t i = 0; i < MAX_VECTOR2_VARIABLES; i++) {
            vec2_allocations[i] = false;
        }
        for (int8_t i = 0; i < MAX_VECTOR3_VARIABLES; i++) {
            vec3_allocations[i] = false;
        }
        for (int8_t i = 0; i < MAX_VECTOR4_VARIABLES; i++) {
            vec4_allocations[i] = false;
        }
        float_variable_count = 0;
        vec2_variable_count = 0;
        vec3_variable_count = 0;
        vec4_variable_count = 0;
    }

#define RETURN_VARIABLE(type, max_variables)                      \
    do {                                                          \
        if (index >= max_variables) {                             \
            return nullptr;                                       \
        }                                                         \
        type *variables = type##_variables;                       \
        bool *allocations = type##_allocations;                   \
        int8_t *index_queue = type##_index_queue;                 \
        int8_t *variable_count = &type##_variable_count;          \
        if (!allocations[index]) {                                \
            allocations[index] = true;                            \
            index_queue[*variable_count] = index;                 \
            ++(*variable_count);                                  \
        }                                                         \
        return variables + index;                                 \
    } while (0)


    /*
        \brief Gets the pointer of the float variable with the specified index in
            the shader context.
    
        The maximum number of float variables that can be used in the shader_context
        is MAX_FLOAT_VARIABLES.

        \param index The variable index, range from 0 to MAX_FLOAT_VARIABLES-1.
        \return Returns variable pointer if successful. Returns NULL if contex is a
                null pointer or index is out of range.
    */
    float* shader_context_float(int8_t index) {
        RETURN_VARIABLE(float, MAX_FLOAT_VARIABLES);
    }

    /*
        \brief Gets the pointer of the vec2 variable with the specified index in
            the shader context.

        The maximum number of vec2 variables that can be used in the shader_context
        is MAX_VECTOR2_VARIABLES.

        \param index The variable index, range from 0 to MAX_VECTOR2_VARIABLES-1.
        \return Returns variable pointer if successful. Returns NULL if contex is a
                null pointer or index is out of range.
    */
    vec2* shader_context_vec2(int8_t index) {
        RETURN_VARIABLE(vec2, MAX_FLOAT_VARIABLES);
    }

    /*
        \brief Gets the pointer of the vec3 variable with the specified index in
            the shader context.

        The maximum number of vec3 variables that can be used in the shader_context
        is MAX_VECTOR3_VARIABLES.

        \param index The variable index, range from 0 to MAX_VECTOR3_VARIABLES-1.
        \return Returns variable pointer if successful. Returns NULL if contex is a
                null pointer or index is out of range.
    */
    vec3* shader_context_vec3(int8_t index) {
        RETURN_VARIABLE(vec3, MAX_VECTOR3_VARIABLES);
    }

    /*
        \brief Gets the pointer of the vec4 variable with the specified index in
            the shader context.

        The maximum number of vec4 variables that can be used in the shader_context
        is MAX_VECTOR4_VARIABLES.

        \param index The variable index, range from 0 to MAX_VECTOR4_VARIABLES-1.
        \return Returns variable pointer if successful. Returns NULL if contex is a
                null pointer or index is out of range.
    */
    vec4* shader_context_vec4(int8_t index) {
        RETURN_VARIABLE(vec4, MAX_VECTOR3_VARIABLES);
    }

#undef RETURN_VARIABLE
};

#undef MAX_FLOAT_VARIABLES
#undef MAX_VECTOR2_VARIABLES
#undef MAX_VECTOR3_VARIABLES
#undef MAX_VECTOR4_VARIABLES