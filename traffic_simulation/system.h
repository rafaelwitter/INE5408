//!  Copyright [2017] <João Vicente Souto>
#ifndef STRUCTURES_SYSTEM_H
#define STRUCTURES_SYSTEM_H

#include <cstdint>  // std::std::size_t
#include <stdexcept>  // C++ exceptions
#include <cstdlib>
#include <stdlib.h>
#include "./vehicle.h"
#include "./event.h"
#include "./entry_road.h"
#include "./exit_road.h"
#include "./semaphore.h"
#include "./linked_queue_of_vehicles.h"
#include "./structures/linked_list.h"
#include "./structures/array_list.h"

#define DEBUG false

namespace structures {

//! Classe System
/*! Sistema de simulação de tráfego.
 *  \author João Vicente Souto.
 *  \since 25/04/17
 *  \version 1.0
 */
class System {
 public:
    System(std::size_t execution_time, std::size_t semaphore_time);
    ~System();

    void init();
    void run();
    void result();

 private:
    std::size_t _execution_time,  //!< Tempo de execução
                _semaphore_time,  //!< Tempo de troca de sinal
                _global_clock{0u},  //!< Relógio
                _input_counter{0u},  //!< Contador de entrada
                _output_counter{0u},  //!< Contador de saída
                _semaphore_counter{0u},  //!< Contador troca de sinal
                _exchange_counter{0u};  //!< Contador troca de pista

    ArrayList<EntryRoad*> _entry_roads{8u};  //! Estradas aferentes
    ArrayList<ExitRoad*> _exit_roads{6u};  //!< Estradas eferentes

