//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
<<<<<<< current
//
=======
// 
>>>>>>> incoming
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
<<<<<<< current
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "AsioScheduler.h"

#include "AsioTask.h"

#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/steady_timer.hpp>

#include <chrono>
#include <functional>
=======
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "AsioScheduler.h"
#include "AsioTask.h"
#include <chrono>
#include <functional>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/executor_work_guard.hpp>
>>>>>>> incoming

namespace artery
{

Register_Class(AsioScheduler)

using namespace omnetpp;

<<<<<<< current
template <typename PERIOD>
struct clock_resolution {
    static const SimTimeUnit unit;
};

template <>
const SimTimeUnit clock_resolution<std::milli>::unit = SIMTIME_MS;

template <>
const SimTimeUnit clock_resolution<std::micro>::unit = SIMTIME_US;

template <>
=======
template<typename PERIOD>
struct clock_resolution { static const SimTimeUnit unit; };

template<>
const SimTimeUnit clock_resolution<std::milli>::unit = SIMTIME_MS;

template<>
const SimTimeUnit clock_resolution<std::micro>::unit = SIMTIME_US;

template<>
>>>>>>> incoming
const SimTimeUnit clock_resolution<std::nano>::unit = SIMTIME_NS;

constexpr SimTimeUnit steady_clock_resolution()
{
<<<<<<< current
    return clock_resolution<std::chrono::steady_clock::period>::unit;
=======
	return clock_resolution<std::chrono::steady_clock::period>::unit;
>>>>>>> incoming
}

std::chrono::steady_clock::duration steady_clock_duration(const SimTime t)
{
<<<<<<< current
    return std::chrono::steady_clock::duration{t.inUnit(steady_clock_resolution())};
}


AsioScheduler::AsioScheduler() : m_work_guard(boost::asio::make_work_guard(m_io_context)), m_timer(m_io_context), m_state(FluxState::PAUSED)
=======
	return std::chrono::steady_clock::duration { t.inUnit(steady_clock_resolution()) };
}


AsioScheduler::AsioScheduler() :
	m_work_guard(boost::asio::make_work_guard(m_io_context)),
	m_timer(m_io_context),
	m_state(FluxState::PAUSED)
>>>>>>> incoming
{
}

std::string AsioScheduler::info() const
{
<<<<<<< current
    return std::string("Asio Scheduler (") + SimTime(1, steady_clock_resolution()).str() + " resolution)";
=======
	return std::string("Asio Scheduler (") + SimTime(1, steady_clock_resolution()).str() + " resolution)";
>>>>>>> incoming
}

cEvent* AsioScheduler::guessNextEvent()
{
<<<<<<< current
    return sim->getFES()->peekFirst();
=======
	return sim->getFES()->peekFirst();
>>>>>>> incoming
}

cEvent* AsioScheduler::takeNextEvent()
{
<<<<<<< current
    while (true) {
        cEvent* event = sim->getFES()->peekFirst();
        if (event) {
            if (event->isStale()) {
                cEvent* tmp = sim->getFES()->removeFirst();
                ASSERT(tmp == event);
                delete tmp;
            } else {
                m_run_until = m_reference + steady_clock_duration(event->getArrivalTime());
                try {
                    ASSERT(!m_io_context.stopped());
                    setTimer();
                    while (m_state == FluxState::DWADLING) {
                        m_io_context.run_one();
                    }
                    m_timer.cancel();
                    m_io_context.poll();
                } catch (boost::system::system_error& e) {
                    cRuntimeError("AsioScheduler IO error: %s", e.what());
                }

                if (m_state == FluxState::SYNC) {
                    return sim->getFES()->removeFirst();
                } else {
                    return nullptr;
                }
            }
        } else {
            throw cTerminationException(E_ENDEDOK);
        }
    }
=======
	while (true) {
		cEvent* event = sim->getFES()->peekFirst();
		if (event) {
			if (event->isStale()) {
				cEvent* tmp = sim->getFES()->removeFirst();
				ASSERT(tmp == event);
				delete tmp;
			} else {
				m_run_until = m_reference + steady_clock_duration(event->getArrivalTime());
				try {
					ASSERT(!m_io_context.stopped());
					setTimer();
					while (m_state == FluxState::DWADLING) {
						m_io_context.run_one();
					}
					m_timer.cancel();
					m_io_context.poll();
				} catch (boost::system::system_error& e) {
					cRuntimeError("AsioScheduler IO error: %s", e.what());
				}

				if (m_state == FluxState::SYNC) {
					return sim->getFES()->removeFirst();
				} else {
					return nullptr;
				}
			}
		} else {
			throw cTerminationException(E_ENDEDOK);
		}
	}
>>>>>>> incoming
}

void AsioScheduler::putBackEvent(cEvent* event)
{
<<<<<<< current
    sim->getFES()->putBackFirst(event);
=======
	sim->getFES()->putBackFirst(event);
>>>>>>> incoming
}

void AsioScheduler::startRun()
{
<<<<<<< current
    m_state = FluxState::SYNC;
    if (m_io_context.stopped()) {
        m_io_context.restart();
    }
    m_reference = std::chrono::steady_clock::now();
=======
	m_state = FluxState::SYNC;
	if (m_io_context.stopped()) {
		m_io_context.restart();
	}
	m_reference = std::chrono::steady_clock::now();
>>>>>>> incoming
}

