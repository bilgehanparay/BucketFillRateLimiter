#include <iostream>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

void print(const boost::system::error_code& e){
    std::cout << "timer expired\n";
}

int main(){
    
    /**
     * All programs to use asio need at least one io-service
     */
    boost::asio::io_service io;
    
    /**
     * Any core asio class take io_service in their first argument
     */
    boost::asio::deadline_timer t(io, boost::posix_time::seconds(5));

    /**
     * the call to wait() will not return until timeout is expired
     * from the time deadline timer is created
     * Deadline timer is always on two states expired/not_expired
     * if wait() is called on expired timer, it returns immediately
     */
    //t.wait();
    /**
     * Instead of blocking wait() above, we call async_wait() to perform
     * async wait
     */
    t.async_wait(&print);

    /**
     * Boost.Asio guarintees that our async callback function is called
     * only in thread where io_service.run() is called.
     * It is important to give io_service some work before calling
     * io_service.run() otherwise run() returns immediately
     */
    io.run();
    return 0;
}