    LinkedList<Event>* _events;  //!< Eventos
    Semaphore* _semaphore;  //!< Semáforo
};

//! Construtor padrão
/*! Atribuindo os valores bases para o funcionamento do sistema.
 *  \param execution_time Tempo de execução
 *  \param semaphore_time Tempo de um semáforo
 */
System::System(std::size_t execution_time, std::size_t semaphore_time):
_execution_time{execution_time},
_semaphore_time{semaphore_time}
{
    _events = new LinkedList<Event>();
}

//! Destrutor
/*! Deleta o sistema
 */
System::~System() {
    delete _events;
    delete _semaphore;
}

//! Inícia todas as estradas e eventos iniciais
/*! Executa uma série e passos antes de rodar o sistema.
 */
void System::init() {

    // Criando as ruas
    // Aferentes
    EntryRoad* N1_S = new EntryRoad((char*)"N1_S\0", 60, 500, 20, 5, 80, 10, 10);
    EntryRoad* S1_N = new EntryRoad((char*)"S1_N\0", 60, 500, 30, 7, 10, 10, 80);
    EntryRoad* O1_L = new EntryRoad((char*)"O1_L\0", 80, 2000, 10, 2, 10, 80, 10);
    EntryRoad* L1_O = new EntryRoad((char*)"L1_O\0", 30, 400, 10, 2, 30, 30, 40);
    EntryRoad* N2_S = new EntryRoad((char*)"N2_S\0", 40, 500, 20, 5, 40, 30, 30);
    EntryRoad* S2_N = new EntryRoad((char*)"S2_N\0", 40, 500, 60, 15, 30, 30, 40);
    // Centrais
    EntryRoad* C1_L = new EntryRoad((char*)"C1_L\0", 60, 300, 0, 0, 30, 40, 30);
    EntryRoad* C1_O = new EntryRoad((char*)"C1_O\0", 60, 300, 0, 0, 30, 40, 30);
    // Eferente
    ExitRoad* N1_N = new ExitRoad((char*)"N1_N\0", 60, 500);
    ExitRoad* N2_N = new ExitRoad((char*)"N2_N\0", 40, 500);
    ExitRoad* O1_O = new ExitRoad((char*)"O1_O\0", 80, 2000);
    ExitRoad* L1_L = new ExitRoad((char*)"L1_L\0", 60, 500);
    ExitRoad* S1_S = new ExitRoad((char*)"S1_S\0", 60, 500);
    ExitRoad* S2_S = new ExitRoad((char*)"S2_S\0", 40, 500);

    // Interligando os cruzamentos: (left, front, right)
    N1_S->crossroads(C1_L, S1_S, O1_O);
    S1_N->crossroads(O1_O, N1_N, C1_L);
    O1_L->crossroads(N1_N, C1_L, S1_S);
    L1_O->crossroads(S2_S, C1_O, N2_N);
    N2_S->crossroads(L1_L, S2_S, C1_O);
    S2_N->crossroads(C1_O, N2_N, L1_L);
    C1_L->crossroads(N2_N, L1_L, S2_S);
    C1_O->crossroads(S1_S, O1_O, N1_N);

    _entry_roads.push_back(N1_S);
    _entry_roads.push_back(S1_N);
    _entry_roads.push_back(O1_L);
    _entry_roads.push_back(L1_O);
    _entry_roads.push_back(N2_S);
    _entry_roads.push_back(S2_N);
    _entry_roads.push_back(C1_L);
    _entry_roads.push_back(C1_O);

    _exit_roads.push_back(N1_N);
    _exit_roads.push_back(N2_N);
    _exit_roads.push_back(O1_O);
    _exit_roads.push_back(L1_L);
    _exit_roads.push_back(S1_S);
    _exit_roads.push_back(S2_S);

    // Inputs iniciais
    for (auto i = 0u; i<6; ++i) {
        std::size_t event_time = _global_clock + _entry_roads[i]->input_frequency();
        Event input('i', event_time, _entry_roads[i]);
        _events->insert_sorted(input);
    }

    // Primeiro evento de troca de semáforo
    _semaphore = new Semaphore(_semaphore_time, _entry_roads);
    std::size_t event_time = _global_clock + _semaphore_time;
    Event semaphore('s', event_time, _semaphore);
    _events->insert_sorted(semaphore);
}

//! Inícia todas as estradas e eventos iniciais
/*! Executa uma série e passos antes de rodar o sistema.
 */
void System::run() {

    while (_global_clock < _execution_time) {
        auto events_made = 0;

        int i = 0;  // Incrementa caso não consiga realizar um evento
        Event current_event = _events->at(i);
        while (current_event.event_time() <= _global_clock) {

            switch (current_event.type()) {

                // Evento de saída
                case 's': {
                    _semaphore->change();
                    ++_semaphore_counter;

                    _events->pop(i);

                    auto event_time = _global_clock + _semaphore_time;
                    Event semaphore('s', event_time, _semaphore);
                    _events->insert_sorted(semaphore);

                    events_made++;
                    break;
                }

                // Evento de saída
                case 'o': {
                    ExitRoad* road = (ExitRoad*) current_event.source();
                    delete road->dequeue();
                    ++_output_counter;

                    _events->pop(i);
                    events_made++;
                    break;
                }

                // Evento de entrada
                case 'i': {
                    events_made++;
                    Vehicle* new_vehicle = new Vehicle();
                    EntryRoad* road = (EntryRoad*) current_event.source();

                    try {
                        road->enqueue(new_vehicle);
                        ++_input_counter;

                        // Elimina evento completado
                        _events->pop(i);

                        auto event_time = _global_clock + road->time_of_route();
                        Event change('c', event_time, road);
                        _events->insert_sorted(change);

                        event_time = current_event.event_time() + road->input_frequency();
                        Event input('i', event_time, road);
                        _events->insert_sorted(input);

                    } catch(std::out_of_range error) {
                        delete new_vehicle;
                        if (DEBUG)
                            printf("Entrada falhou: Rua: %s engarrafada.\n",
                                    road->name());
                        ++i;
                    }
                    break;
                }

                // Evento de troca de pista
                case 'c': {
                    events_made++;
                    EntryRoad* road = (EntryRoad*) current_event.source();
                    if (!_semaphore->open(road)) {
                        ++i;
                        break;
                    }

                    Vehicle* first_vehicle = road->front();
                    auto direction = first_vehicle->direction();
                    LinkedQueueOfVehicles* temp = (LinkedQueueOfVehicles*) road->crossroads(direction);

                    if (temp->type() == 'a') {
                        EntryRoad* aferente = (EntryRoad*) road->crossroads(direction);
                        try {
                            aferente->enqueue(first_vehicle);
                            road->dequeue();
                            ++_exchange_counter;

                            // Elimina evento completado
                            _events->pop(i);

                            auto event_time = _global_clock + aferente->time_of_route();
                            Event change('c', event_time, aferente);
                            _events->insert_sorted(change);

                            // Soma o tempo de saída do carro
                            ++_global_clock;

                        } catch(std::out_of_range error) {
                            if (DEBUG)
                                printf("Troca de %s para %s falhou.\n",
                                        road->name(), aferente->name());
                            ++i;
                        }
                    } else {
                        ExitRoad* eferente = (ExitRoad*) road->crossroads(direction);
                        try {
                            eferente->enqueue(first_vehicle);
                            road->dequeue();
                            ++_exchange_counter;

                            // Elimina evento completado
                            _events->pop(i);

                            auto event_time = _global_clock + eferente->time_of_route();
                            Event out('o', event_time, eferente);
                            _events->insert_sorted(out);

                            // Soma o tempo de saída do carro
                            ++_global_clock;

                        } catch(std::out_of_range error) {
                            if (DEBUG)
                                printf("Troca de %s para %s falhou.\n",
                                        road->name(), eferente->name());
                            ++i;
                        }
                    }
                    break;
                }

                default: {
                    break;
                }
            }
            current_event = _events->at(i);
        }

        ++_global_clock;
        if (events_made == 0)
            _global_clock = current_event.event_time();
    }
}

void System::result() {
    std::size_t inside_roads = 0;
    for (auto i = 0; i < _entry_roads.size(); ++i)
      inside_roads += _entry_roads[i]->cars_on_the_road();
    for (auto i = 0; i < _exit_roads.size(); ++i)
      inside_roads += _exit_roads[i]->cars_on_the_road();

    printf("\nResultados gerais:\n");
    printf("Operação             |  Quant.\n");
    printf("Entrada de veículos  |  %lu\n", _input_counter);
    printf("Veículos nas ruas    |  %lu\n", inside_roads);
    printf("Saída de  veíulos    |  %lu\n", _output_counter);
    printf("Troca de pista       |  %lu\n", _exchange_counter);
    printf("Troca de semáforo    |  %lu\n", _semaphore_counter);
    printf("Eventos restantes    |  %lu\n", _events->size());
    printf("\nIntegridade do sistema\n");
    printf("Entrada - veículos nas ruas = saída:\n%lu - %lu = %lu\n",
            _input_counter, inside_roads, _input_counter-inside_roads);
    printf("Eventos restantes - 6 in - 1 sem = veículos nas ruas:\n%lu - 6 - 1 = %lu\n",
            _events->size(), _events->size()-7);
}

}  //  namespace structures

#endif