void AsioScheduler::endRun()
{
<<<<<<< current
    m_state = FluxState::PAUSED;
    m_io_context.stop();
=======
	m_state = FluxState::PAUSED;
	m_io_context.stop();
>>>>>>> incoming
}

void AsioScheduler::executionResumed()
{
<<<<<<< current
    m_state = FluxState::SYNC;
    m_reference = std::chrono::steady_clock::now();
    m_reference -= steady_clock_duration(simTime());
=======
	m_state = FluxState::SYNC;
	m_reference = std::chrono::steady_clock::now();
	m_reference -= steady_clock_duration(simTime());
>>>>>>> incoming
}

void AsioScheduler::handleTimer(const boost::system::error_code& ec)
{
<<<<<<< current
    if (getEnvir()->idle()) {
        m_state = FluxState::PAUSED;
    } else if (ec) {
        m_state = FluxState::SYNC;
    } else {
        setTimer();
    }
=======
	if (getEnvir()->idle()) {
		m_state = FluxState::PAUSED;
	} else if (ec) {
		m_state = FluxState::SYNC;
	} else {
		setTimer();
	}
>>>>>>> incoming
}

void AsioScheduler::setTimer()
{
<<<<<<< current
    static const auto max_timer = std::chrono::milliseconds(100);
    const auto now = std::chrono::steady_clock::now();
    if (m_run_until > now) {
        m_state = FluxState::DWADLING;
        m_timer.expires_at(std::min(m_run_until, now + max_timer));
        m_timer.async_wait(std::bind(&AsioScheduler::handleTimer, this, std::placeholders::_1));
    } else {
        m_state = FluxState::SYNC;
    }
=======
	static const auto max_timer = std::chrono::milliseconds(100);
	const auto now = std::chrono::steady_clock::now();
	if (m_run_until > now) {
		m_state = FluxState::DWADLING;
		m_timer.expires_at(std::min(m_run_until, now + max_timer));
		m_timer.async_wait(std::bind(&AsioScheduler::handleTimer, this, std::placeholders::_1));
	} else {
		m_state = FluxState::SYNC;
	}
>>>>>>> incoming
}

std::unique_ptr<AsioTask> AsioScheduler::createTask(cModule& mod)
{
<<<<<<< current
    std::unique_ptr<AsioTask> result;
    boost::asio::ip::tcp::socket socket(m_io_context);
    result.reset(new AsioTask(*this, std::move(socket), mod));
    return result;
=======
	std::unique_ptr<AsioTask> result;
	boost::asio::ip::tcp::socket socket(m_io_context);
	result.reset(new AsioTask(*this, std::move(socket), mod));
	return result;
>>>>>>> incoming
}

void AsioScheduler::cancelTask(AsioTask* task)
{
<<<<<<< current
    if (task != nullptr) {
        sim->getFES()->remove(task->getDataMessage());
    }
=======
	if (task != nullptr) {
		sim->getFES()->remove(task->getDataMessage());
	}
>>>>>>> incoming
}

void AsioScheduler::processTask(AsioTask* task)
{
<<<<<<< current
    using namespace std::placeholders;
    auto& buffer = task->m_message->getBuffer();
    auto handler = std::bind(&AsioScheduler::handleTask, this, task, _1, _2);
    task->m_socket.async_read_some(boost::asio::buffer(buffer.data(), buffer.size()), handler);
=======
	using namespace std::placeholders;
	auto& buffer = task->m_message->getBuffer();
	auto handler = std::bind(&AsioScheduler::handleTask, this, task, _1, _2);
	task->m_socket.async_read_some(boost::asio::buffer(buffer.data(), buffer.size()), handler);
>>>>>>> incoming
}

void AsioScheduler::handleTask(AsioTask* task, const boost::system::error_code& ec, std::size_t bytes)
{
<<<<<<< current
    if (!ec) {
        using namespace std::chrono;
        const auto arrival_clock = steady_clock::now() - m_reference;
        const SimTime arrival_simtime{arrival_clock.count(), steady_clock_resolution()};
        ASSERT(simTime() <= arrival_simtime);

        AsioData* msg = task->getDataMessage();
        msg->setLength(bytes);
        msg->setArrival(task->getDestinationModule()->getId(), -1, arrival_simtime);
        sim->getFES()->insert(msg);
    } else if (ec != boost::asio::error::operation_aborted) {
        throw cRuntimeError("AsioScheduler: Failed reading from socket: %s", ec.message().c_str());
    }
}

}  // namespace artery
=======
	if (!ec) {
		using namespace std::chrono;
		const auto arrival_clock = steady_clock::now() - m_reference;
		const SimTime arrival_simtime { arrival_clock.count(), steady_clock_resolution() };
		ASSERT(simTime() <= arrival_simtime);

		AsioData* msg = task->getDataMessage();
		msg->setLength(bytes);
		msg->setArrival(task->getDestinationModule()->getId(), -1, arrival_simtime);
		sim->getFES()->insert(msg);
	} else if (ec != boost::asio::error::operation_aborted) {
		throw cRuntimeError("AsioScheduler: Failed reading from socket: %s", ec.message().c_str());
	}
}

} // namespace artery
>>>>>>> incoming
