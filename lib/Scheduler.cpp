#include <vector>

struct Holder {};

class any {
private:
    struct Base {
        virtual ~Base() = default;
        virtual Base* copy() const = 0;
        virtual void* GetValue() = 0;
    };

    template <typename T>
    struct Derived : Base {
        T value_;

        Derived(const T& value) : value_(value) {}

        Base* copy() const override {
            return new Derived(value_);
        }

        void* GetValue() override {
            return static_cast<void*>(&value_);
        }

        ~Derived() override = default;
    };

    Base* data_;

public:
    template <typename T>
    any(const T& value) : data_(new Derived<T>(value)) {}

    any(const any& other) : data_(other.data_->copy()) {}

    any& operator=(const any& other) {
        if (this == &other) {
            return *this;
        }
        data_ = other.data_->copy();
        return *this;
    }

    ~any() {
        delete data_;
    }

    template <typename T>
    T get() {
        return *reinterpret_cast<T*>(data_->GetValue());
    }
};

class TTaskScheduler {
private:
    struct Task {
        Task(TTaskScheduler* scheduler) : scheduler_(scheduler) {}
        virtual any GetResult() = 0;
        virtual ~Task() = default;
        virtual Task* copy() const = 0;
        bool is_solved_ = false;
        TTaskScheduler* scheduler_;
    };

    template <typename T>
    struct Type {
        Type(size_t ind) : index(ind) {}
        using type = T;
        size_t index;
    };

    template <typename T>
    auto SolveTask(const Type<T>& value) {
        return list_of_tasks_[value.index]->GetResult().template get<typename Type<T>::type>();
    }

    template <typename T>
    T SolveTask(const T& value) {
        return value;
    }

    template<typename T>
    struct TaskZeroArgs : Task {
        TaskZeroArgs(const T& pred, TTaskScheduler* scheduler) : pred_(pred), Task{scheduler} {}

        any GetResult() override {
            if (is_solved_) {
                return result_;
            }
            is_solved_ = true;
            result_ = pred_();
            return result_;
        }

        Task* copy() const override {
            return new TaskZeroArgs(pred_, scheduler_);
        }

        T pred_;
        any result_ = Holder();
    };

    template<typename T, typename U>
    struct TaskOneArg : Task {
        TaskOneArg(const T& pred, const U& arg, TTaskScheduler* scheduler) : pred_(pred), arg_(arg), Task{scheduler} {}

        any GetResult() override {
            if (is_solved_) {
                return result_;
            }
            is_solved_ = true;
            result_ = pred_(scheduler_->SolveTask(arg_));
            return result_;
        }

        Task* copy() const override {
            return new TaskOneArg(pred_, arg_, scheduler_);
        }

        T pred_;
        U arg_;
        any result_ = Holder();
    };

    template <typename T, typename U, typename D>
    struct TaskTwoArgs : public Task {
        TaskTwoArgs(const T& pred, const U& arg1, const D& arg2, TTaskScheduler* scheduler) : pred_(pred), arg1_(arg1),
                arg2_(arg2), Task{scheduler} {}

        any GetResult() override {
            if (is_solved_) {
                return result_;
            }
            is_solved_ = true;
            result_ = pred_(scheduler_->SolveTask(arg1_), scheduler_->SolveTask(arg2_));
            return result_;
        }

        Task* copy() const override {
            return new TaskTwoArgs(pred_, arg1_, arg2_, scheduler_);
        }

        T pred_;
        U arg1_;
        D arg2_;
        any result_ = Holder();
    };

    std::vector<Task*> list_of_tasks_;

public:
    template <typename T>
    T getResult(size_t index) {
        return list_of_tasks_[index]->GetResult().get<T>();
    }

    template <typename T>
    Type<T> getFutureResult(size_t index) {
        return Type<T>(index);
    }

    template <typename T>
    size_t add(const T& pred) {
        list_of_tasks_.push_back(new TaskZeroArgs(pred, this));
        return list_of_tasks_.size() - 1;
    }

    template <typename T, typename U>
    size_t add(const T& pred, const U& arg) {
        list_of_tasks_.push_back(new TaskOneArg(pred, arg, this));
        return list_of_tasks_.size() - 1;
    }

    template <typename T, typename U, typename D>
    size_t add(const T& pred, const U& arg1, const D& arg2) {
        list_of_tasks_.push_back(new TaskTwoArgs(pred, arg1, arg2, this));
        return list_of_tasks_.size() - 1;
    }

    void executeAll() {
        for (size_t i = 0; i < list_of_tasks_.size(); ++i) {
            list_of_tasks_[i]->GetResult();
        }
    }

    TTaskScheduler() = default;

    TTaskScheduler(const TTaskScheduler& other) {
        for (auto& el : other.list_of_tasks_) {
            auto new_el = el->copy();
            new_el->scheduler_ = this;
            list_of_tasks_.push_back(new_el);
        }
    }

    TTaskScheduler& operator=(const TTaskScheduler& other) {
        if (this == &other) {
            return *this;
        }
        for (auto& el : list_of_tasks_) {
            delete el;
        }
        list_of_tasks_.clear();
        for (auto& el : other.list_of_tasks_) {
            auto new_el = el->copy();
            new_el->scheduler_ = this;
            list_of_tasks_.push_back(new_el);
        }
    }

    ~TTaskScheduler() {
        for (auto& el : list_of_tasks_) {
            delete el;
        }
    }
};
