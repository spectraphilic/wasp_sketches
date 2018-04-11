#include "WaspUIO.h"


/**
 * Print the binary frame to USB.
 */

void WaspUIO::showBinaryFrame()
{
   uint8_t *p;
   uint8_t nbytes;
   char buffer[17];
   uint8_t i;
   char c;
   uint8_t sensor_id, nfields, type, decimals;
   uint8_t len;
   char name[20];
   char value_str[50];
   int8_t diff;

   // Binary Frame
   cr.println(F("=== Binary Frame: %d fields in %d bytes ==="), frame.numFields, frame.length);
   p = frame.buffer;

   // Start delimiter
   if (strncmp((const char*) p, "<=>", 3) != 0)
   {
     cr.println(F("Error reading Start delimiter <=>"));
     return;
   }
   p += 3;

   // Frame type (TODO Print text identifier: Information, TimeOut, ...)
   // Don't clear the most significant bit, we already know it's zero
   cr.println(F("Frame type: %d"), *p++);

   // Number of bytes
   nbytes = *p++;
   //println(F("Number of bytes left: %d"), nbytes);

   // Serial ID
   //println(F("BOOT VERSION %c"), _boot_version);
   if (_boot_version >= 'G')
   {
     Utils.hex2str(p, buffer, 8);
     p += 8;
   }
   else
   {
     Utils.hex2str(p, buffer, 4);
     p += 4;
   }
   cr.println(F("Serial ID: 0x%s"), buffer);

   // Waspmote ID
   for (i = 0; i < 17 ; i++)
   {
     c = (char) *p++;
     if (c == '#')
     {
       buffer[i] = '\0';
       break;
     }
     buffer[i] = c;
   }
   if (c != '#')
   {
     cr.println(F("Error reading Waspmote ID"));
     return;
   }
   cr.println(F("Waspmote ID: %s"), buffer);

   // Sequence
   cr.println(F("Sequence: %d"), *p++);

   // Payload
   for (i = 0; i < frame.numFields; i++)
   {
     sensor_id = *p++;
     // Use always the v15 tables
     // It would make sense to use the v12 only with a v12 board *and* one of
     // the Libelium's shields (eg the Agr board). But we don't support that
     // configuration.
     strcpy_P(name, (char*)pgm_read_word(&(FRAME_SENSOR_TABLE[sensor_id])));
     nfields = (uint8_t)pgm_read_word(&(FRAME_SENSOR_FIELD_TABLE[sensor_id]));
     type = (uint8_t)pgm_read_word(&(FRAME_SENSOR_TYPE_TABLE[sensor_id]));
     decimals = (uint8_t)pgm_read_word(&(FRAME_DECIMAL_TABLE[sensor_id]));

     if (nfields == 0)
     {
       nfields = *p++;
       // Special case, we store ints in a compressed format
       if (type == 1)
       {
         for (uint8_t j=0; j < nfields; j++)
	 {
           if (j > 0)
	   {
	     diff = *(int8_t *)p; p++;
	     if (diff != -128)
	     {
               cr.println(F("Sensor %d (%s): %hhd"), sensor_id, name, diff);
	       continue;
	     }
	   }

           cr.println(F("Sensor %d (%s): %d"), sensor_id, name, *(int *)p);
           p += 2;
         }
         continue;
       }
     }

     for (; nfields > 0; nfields--)
     {
       if (type == 0) // uint8_t
       {
         cr.println(F("Sensor %d (%s): %d"), sensor_id, name, *p++);
       }
       else if (type == 1) // int
       {
         cr.println(F("Sensor %d (%s): %d"), sensor_id, name, *(int *)p);
         p += 2;
       }
       else if (type == 2) // double
       {
         Utils.float2String(*(float *)p, value_str, decimals);
         cr.println(F("Sensor %d (%s): %s"), sensor_id, name, value_str);
         p += 4;
       }
       else if (type == 3) // char*
       {
         len = *p++;
         if (len > sizeof(value_str) - 1)
         {
           cr.println(F("Error reading sensor value, string too long %d"), len);
           return;
         }
         strncpy(value_str, (char*) p, len);
         p += len;
         cr.println(F("Sensor %d (%s): %s"), sensor_id, name, value_str);
       }
       else if (type == 4) // uint32_t
       {
         cr.println(F("Sensor %d (%s): %lu"), sensor_id, name, *(uint32_t *)p);
         p += 4;
       }
       else if (type == 5) // uint8_t*
       {
         cr.println(F("Sensor %d (%s): unsupported type %d"), sensor_id, name, type); // TODO
       }
       else
       {
         cr.println(F("Sensor %d (%s): unexpected type %d"), sensor_id, name, type);
       }
     }
   }
   cr.println(F("=========================================="));
}
