#include "hx711.cxx"

int main(){
  printf("0\n");
  HX711 lc = HX711(3, 2);
  printf("1\n");
  printf("read: %ld\n", lc.read());
  printf("read avg: %ld\n", lc.readAverage(20));
  lc.setScale(54000.2);
  printf("2\n");
  lc.tare();
  printf("3\n");
  lc.getUnits();
  return 0;
}
