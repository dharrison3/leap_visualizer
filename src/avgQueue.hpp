//
//  avgQueue.hpp
//  handyCam
//
//  Created by Drake S. Harrison on 6/21/17.
//
//

#include <stdio.h>
#include <iostream>
#include <cstdlib>

template <class type>
class avgQueue {
public:
    
    //Default Constructor
    avgQueue();
    
    //Constructor
    avgQueue(int n);
    
    //Destructor
    ~avgQueue();
    
    // Assignment Operator
    avgQueue<type> & operator=(const avgQueue<type> rhs);
    
    //Push x onto queue
    void push(type x);
    
    //Average all elements in the queue
    type average() const;
    
    //Print the queue
    void print() const;
    void print_raw() const;
    
    //Get the nth newest element
    type nth_newest(int i) const;
    
    //Make empty function
    void emptyQueue();

private:
    
    type* data;
    
    int idx;   //array index
    
    int N;  //size of queue
    
    int num;    //number of elements currently in queue
    
};

/////////////////////////////////////////////////////////
// Templated Implementation
template <class type>
avgQueue<type>::avgQueue() {  //creates default queue of 1 element
    N = 1;
    data = new type[N];
}

template <class type>
avgQueue<type>::avgQueue(int n) {
    N = n;
    data = new type[N];    //create a dynamic array of size N
    
    idx = 0;
    
    num = 0;
}

template <class type>
avgQueue<type>::~avgQueue(){
    delete[] data;
}

template <class type>
avgQueue<type> & avgQueue<type>::operator=(const avgQueue<type> rhs) {
    num = rhs.num;
    idx = rhs.idx;
    N = rhs.N;
    data = new type[N];
    for (int i = 0; i < N; i++)
        data[i] = type(rhs.data[i]);
    return *this;
}

template <class type>
void avgQueue<type>::push(type x){
    data[idx] = x;
    idx = (idx + 1) % N;
    
    if(num < N){
        num++;
    }
}

template <class type>
type avgQueue<type>::average() const {
    if (num > 0) {
        type sum = data[0];
        for (int i = 1; i < num; i++) {
            sum += data[i];
        }
        return sum / num;
    }
    else {
        type t;
        return t;
    }
}

template <class type>
void avgQueue<type>::print() const {
    if (num > 0) {
        std::cout << "queue contents" << std::endl;
        for (int i = 0; i < num; i++) {
            std::cout << nth_newest(i) << std::endl;
        }
        //std::cout << "queue avg:" << average() << std::endl;
    }
    else {
        std::cout << "queue empty." << std::endl;
    }
}

template <class type>
void avgQueue<type>::print_raw() const {
    std::cout << "raw queue contents" << std::endl;
    for (int i = 0; i < N; i++) {
        if (i == idx)
            std::cout << "idx-->";
        else
            std::cout << "      ";
        
        std::cout << data[i] << std::endl;
    }
}

template <class type>
type avgQueue<type>::nth_newest(int i) const {
    int data_index = idx - i - 1;
    return data[(data_index + N) % N];
}

template <class type>
void avgQueue<type>::emptyQueue() {
    idx = 0;
    num = 0;
}

