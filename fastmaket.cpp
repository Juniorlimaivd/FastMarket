#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mraa.hpp"
#include "HIDBarcodeScanner-master/BCodeScanner.cpp"
#include "../src/HX711.cpp"
#include "mraa.hpp"
#include <exception>
#include <omp.h>
#include <string>
#define DEFAULT_IOPIN 8

static int iopin;
int running = 0;

void
sig_handler(int signo)
{
    if (signo == SIGINT) {
        printf("closing IO%d nicely\n", iopin);
        running = -1;
    }
}



int
main(int argc, char** argv)
{
    if (argc < 2) {
        //printf("Provide an int arg if you want to flash on something other than %d\n", DEFAULT_IOPIN);
        iopin = DEFAULT_IOPIN;
    } else {
        iopin = strtol(argv[1], NULL, 10);
    }

    signal(SIGINT, sig_handler);
// -----------BarCode ------------
    BCodeScanner barcode = BCodeScanner("/dev/input/event2");
    barcode.begin();
    if(!barcode.isAvailable()){
        //mostrar led
    }
 // -----------BarCode ------------

    mraa::Gpio* gpio = new mraa::Gpio(iopin);
    if (gpio == NULL) {
        return mraa::ERROR_UNSPECIFIED;
    }
    mraa::Result response = gpio->dir(mraa::DIR_OUT);
    if (response != mraa::SUCCESS) {
        mraa::printError(response);
        return 1;
    }

    //---------------------------- Serial -----------------
    mraa::Uart* dev;
    try {
        dev = new mraa::Uart(0);
    } catch (std::exception& e) {
        std::cout << e.what() << ", likely invalid platform config" << std::endl;
    }

    if (dev->setBaudRate(115200) != mraa::SUCCESS) {
        std::cout << "Error setting parity on UART" << std::endl;
    }

    if (dev->setMode(8, mraa::UART_PARITY_NONE, 1) != mraa::SUCCESS) {
        std::cout << "Error setting parity on UART" << std::endl;
    }

    if (dev->setFlowcontrol(false, false) != mraa::SUCCESS) {
        std::cout << "Error setting flow control UART" << std::endl;
    }
    dev->flush();
    while(true){
        std::cout << "lido: " << dev->readStr(10) << std::endl;
    }

// --------------------------- Serial -----------------------
    while (running == 0) {

    #pragma omp parallel sections num_threads(NTHREADS)
        {
        #pragma omp section
            {
                std::cout << "lido: " << dev->readStr(10) << std::endl;
            }

        }
    }

    delete dev;
    delete gpio;
    return response;
    //! [Interesting]
}


    

    
        
     

