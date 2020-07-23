#include <iostream>
#include <random>
#include <thread>
#include <future>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait()
    // to wait for and receive new messages and pull them from the queue using move semantics.
    // The received object should then be returned by the receive function.
    std::unique_lock<std::mutex> lock(_mutex);
    _condition.wait(lock, [this](){return !_deque.empty();});

    auto msg = std::move(_deque.back());
    _deque.pop_back();
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex>
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lock(_mutex);
    _deque.emplace_back(msg);
    _condition.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop
    // runs and repeatedly calls the receive function on the message queue.
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true)
    {
        if (_queue.receive() == TrafficLightPhase::green)
        {
            return;
        }
    }
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles
    // and toggles the current phase of the traffic light between red and green and sends an update method
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds.
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.

    auto startTime = std::chrono::system_clock::now();
    auto interval = std::chrono::seconds(rand() % 6 + 4);

    while (true)
    {
        // establish how much time passed
        auto now = std::chrono::system_clock::now();
        auto elapsed = now - startTime;

        // check if the interval is reached
        if (elapsed > interval){
            // toggle phase
            if (TrafficLight::getCurrentPhase() == TrafficLightPhase::green){
                TrafficLight::setCurrentPhase(TrafficLightPhase::red);
            }
            else {
                TrafficLight::setCurrentPhase(TrafficLightPhase::green);
            }

            // send the update method
            auto message = std::async(std::launch::async,
                                    &MessageQueue<TrafficLightPhase>::send,
                                    &_queue,
                                    std::move(TrafficLight::getCurrentPhase()));

            message.wait();

            // start measuring time past from this moment
            startTime = std::chrono::system_clock::now();
            // make a new random interval
            interval = std::chrono::seconds(rand() % 6 + 4);
        }

        // extra wait between cycles
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

