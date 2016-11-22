#include <stdio.h>
#include <string.h>
#include <string>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <linux/input.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstdlib>
#include "BCodeScanner.cpp"




int main(){


    BCodeScanner bc = BCodeScanner("/dev/input/event2");
    bc.begin();
    if(bc.isValid()){
        printf(" device found\n");
        std::cout << bc.readBC();
    }else{
        printf("device NOT found\n");
    }

    return 0;
}
