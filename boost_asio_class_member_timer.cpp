#include <iostream>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <thread>         // std::thread

/**
 * Timer fires once a second in this program
 */

/**
 * The deadline timer callback is now class member
 * and operatos on class attributes
 */
class User{
    public:
    User(boost::asio::io_service& io, int id) 
        : timer_(io, boost::posix_time::seconds(2)),
          count_(0),
          id_(id){
            timer_.async_wait(boost::bind(&User::print, this));
          }
    ~User(){
        std::cout << "final Count is " << count_ << std::endl;
    }
    void print(){
        // increment counter at each call
        if(count_ < 100000){
            ++(count_);
            std::cout << "count_: " << count_ << " id: " << id_ << std::endl; 
            timer_.expires_at(timer_.expires_at()+boost::posix_time::seconds(1));
            timer_.async_wait(boost::bind(boost::bind(&User::print, this)));
        }
    }
    private:
    boost::asio::deadline_timer timer_;
    int count_; 
    int id_; 
};
 

int main(){
    
    boost::asio::io_service io;
    //std::vector<boost::thread> pool;
    std::vector<std::thread> pool;
    std::vector<User*> users;
    // new thread for each user
    for(int i=0; i<10; i++){
        User* u = new User(io, i);
        users.push_back(u);
        pool.push_back( std::thread([&io]() {io.run(); }) );
    }
    for(auto &thread:pool)
        thread.join();
    return 0;
}