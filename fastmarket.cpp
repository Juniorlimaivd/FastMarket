#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mraa.hpp"
#include "HIDBarcodeScanner-master/BCodeScanner.cpp"
#include <exception>
#include <omp.h>
#include <string>
#include "socketgalileo/comunicationModule.cpp"
#include <iostream>
#include <math.h>

#define INVALID -1
#define WAITING_CONNECTION 0
#define CONNECTED 1
#define OPERATION_ON_CART 8
#define ADD_1 3
#define ADD_2 4
#define CANCEL_1 5
#define CANCEL_2 6
#define FINISH 7

#define VARIANCE 0.050
int running = 1;
int state = WAITING_CONNECTION;
int laststate = WAITING_CONNECTION;

mraa::Gpio* red;
mraa::Gpio* green;
mraa::Gpio* blue;
mraa::Uart* weightSerial;

mraa::Gpio* mux1d1;
mraa::Gpio* mux1d2;
mraa::Gpio* mux1d3;

mraa::Gpio* mux2d1;
mraa::Gpio* mux2d2;
mraa::Gpio* mux2d3;

mraa::Aio* a1;
mraa::Aio* a2;

bool haveBarCode=false;

std::string barcode_number;

//bool handmap[13] = {false,false,false,false,false,false,false,false,false,false,false,false,false};

void initRGB()
{

    red = new mraa::Gpio(12);
    if (red == NULL) {
        exit(mraa::ERROR_UNSPECIFIED);
    }
    mraa::Result response = red->dir(mraa::DIR_OUT);
    if (response != mraa::SUCCESS) {
        mraa::printError(response);
         exit(1);
    }

    green = new mraa::Gpio(11);
    if (green == NULL) {
        exit(mraa::ERROR_UNSPECIFIED);
    }
    mraa::Result response1 = green->dir(mraa::DIR_OUT);
    if (response != mraa::SUCCESS) {
        mraa::printError(response);
        exit(1);
    }

    blue = new mraa::Gpio(10);
    if (blue == NULL) {
        exit(mraa::ERROR_UNSPECIFIED);
    }
    mraa::Result response2 = blue->dir(mraa::DIR_OUT);
    if (response != mraa::SUCCESS) {
        mraa::printError(response);
        exit(1);
    }

    printf("Init RBG Sucessful\n");
}

void initHandModule()
{
    mux1d1 = new mraa::Gpio(4);

    if (mux1d1 == NULL) {
        exit(mraa::ERROR_UNSPECIFIED);
    }

    mraa::Result response = mux1d1->dir(mraa::DIR_OUT);
    if (response != mraa::SUCCESS) {
        mraa::printError(response);
        exit(1);
    }

    mux1d2 = new mraa::Gpio(5);
    if (mux1d2 == NULL) {
        exit(mraa::ERROR_UNSPECIFIED);
    }

    mraa::Result response2 = mux1d2->dir(mraa::DIR_OUT);
    if (response2 != mraa::SUCCESS) {
        mraa::printError(response);
        exit(1);
    }

    mux1d3 = new mraa::Gpio(6);
    if (mux1d3 == NULL) {
            exit(mraa::ERROR_UNSPECIFIED);
    }

    mraa::Result response3 = mux1d3->dir(mraa::DIR_OUT);
    if (response3 != mraa::SUCCESS) {
        mraa::printError(response);
        exit(1);
    }

    mux2d1 = new mraa::Gpio(7);
    if (mux2d1 == NULL) {
        exit(mraa::ERROR_UNSPECIFIED);
    }

    mraa::Result response4 = mux2d1->dir(mraa::DIR_OUT);
    if (response4 != mraa::SUCCESS) {
        mraa::printError(response);
        exit(1);
    }

    mux2d2 = new mraa::Gpio(8);
    if (mux2d2 == NULL) {
        exit(mraa::ERROR_UNSPECIFIED);
    }
    mraa::Result response5 = mux2d2->dir(mraa::DIR_OUT);
    if (response5 != mraa::SUCCESS) {
        mraa::printError(response);
        exit(1);
    }

    mux2d3 = new mraa::Gpio(9);
    if (mux2d3 == NULL) {
        exit(mraa::ERROR_UNSPECIFIED);
    }
    mraa::Result response6 = mux2d3->dir(mraa::DIR_OUT);
    if (response6 != mraa::SUCCESS) {
        mraa::printError(response);
        exit(1);
    }


    a1 = new mraa::Aio(0);
    if (a1 == NULL) {
        exit(MRAA_ERROR_UNSPECIFIED);
    }

    a2 = new mraa::Aio(1);
    if (a2 == NULL) {
        exit(MRAA_ERROR_UNSPECIFIED);
    }

    printf("Init HandModule sucefull\n");
}

