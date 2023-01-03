#ifndef DEBUG_H
#define DEBUG_H

#include <sml/sml_file.h>
#include <sml/sml_value.h>
#include "unit.h"

#ifdef DEBUG
#define SERIAL_DEBUG true
#else
#define SERIAL_DEBUG false
#endif

void DEBUG_DUMP_BUFFER(byte *buf, int size)
{
#if (defined(SERIAL_DEBUG_VERBOSE) && SERIAL_DEBUG_VERBOSE)
    Debug("----DATA----");
    for (int i = 0; i < size; i++)
    {
        if (buf[i] < 16)
        {
            Debug.print("0");
        }
        Debug.print(buf[i], HEX);
        Debug.print(" ");
        if (((i + 1) % 16) == 0)
        {
            Debug.println();
        }
    }
    Debug.println();
    Debug("---END OF DATA---");
#endif
}

void DEBUG_SML_FILE(sml_file *file)
{
#if (defined(SERIAL_DEBUG) && SERIAL_DEBUG)

    sml_file_print(file);

    // read here some values ...
    Debug.printf("OBIS data\n");
    for (int i = 0; i < file->messages_len; i++)
    {
        sml_message *message = file->messages[i];
        if (*message->message_body->tag == SML_MESSAGE_GET_LIST_RESPONSE)
        {
            sml_list *entry;
            sml_get_list_response *body;
            body = (sml_get_list_response *)message->message_body->data;
            for (entry = body->val_list; entry != NULL; entry = entry->next)
            {
                if (!entry->value)
                { // do not crash on null value
                    Debug.printf("Error in data stream. entry->value should not be NULL. Skipping this.\n");
                    continue;
                }
                if (entry->value->type == SML_TYPE_OCTET_STRING)
                {
                    char *str;
                    printf("%d-%d:%d.%d.%d*%d#%s#\n",
                           entry->obj_name->str[0], entry->obj_name->str[1],
                           entry->obj_name->str[2], entry->obj_name->str[3],
                           entry->obj_name->str[4], entry->obj_name->str[5],
                           sml_value_to_strhex(entry->value, &str, true));
                    free(str);
                }
                else if (entry->value->type == SML_TYPE_BOOLEAN)
                {
                    Debug.printf("%d-%d:%d.%d.%d*%d#%s#\n",
                           entry->obj_name->str[0], entry->obj_name->str[1],
                           entry->obj_name->str[2], entry->obj_name->str[3],
                           entry->obj_name->str[4], entry->obj_name->str[5],
                           entry->value->data.boolean ? "true" : "false");
                }
                else if (((entry->value->type & SML_TYPE_FIELD) == SML_TYPE_INTEGER) ||
                         ((entry->value->type & SML_TYPE_FIELD) == SML_TYPE_UNSIGNED))
                {
                    double value = sml_value_to_double(entry->value);
                    int scaler = (entry->scaler) ? *entry->scaler : 0;
                    int prec = -scaler;
                    if (prec < 0)
                        prec = 0;
                    value = value * pow(10, scaler);
                    Debug.printf("%d-%d:%d.%d.%d*%d#%.*f#",
                           entry->obj_name->str[0], entry->obj_name->str[1],
                           entry->obj_name->str[2], entry->obj_name->str[3],
                           entry->obj_name->str[4], entry->obj_name->str[5], prec, value);
                    const char *unit = NULL;
                    if (entry->unit && // do not crash on null (unit is optional)
                        (unit = dlms_get_unit((unsigned char)*entry->unit)) != NULL)
                        printf("%s", unit);
                    Debug.printf("\n");
                    // flush the stdout puffer, that pipes work without waiting
                    fflush(stdout);
                }
            }
        }
    }
#endif
}

#endif
