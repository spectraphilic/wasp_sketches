#include "WaspUIO.h"

/* Find the index in a list, return -1 if not found */
int8_t WaspUIO::index(const char* const list[], size_t size, const char* str)
{
  for (size_t i=0; i < size; i++)
  {
    if (strcmp_P(str, (const char*)pgm_read_word(&(list[i]))) == 0)
    {
      return i;
    }
  }

  return -1;
}


/* Sort in place integer array. Bubble sort. */
void WaspUIO::sort_uint16(uint16_t* array, uint8_t size)
{
  bool done;
  uint16_t temp;
  uint8_t i;

  done = false;
  while (done == false)
  {
    done = true;
    for (i = 0; i < size; i++)
    {
      // numbers are out of order - swap
      if (array[i] > array[i+1])
      {
        temp = array[i+1];
        array [i+1] = array[i];
        array [i] = temp;
        done = false;
      }
    }
  }
}

/* Return median value of the given array. Modifies (sorts) the array. */
uint16_t WaspUIO::median_uint16(uint16_t* array, uint8_t size)
{
  sort_uint16(array, size);

  if (size % 2 == 1)
  {
    return array[size/2];
  }
  else
  {
    return (array[size/2] + array[size/2 - 1]) / 2;
  }
}

/* Return standard deviation of the given array to the given value. */
uint16_t WaspUIO::sd_uint16(uint16_t* array, uint8_t size, uint16_t mean)
{
  uint32_t sd = 0;

  for (uint8_t i=0; i<size; i++)
  {
    uint16_t value = array[i];
    value = (value > mean) ? (value - mean) : (mean - value);
    sd += (value * value);
  }

  return (uint16_t) sqrt(sd / size);
}