int jugdeLEDS(bool* handmap)
{
  if(handmap[0]&&(handmap[2]||handmap[3]||handmap[4])&&!handmap[1]) return 2;
  else if(handmap[1]&&(handmap[3]||handmap[4])&&!handmap[2]) return 2;
  else if(handmap[2]&&!handmap[3]&&handmap[4]) return 2;
  else if(handmap[5]&&!handmap[6]&&(handmap[7]||handmap[8]||handmap[9]||handmap[10]||handmap[11]||handmap[12])) return 2;
  else if(handmap[6]&&!handmap[7]&&(handmap[8]||handmap[9]||handmap[10]||handmap[11]||handmap[12])) return 2;
  else if(handmap[7]&&!handmap[8]&&(handmap[9]||handmap[10]||handmap[11]||handmap[12])) return 2;
  else if(handmap[8]&&!handmap[9]&&(handmap[10]||handmap[11]||handmap[12])) return 2;
  else if(handmap[9]&&!handmap[10]&&(handmap[11]||handmap[12])) return 2;
  else if(handmap[10]&&!handmap[11]&&handmap[12]) return 2;
  else {
      int a=0;
      for(int i=0; i<13;i++){
          if(!handmap[i])  a++;
        }
      if(a==13)return 0;
      else return 1;
    }

}

int haveHands()
{
    bool handmap[13] = {false,false,false,false,false,false,false,false,false,false,false,false,false};

    int mux0,mux1,r0,r1,r2,r3,r4,r5;
    for (mux0=0; mux0<=7; mux0++)
    {

        r0 = mux0 & 1;      // old version of setting the bits
        r1 = (mux0>>1) & 1;      // old version of setting the bits
        r2 = (mux0>>2) & 1;      // old version of setting the bits

        mux1d1->write(r0);
        mux1d2->write(r1);
        mux1d3->write(r2);

        int alarm = a1->read();

        switch(mux0)
        {
            case 0:

                if (alarm < 40)
                {
                    //printf("m0l0\n");
                    handmap[2]=true;
                }
                else
                {
                    //printf("nao ativ\n");
                }
                break;
            case 1:

                if (alarm < 100)
                {
                    printf("m0l1\n");
                    handmap[mux0]=true;
                }
                break;
            case 2:

                if (alarm < 85)
                {
                  //  printf("m0l2\n");
                    handmap[0]=false;
                }
                break;
            case 3:

                if (alarm < 50)
                {
                  //  printf("m0l3\n");
                    handmap[mux0]=true;
                }
                break;
            case 4:
                if (alarm < 100)
                {
                    //printf("m0l4\n");
                    handmap[mux0]=true;
                }
                break;
            case 5:
                if (alarm < 50)
                {
                  // printf("m0l5\n");
                    handmap[mux0]=true;
                }
                break;
            case 6:
                if (alarm < 100)
                {
                  // printf("m0l6\n");
                    handmap[mux0]=true;
                }
                break;
            case 7:
                if (alarm < 100)
                {
                  //  printf("m0l7\n");
                    handmap[mux0]=true;
                }
                break;
            default:
              break;
        }

//        printf("%d",mux0);
//        printf(" ");
//        printf("%d\n",alarm);
//        sleep(0.2);

    }

    for (mux1=0; mux1<=4; mux1++)
    {

        r3 = mux1 & 1;      // old version of setting the bits
        r4 = (mux1>>1) & 1;      // old version of setting the bits
        r5 = (mux1>>2) & 1;      // old version of setting the bits

        mux2d1->write(r3);
        mux2d2->write(r4);
        mux2d3->write(r5);

        int alarm = a2->read();

        switch(mux1){
          case 0:
              if (alarm < 100)
              {
               // printf("m1l0\n");
                  handmap[mux1+8]=true;
              }
              break;
          case 1:
              if (alarm < 25)
              {
               //  printf("m1l1\n");
                  handmap[mux1+8]=true;
              }
              break;
          case 2:
              if (alarm < 10)
              {
               // printf("m1l2\n");
                  handmap[mux1+8]=true;
              }
              break;
          case 3:
              if (alarm < 100)
              {
                 //printf("m1l3\n");
                  handmap[mux1+8]=true;
              }
              break;
          case 4:
              if (alarm < 0)
              {
                  //printf("m1l4\n");
                  handmap[mux1+8]=true;
              }

              break;
          default:
            break;
        }

//        printf("%d",mux1 + 8);
//        printf(" ");
//        printf("%d\n",alarm);
//        sleep(0.2);
    }

    return jugdeLEDS(handmap);

}

void lightRed()
{

    red->write(0);
    green->write(1);
    blue->write(1);
}

void lightGreen()
{

    red->write(1);
    green->write(0);
    blue->write(1);
}

void lightBlue()
{

    red->write(1);
    green->write(1);
    blue->write(0);
}

