#include <iostream>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>

/**
 * Timer fires once a second in this program
 */

/**
 * The handler should take timer object as parameter to 
 * respawn new async_wait()
 */
void print(const boost::system::error_code& e,
           boost::asio::deadline_timer* t, 
           int* count){
    // increment counter at each call
    if(*count < 5){
        ++(*count); 
    
    t->expires_at(t->expires_at()+boost::posix_time::seconds(1));
    t->async_wait(boost::bind(print, boost::asio::placeholders::error, 
                              t, count));
    }
}   

int main(){
    
    /**
     * All programs to use asio need at least one io-service
     */
    boost::asio::io_service io;
    
    /**
     * Any core asio class take io_service in their first argument
     */
    int count = 0;
    boost::asio::deadline_timer t(io, boost::posix_time::seconds(1));

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
    // t.async_wait(&print);
    
    /**
     * Boost.Asio guarintees that our async callback function is called
     * only in thread where io_service.run() is called.
     * It is important to give io_service some work before calling
     * io_service.run() otherwise run() returns immediately
     */
    t.async_wait(boost::bind(print, boost::asio::placeholders::error, 
                              &t, &count));
    io.run();
    std::cout << " count: " << count << "\n";
    return 0;
}