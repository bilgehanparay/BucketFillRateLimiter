#include <iostream>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <thread>         // std::thread
#include <mutex>
/**
 * Simplest Bucket Fill Rate Limit Algorithm 
 * using boost::asio
 */

class User{
    public:
    /**
    * Create a new user with rate limit timer,
    * Every 15 seconds increment bucket to 4
    * 4 connections in 60 seconds allowed
    */
    User(boost::asio::io_service& io, int id) 
        : timer_(io, boost::posix_time::seconds(60)),
          bucket_(4),
          id_(id){
            timer_.async_wait(boost::bind(&User::fill, this));
          }
    /**
     * When the user is destroyed, associated timer will be destroyed
     * hence the io.run() in this user thread should return due to 
     * no work
     */
    ~User(){
        std::cout << "final bucket was " << bucket_ << std::endl;
    }

    void fill(){
        //lock guard for variable bucket
        const std::lock_guard<std::mutex> lock(mtx);
        // increment bucket
        bucket_ = 4;
        //std::cout << "count_: " << count_ << " id: " << id_ << std::endl; 
        timer_.expires_at(timer_.expires_at()+boost::posix_time::seconds(60));
        timer_.async_wait(boost::bind(boost::bind(&User::fill, this)));
        
    }
    void empty(){
        bucket_ = bucket_ >0?bucket_-1:0;
    }
    int getBucket(){
        const std::lock_guard<std::mutex> lock(mtx);
        return bucket_;
    }
    int getID() const{return id_;};
    private:
    boost::asio::deadline_timer timer_;
    int bucket_; 
    int id_; 
    std::mutex mtx;
};

/**
 * @brief App class manages
 *  New user created
 *  User deleted
 *  refuse/accept user request
 */
class App{
    public:
        App(){}
        // todo: ids must be unique
        void join_user(boost::asio::io_service& io, int id){
            User *u = new User(io, id);
            users.push_back(u);
            threads.push_back( std::thread([&io]() {io.run(); }) );
        }
        void remove_user(int id){
            auto it = std::find_if(users.begin(), users.end(), [&id](const User* obj) {return (*obj).getID() == id;} );
            if(it != users.end()){
                auto ix = std::distance(users.begin(), it);
                /*
                * Should call User destructor
                * which deletes deadline_timer
                * which stops io.run for this user due to no work
                * which terminates thread due to io.run returns
                * todo: verify above
                */
                users.erase(it);
                threads.at(ix).detach();
                threads.erase(threads.begin()+ix);
            }else{
                std::cout << "user not found\n";
            }                 
        }
        bool user_request(int id){
            auto it = std::find_if(users.begin(), users.end(), [&id](const User* obj) {return (*obj).getID() == id;} );
            if(it != users.end()){
                // empty bucket from user
                if((*it)->getBucket() > 0){
                    (*it)->empty();
                    std::cout << "Accept user request, id: " << id << std::endl;
                    return true;
                }else{
                    std::cout << "Refuse user request, id: " << id << std::endl;
                    return false;
                }
            }else{
                std::cout << "cant' comply with request user not found\n";
                return false;
            }
        }
        int getNumOfUsers(){return users.size();}
    private:
        std::vector<User*> users;
        std::vector<std::thread> threads; // todo: vector of pairs <thread, id>
};

int main(){
    boost::asio::io_service io;
    App app;
    
    // start simulation
    // join 3 new users
    app.join_user(io, 1);
    app.join_user(io, 2);
    app.join_user(io, 3);

    // wait 
    sleep(5);

    app.user_request(2); // prints accept request
    app.user_request(1); // prints accept request
    app.user_request(2); // prints accept request
    app.user_request(3); // prints accept request
    
    // sleep 5
    sleep(5);
    app.user_request(2); // prints accept request
    app.user_request(2); // prints accept request
    app.user_request(2); // prints refuse request
    app.user_request(2); // prints refuse request
    app.user_request(2); // prints refuse request
    
    sleep(65);
    app.user_request(2); // prints accept request

    // user 3 disconneted
    app.remove_user(3);
    app.remove_user(2);

    sleep(5);

    app.join_user(io, 10);
    app.user_request(10);
    app.user_request(10);
    app.user_request(10);
    app.user_request(10);
    app.user_request(10);

    app.remove_user(7); // user not found
    app.remove_user(10);
    app.remove_user(1);
    std::cout << "total users: " << app.getNumOfUsers() << "\n"; // 0 users
    // todo: app should wait in loop or gracefully destroy itself
    while(1){
        sleep(1); // CTRL-C to interrup
    }
}