void turnOffLeds()
{

    red->write(1);
    green->write(1);
    blue->write(1);
}

void lightYellow()
{
    red->write(0);
    green->write(0);
    blue->write(1);

}

void initWeightModule()
{
    try {
        weightSerial = new mraa::Uart(0);
    } catch (std::exception& e) {
        std::cout << e.what() << ", likely invalid platform config" << std::endl;
    }

    if (weightSerial->setBaudRate(115200) != mraa::SUCCESS) {
        std::cout << "Error setting parity on UART" << std::endl;
    }

    if (weightSerial->setMode(8, mraa::UART_PARITY_NONE, 1) != mraa::SUCCESS) {
        std::cout << "Error setting parity on UART" << std::endl;
    }

    if (weightSerial->setFlowcontrol(false, false) != mraa::SUCCESS) {
        std::cout << "Error setting flow control UART" << std::endl;
    }



    std::string teste;
    weightSerial->flush();
    teste = weightSerial->readStr(1);

    if(teste == "1")
    {
        weightSerial->readStr(7);

    }
    else
    {
        weightSerial->readStr(8);
    }

    printf("Init Weight Sucessfull\n");
}

double readWeight()
{
    std::string weightStr;
    weightSerial->writeStr("1");
    weightStr = weightSerial->readStr(1);
    if(weightStr == "1")
    {
        weightStr = weightSerial->readStr(6);
    }
    else if(weightStr == "2")
    {
        weightStr = weightSerial->readStr(7);
    }
    weightSerial->flush();
    //std::cout << weightStr << std::endl;
    double weightValue = atof(weightStr.c_str());

    return weightValue;
}

