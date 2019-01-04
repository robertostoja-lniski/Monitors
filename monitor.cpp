
#include <stdio.h> 
#include <stdlib.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <string.h> 
#include <errno.h> 
#include <fcntl.h> 
#include <pthread.h> 
#include <unistd.h>
#include <semaphore.h>
#include <iostream>
#include <chrono>
#include <ctime>  

#include "monitor.h"
#include "buffer.hpp"
#include <random>

#define PRODUCTS_NUM 3
#define CUSTOMERS_TYPES_NUM 2

class Minitor : public Monitor
{
private:

    Condition *full;
    Condition *empty;
    Condition *multipleWaiting;

    Buffer *buffer;
    int *count;
    int size;
    int *requiredElementsK1;
    int *requiredElementsK2;
    int *counterKGet;
    int *counterGet;
    int lessThanZeroErrorCount;
    int greaterThanZeroErrorCount;
    int *turn;

public:
	Minitor( int size) : size(size), lessThanZeroErrorCount(0), greaterThanZeroErrorCount(0) {

        buffer = new Buffer[size];
        counterGet = new int[3];
        counterGet[0]=counterGet[1]=counterGet[2] = 0;

        counterKGet = new int[2];
        counterKGet[0] = counterKGet[1] = 0;

        setType(0 , 'A');
        setType(1 , 'B');
        setType(2 , 'C');

        int random = randomOffset();

        for(int i = 3; i < size; i++) {

            setType(i, 'A' + random );
            random++;
            random = random % 3;
        }

        full = new Condition[size];
        empty = new Condition[size];
        multipleWaiting = new Condition[2];

        count = new int[3];

        requiredElementsK1 = new int[3];
        requiredElementsK1[0] = 3;
        requiredElementsK1[1] = 2;
        requiredElementsK1[2] = 1;

        requiredElementsK2 = new int[3];
        requiredElementsK2[0] = 1;
        requiredElementsK2[1] = 2;
        requiredElementsK2[2] = 3;

        turn = new int();
        *turn = 0;
    }
    int getSize() {
        return size;
    }
   // void setRequiredElements(int type) {}
    char randomOffset() {

        time_t tt;

        int seed = time(&tt);
        srand(seed);

        return rand() % 3;
    }
    void printCounts() {

        std::cout << count[0] << " " << count[1] << " " << count[2] << std::endl;
    }
    void setType(int index, char Type) {
        
        enter();

        buffer[index].setType(Type);

        leave();
    }
    void setInitialValue(int val) {

        for (int i =0 ; i< size; i++) {
            
            for(int j = 0; j <val ; j++) {

                buffer[i].addElement();
                count[buffer[i].getType() - 'A'] ++;
            }
            
        }
    }
    int productsForKExist(int type) {

       int *requiredElements;

       if(type == -1) {

           requiredElements = requiredElementsK1;
       } else {

           requiredElements = requiredElementsK2;
       }

       for(int i = 0; i < PRODUCTS_NUM; ++i) {

           if(count[i] < requiredElements[i]) {
               
               return false;
           }
       }
       return true;
    }
    void addElement(int index) {

        enter();

        if(buffer[index].getAmount() == MAX_BUFFER_AMOUNT)
        {
            std::cout << "My buffer is full" << std::endl;
            wait(full[index]);
        }
        
        buffer[index].addElement();
        count[getNumericBufferType(index)] ++;

        // turn nie ma zwiazku z synchronizacja !
        if (*turn == 1)
        {   
            *turn = 0;
            if (productsForKExist(-1))
            {

                signal(multipleWaiting[0]);
            }
            else
            {

                if (productsForKExist(-2))
                {

                    signal(multipleWaiting[1]);
                }
            }
            
        }
        else
        {
            *turn = 1;
            if (productsForKExist(-2))
            {

                signal(multipleWaiting[1]);
            }
            else
            {

                if (productsForKExist(-1))
                {

                    signal(multipleWaiting[0]);
                }
                
            }
        }
            if (buffer[index].getAmount() == 1)
            {

                std::cout << "My buffer was empty" << std::endl;
                signal(empty[index]);
            }

            leave();
  
    }
    
