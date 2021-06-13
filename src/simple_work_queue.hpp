#ifndef __SIMPLE_WORK_QUEUE_HPP_
#define __SIMPLE_WORK_QUEUE_HPP_

#include<deque>
#include<pthread.h>

using namespace std;

struct work_queue {
    deque<long> jobs;
    pthread_mutex_t jobs_mutex;
    
    /* add a new job to the work queue
     * and return the number of jobs in the queue */
    int add_job(long num) {
        pthread_mutex_lock(&this->jobs_mutex);
        jobs.push_back(num);
        size_t len = jobs.size();
        pthread_mutex_unlock(&this->jobs_mutex);
        return len;
    }
    
    /* return FALSE if no job is returned
     * otherwise return TRUE and set *job to the job */
    bool remove_job(long *job) {
        pthread_mutex_lock(&this->jobs_mutex);
        bool success = !this->jobs.empty();
        if (success) {
            *job = this->jobs.front();
            this->jobs.pop_front();
        }
        pthread_mutex_unlock(&this->jobs_mutex);
        return success;
    }
};

#endif