int main()
{
    //INICIALIZACAO

    //INIT LEDS
    int handreturn;
    double weightAt=0.0,expectedWeight,validWeight=0.0,weightInProcess,weightAnt=0.0;
    double w1,w2 = 0.0;
    ComunicationModule* inst = ComunicationModule::instance();
    bool haveWeight=false;
    initRGB();


    char path[] = "/dev/input/event2";
    BCodeScanner barcode = BCodeScanner(path);
    barcode.begin();
    if(!barcode.isValid()){
        printf("Failure init barcode.\n");
        lightRed();
        exit(EXIT_FAILURE);
    }

    initHandModule();

    initWeightModule();

    bool haveConection = false;

    #pragma omp parallel sections num_threads(3) shared(haveBarCode,barcode_number,haveWeight,haveConection,expectedWeight,validWeight,weightInProcess,weightAnt)
    {
        #pragma omp section
        {
            while(running)
            {
                if(haveConection)
                {

                    std::string data;
                    data = inst->conn->recv(256);
                    std::cout << data << std::endl;
                    std::string delimiter = ":";

                    std::string header;

                    header = data.substr(0,data.find(delimiter));

                    data.erase(0,data.find(delimiter)+delimiter.length());

                    if(header.compare("bcr")==0)
                    {
                      std::cout << data << std::endl;
                      expectedWeight = atof(data.c_str());
                      //printf("%lf",expectedWeight);
                      haveWeight = true;

                    }
                    else if(header.compare("finish")==0)
                    {
                      inst->resetConnection();
                      inst->start();
                    }
                    else if(header.compare("cart")==0)
                    {
                      //mandar nova situação do carrinho
                    }
                }
            }
        }
        #pragma omp section
        {
            while(true)
            {
                if(state==OPERATION_ON_CART || state==CANCEL_1)
                {
                    barcode_number = barcode.readBC();
                    haveBarCode = true;
                }
            }
        }

        #pragma omp section
        {
            while(running)
            {

                switch(state)
                {
                    case WAITING_CONNECTION:
                       printf("estado waiting connection\n");

                        inst->setup("172.20.10.2",1235);
                        inst->start();

                        haveConection = true;
                        state = CONNECTED;

                        laststate = WAITING_CONNECTION;
                    break;

                    case CONNECTED:

                        if(laststate!=CONNECTED)
                        {
                            validWeight += weightAt;
                            printf("estado connected\n");
                            turnOffLeds();
                            sleep(0.2);
                            lightGreen();
                            inst->updateCartSituation("valid");
                            laststate = CONNECTED;
                        }

                        handreturn = haveHands();
                        printf("mao: %d\n",handreturn);
                        if(handreturn >= 2)
                        {
                            state = INVALID;
                        }
                        else if(handreturn == 1)
                        {
                            state = OPERATION_ON_CART;
                        }

                    break;

                    case OPERATION_ON_CART:

                        if(laststate!=OPERATION_ON_CART)
                        {
                            printf("estado operation on cart\n");
                            //lightYellow();
                            lightBlue();
                            laststate=OPERATION_ON_CART;
                            printf("onehand\n");
                        }

                        weightAnt = weightAt;


                        handreturn = haveHands();
                        printf("mao: %d\n",handreturn);
                        w1 = weightAt;
                        weightAt = readWeight();
                        printf("peso: %f\n", weightAt);
                        if(fabs(w1-weightAt) > 0.50){
                            printf("vini variacaoooooooooooooooooooooooooooooooooooooooooo\n");
                        }

                        if(handreturn >= 2)
                        {
                            state = INVALID;
                        }
                        else if(haveBarCode == true)
                        {
                            printf("here!!!\n");
                            haveBarCode = false;
                            w1 = readWeight();
                            w2 = readWeight();
                            printf("%lf",fabs(w1-w2));
                            if(fabs(w1-w2) <= VARIANCE)
                            {
                                inst->sendBarcode(barcode_number,"add");
                                int running2 = true;


                                while(!haveWeight);



                               // printf("expected weight: %lf",expectedWeight);

                                haveWeight = false;

                                state = ADD_1;
                            }

                            weightAnt = weightAt;
                        }
                        else
                        {
                            if((validWeight > weightAt) && fabs(weightAnt - weightAt) >= VARIANCE)
                            {
                                    weightInProcess = (validWeight - weightAt);
                                    printf("Valid Weight: %lf\n", validWeight);
                                    printf("At Weight: %lf\n", weightAt);
                                    printf("Ant Weight: %lf\n", weightAnt);
                                    state = CANCEL_1;
                            }

                            if(handreturn == 0)
                            {
                                state = CONNECTED;
                            }

                            weightAnt = weightAt;

                        }



                    break;

                    case ADD_1:

                        if(laststate!=ADD_1)
                        {
                            printf("estado add1\n");
                            //lightYellow();
                            laststate=ADD_1;
                           // printf("PASSBARCODE\n");
                        }
                        w2 = weightAt;
                        weightAt = readWeight();
                        //printf("%lf",weightAt);
                        if(fabs(weightAt - w2) <= 1.1*expectedWeight && fabs(weightAt - validWeight) >= 0.9*expectedWeight)
                        {

                             state = ADD_2;
                        }

                    break;

                    case ADD_2:
                        if(laststate!=ADD_2)
                        {
                            printf("estado add2\n");
                            laststate=ADD_2;
                        }

                        handreturn = haveHands();

                        if(handreturn == 0)
                        {
                            printf("finish compras\n");
                            state = CONNECTED;
                        }else{
                            //printf("hand: %d\n", handreturn);
                        }

                    break;

                    case CANCEL_1:
                        if(laststate!=CANCEL_1)
                        {
                            printf("estado cancel1\n");
                            //lightYellow();
                            laststate=CANCEL_1;
                           // printf("PASSBARCODE\n");
                        }
                        w1 = w2;
                        w2 = readWeight();
                        handreturn = haveHands();
                        if(fabs(w1-w2)<= VARIANCE && handreturn>=2)
                        {
                            state = OPERATION_ON_CART;
                        }

                        weightAt = readWeight();
                        if(haveBarCode == true)
                        {
                            printf("leu barcode\n");
                            haveBarCode = false;
                            w1 = w2;
                            w2 = readWeight();
                            if(fabs(w1-w2) <= VARIANCE)
                            {
                                printf("peos valido para consulta\n");

                                int running2 = true;

                                inst->sendBarcode(barcode_number,"cancel");
                                while(!haveWeight);

                                w1 = w2;
                                w2 = readWeight();
                                if(fabs(w1-w2)<= VARIANCE)
                                {
                                    state = CANCEL_2;

                                }
                                else
                                {
                                    state = INVALID;
                                }


                                haveWeight = false;

                            }

                        }
                    break;

                    case CANCEL_2:
                        if(laststate!=CANCEL_2)
                        {
                            printf("estado cancel2\n");
                            //lightYellow();
                            laststate=CANCEL_2;
                           // printf("PASSBARCODE\n");
                        }

                        handreturn = haveHands();

                        if(handreturn == 0)
                        {
                            printf("finish compras2\n");
                            state = CONNECTED;
                        }else{
                            //printf("hand: %d\n", handreturn);
                        }


                    break;

                    case INVALID:
                        printf("invalid\n");
                        handreturn = haveHands();
                        w1 = w2;
                        w2 = readWeight();
                        if(fabs(w1-w2) > VARIANCE || handreturn > 0){
                            printf("fodeuuuuuuu\n");
                        }else{
                            state = CONNECTED;
                        }


                        if(laststate!=INVALID)
                        {
                            lightRed();
                            printf("invalid\n");
                            laststate = INVALID;
                        }

                    break;


                }
            }
        }

    }

    //asd

    turnOffLeds();

    printf("sera\n");
    return 0;
}
