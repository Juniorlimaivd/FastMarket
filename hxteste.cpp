#include <unistd.h>
#include <iostream>
#include <signal.h>
#include "mraa.hpp"
#include "../src/HX711.cpp"

int main(){
  
  HX711 lc = HX711(3,2,128);
  lc.setScale(54000);
  lc.tare();
  for(int i = 0; i < 100; i++){
   // printf("peso: %lf\n", lc.getUnits());
  }
  return 0;
}


