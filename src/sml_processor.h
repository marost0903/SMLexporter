#include <sml/sml_file.h>
#include <string.h>

using namespace std;

namespace sml_processor
{
  void extract_obis(bool numericOnly, sml_file *file, void (*callback)(String obis, String data))
  {

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
            continue;
          }

          char obisIdentifier[32];
          char buffer[255];

          sprintf(obisIdentifier, "%d-%d:%d.%d.%d/%d",
                  entry->obj_name->str[0], entry->obj_name->str[1],
                  entry->obj_name->str[2], entry->obj_name->str[3],
                  entry->obj_name->str[4], entry->obj_name->str[5]);

          if (((entry->value->type & SML_TYPE_FIELD) == SML_TYPE_INTEGER) ||
              ((entry->value->type & SML_TYPE_FIELD) == SML_TYPE_UNSIGNED))
          {
            double value = sml_value_to_double(entry->value);
            int scaler = (entry->scaler) ? *entry->scaler : 0;
            int prec = -scaler;
            if (prec < 0)
              prec = 0;
            value = value * pow(10, scaler);
            sprintf(buffer, "%.*f", prec, value);
            callback(obisIdentifier, String(buffer));
          }
          else if (!numericOnly)
          {
            if (entry->value->type == SML_TYPE_OCTET_STRING)
            {
              char *value;
              sml_value_to_strhex(entry->value, &value, true);
              callback(obisIdentifier, String(value));
              free(value);
            }
            else if (entry->value->type == SML_TYPE_BOOLEAN)
            {
              callback(obisIdentifier, entry->value->data.boolean ? "1" : "0");
            }
          }
        }
      }
    }
  }
}