    int getNumericBufferType(int i) {

        return buffer[i].getType() - 'A';
    }
    void removeElements(int type) {

        enter();
        
        if(!productsForKExist( type )) {

                std::cout << " I cannot get anything" << std::endl;
                // -1 changes to 0 , -2 changes to 1
                int numericType = (type)*-1 -1;
                wait(multipleWaiting[numericType]);
                
        } 
        
        for (int index = 0; index < size; ++index)
        {

            if (buffer[index].getAmount() > 0)
            {

                int elementsToRemove = std::min(buffer[index].getAmount(), requiredElementsK1[buffer[index].getType() - 'A']);
                //  std::cout << elementsToRemove << std::endl;
                buffer[index].removeElement(elementsToRemove);
                count[getNumericBufferType(index)] -= elementsToRemove;
                requiredElementsK1[buffer[index].getType() - 'A'] -= elementsToRemove;

                if(buffer[index].getAmount() == MAX_BUFFER_AMOUNT - elementsToRemove) {

                      signal(full[index]);
                }
            }

            if ((requiredElementsK1[0] == 0 && requiredElementsK1[1] == 0 && requiredElementsK1[2] == 0))
            {

                std::cout << "K " << type << " has taken!" << std::endl;

                if(type == -1) {

                    counterKGet[0]++;
                    requiredElementsK1[0] = 3;
                    requiredElementsK1[1] = 2;
                    requiredElementsK1[2] = 1;

                    //count[0] -= requiredElementsK1[0];
                   // count[1] -= requiredElementsK1[1];
                   //count[2] -= requiredElementsK1[2];

                }
                if(type == -2) {

                    counterKGet[1]++;
                    requiredElementsK2[0] = 1;
                    requiredElementsK2[1] = 2;
                    requiredElementsK2[2] = 3;

                   // count[0] -= requiredElementsK2[0];
                   // count[1] -= requiredElementsK2[1];
                   // count[2] -= requiredElementsK2[2];

                }
                
            }
        }

        leave();

    }
    void removeElement(int index, int type){

        enter();
        
        if(buffer[index].getAmount() == 0) {

            wait(empty[index]);
        }
        

        // Customer K1 i K2 o ujemnym id sciagaja z count na sam koniec
        if(type >= 0) {
            
            buffer[index].removeElement();
            count[getNumericBufferType(index)] --;
           // if(buffer[index].getAmount() < 0) {
                counterGet[type]++;
           // }
            
            std::cout << "Consumer " << type << " has taken ! " << std::endl;
            
        }
        // checks error
        if(buffer[index].getAmount() < 0) {

            lessThanZeroErrorCount ++;
        }
        if(buffer[index].getAmount() > 10) {

            greaterThanZeroErrorCount ++;
        }
        
        if(buffer[index].getAmount() == MAX_BUFFER_AMOUNT - 1) {

           signal(full[index]);
        }
        leave();
    }
    void printTab() {

        for(int i =0 ;i < size ;i ++) {

            std::cout <<  i << " BUFFER IS TYPE " << getType(i) << " AND HAS " <<getAmount(i) << " elements." << std::endl;
               
        }
       
        std::cout << "Customer K1 " << counterKGet[0] << " Customer K2 " << counterKGet[1] << " Customer A " << counterGet[0] << " Customer B " << counterGet[1] << " Customer C " << counterGet[2]  << std::endl;
        std::cout << "There were " << lessThanZeroErrorCount << " less than zero errors and " << greaterThanZeroErrorCount << " greater than zero errors." << std::endl;
    }
    int getAmount(int index) {

        enter();

        int result = buffer[index].getAmount();

        leave();

        return result;
    }
    
    
    char getType(int index) {

        enter();

        char result = buffer[index].getType();

        leave();

        return result;
    }

    int getCount(int i) {

        return count[i];
    }
};

Minitor *mini;

int randomIndex(int firstIncluded, int lastEcluded) {

    
    return firstIncluded + rand() % lastEcluded;
}

void *production(void*) {

        srand(time(NULL));
        int index = 0;
    
        while(true) {

            index = randomIndex(0, mini->getSize());
            mini->addElement(index);     
            mini->printTab(); 
            mini->printCounts();      
           //usleep(10);
        }
        
        
    }
void *consumption(void* args) {

        int type=*((int *)args);
      //  std::cout << type << std::endl;
        int index = 0;

        while(true) {
            
            if(type < 0) {

                mini -> removeElements(type);
                //usleep(1);
            } else {

               
                mini->removeElement( type, mini->getNumericBufferType(type));
                usleep(10000);
              
                
            }
            
        }
    
}
int main() {

    int bufferNum;

    std::cout << "Podaj liczbe buforow" << std::endl;
    std::cin >> bufferNum;

    mini = new Minitor(bufferNum);

    int custTypes[bufferNum + 2]; 
    int size = 0;
    int numABC;
    pthread_t *threadsTab;

    std::cout << "Podaj liczbe klientow K1, K2 oraz ABC" << std::endl;
    for(int i = 0; i < CUSTOMERS_TYPES_NUM; i++) {

        std::cin >> custTypes[i];
        size += custTypes[i];

    }
    std::cin >> numABC;

    for(int i = CUSTOMERS_TYPES_NUM; i < bufferNum + 2; i++) {

        custTypes[i] = numABC;
    }

    threadsTab = new pthread_t[size];

    int threadIdIndex = 0;
    int codeTab[bufferNum + 2];
    codeTab[0] = -1; // K1
    codeTab[1] = -2; // K2

    for(int i = 2; i< bufferNum + 2; i++) {

        codeTab[i] = i - 2;
    }


    pthread_t prod;
    pthread_create(&prod, NULL, production, NULL);

    for(int i = 0 ; i < bufferNum + 2; ++i) {

        for(int j = 0; j < custTypes[i]; j++) {
            
            pthread_create(&threadsTab[threadIdIndex], NULL, consumption, &codeTab[i]);

        }
    }
    
    for(int i = 0; i< size; i++) {

        pthread_join(threadsTab[i], NULL);
    }

    
}


