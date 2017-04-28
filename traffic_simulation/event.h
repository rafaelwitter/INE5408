//!  Copyright [2017] <João Vicente Souto>
#ifndef STRUCTURES_EVENT_H
#define STRUCTURES_EVENT_H

#include <cstdint>
#include <stdlib.h>
#include "./linked_queue_of_cars.h"

namespace structures {

  class Event {
  public:
    Event(std::size_t time, LinkedQueueOfCars *road);
    ~Event();

    std::size_t time() const;
    std::size_t road() const;
    virtual std::size_t task();

  protected:
    std::size_t time_;
    LinkedQueueOfCars *road_;
  }

  Event::Event(std::size_t time, LinkedQueueOfCars *road) :
  time_{time},
  road_{road}
  {}

  std::size_t time() const {
    return time_;
  }

  std::size_t road() const {
    return road_;
  }

}  //  namespace structure

#endif
