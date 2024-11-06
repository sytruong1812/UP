#pragma once

namespace GenericTable {

    typedef enum object_types_ {
        TYPE_NULL   = 101,
        TYPE_INT    = 102,
        TYPE_FLOAT  = 103,
        TYPE_LONG   = 104,
        TYPE_DOUBLE = 105,
        TYPE_LLONG  = 106,
        TYPE_ULLONG = 107,
        TYPE_STRING = 108,
        TYPE_BLOB   = 109,
    } ObjectTypes;

    typedef union object_value_ {
        int int_;
        float float_;
        long long_;
        double double_;
        long long llong_;
        unsigned long long ullong_;
        struct {
            char* str;
            size_t length;
        } text_;
        struct {
            uint8_t* data;
            size_t length;
        } blob_;
    } ObjectValue;

    struct Object {
        ObjectTypes object_type;
        ObjectValue object_value;
        Object() : object_type(TYPE_NULL), object_value() {}
        ~Object() 
        {
            switch (object_type) {
            case TYPE_STRING:
                if (object_value.text_.str != NULL) {
                    delete[] object_value.text_.str;
                    object_value.text_.str = NULL;
                    object_value.text_.length = 0;
                }
                break;
            case TYPE_BLOB:
                if (object_value.blob_.data != NULL) {
                    delete[] object_value.blob_.data;
                    object_value.blob_.data = NULL;
                    object_value.blob_.length = 0;
                }
                break;
            default:
                break;
            }
        }
    };
}
