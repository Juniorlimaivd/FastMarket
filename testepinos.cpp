#include "mraa.hpp"

int main()
{
    mraa::Gpio* sck = new mraa::Gpio(2);
   if (sck == NULL) {
       return mraa::ERROR_UNSPECIFIED;
   }
   mraa::Result response1 = sck->dir(mraa::DIR_OUT);
   if (response1 != mraa::SUCCESS) {
       mraa::printError(response1);
       return 1;

   }

   mraa::Gpio* data = new mraa::Gpio(3);
  if (data == NULL) {
      return mraa::ERROR_UNSPECIFIED;
  }
   mraa::Result response2 = data->dir(mraa::DIR_IN);
   if (response1 != mraa::SUCCESS) {
       mraa::printError(response1);
       return 1;
   }

   printf("ok\n");





    return 0;
}
